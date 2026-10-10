#ifndef PARAMETERS_HPP
#define PARAMETERS_HPP

#include "types.hpp"
#include "vector.hpp"
#include "time_manager.hpp"
#include <mutex>

// Constants
constexpr f64_t COULOMB_CONST = 8987551786.2;
constexpr f64_t ELECTRON_CHARGE = 0.0000000000000000001602176634;

inline std::mutex globalMutex;

inline std::string loadedSim = "";

inline bool running = true;

// Global variables
inline f64_t ldCoeff = 1.88e-10;
inline f64_t adCoeff = 5.0e-28;

inline f64_t lastAvgLinVel = 0.0;

inline f64_t avgLinVel = 0.8e-8;
inline f64_t maxLinVel = 1.0e-8;
inline f64_t maxAngVel = 5.0;

// There is a problem where everything is scaled by 1.5 so "2/3" does what "1.0" should
inline f64_t simScale = (2.0 / 3.0) * 9.5; // if set to 1.0 -> 1px (on screen) = 1nm (in the simulation)

inline lle::Vec4<f64_t> simArea{0.0, 0.0, 100.0, 100.0};

inline f64_t transcrMultiplier = 0.1;
inline f64_t translatMultiplier = 0.1;
inline f64_t dnaReplMultiplier = 1.0;
inline f64_t proteinInactivityTime = 1.0;

inline f64_t hydrophobicInteraction = 35.0;
inline f64_t hydrohpilicInteraction = 2.0;
inline f64_t oppositeInteraction = -2.0;

inline f64_t maxInteractionDist = 0.2;
inline f64_t contactDist = 0.1;


struct FrameTimeChange {
    u64_t frame;
    f64_t oldValue;
};

inline TimeManager &globalTimer = TimeManager::Instance();
inline const f64_t &simFrameTime = globalTimer.getFrameTime();
inline const u64_t &frameCounter = globalTimer.getFrameCounter();

inline std::unordered_map<u8_t, std::string> compoundMap{
    {0, ""},
        {1, "sample-phospholipid"},
        {2, "sample-phospholipid-long"},
        {3, "sample-dna"},
        {4, "sample-rna-p1"},
        {5, "sample-rna-p2"},
        {6, "sample-rna-p3"},
        {7, "sample-protein-1-transcription"},
        {8, "sample-protein-2-translation"},
        {9, "sample-protein-3-dna-repl"},
};

#endif // !PARAMETERS_HPP
