/** Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License */
#ifndef LOGOUTPUT_H
#define LOGOUTPUT_H

#ifdef USE_G3LOG

#include "g3log/g3log.hpp"
#include "g3log/logworker.hpp"
#include "g3log/std2_make_unique.hpp"
using namespace g3;

#include "g3logcoloroutsink.h"

#endif

#ifdef USE_G3LOG

#define cLogError(fmt, ...)                      LOGF(WARNING, fmt, ##__VA_ARGS__)
#define cLog(fmt, ...)                           LOGF(INFO, fmt, ##__VA_ARGS__)
#define cLogProgress(type, value, maxValue)      LOGF(INFO, "Progress:%s:%i:%i", type, value, maxValue)

#endif

namespace cura {

void increaseVerboseLevel();
void enableProgressLogging();

#ifndef USE_G3LOG
//Report an error message (always reported, independed of verbose level)
void cLogError(const char* fmt, ...);
//Report a message if the verbose level is 1 or higher. (defined as _log to prevent clash with log() function from <math.h>)
void cLog(const char* fmt, ...);

//Report engine progress to interface if any. Only if "enableProgressLogging()" has been called.
void cLogProgress(const char* type, int value, int maxValue);
#endif

}//namespace cura

#endif//LOGOUTPUT_H
