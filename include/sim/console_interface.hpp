#ifndef CONSOLE_INTERFACE_HPP
#define CONSOLE_INTERFACE_HPP

#include "parameters.hpp"
#include "compound_templates.hpp"
#include "save_load.hpp"
#include <cstring>
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define TRY_INPUT_EVAL(expr) globalMutex.lock(); try { expr globalMutex.unlock(); } catch (std::exception &e) { globalMutex.unlock(); std::cout << "Invalid Input" << std::endl; inputInvalid = true; continue; }

inline void consoleIO() {
    bool inputInvalid = true;
    std::string input;
    std::vector<std::string> inputParsed;
    while (1) {

        if (inputInvalid)  {
            std::cout
            << "Console Input\n"
            << "  'c' for controls and commands\n"
            << "  'l' for a list of simulation parameters\n"
            << "  'lc' for a list of available compounds\n"
            << "  'li' for a list of available interactions\n"
            << "  'e'/'exit' to save and exit (DO NOT CLOSE MANUALLY OR SIMULATION WILL BE LOST)"
            << std::endl;
        }
        std::cout << "\n> ";
        std::getline(std::cin, input);

        if (input.size() == 0) {
            inputInvalid = true;
            continue;
        }

        inputParsed.clear();
        std::istringstream inputStream(input);
        std::string token;
        while (std::getline(inputStream, token, ' ')) {
            inputParsed.push_back(std::move(token));
        }
        const char *inputCStr = inputParsed[0].c_str();

        
        if (strcmp(inputCStr, "c") == 0 && inputParsed.size() == 1) {
            std::cout
            << "general controls:\n"
            << "  SPACE - pause or unpause the simulation (will increate FPS and thus placement frequency when holding RIGHTMOUSE)\n"
            << "  SCROLL - zoom in or out of the simulation\n"
            << "  MIDDLEMOUSE - move the simulation area while pressed\n"
            << "  ARROWKEYS - move the simulation area\n"
            << "  R - return to the imulation area"
            << "  I - insert mode to place compounds\n"
            << "  P - push mode to push compounds with the mouse cursor\n"
            << "  O - destroy mode to destroy/delete compounds with the mouse cursor\n"
            << '\n'
            << "insert mode controls:\n"
            << "  LETMOUSE - place one compound of the selected type when clicked\n"
            << "  RIGHTMOUSE - repedetly place one compound of the selected type when held down\n"
            << '\n'
            << "commands:\n"
            << "  'c' - get controls and commands\n"
            << "  'l' - get a list of simulation parameters\n"
            << "  'lp' - get a list of available compounds\n"
            << "  'li' - get a list of available interactions"
            << "  'e'/'exit' - exit the application and save the current simulation (will open a prompt)\n"
            << "  'e'/'save' - save the current simulation (will open a prompt)"
            << std::endl;
        } else if (strcmp(inputCStr, "l") == 0 && inputParsed.size() == 1) {
            globalMutex.lock();

            std::cout
            << "values can be edited with this syntax: '<value name> <value(s) (val1 val2 ...)>\n"
            << "examples: avglinvel 12.3 | simareaext 150 100 | num1 phospholipid\n"
            << '\n'
            << "variables:\n"
            << "  simareaext [" << simArea.Z << ", " << simArea.W << "] - size of the simulation area (white rectangle)\n"
            << "  lindragcoeff [" << ldCoeff << "] - linear drag coefficient of the simulation (affects linear drag)\n"
            << "  angdragcoeff [" << adCoeff << "] - angular drag coefficient of the simulation (affects angular drag)\n"
            << "  avglinvel [" << avgLinVel << "nm/s] - forced average liear velocity of the sum of all compounds in the simulation\n"
            << "  maxlinvel [" << maxLinVel << "nm/s] - max linear velocity a compound can have\n"
            << "  maxangvel [" << maxAngVel << "rad/s] - max angular velocity a compound can have\n"
            << "  hhstrength [" << hydrohpilicInteraction << "] - interaction strength between two hydrophilic particles (<0 -> repulsive)\n"
            << "  ppstrength [" << hydrophobicInteraction << "] - interaction strength between two hydrophobic particles (<0 -> repulsive)\n"
            << "  hpstrength [" << oppositeInteraction << "] - interaction strength between a hydrophilic and a hydrophobic particle (<0 -> repulsive)\n"
            << "  maxinterdist [" << maxInteractionDist << "nm] - distance between the centers of two particles at which they will start interacting\n"
            << "  contactdist [" << contactDist << "nm] - distance between the surfaces of two particles at which special interactions will occur\n"
            << "  transcrspeed [" << transcrMultiplier << "s] - approximate time it takes to move to the next particle of a compound during transcription\n"
            << "  translspeed [" << translatMultiplier << "s] - approximate time it takes to move to the next particle of a compound during translation\n"
            << "  replspeed [" << dnaReplMultiplier << "s] - approximate time it takes to move to the next particle of a compound during (DNA) replication\n"
            << "  inacttime [" << proteinInactivityTime << "s] - time a compound/protein might be inactive after a special interaction\n"
            << '\n'
            << "Compound numkey mappings:\n"
            << "  num1 [" << compoundMap[1] << "]\n"
            << "  num2 [" << compoundMap[2] << "]\n"
            << "  num3 [" << compoundMap[3] << "]\n"
            << "  num4 [" << compoundMap[4] << "]\n"
            << "  num5 [" << compoundMap[5] << "]\n"
            << "  num6 [" << compoundMap[6] << "]\n"
            << "  num7 [" << compoundMap[7] << "]\n"
            << "  num8 [" << compoundMap[8] << "]\n"
            << "  num9 [" << compoundMap[9] << "]\n"
            << "  num0 [" << compoundMap[0] << "]\n"
            << std::endl;

            globalMutex.unlock();

            inputInvalid = false;

        } else if (strcmp(inputCStr, "lc") == 0 && inputParsed.size() == 1) {
            globalMutex.lock();

            std::cout << "available compounds:";
            for (auto &it : compounds_templates) {
                if (strcmp(it.first.c_str(), "pusher") != 0 && strcmp(it.first.c_str(), "destroyer") != 0) {
                    std::cout << " '" << it.first << '\'';
                }
            }
            std::cout << std::endl;

            globalMutex.unlock();

            inputInvalid = false;

        } else if (strcmp(inputCStr, "li") == 0 && inputParsed.size() == 1) {
            globalMutex.lock();
            std::cout << "Listing interactions is not implemented yet" << std::endl;
            globalMutex.unlock();

            inputInvalid = false;

        } else if (
                (strcmp(inputCStr, "exit") == 0 ||
                 strcmp(inputCStr, "e") == 0 ||
                 strcmp(inputCStr, "save") == 0 ||
                 strcmp(inputCStr, "s") == 0) && inputParsed.size() == 1) {
            globalMutex.lock();

            std::string saveFileName = "";
            
            if (!std::filesystem::exists("./saved_sims")) {
                std::filesystem::create_directory("./saved_sims");
            }

            while (1) {
                if (!loadedSim.empty()) {
                    std::cout << "Enter the name of the save file (ENTER for " << std::filesystem::path(loadedSim).stem() << ", 'none' to discard)\n> ";
                    std::getline(std::cin, saveFileName);
                    if (saveFileName.length() > 0 && saveFileName != "none") {
                        try {
                            save(compounds, "./saved_sims/" + saveFileName + ".json");
                            break;
                        } catch (std::exception e) {
                            std::cout << "ERROR filename invalid\n";
                            saveFileName = "";
                        };
                    } else {
                        save(compounds, loadedSim);
                        break;
                    };
                } else {
                    std::cout << "Enter the name of the save file (ENTER or 'none' to discard)\n> ";
                    std::getline(std::cin, saveFileName);
                    if (saveFileName.length() > 0 && saveFileName != "none") {
                        try {
                            save(compounds, "./saved_sims/" + saveFileName + ".json");
                            break;
                        } catch (std::exception e) {
                            std::cout << "ERROR filename invalid\n";
                            saveFileName = "";
                        };
                    } else break;

                }
            }

            globalMutex.unlock();
            
            if (strcmp(inputCStr, "exit") == 0 || strcmp(inputCStr, "e") == 0) { break; }

        } else if (strcmp(inputCStr, "simareaext") == 0 && inputParsed.size() == 3) { TRY_INPUT_EVAL( simArea.Z = std::stod(inputParsed[1]); simArea.W = std::stod(inputParsed[2]); )
        } else if (strcmp(inputCStr, "lindragcoeff") == 0 && inputParsed.size() == 2) { TRY_INPUT_EVAL( ldCoeff = std::stod(inputParsed[1]); )
        } else if (strcmp(inputCStr, "angdragcoeff") == 0 && inputParsed.size() == 2) { TRY_INPUT_EVAL( adCoeff = std::stod(inputParsed[1]); )
        } else if (strcmp(inputCStr, "avglinvel") == 0 && inputParsed.size() == 2) { TRY_INPUT_EVAL( avgLinVel = std::stod(inputParsed[1]); )
        } else if (strcmp(inputCStr, "maxlinvel") == 0 && inputParsed.size() == 2) { TRY_INPUT_EVAL( maxLinVel = std::stod(inputParsed[1]); )
        } else if (strcmp(inputCStr, "maxangvel") == 0 && inputParsed.size() == 2) { TRY_INPUT_EVAL( maxAngVel = std::stod(inputParsed[1]); )
        } else if (strcmp(inputCStr, "hhstrength") == 0 && inputParsed.size() == 2) { TRY_INPUT_EVAL( hydrohpilicInteraction = std::stod(inputParsed[1]); )
        } else if (strcmp(inputCStr, "ppstrength") == 0 && inputParsed.size() == 2) { TRY_INPUT_EVAL( hydrophobicInteraction = std::stod(inputParsed[1]); )
        } else if (strcmp(inputCStr, "hpstrength") == 0 && inputParsed.size() == 2) { TRY_INPUT_EVAL( oppositeInteraction = std::stod(inputParsed[1]); )
        } else if (strcmp(inputCStr, "maxinterdist") == 0 && inputParsed.size() == 2) { TRY_INPUT_EVAL( maxInteractionDist = std::stod(inputParsed[1]); )
        } else if (strcmp(inputCStr, "contactdist") == 0 && inputParsed.size() == 2) { TRY_INPUT_EVAL( contactDist = std::stod(inputParsed[1]); )
        } else if (strcmp(inputCStr, "transcrspeed") == 0 && inputParsed.size() == 2) { TRY_INPUT_EVAL( transcrMultiplier = std::stod(inputParsed[1]); )
        } else if (strcmp(inputCStr, "translspeed") == 0 && inputParsed.size() == 2) { TRY_INPUT_EVAL( translatMultiplier = std::stod(inputParsed[1]); )
        } else if (strcmp(inputCStr, "replspeed") == 0 && inputParsed.size() == 2) { TRY_INPUT_EVAL( dnaReplMultiplier = std::stod(inputParsed[1]); )
        } else if (strcmp(inputCStr, "inacttime") == 0 && inputParsed.size() == 2) { TRY_INPUT_EVAL( proteinInactivityTime = std::stod(inputParsed[1]); )
        } else if (strcmp(inputCStr, "num0") == 0 && inputParsed.size() == 2) {
            if (compounds_templates.find(inputParsed[1]) == compounds_templates.end()) {
                std::cout << "Assigning non existant compound '" << inputParsed[1] << "' to num1" << std::endl;
            } else {
                std::cout << "Assigning compound '" << inputParsed[1] << "' to num1" << std::endl;
            }
            TRY_INPUT_EVAL( compoundMap[0] = inputParsed[1]; )
        } else if (strcmp(inputCStr, "num1") == 0 && inputParsed.size() == 2) {
            if (compounds_templates.find(inputParsed[1]) == compounds_templates.end()) {
                std::cout << "Assigning non existant compound '" << inputParsed[1] << "' to num2" << std::endl;
            } else {
                std::cout << "Assigning compound '" << inputParsed[1] << "' to num2" << std::endl;
            }
            TRY_INPUT_EVAL( compoundMap[1] = inputParsed[1]; )
        } else if (strcmp(inputCStr, "num2") == 0 && inputParsed.size() == 2) {
            if (compounds_templates.find(inputParsed[1]) == compounds_templates.end()) {
                std::cout << "Assigning non existant compound '" << inputParsed[1] << "' to num3" << std::endl;
            } else {
                std::cout << "Assigning compound '" << inputParsed[1] << "' to num3" << std::endl;
            }
            TRY_INPUT_EVAL( compoundMap[2] = inputParsed[1]; )
        } else if (strcmp(inputCStr, "num3") == 0 && inputParsed.size() == 2) {
            if (compounds_templates.find(inputParsed[1]) == compounds_templates.end()) {
                std::cout << "Assigning non existant compound '" << inputParsed[1] << "' to num4" << std::endl;
            } else {
                std::cout << "Assigning compound '" << inputParsed[1] << "' to num4" << std::endl;
            }
            TRY_INPUT_EVAL( compoundMap[3] = inputParsed[1]; )
        } else if (strcmp(inputCStr, "num4") == 0 && inputParsed.size() == 2) {
            if (compounds_templates.find(inputParsed[1]) == compounds_templates.end()) {
                std::cout << "Assigning non existant compound '" << inputParsed[1] << "' to num5" << std::endl;
            } else {
                std::cout << "Assigning compound '" << inputParsed[1] << "' to num5" << std::endl;
            }
            TRY_INPUT_EVAL( compoundMap[4] = inputParsed[1]; )
        } else if (strcmp(inputCStr, "num5") == 0 && inputParsed.size() == 2) {
            if (compounds_templates.find(inputParsed[1]) == compounds_templates.end()) {
                std::cout << "Assigning non existant compound '" << inputParsed[1] << "' to num6" << std::endl;
            } else {
                std::cout << "Assigning compound '" << inputParsed[1] << "' to num6" << std::endl;
            }
            TRY_INPUT_EVAL( compoundMap[5] = inputParsed[1]; )
        } else if (strcmp(inputCStr, "num6") == 0 && inputParsed.size() == 2) {
            if (compounds_templates.find(inputParsed[1]) == compounds_templates.end()) {
                std::cout << "Assigning non existant compound '" << inputParsed[1] << "' to num7" << std::endl;
            } else {
                std::cout << "Assigning compound '" << inputParsed[1] << "' to num7" << std::endl;
            }
            TRY_INPUT_EVAL( compoundMap[6] = inputParsed[1]; )
        } else if (strcmp(inputCStr, "num7") == 0 && inputParsed.size() == 2) {
            if (compounds_templates.find(inputParsed[1]) == compounds_templates.end()) {
                std::cout << "Assigning non existant compound '" << inputParsed[1] << "' to num8" << std::endl;
            } else {
                std::cout << "Assigning compound '" << inputParsed[1] << "' to num8" << std::endl;
            }
            TRY_INPUT_EVAL( compoundMap[7] = inputParsed[1]; )
        } else if (strcmp(inputCStr, "num8") == 0 && inputParsed.size() == 2) {
            if (compounds_templates.find(inputParsed[1]) == compounds_templates.end()) {
                std::cout << "Assigning non existant compound '" << inputParsed[1] << "' to num9" << std::endl;
            } else {
                std::cout << "Assigning compound '" << inputParsed[1] << "' to num9" << std::endl;
            }
            TRY_INPUT_EVAL( compoundMap[8] = inputParsed[1]; )
        } else if (strcmp(inputCStr, "num9") == 0 && inputParsed.size() == 2) {
            if (compounds_templates.find(inputParsed[1]) == compounds_templates.end()) {
                std::cout << "Assigning non existant compound '" << inputParsed[1] << "' to num0" << std::endl;
            } else {
                std::cout << "Assigning compound '" << inputParsed[1] << "' to num0" << std::endl;
            }
            TRY_INPUT_EVAL( compoundMap[9] = inputParsed[1]; )
        } else { inputInvalid = true; }
    }

    globalMutex.lock();

    running = false;

    globalMutex.unlock();

}

#undef TRY_INPUT_EVAL

#endif // !CONSOLE_INTERFACE_HPP
