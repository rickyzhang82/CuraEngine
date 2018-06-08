#ifndef POLYGONHELPER_H
#define POLYGONHELPER_H

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>
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
const string CURA_DEBUG_FILE_NAME_PARTS_ORDER  = "parts_order.txt";
const string CURA_DEBUG_ROOT_OUTPUT_FILE_PATH   = "/tmp";

class PolygonHelper
{
public:
    /**
     * @brief savePartsToFile save the closed polygons of parts into external text file. See file format in https://gist.github.com/rickyzhang82/33831c6d5a3eaaa3de3ffb5122f15b69
     * @param storage
     */
    static void savePartsToFile(SliceDataStorage& storage);
    /**
     * @brief saveVolumeIndexToPartsOrderFile
     * @param volumeIdx
     */
    static void saveVolumeIndexToPartsOrderFile(int volumeIdx);
    /**
     * @brief saveOptimizedPartsOrderToFile
     * @param polyOrder
     * @param layerNr
     */
    static void saveOptimizedPartsOrderToFile(std::vector<int>& polyOrder, int layerNr);
    /**
     * @brief closePartsOrderFile
     */
    static void closePartsOrderFile();
    /**
     * @brief saveVolumeIndexToPointPairsFile save volume index to point paris file
     * @param volumeIdx
     */
    static void saveVolumeIndexToPointPairsFile(int volumeIdx);
    /**
     * @brief saveLayerIndexToPointPairsFile save layer index to point pairs file
     * @param layerNr
     */
    static void saveLayerIndexToPointPairsFile(int layerNr);
    /**
     * @brief savePointPairsInPartsToFile save the entry point and the exit point of the parts into external text file. See file format in https://gist.github.com/rickyzhang82/a6b62a2948c4e1e2fe8893f7cc7160aa
     * @param gcodeLayer
     */
    static void savePointPairsInPartsToFile(GCodePlanner& gcodeLayer);
    /**
     * @brief closePointPairsFile close point pairs file stream
     */
    static void closePointPairsFile();
    /**
     * @brief setModelMin set model min for model translation purpose
     * @param modelMin
     */
    static void setModelMin(Point3 modelMin);
protected:
    PolygonHelper();
    /**
     * @brief pointFile
     */
    static shared_ptr<ofstream> pointFile;
    /**
     * @brief partOrderFile
     */
    static shared_ptr<ofstream> partOrderFile;
    /**
     * @brief lazyInitPointFile lazy initialiation point pairs file
     */
    static void lazyInitPointFile();
    /**
     * @brief lazyInitPartsOrderFile lazy initialization parts order file
     */
    static void lazyInitPartsOrderFile();

    static shared_ptr<Point3> modelMin;
};

}
#endif // POLYGONHELPER_H
