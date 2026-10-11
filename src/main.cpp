#include "nlohmann/json_fwd.hpp"
#include "vector.hpp"
#include "types.hpp"
#include "console_interface.hpp"

#include <chrono>
#include <ctime>
#include <fstream>
#include <ostream>
#include <thread>
#include <unordered_map>
#include <window.hpp>
#include <parameters.hpp>
#include <functions.hpp>
#include <compound.hpp>
#include <compound_templates.hpp>
#include <save_load.hpp>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_surface.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <cstring>
#include <iostream>
#include <string>
#include <ctime>
#include <filesystem>
#include <inicpp.hpp>
/*#include <future>
#include <ctpl.h>*/

enum Mode {
    PLACE,
    PUSH,
    DELETE,
};


void printHelp() {
    std::cout
#ifdef OS_WINDOWS
    << "usage: LifeLike-Engine.exe [PATH] [OPTION ARGS]\n\n"
#elifdef OS_LINUX
    << "usage: LifeLike-Engine [-h/--help] [-v/--version] [PATH] [OPTION ARG]\n\n"
#endif

    << " PATH:\n"
    << "\tpath to a .json file in saved_sims full path, relative path or name of the file (without .json)\n"
    << "\tused to load a previously saved simulation\n\n"

    << " OPTION ARG:\n"
    << "\tlist compounds\t\t\tlists all available compounds\n"
    << "\tlist interactions\t\tlists all available interations\n"
    << "\tset templates [-d] <dir>\tpersistently specify a directory where templates for compounds, etc. are stored\n"
    << "\t\t\t\t\tthe previously used directory will be deleted if empty or kept when '-k' is passed\n"
    << "\tget templates\t\t\tget the directory where templates for compounds, etc. are stored\n"
    << std::endl;
}


void printVersion() {
    std::cout
    << "LifeLike Engine : "
    << "1.0.0"
    << std::endl;
}


void printCompounds() {
    std::cout << "available compounds:";
    for (auto &it : compounds_templates) {
        if (strcmp(it.first.c_str(), "pusher") != 0 && strcmp(it.first.c_str(), "destroyer") != 0) {
            std::cout << "\n'" << it.first << '\'';
        }
    }
    std::cout << std::endl;
}


void printInteractions() {
    std::cout << "Listing interactions is not implemented yet" << std::endl;
}


void printWrongParams(int argc, char *argv[]) {
    std::cerr << "invalid input:\n";
    for (size_t i = 1; i < argc; i++) {
        std::cerr << ' ' << argv[i];
    }
    std::cerr << "\n\npass [-h] or [--help] to get a list of possible options"<< std::endl;
}

std::string findSaveFile(char *s) {
    if (std::filesystem::exists(s)) {
        return std::string{s};
    } else if (std::string alt_path{std::string("./saved_sims/") + s + ".json"}; std::filesystem::exists(alt_path)) {
        return alt_path;
    } else return "";
}

void createTemplateFiles(std::string path) {
    std::filesystem::create_directories(path);
    std::ofstream output(std::filesystem::path(path) / "compounds.json");
    output << nlohmann::json::parse("{}").dump(4);
}

void centerSimArea(const lle::Vec2<i32_t> &wSize) {
    simArea.X = ((wSize.X / 2.0) / simScale) - (simArea.Z / 2.0);
    simArea.Y = ((wSize.Y / 2.0) / simScale) - (simArea.W / 2.0);

}


int main(int argc, char *argv[]) {

    inicpp::IniManager configFile("./config.ini");
    configFile.setComment("last accessed", "LifeLike Engine config file DO NOT MANUALLY EDIT");
    std::time_t t = time(0);

    std::string dateTime = std::ctime(&t);
    dateTime.erase(dateTime.length() - 1, 1);
    configFile.set("last accessed", dateTime);

    if (!configFile["LifeLike Engine"].isKeyExist("templates_location") || !std::filesystem::exists(configFile["LifeLike Engine"]["templates_location"].String())) {
        configFile.set("LifeLike Engine", "templates_location", std::filesystem::absolute("./templates").string());
        createTemplateFiles(std::filesystem::absolute("./templates").string());
    }

    if (argc > 1) {
        for (size_t i = 1; i < argc; i++) {
            if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
                printHelp();
                return 0;
            } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
                printVersion();
                return 0;
            }
        }

        if(strcmp(argv[1], "list") == 0) {
            if (strcmp(argv[2], "compounds") == 0) {
                loadTemplates(configFile["LifeLike Engine"]["templates_location"]);
                printCompounds();
                return 0;
            } else if (strcmp(argv[2], "interactions") == 0) {
                printInteractions();
                return 0;
            } else {
                printWrongParams(argc, argv);
                return 1;
            }
        } else if (strcmp(argv[1], "set") == 0) {
            if (strcmp(argv[2], "templates") == 0 && std::filesystem::exists(argv[3])) {
                if (strcmp(argv[3], "-k") == 0 && std::filesystem::exists(argv[4])) {
                    configFile.set("LifeLike Engine", "templates_location", argv[4]);
                    createTemplateFiles(argv[4]);
                } else if (std::filesystem::exists(argv[3])) {
                    std::string oldPath = configFile["LifeLike Engine"]["templates_location"];
                    configFile.set("LifeLike Engine", "templates_location", argv[3]);
                    if (std::filesystem::exists(oldPath) && std::filesystem::is_empty(oldPath)) {
                        std::filesystem::remove(oldPath);
                    }
                    createTemplateFiles(argv[3]);
                } else {
                    printWrongParams(argc, argv);
                    return 1;
                }

            } else {
                printWrongParams(argc, argv);
                return 1;
            }
        } else if (strcmp(argv[1], "get") == 0) {
            if (strcmp(argv[2], "templates") == 0) {
                std::cout << configFile["LifeLike Engine"]["templates_location"] << std::endl;
            } else {
                printWrongParams(argc, argv);
                return 1;
            }
        } else if (auto path = findSaveFile(argv[1]); !path.empty()) {
            try {
                load(compounds, path);
                loadedSim = path;
            } catch (nlohmann::json_abi_v3_12_0::detail::parse_error e) {
                std::cerr << "failed to load save file\n" << path << "\nthe file contents might not be in a valid JOSN format\n" << e.what() << std::endl;
                return 1;
            }
        } else {
            printWrongParams(argc, argv);
            return 1;
        }
    }

    //const auto threadCount = std::thread::hardware_concurrency();
    //std::cout << "Available treads: " << threadCount << '\n';
    //ctpl::thread_pool workers(threadCount);
    //std::future<void> futures[threadCount];
    
    bool exit = false;

    lle::Window w{{1000, 1000}, "LifeLike Engine"};
    SDL_Event e{};

    f64_t dt = 0.0;

    Mode mode = PLACE;
    bool paused = true;

    f64_t totalSpeed;
    f64_t _f64; // Unused value
    std::string spawnCompound = "";
    bool spawn = false;
    lle::Vec2<f64_t> simMove{0.0, 0.0};

    Compound pusher{compounds_templates.at("pusher"), {1.0e21, 1.0e21}};
    Compound deleter{compounds_templates.at("destroyer"), {1.0e21, 1.0e21}};

    loadTemplates(configFile["LifeLike Engine"]["templates_location"]);

    if (loadedSim.empty()) {
        centerSimArea(w.GetSize());
    }

    std::thread consoleInterface(consoleIO);
    consoleInterface.detach();

    while (w.isOpen()) {

        globalMutex.lock();

        if (!running) w.Close();

        dt = w.DeltaTime();
        lle::Vec2<f32_t> mousePos = w.GetMousePos();

        lle::Vec2<i32_t> windowSize = w.GetSize();

        while (SDL_PollEvent(&e)) {
            w.EventHandling(e);
            switch (e.type) {
                case SDL_EVENT_KEY_DOWN:
                    switch (e.key.key) {
                        case SDLK_RETURN:
                            std::cout << "FPS: " << 1.0 / w.DeltaTime() <<
                                         ", Time facotor: " << simFrameTime / w.DeltaTime() <<
                                         ", Compounds: " << compounds.size() <<
                                         ", Frame: " << frameCounter <<
                                         ", Simulation uptime: " << frameCounter * 0.001 << "s" <<
                                         ", Timer entries: " << globalTimer.getEntryCount() << '\n';
                            break;
                        case SDLK_UP:
                            simMove.Y = 1000.0 / simScale;
                            break;
                        case SDLK_DOWN:
                            simMove.Y = -1000.0 / simScale;
                            break;
                        case SDLK_RIGHT:
                            simMove.X = -1000.0 / simScale;
                            break;
                        case SDLK_LEFT:
                            simMove.X = 1000.0 / simScale;
                            break;
                        case SDLK_SPACE:
                            paused = !paused;
                            break;
                        case SDLK_I:
                            mode = PLACE;
                            pusher.pos = {1.0e21, 1.0e21};
                            deleter.pos = {1.0e21, 1.0e21};
                            break;
                        case SDLK_O:
                            mode = DELETE;
                            pusher.pos = {1.0e21, 1.0e21};
                            break;
                        case SDLK_P:
                            mode = PUSH;
                            deleter.pos = {1.0e21, 1.0e21};
                            break;
                        case SDLK_R:
                            centerSimArea(w.GetSize());
                            break;
                        case SDLK_0:
                            spawnCompound = compoundMap[0];
                            break;
                        case SDLK_1:
                            spawnCompound = compoundMap[1];
                            break;
                        case SDLK_2:
                            spawnCompound = compoundMap[2];
                            break;
                        case SDLK_3:
                            spawnCompound = compoundMap[3];
                            break;
                        case SDLK_4:
                            spawnCompound = compoundMap[4];
                            break;
                        case SDLK_5:
                            spawnCompound = compoundMap[5];
                            break;
                        case SDLK_6:
                            spawnCompound = compoundMap[6];
                            break;
                        case SDLK_7:
                            spawnCompound = compoundMap[7];
                            break;
                        case SDLK_8:
                            spawnCompound = compoundMap[8];
                            break;
                        case SDLK_9:
                            spawnCompound = compoundMap[9];
                            break;
                        default:
                            break;
                    } 
                    break;
                case SDL_EVENT_KEY_UP:
                    switch (e.key.key) {
                    case SDLK_UP:
                        if (simMove.Y > 0.0) simMove.Y = 0.0;
                        break;
                    case SDLK_DOWN:
                        if (simMove.Y < 0.0) simMove.Y = 0.0;
                        break;
                    case SDLK_RIGHT:
                        if (simMove.X < 0.0) simMove.X = 0.0;
                        break;
                    case SDLK_LEFT:
                        if (simMove.X > 0.0) simMove.X = 0.0;
                        break;
                    default:
                        break;
                    }
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    switch (e.button.button) {
                    case SDL_BUTTON_LEFT:
                        if (mode == PLACE) {
                            if (compounds_templates.contains(spawnCompound)) {
                                compounds.emplace_back(Compound(compounds_templates.at(spawnCompound), transform(mousePos)));
                            }
                        }
                        break;
                    case SDL_BUTTON_RIGHT:
                        if (mode == PLACE) {
                            spawn = true;
                        }
                        break;
                    default:
                        break;
                    }
                    break;
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    switch (e.button.button) {
                    case SDL_BUTTON_RIGHT:
                        spawn = false;
                        break;
                    default:
                        break;
                    }
                case SDL_EVENT_MOUSE_WHEEL:
                    if (e.wheel.y < 0.0) {
                        f32_t oldScale = simScale;

                        auto worldX = mousePos.X / oldScale - simArea.X;
                        auto worldY = mousePos.Y / oldScale - simArea.Y;

                        simScale /= 1.07f;

                        simArea.X = mousePos.X / simScale - worldX;
                        simArea.Y = mousePos.Y / simScale - worldY;
                    } else if (e.wheel.y > 0.0 && e.wheel.y < 10.0) {
                        f32_t oldScale = simScale;

                        auto worldX = mousePos.X / oldScale - simArea.X;
                        auto worldY = mousePos.Y / oldScale - simArea.Y;

                        simScale *= 1.07f;

                        simArea.X = mousePos.X / simScale - worldX;
                        simArea.Y = mousePos.Y / simScale - worldY;
                    }
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                    if (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_MASK(SDL_BUTTON_MIDDLE)) {
                        simArea.X += e.motion.xrel / simScale;
                        simArea.Y += e.motion.yrel / simScale;
                    }
                    break;
                default:
                    break;
            }
        }

        simArea.X += simMove.X * dt;
        simArea.Y += simMove.Y * dt;

        switch (mode) {
            case PLACE:
                if (spawn && compounds_templates.contains(spawnCompound)) {
                    compounds.emplace_back(Compound(compounds_templates.at(spawnCompound), transform(mousePos)));
                }
                break;
            case PUSH:
                pusher.pos = transform(mousePos);
                break;
            case DELETE:
                deleter.pos = transform(mousePos);
                break;
            default:
                break;
        }

        if (paused) {
            for (size_t i = 0; i < compounds.size(); i++) {
                compounds[i].Update(&w, simFrameTime, false);
            }
        } else {
            /*
            const size_t batchSize = std::ceil(compounds.size() / (f64_t)threadCount);

            for (u32_t i = 0; i < threadCount; i++) {
                futures[i] = workers.push([start = i * batchSize, end = std::min((i + 1) * batchSize, compounds.size())](int) {
                    for (size_t i = start; i < end; i++) {
                        for (size_t j = i + 1; j < compounds.size(); j++) {
                            Compound::Calculate(compounds[i], compounds[j], simFrameTime);
                        }
                        Compound::Calculate(compounds[i], pusher, simFrameTime);
                        Compound::Calculate(compounds[i], deleter, simFrameTime);
                    }
                });
            }

            for (auto &future : futures) future.get();
            */
            
            for (size_t i = 0; i < compounds.size(); i++) {
                for (size_t j = i + 1; j < compounds.size(); j++) {
                    Compound::Calculate(compounds[i], compounds[j], simFrameTime);
                }
                Compound::Calculate(compounds[i], pusher, simFrameTime);
                Compound::Calculate(compounds[i], deleter, simFrameTime);
            }

            for (size_t i = 0; i < compounds.size(); i++) {
                compounds[i].Update(&w, simFrameTime, true);
            }

            lastAvgLinVel = Compound::totalLinVel / std::max(compounds.size(), static_cast<size_t>(1));
            Compound::totalLinVel = 0.0;
        }

        pusher.particles[0].Draw(&w, {pusher.pos.X + simArea.X, pusher.pos.Y + simArea.Y}, simScale);
        deleter.particles[0].Draw(&w, {deleter.pos.X + simArea.X, deleter.pos.Y + simArea.Y}, simScale);

        w.DrawRectangle(
            {simArea.X * simScale, simArea.Y * simScale},
            {simArea.Z * simScale, simArea.W * simScale},
            {255, 255, 255},
            lle::ShaderTypes::OUTLINE
        );

        w.Update();

        std::erase_if(compounds, [&](const Compound& x) {
            return deleteElems.contains(const_cast<Compound*>(&x));
        });
        deleteElems.clear();

        compounds.reserve(compounds.size() + addElems.size());

        compounds.insert(
            compounds.end(),
            std::make_move_iterator(addElems.begin()),
            std::make_move_iterator(addElems.end())
        );

        addElems.clear();

        if (!paused) globalTimer.Increment();

        globalMutex.unlock();

        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }


    std::cout << " \nThank you for using LifeLike Engine :)" << std::endl;

    return 0;
}
