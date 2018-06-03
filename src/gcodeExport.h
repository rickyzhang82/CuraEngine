/** Copyright (C) 2013 David Braam - Released under terms of the AGPLv3 License */
#ifndef GCODEEXPORT_H
#define GCODEEXPORT_H

#include <stdio.h>
#include <utility>
#include <map>
#include <memory>

#include "settings.h"
#include "comb.h"
#include "utils/intpoint.h"
#include "utils/polygon.h"
#include "timeEstimate.h"

namespace cura {

//The GCodeExport class writes the actual GCode. This is the only class that knows how GCode looks and feels.
//  Any customizations on GCodes flavors are done in this class.
class GCodeExport
{
private:
    FILE* f;
    double extrusionAmount;
    double extrusionPerMM;
    double retractionAmount;
    double retractionAmountPrime;
    int retractionZHop;
    double extruderSwitchRetraction;
    double minimalExtrusionBeforeRetraction;
    double extrusionAmountAtPreviousRetraction;
    Point3 currentPosition;
    Point3 startPosition;
    Point extruderOffset[MAX_EXTRUDERS];
    char extruderCharacter[MAX_EXTRUDERS];
    int currentSpeed, retractionSpeed;
    int zPos;
    bool isRetracted;
    int extruderNr;
    int currentFanSpeed;
    int flavor;
    std::string preSwitchExtruderCode;
    std::string postSwitchExtruderCode;
    
    double totalFilament[MAX_EXTRUDERS];
    double totalPrintTime;
    TimeEstimateCalculator estimateCalculator;
public:
    
    GCodeExport();
    
    ~GCodeExport();
    
    void replaceTagInStart(const char* tag, const char* replaceValue);
    
    void setExtruderOffset(int id, Point p);
    void setSwitchExtruderCode(std::string preSwitchExtruderCode, std::string postSwitchExtruderCode);
    
    void setFlavor(int flavor);
    int getFlavor();
    
    void setFilename(const char* filename);
    
    bool isOpened();
    
    void setExtrusion(int layerThickness, int filamentDiameter, int flow);
    
    void setRetractionSettings(int retractionAmount, int retractionSpeed, int extruderSwitchRetraction, int minimalExtrusionBeforeRetraction, int zHop, int retractionAmountPrime);
    
    void applyAccelerationSettings(ConfigSettings& config);
    
    void setZ(int z);
    
    Point getPositionXY();
    
    void resetStartPosition();

    Point getStartPositionXY();

    int getPositionZ();

    int getExtruderNr();
    
    double getTotalFilamentUsed(int e);

    double getTotalPrintTime();
    void updateTotalPrintTime();
    
    void writeComment(const char* comment, ...);

    void writeLine(const char* line, ...);
    
    void resetExtrusionValue();
    
    void writeDelay(double timeAmount);
    
    void writeMove(Point p, int speed, int lineWidth);
    
    void writeRetraction(bool force=false);
    
    void switchExtruder(int newExtruder);
    
    void writeCode(const char* str);
    
    void writeFanCommand(int speed);
    
    void finalize(int maxObjectHeight, int moveSpeed, const char* endCode);

    int getFileSize();
    void tellFileSize();
};

//The GCodePathConfig is the configuration for moves/extrusion actions. This defines at which width the line is printed and at which speed.
class GCodePathConfig
{
public:
    int speed;
    int lineWidth;
    const char* name;
    bool spiralize;
    
    GCodePathConfig() : speed(0), lineWidth(0), name(nullptr), spiralize(false) {}
    GCodePathConfig(int speed, int lineWidth, const char* name) : speed(speed), lineWidth(lineWidth), name(name), spiralize(false) {}
    
    void setData(int speed, int lineWidth, const char* name)
    {
        this->speed = speed;
        this->lineWidth = lineWidth;
        this->name = name;
    }
};

class GCodePath
{
public:
    GCodePathConfig* config;
    bool retract;
    int extruder;
    vector<Point> points;
    bool done;//Path is finished, no more moves should be added, and a new path should be started instead of any appending done to this one.
};

/**
 * @brief POINTS_PAIR a pair of entry point and exit point of the part
 */
typedef std::pair<Point, Point> POINTS_PAIR;

/**
 * @brief partIndexToPointsPairMap a map from the index of part to points pair
 */
typedef std::map<int, std::shared_ptr<POINTS_PAIR>> PART_INDEX_TO_POINTS_PAIR_MAP;

//The GCodePlanner class stores multiple moves that are planned.
// It facilitates the combing to keep the head inside the print.
// It also keeps track of the print time estimate for this planning so speed adjustments can be made for the minimal-layer-time.
class GCodePlanner
{
private:
    GCodeExport& gcode;
    
    Point lastPosition;
    vector<GCodePath> paths;
    Comb* comb;
    std::shared_ptr<PART_INDEX_TO_POINTS_PAIR_MAP> partIndexToPointsPairMap;
    
    GCodePathConfig travelConfig;
    int extrudeSpeedFactor;
    int travelSpeedFactor;
    int currentExtruder;
    int retractionMinimalDistance;
    bool forceRetraction;
    bool alwaysRetract;
    double extraTime;
    double totalPrintTime;
private:
    GCodePath* getLatestPathWithConfig(GCodePathConfig* config);
    void forceNewPathStart();
    void addPolygon(PolygonRef polygon, int startIdx, GCodePathConfig* config);
public:
    GCodePlanner(GCodeExport& gcode, int travelSpeed, int retractionMinimalDistance);
    virtual ~GCodePlanner();
    
    bool setExtruder(int extruder)
    {
        if (extruder == currentExtruder)
            return false;
        currentExtruder = extruder;
        return true;
    }
    
    int getExtruder()
    {
        return currentExtruder;
    }

    void setCombBoundary(Polygons* polygons)
    {
        if (comb)
            delete comb;
        if (polygons)
            comb = new Comb(*polygons);
        else
            comb = nullptr;
    }
    
    void setAlwaysRetract(bool alwaysRetract)
    {
        this->alwaysRetract = alwaysRetract;
    }
    
    void forceRetract()
    {
        forceRetraction = true;
    }
    
    void setExtrudeSpeedFactor(int speedFactor)
    {
        if (speedFactor < 1) speedFactor = 1;
        this->extrudeSpeedFactor = speedFactor;
    }
    int getExtrudeSpeedFactor()
    {
        return this->extrudeSpeedFactor;
    }
    void setTravelSpeedFactor(int speedFactor)
    {
        if (speedFactor < 1) speedFactor = 1;
        this->travelSpeedFactor = speedFactor;
    }
    int getTravelSpeedFactor()
    {
        return this->travelSpeedFactor;
    }
    
    void addTravel(Point p);
    
    void addExtrusionMove(Point p, GCodePathConfig* config);
    
    void moveInsideCombBoundary(int distance);

    void addPolygonsByOptimizer(Polygons& polygons, GCodePathConfig* config);

    /**
     * @brief addPolygonsByOptimizer add polygons into paths of GCodePlanner with optimized order.
     * @param polygons a set of closed polygon
     * @param config path config
     * @param partIndex the index of original part in the parts, i.e. SliceLayer::parts :- vector<SliceLayerPart>. Not the optimized order!
     */
    void addPolygonsByOptimizer(Polygons& polygons, GCodePathConfig* config, int partIndex);

    void forceMinimalLayerTime(double minTime, int minimalSpeed);
    
    void writeGCode(bool liftHeadIfNeeded, int layerThickness);

    std::shared_ptr<PART_INDEX_TO_POINTS_PAIR_MAP> getPartIndexToPointsPairMap() {
        return this->partIndexToPointsPairMap;
    }
};

}//namespace cura

#endif//GCODEEXPORT_H
