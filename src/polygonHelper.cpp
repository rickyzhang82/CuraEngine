#include "polygonHelper.h"
#include "utils/logoutput.h"

namespace cura {

shared_ptr<ofstream> PolygonHelper::pointFile = nullptr;
shared_ptr<Point3> PolygonHelper::modelMin = nullptr;

PolygonHelper::PolygonHelper()
{

}

void PolygonHelper::setModelMin(Point3 modelMin)
{
    PolygonHelper::modelMin = std::make_shared<Point3>(modelMin.x , modelMin.y, modelMin.z);
}

void PolygonHelper::savePartsToFile(SliceDataStorage& storage)
{
    if(nullptr == modelMin) {
        cLog("Failed to provide modelMin!\n");
        throw new std::runtime_error("No valid modelMin!");
    }

    Point3 modelSize = storage.modelSize;
    ofstream partFile;

    string filePath = CURA_DEBUG_ROOT_OUTPUT_FILE_PATH + "/" + CURA_DEBUG_FILE_NAME_PARTS;
    // open file
    partFile.open (filePath.c_str());
    // output model size
    partFile << "model size:" << modelSize.x << " " << modelSize.y <<endl;
    // loop through volumes
    for(unsigned int volumeIdx=0; volumeIdx<storage.volumes.size(); volumeIdx++)
    {
        // output current volumen index
        partFile << "volume index:" << volumeIdx <<endl;
        // loop through layers
        for(unsigned int layerNr=0; layerNr<storage.volumes[volumeIdx].layers.size(); layerNr++)
        {
            // output current index of layer
            partFile << "layer index:" << layerNr <<endl;
            SliceLayer* layer = &storage.volumes[volumeIdx].layers[layerNr];
            // loop through parts
            for(unsigned int partNr = 0; partNr < layer->parts.size(); partNr++)
            {
                // output current index of part
                partFile << "part index:" << partNr <<endl;
                SliceLayerPart* part = &layer->parts[partNr];
                for(unsigned int polygonNr = 0; polygonNr < part->outline.size(); polygonNr++)
                {
                    // output current outline
                    partFile << "outline index:" << polygonNr <<endl;
                    // the first polygon is the outer wall of the part!
                    for(unsigned int pointNr = 0; pointNr < part->outline[polygonNr].size(); pointNr++)
                    {
                        partFile << (part->outline[polygonNr][pointNr].X - modelMin->x)
                                 << " "
                                 << (part->outline[polygonNr][pointNr].Y - modelMin->y)
                                 <<" ";
                    }
                    // end of outline
                    partFile<<endl;
                }
            }
        }

    }
    // close file
    partFile.close();
}

void PolygonHelper::lazyInitPointFile()
{
    // lazy initiliation
    if(nullptr == pointFile) {
        string filePath = CURA_DEBUG_ROOT_OUTPUT_FILE_PATH + "/" + CURA_DEBUG_FILE_NAME_POINTS_PAIRS;
        pointFile = std::make_shared<ofstream>();
        pointFile->open (filePath.c_str());
        cLog("Created a new point pairs file %s\n", filePath.c_str());
    }
}

void PolygonHelper::saveLayerIndexToPointPairsFile(int layerNr)
{
    lazyInitPointFile();
    *pointFile << "layer index:" << layerNr << endl;
}

void PolygonHelper::saveVolumeIndexToPointPairsFile(int volumeIdx)
{
    lazyInitPointFile();
    *pointFile << "volume index:" << volumeIdx << endl;
}

void PolygonHelper::savePointPairsInPartsToFile(GCodePlanner& gcodeLayer)
{
    if(nullptr == modelMin) {
        cLog("Failed to provide modelMin!\n");
        throw new std::runtime_error("No valid modelMin!");
    }

    lazyInitPointFile();

    auto pointPairMap = gcodeLayer.getPartIndexToPointsPairMap();
    // loop through points pair by part order
     for (PART_INDEX_TO_POINTS_PAIR_MAP::iterator it=pointPairMap->begin(); it!=pointPairMap->end(); ++it) {
        *pointFile << "part index:" << it->first << endl;
         auto pointPair = it->second;
        *pointFile << pointPair->first.X - modelMin->x
                   << " "
                   << pointPair->first.Y - modelMin->y
                   << " "
                   << pointPair->second.X - modelMin->x
                   << " "
                   << pointPair->second.Y - modelMin->y
                   << endl;
     }
}

void PolygonHelper::closePointPairsFile()
{
    if(nullptr != pointFile) {
        pointFile->close();
        cLog("Closed point pairs file.\n");
        pointFile = nullptr;
    }
}

}
