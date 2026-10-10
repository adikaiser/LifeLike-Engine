#ifndef TIME_MANAGER_HPP
#define TIME_MANAGER_HPP

#include "types.hpp"
#include <iostream>
#include <string>
#include <unordered_map>
#include <tuple>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Singleton class that keeps track of time related data
class TimeManager {
private:
    f64_t simFrameTime = 0.001;
    u64_t frameCounter = 1;
    u64_t lastChange = 1;
    std::unordered_map<u64_t, std::tuple<f64_t, u64_t, size_t>> timePoints{};
    TimeManager() {};
    TimeManager(TimeManager&) = delete;
    TimeManager(TimeManager&&) = delete;
    TimeManager& operator=(TimeManager) = delete;
    TimeManager& operator=(TimeManager&&) = delete;

public:

    const f64_t& getFrameTime() const { return simFrameTime; }

    void setFrameTime(f64_t newTime) {
        for (auto &[_, value] : timePoints) {
            get<0>(value) += (frameCounter - std::max(lastChange, get<1>(value))) * simFrameTime;
        }
        lastChange = frameCounter;
        simFrameTime = newTime;
    }

    const u64_t& getFrameCounter() const { return frameCounter; }

    void Increment() {
        frameCounter++;
    }

    void recordNow() {
        if (!timePoints.contains(frameCounter)) {
            timePoints[frameCounter] = {0.0, frameCounter, 1};
        } else {
            get<2>(timePoints[frameCounter])++;
        }
    }

    f64_t Read(u64_t frame) {
        f64_t t = 0.0;
        if (timePoints.contains(frame)) {
            t = get<0>(timePoints[frame]) + (frameCounter - std::max(lastChange, get<1>(timePoints[frame]))) * simFrameTime;
        } else {
            std::cout << "ERROR trying to read a non existent time point: " << frame << '\n';
            std::abort();
        }
        return t;
    }

    void deleteEntry(u64_t frame) {
        if (get<2>(timePoints[frame]) <= 1) timePoints.erase(frame);
        else get<2>(timePoints[frame])--;
    }

    u64_t getEntryCount() const {
        return timePoints.size();
    }

    void Save(json *saveData) {
        (*saveData)["sim_frame_time"] = simFrameTime;
        (*saveData)["sim_frame_counter"] = frameCounter;
        (*saveData)["last_sim_time_change"] = lastChange;
        auto &savePoints = (*saveData)["time_points"];
        for (const auto &[key, value] : timePoints) {
            savePoints[std::to_string(key)] = {get<0>(value), get<1>(value), get<2>(value)};
        }
    }

    void Load(json *loadData) {
        simFrameTime = (*loadData)["sim_frame_time"];
        frameCounter = (*loadData)["sim_frame_counter"];
        lastChange = (*loadData)["last_sim_time_change"];
        auto &loadPoints = (*loadData)["time_points"];
        for (const auto &[key, value] : loadPoints.items()) {
            timePoints[std::stoull(key)] = {value[0], value[1], value[2]};
        }
    }

    static TimeManager& Instance() {
        static TimeManager inst{};
        return inst;
    }

};

#endif // !TIME_MANAGER_HPP
