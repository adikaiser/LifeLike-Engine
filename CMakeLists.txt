cmake_minimum_required(VERSION 3.10)
project(LifeLike-Engine)


set(TARGET_NAME ${PROJECT_NAME})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(SRC ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(ASSETS ${CMAKE_CURRENT_SOURCE_DIR}/assets)
set(THIRD_PARTY ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# --------------------------------------------------------------------
# Third-party
# --------------------------------------------------------------------

find_package(Vulkan REQUIRED)

set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

add_subdirectory(${THIRD_PARTY}/SDL)
add_subdirectory(${THIRD_PARTY}/json)
add_subdirectory(${THIRD_PARTY}/glm)

#[[add_library(nlohmann_json INTERFACE)
target_include_directories(nlohmann_json INTERFACE
    ${THIRD_PARTY}/json/include
)]]

add_library(inicpp INTERFACE)
target_include_directories(inicpp INTERFACE
    ${THIRD_PARTY}/inicpp
)

#add_library(ctpl INTERFACE)
#target_include_directories(ctpl INTERFACE
#    ${THIRD_PARTY}/CTPL
#)

# --------------------------------------------------------------------
# Header-only library
# --------------------------------------------------------------------

add_library(core INTERFACE)

target_include_directories(core INTERFACE
    ${INC}/core
)

# --------------------------------------------------------------------
# Graphics
# --------------------------------------------------------------------

add_library(graphics STATIC
    ${SRC}/window.cpp
    ${SRC}/vulkan_handle.cpp
)

target_include_directories(graphics PUBLIC ${INC}/graphics)

target_link_libraries(graphics PUBLIC
    core
    Vulkan::Vulkan
    SDL3::SDL3
    glm::glm-header-only
)

# --------------------------------------------------------------------
# Assets
# --------------------------------------------------------------------

add_library(assets
    ${ASSETS}/compound_templates.cpp
)

target_include_directories(assets PUBLIC
    ${INC}/assets
    ${INC}/sim
)

target_link_libraries(assets PUBLIC
    core
    graphics
    nlohmann_json::nlohmann_json)

# --------------------------------------------------------------------
# Executable
# --------------------------------------------------------------------

add_executable(${TARGET_NAME}
    src/main.cpp
)

target_include_directories(${TARGET_NAME} PRIVATE
    ${INC}/assets
    ${INC}/sim
)

target_link_libraries(${TARGET_NAME} PRIVATE
    graphics
    assets
    #ctpl
    nlohmann_json::nlohmann_json
    inicpp
)

target_link_options(${TARGET_NAME} PRIVATE
    -static-libstdc++
    -static-libgcc
)

# --------------------------------------------------------------------
# Shaders
# --------------------------------------------------------------------

set(SHADER_TARGET_DIR "$<TARGET_FILE_DIR:${TARGET_NAME}>")
set(SHADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADER_COMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders_compiled")
file(MAKE_DIRECTORY "${SHADER_COMP_DIR}")

set(SHADER_FILES
    "${SHADERS_DIR}/circle.vert"
    "${SHADERS_DIR}/circle-filled.frag"
    "${SHADERS_DIR}/circle-gradient.frag"
    "${SHADERS_DIR}/circle-outline.frag"
    "${SHADERS_DIR}/rect.vert"
    "${SHADERS_DIR}/rect-filled.frag"
    "${SHADERS_DIR}/rect-gradient.frag"
    "${SHADERS_DIR}/rect-outline.frag"
)

set(COMPILED_SHADERS)

foreach(SHADER_FILE ${SHADER_FILES})
    cmake_path(GET SHADER_FILE FILENAME FILE)

    set(SPV_FILE "${SHADER_COMP_DIR}/${FILE}.spv")

    add_custom_command(
        OUTPUT "${SPV_FILE}"
        COMMAND "${CMAKE_COMMAND}"
            "-DSHADER_FILE=${SHADER_FILE}"
            "-DTARGET_DIR=${SHADER_COMP_DIR}"
            "-P ${SHADERS_DIR}/compile.cmake"
        DEPENDS
            "${SHADER_FILE}"
        COMMENT "Compiling Vulkan shader [${FILE}]"
        VERBATIM
    )

    list(APPEND COMPILED_SHADERS "${SPV_FILE}")

endforeach()

add_custom_target(Vulkan-shaders ALL
    DEPENDS ${COMPILED_SHADERS}
)

add_custom_command(TARGET Vulkan-shaders POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
    "-E" "copy_if_different"
    ${COMPILED_SHADERS}
    "${SHADER_TARGET_DIR}"
)
