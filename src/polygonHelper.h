#ifndef POLYGONHELPER_H
#define POLYGONHELPER_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <memory>
using namespace std;

#include "utils/polygon.h"
#include "sliceDataStorage.h"
#include "gcodeExport.h"

/**
 * @brief The PolygonHelper class helper method for polygon
 */
namespace cura {

const string CURA_DEBUG_FILE_NAME_PARTS         = "parts_by_layers.txt";
const string CURA_DEBUG_FILE_NAME_POINTS_PAIRS  = "parts_points_pairs.txt";
const string CURA_DEBUG_ROOT_OUTPUT_FILE_PATH   = "/tmp";

class PolygonHelper
{
public:
    PolygonHelper();
    /**
     * @brief savePartsToFile save the closed polygons of parts into external text file. See file format in https://gist.github.com/rickyzhang82/33831c6d5a3eaaa3de3ffb5122f15b69
     * @param storage
     */
    static void savePartsToFile(SliceDataStorage& storage);
    /**
     * @brief savePointPairsInPartsToFile save the entry point and the exit point of the parts into external text file. See file format in https://gist.github.com/rickyzhang82/a6b62a2948c4e1e2fe8893f7cc7160aa
     * @param gcodeLayer
     */
    static void savePointPairsInPartsToFile(GCodePlanner& gcodeLayer);
    static void closePointPairsFile();
protected:
    /**
     * @brief pointFile
     */
    static std::shared_ptr<ofstream> pointFile;
};

}
#endif // POLYGONHELPER_H
