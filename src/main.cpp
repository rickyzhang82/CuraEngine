/** Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <signal.h>
#if defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
#include <execinfo.h>
#include <sys/resource.h>
#endif
#include <stddef.h>
#include <vector>

#include "utils/gettime.h"
#include "utils/logoutput.h"
#include "sliceDataStorage.h"

#include "modelFile/modelFile.h"
#include "settings.h"
#include "optimizedModel.h"
#include "multiVolumes.h"
#include "polygonOptimizer.h"
#include "slicer.h"
#include "layerPart.h"
#include "inset.h"
#include "skin.h"
#include "infill.h"
#include "bridge.h"
#include "support.h"
#include "pathOrderOptimizer.h"
#include "skirt.h"
#include "raft.h"
#include "comb.h"
#include "gcodeExport.h"
#include "polygonHelper.h"
#include "fffProcessor.h"

#ifdef USE_G3LOG
#include "g3log/g3log.hpp"
#include "g3log/logworker.hpp"
#include "g3log/std2_make_unique.hpp"

#include "utils/g3logcoloroutsink.h"

using namespace g3;

#endif

using namespace cura;

void print_usage()
{
    cLogError("usage: CuraEngine [-h] [-v] [-m 3x3matrix] [-c <config file>] [-s <settingkey>=<value>] -o <output.gcode> <model.stl>\n");
}

//Signal handler for a "floating point exception", which can also be integer division by zero errors.
void signal_FPE(int n)
{
    (void)n;
    cLogError("Arithmetic exception.\n");
    exit(1);
}

using namespace cura;

int main(int argc, char **argv)
{
#if defined(__linux__) || (defined(__APPLE__) && defined(__MACH__))
    //Lower the process priority on linux and mac. On windows this is done on process creation from the GUI.
    setpriority(PRIO_PROCESS, 0, 10);
#endif

#ifdef USE_G3LOG
    // create log worker
    auto worker = LogWorker::createLogWorker();
    worker->addDefaultLogger("CuraEngine.log", "/tmp");
    worker->addSink(std2::make_unique<ColorCoutSink>(), &ColorCoutSink::ReceiveLogMessage);

    // logger is initialized
    initializeLogging(worker.get());
#endif

    //Register the exception handling for arithmic exceptions, this prevents the "something went wrong" dialog on windows to pop up on a division by zero.
    signal(SIGFPE, signal_FPE);

    ConfigSettings config;
    fffProcessor processor(config);
    std::vector<std::string> files;

    cLogError("Cura_SteamEngine version %s\n", VERSION);
    cLogError("Copyright (C) 2014 David Braam\n");
    cLogError("\n");
    cLogError("This program is free software: you can redistribute it and/or modify\n");
    cLogError("it under the terms of the GNU Affero General Public License as published by\n");
    cLogError("the Free Software Foundation, either version 3 of the License, or\n");
    cLogError("(at your option) any later version.\n");
    cLogError("\n");
    cLogError("This program is distributed in the hope that it will be useful,\n");
    cLogError("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
    cLogError("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
    cLogError("GNU Affero General Public License for more details.\n");
    cLogError("\n");
    cLogError("You should have received a copy of the GNU Affero General Public License\n");
    cLogError("along with this program.  If not, see <http://www.gnu.org/licenses/>.\n");

    if(!config.readSettings()) {
        cLogError("Default config '%s' not used\n", DEFAULT_CONFIG_PATH);
    }
    for(int argn = 1; argn < argc; argn++)
        cLog("Arg: %s\n", argv[argn]);

    for(int argn = 1; argn < argc; argn++)
    {
        char* str = argv[argn];
        if (str[0] == '-')
        {
            for(str++; *str; str++)
            {
                switch(*str)
                {
                case 'h':
                    print_usage();
                    exit(1);
                case 'v':
                    cura::increaseVerboseLevel();
                    break;
                case 'p':
                    cura::enableProgressLogging();
                    break;
                case 'g':
                    argn++;
                    //Connect the GUI socket to the given port number.
                    processor.guiConnect(atoi(argv[argn]));
                    break;
                case 'b':
                    argn++;
                    //The binaryMeshBlob is depricated and will be removed in the future.
                    binaryMeshBlob = fopen(argv[argn], "rb");
                    break;
                case 'o':
                    argn++;
                    if (!processor.setTargetFile(argv[argn]))
                    {
                        cLogError("Failed to open %s for output.\n", argv[argn]);
                        exit(1);
                    }
                    break;
                case 'c':
                    {
                        // Read a config file from the given path
                        argn++;
                        if(!config.readSettings(argv[argn])) {
                            cLogError("Failed to read config '%s'\n", argv[argn]);
                        }
                    }
                    break;
                case 's':
                    {
                        //Parse the given setting and store it.
                        argn++;
                        char* valuePtr = strchr(argv[argn], '=');
                        if (valuePtr)
                        {
                            *valuePtr++ = '\0';

                            if (!config.setSetting(argv[argn], valuePtr))
                                cLogError("Setting not found: %s %s\n", argv[argn], valuePtr);
                        }
                    }
                    break;
                case 'm':
                    //Read the given rotation/scale matrix
                    argn++;
                    sscanf(argv[argn], "%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf,%lf",
                        &config.matrix.m[0][0], &config.matrix.m[0][1], &config.matrix.m[0][2],
                        &config.matrix.m[1][0], &config.matrix.m[1][1], &config.matrix.m[1][2],
                        &config.matrix.m[2][0], &config.matrix.m[2][1], &config.matrix.m[2][2]);
                    break;
                case '-':
                    try {
                        //Catch all exceptions, this prevents the "something went wrong" dialog on windows to pop up on a thrown exception.
                        // Only ClipperLib currently throws exceptions. And only in case that it makes an internal error.
                        if (files.size() > 0)
                            processor.processFile(files);
                        files.clear();
                    }catch(...){
                        cLogError("Unknown exception\n");
                        exit(1);
                    }
                    break;
                default:
                    cLogError("Unknown option: %c\n", *str);
                    break;
                }
            }
        }else{
            if (argv[argn][0] == '$')
            {
                try {
                    //Catch all exceptions, this prevents the "something went wrong" dialog on windows to pop up on a thrown exception.
                    // Only ClipperLib currently throws exceptions. And only in case that it makes an internal error.
                    std::vector<std::string> tmp;
                    tmp.push_back(argv[argn]);
                    processor.processFile(tmp);
                }catch(...){
                    cLogError("Unknown exception\n");
                    exit(1);
                }
            }else{
                files.push_back(argv[argn]);
            }
        }
    }
    try {
        //Catch all exceptions, this prevents the "something went wrong" dialog on windows to pop up on a thrown exception.
        // Only ClipperLib currently throws exceptions. And only in case that it makes an internal error.
        if (files.size() > 0)
            processor.processFile(files);
    }catch(...){
        cLogError("Unknown exception\n");
        exit(1);
    }
    //Finalize the processor, this adds the end.gcode. And reports statistics.
    processor.finalize();
    return 0;
}
