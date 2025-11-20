#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <sstream>
#include <deque>
#include "utils.h"

const int IGNORE_DISP = 17;
class LaneObj
{
public:
    LaneObj(std::uint32_t id, int pos, long long ts) : id(id), pos(pos), last_time{ts} {}

    std::uint32_t getId() const { return id; }
    int getPos() const { return pos; }

    int getPotentialPos(long long loss_time=10) const
    {
        return pos + static_cast<int>(speedPerMS * loss_time);
    } 

    void setPos(int newPos, long long ts)
    {
        float speed_new = float(newPos - pos) / float(ts - last_time);
        pos = newPos;
        if (speedPerMS > 0)
        {
            // set new min
            speedPerMS = std::min(speed_new, speedPerMS);
        }
        else
        {
            speedPerMS = speed_new;
        }
        logInfo(id, "speed per Ms:", speedPerMS);
        last_time = ts;
    }
    void markPassed()
    {
        passed = true;
    }
    bool isPassed() const
    {
        return passed;
    }

protected:
    std::uint32_t id;
    int pos;
    long long last_time = 0;
    float speedPerMS = 0;
    bool passed = false;
};

/**
 * @class NaiveTracker
 * @brief A simple tracker for detecting objects passing through a specified area.
 * It does not handle occlusions
 *
 * The NaiveTracker class is designed to track objects and determine if they have passed a certain area (exitAreaY).
 * It expects sorted detections for proper functionality.
 * WARN: For no occulsion we have to use lower threshold for detections
 * @warning This tracker expects sorted detections.
 *
 * @param exitAreaY The Y-coordinate of the exit area. Objects passing this Y-coordinate are considered passed.
 *
 */
class NaiveTracker
{
private:
    int getNewId() const
    {
        static int id = 0;
        id++;
        if (id > 1000)
        {
            id = 0;
        }
        return id;
    }

public:
    NaiveTracker() {}

    NaiveTracker(const std::string &lane_name) : name{lane_name} {}

    void setExitAreaY(int y)
    {
        exitAreaY = y;
    }

    /*
     * @method updateTracker
     * @brief Updates the tracker with new detections. WARN: Expects sorted detections. inreasing order
     * @param detections A vector of detections to update the tracker with.
     */
    auto updateTracker(const std::vector<int> &detections, long long ts) -> bool
    {
        std::deque<LaneObj> matchedTrackers;
        // imagine we have tracked obj positions [3,4,7,8] and detections [1,2, 10, 20]
        // we start from the last element in tracked objects i.e in reverse
        // the first element in detections greater than 8-disp is 10 so we can exclude 20 in the next iteration
        // add it to matchedTrackers
        // as we have only one direction, it goes from left to right
        // we have no other choice but to exclude the last element in detections in the next iteration
        // next we check 7-disp  and we have 10 so we can exclude 10 in the next iteration
        // next we check 4 and it has std::end no greater element in detections
        // it means our best candidates shoud be shifted to the right
        // it means we should reassign 4->10 7->20 leaving 8 unmatched
        for (auto it = lane.rbegin(); it != lane.rend(); ++it)
        {
            auto &l = *it;
            // the first element greater than l.getPos() -Ignore_disp to consider noise and pause frames
            auto upper = detections.end();
            if (detections.size() - matchedTrackers.size() > 0)
            {
                // upper bound does not check the range, we have to be sure ourself
                upper = std::upper_bound(detections.begin(), detections.begin() + detections.size() - matchedTrackers.size(), l.getPos() - IGNORE_DISP);
            }
            if (upper == detections.end())
            {
                // no previous matched , directly add to unmatchedTrackersReverse
                // otherwise shift to the right
                if (matchedTrackers.size() > 0)
                {
                    // remove the last from matchedTrackers
                    auto el = matchedTrackers.back();
                    matchedTrackers.pop_back();
                    matchedTrackers.push_front(l);
                    // unmatchedTrackersReverse.push_back(el);
                }
                else
                {
                    // unmatchedTrackersReverse.push_back(l);
                }
                // NOTE: matchedTrackers size will not change
            }
            else
            {
                // as we have element in detections that is greater than l.getPos() -Ignore_disp
                // our last element in excluded detections should be our best match
                // so we can exclude the last element in detections  in the next iteration or decrease the size of detections
                // for the tracked object being lower than the tracked object
                matchedTrackers.push_front(l);
                // NOTE: matchedTrackers size will increase; so we can exclude the last element in detections
            }
        }

        // clear our lane and reconstruct it using matched detections
        lane.clear();
        // logInfo("Matched Trackers: ", matchedTrackers.size(), " Detections: ", detections.size());
        auto newElementsCount = detections.size() - matchedTrackers.size();
        if (newElementsCount > 0)
        {
            // it means we have new detections that were not tracked
            // add them to tracked objects if they are below the exit area
            // as we will consider them as new objects if they are above the exit area
            for (int i = 0; i < newElementsCount; i++)
            {
                if (detections[i] < exitAreaY)
                {
                    lane.push_back(LaneObj(getNewId(), detections[i], ts));
                }
            }
        }
        long long potential_future_loss=last_time!=0?ts-last_time:5;
        last_time =ts;
        logInfo("potential_future_loss", potential_future_loss);
        bool passed = false;
        for (auto &el : matchedTrackers)
        {
            el.setPos(detections[newElementsCount], ts);
            if (el.getPotentialPos(potential_future_loss/3) > exitAreaY && !el.isPassed())
            {
                logInfo(name, "Object ", el.getId(), " passed at ", ts, "area ", exitAreaY);
                el.markPassed();
                passed = true;
            }
            lane.push_back(el);
            newElementsCount++;
        }

        return passed;
    }

    static void printDetections(const std::string &msg, const std::vector<int> &detections)
    {
        std::stringstream ss;
        for (auto &d : detections)
        {
            ss << d << ",";
        }
        logInfo(msg, ss.str());
    }
    void printLane() const
    {
        std::stringstream ss;
        for (auto &el : lane)
        {
            ss << el.getId() << ": " << el.getPos() << ",";
        }
        logInfo(name, ss.str());
    }

    void check_test()
    {
        std::vector<std::vector<int>> vectors = {
            {},
            {},
            {78, 80},
            {78, 80},
            {132},
            {132},
        };
        NaiveTracker track;
        track.setExitAreaY(440);
        int i = 0;
        for (auto &v : vectors)
        {
            i++;
            logInfo(i, ")");
            NaiveTracker::printDetections("Detections", v);
            auto b = track.updateTracker(v, i);
            track.printLane();
            if (b)
            {
                logInfo("Object passed");
            }
        }
    }

private:
    std::string name;
    std::deque<LaneObj> lane;
    long long last_time=0;
    // std::vector<LaneObj> unmatchedTrackersReverse;
    int exitAreaY;
};
