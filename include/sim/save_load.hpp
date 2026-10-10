#ifndef SAVE_RELOAD_HPP
#define SAVE_RELOAD_HPP

#include "compound.hpp"
#include "compound_templates.hpp"
#include "nlohmann/json_fwd.hpp"
#include "particle.hpp"
#include "parameters.hpp"

#include <filesystem>
#include <string>
#include <vector>
#include <fstream>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

using pSave_t = std::tuple<std::tuple<f32_t, f32_t>, f32_t, std::tuple<u8_t, u8_t, u8_t>, bool>;

inline void save(std::vector<Compound> &cmps, std::string fileName) {
    json saveData = json::parse("{}");
    auto &globals = saveData["globals"];
    auto &timer = globals["timer"];
    auto &compounds = saveData["compounds"];
    auto &mappings = globals["compound_mappings"];

    mappings[0] = compoundMap[0];
    mappings[1] = compoundMap[1];
    mappings[2] = compoundMap[2];
    mappings[3] = compoundMap[3];
    mappings[4] = compoundMap[4];
    mappings[5] = compoundMap[5];
    mappings[6] = compoundMap[6];
    mappings[7] = compoundMap[7];
    mappings[8] = compoundMap[8];
    mappings[9] = compoundMap[9];

    globals["ld_coeff"] = ldCoeff;
    globals["ad_coeff"] = adCoeff;
    globals["last_average_linear_velocity"] = lastAvgLinVel;
    globals["average_linear_velocity"] = avgLinVel;
    globals["max_linear_velocity"] = maxLinVel;
    globals["max_angular_velocity"] = maxAngVel;
    globals["sim_scale"] = simScale;
    globals["sim_area"] = {simArea.X, simArea.Y, simArea.Z, simArea.W};
    globals["transcription_multiplier"] = transcrMultiplier;
    globals["translation_multiplier"] = translatMultiplier;
    globals["dna_replication_multiplier"] = dnaReplMultiplier;
    globals["protein_inactivity_time"] = proteinInactivityTime;
    globals["hydrophobic_interaction"] = hydrophobicInteraction;
    globals["hydrophilic_interaction"] = hydrohpilicInteraction;
    globals["opposite_interaction"] = oppositeInteraction;
    globals["max_interaction_distance"] = maxInteractionDist;
    globals["contact_distance"] = contactDist;
    globalTimer.Save(&timer);

    for (const auto &compound : cmps) {
        compounds.push_back({
            compound.type,
            {compound.pos.X, compound.pos.Y},
            [&compound]() -> std::vector<pSave_t> {
                std::vector<pSave_t> data{};
                for (const auto &particle : compound.particles) {
                    data.push_back({{particle.pos.X, particle.pos.Y}, particle.radius, {particle.color.R, particle.color.G, particle.color.B}, particle.hydrophobic});
                }
                return data;
            }(),
            [&compound]() -> std::unordered_map<std::string, u32_t> {
                std::unordered_map<std::string, u32_t> map{};
                for (const auto &[key, value] : compound.onContact) {
                    map[std::to_string((u32_t)key)] = (u32_t)value(nullptr, 0, nullptr, 0);
                }
                return map;
            }(),
            compound.encodes,
            compound.interactsWith,
            compound.lifetime,
            {compound.linearVel.X, compound.linearVel.Y},
            compound.angularVel,
            compound.spawnTime,
            });
    }

    std::ofstream f(fileName);
    f << saveData.dump(4) << std::endl;
}

inline void load(std::vector<Compound> &cmps, std::string fileName) {
    cmps.clear();

    std::ifstream f(fileName);
    json loadData = json::parse(f);

    auto &globals = loadData["globals"];
    auto &timer = globals["timer"];
    auto &compounds = loadData["compounds"];
    auto &mappings = globals["compound_mappings"];

    for (size_t i = 0; i < mappings.size(); i++) {
        const std::string mapping(mappings[i]);
        if (mapping.empty()) continue;
        compoundMap[i] = mapping;
        if (!compounds_templates.contains(mapping)) {
            std::cout << "compound template '" << mapping << "' was loaded but doesn't exist in the current configuration and won't be usable" << std::endl;
        }
    }

    ldCoeff = globals["ld_coeff"];
    adCoeff = globals["ad_coeff"];
    lastAvgLinVel = globals["last_average_linear_velocity"];
    avgLinVel = globals["average_linear_velocity"];
    maxLinVel = globals["max_linear_velocity"];
    maxAngVel = globals["max_angular_velocity"];
    simScale = globals["sim_scale"];
    simArea = {globals["sim_area"][0], globals["sim_area"][1], globals["sim_area"][2], globals["sim_area"][3]};
    transcrMultiplier = globals["transcription_multiplier"];
    translatMultiplier = globals["translation_multiplier"];
    dnaReplMultiplier = globals["dna_replication_multiplier"];
    proteinInactivityTime = globals["protein_inactivity_time"];
    hydrophobicInteraction = globals["hydrophobic_interaction"];
    hydrohpilicInteraction = globals["hydrophilic_interaction"];
    oppositeInteraction = globals["opposite_interaction"];
    maxInteractionDist = globals["max_interaction_distance"];
    contactDist = globals["contact_distance"];
    globalTimer.Load(&timer);

    for (const auto &compound : compounds) {
        cmps.push_back({
            (CompoundType)compound[0],
            {compound[1][0], compound[1][1]},
            [&compound]() -> std::vector<Particle> {
                std::vector<Particle> particles{};
                for (const auto &pData : compound[2]) {
                    particles.push_back({{pData[0][0], pData[0][1]}, pData[1], {pData[2][0], pData[2][1], pData[2][2]}, (bool)pData[3]});
                }
                return particles;
            }(),
            [&compound]() -> std::unordered_map<CompoundType, std::function<CompoundInteractionType(Compound*, int, Compound*, int)>> {
                std::unordered_map<CompoundType, std::function<CompoundInteractionType(Compound*, int, Compound*, int)>> map{};
                for (const auto &[key, value] : compound[3].items()) {
                    map[(CompoundType)std::stoi(key)] = getInteraction(value);
                }
                return map;
            }(),
            compound[4],
            (CompoundType)compound[5],
            compound[6],
            {compound[7][0], compound[7][1]},
            compound[8],
            compound[9],
        });
    }
}

inline void loadTemplates(std::string dirName) {
    const std::filesystem::path dirPath(dirName);
    const std::filesystem::path compoundsPath(dirPath / "compounds.json");
    if (std::filesystem::exists(compoundsPath)) {
        std::ifstream f(compoundsPath);
        const nlohmann::json data = nlohmann::json::parse(f);
        for (auto &compound : data.items()) {
            const auto &compoundData = compound.value();
            compounds_templates.emplace(std::string(compound.key()), Compound{
                compoundData[0],
                {compoundData[1][0], compoundData[1][1]},
                [&compoundData]() -> std::vector<Particle> {
                    std::vector<Particle> particles{};
                    for (const auto &pData : compoundData[2]) {
                        particles.push_back({{pData[0][0], pData[0][1]}, pData[1], {pData[2][0], pData[2][1], pData[2][2]}, (bool)pData[3]});
                    }
                    return particles;
                }(),
                [&compoundData]() -> std::unordered_map<CompoundType, std::function<CompoundInteractionType(Compound*, int, Compound*, int)>> {
                    std::unordered_map<CompoundType, std::function<CompoundInteractionType(Compound*, int, Compound*, int)>> map{};
                    for (const auto &[key, value] : compoundData[3].items()) {
                        map[(CompoundType)std::stoi(key)] = getInteraction(value);
                    }
                    return map;
                }(),
                compoundData[4],
                [&compoundData] -> CompoundType {
                    u32_t interacion = 0;
                    for (auto &val : compoundData[5]) {
                        interacion |= static_cast<u32_t>(val);
                    }
                    return static_cast<CompoundType>(interacion);
                }(),
                [&compoundData] -> f64_t {
                    try {
                        return compoundData.at(6);
                    } catch (nlohmann::json::exception e) {
                        return -1.0;
                    }
                }()
            });
        }
    }
}

#endif // !SAVE_RELOAD_HPP
