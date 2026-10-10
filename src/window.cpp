#include "window.hpp"
#include "types.hpp"
#include "result.hpp"
#include "vector.hpp"
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"
#include "SDL3/SDL.h"
#include "SDL3/SDL_vulkan.h"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <stdexcept>
#include <string>


u64_t currentTime;
u64_t lastTime;

using namespace lle;

Result Window::mInitSDL3_() const noexcept {

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) return lle::LFS_ERROR;

    return Result::LFS_SUCCESS;
}

Result Window::mCreateWindow_() noexcept {
    
    mSDL3window_ = SDL_CreateWindow(mTitle_.c_str(), mSize_.X, mSize_.Y, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);

    if (mSDL3window_ == nullptr) return lle::LFS_ERROR;

    mIsOpen_ = true;

    SDL_GetWindowSize(mSDL3window_, &mSize_.X, &mSize_.Y);

    return Result::LFS_SUCCESS;
}

void Window::mInitVkHandle_() {
    if (Result res = mVkHandle_.AutoInit(); res != Result::LFS_SUCCESS) throw ResultEvaluation(res);
    if (Result res = mVkHandle_.CreatePipeline("Circle" + std::to_string(ShaderTypes::FILL), "./circle.vert.spv", "./circle-filled.frag.spv"); res != Result::LFS_SUCCESS) throw ResultEvaluation(res);
    mCircleShaders_.emplace_back("Circle" + std::to_string(ShaderTypes::FILL));

    if (Result res = mVkHandle_.CreatePipeline("Circle" + std::to_string(ShaderTypes::GRADIENT), "./circle.vert.spv", "./circle-gradient.frag.spv"); res != Result::LFS_SUCCESS) throw ResultEvaluation(res);
    mCircleShaders_.emplace_back("Circle" + std::to_string(ShaderTypes::GRADIENT));

    if (Result res = mVkHandle_.CreatePipeline("Circle" + std::to_string(ShaderTypes::OUTLINE), "./circle.vert.spv", "./circle-outline.frag.spv"); res != Result::LFS_SUCCESS) throw ResultEvaluation(res);
    mCircleShaders_.emplace_back("Circle" + std::to_string(ShaderTypes::OUTLINE));

    if (Result res = mVkHandle_.CreatePipeline("Rect" + std::to_string(ShaderTypes::FILL), "./rect.vert.spv", "./rect-filled.frag.spv"); res != Result::LFS_SUCCESS) throw ResultEvaluation(res);
mCircleShaders_.emplace_back("Rect" + std::to_string(ShaderTypes::FILL));

    if (Result res = mVkHandle_.CreatePipeline("Rect" + std::to_string(ShaderTypes::GRADIENT), "./rect.vert.spv", "./rect-gradient.frag.spv"); res != Result::LFS_SUCCESS) throw ResultEvaluation(res);
    mCircleShaders_.emplace_back("Rect" + std::to_string(ShaderTypes::GRADIENT));

    if (Result res = mVkHandle_.CreatePipeline("Rect" + std::to_string(ShaderTypes::OUTLINE), "./rect.vert.spv", "./rect-outline.frag.spv"); res != Result::LFS_SUCCESS) throw ResultEvaluation(res);
mCircleShaders_.emplace_back("Rect" + std::to_string(ShaderTypes::OUTLINE));


    mVkHandle_.CreateCommand("Main");

    lastTime = SDL_GetTicks();
}

Window::Window() 
    : mSDL3window_(nullptr),
      mIsOpen_(false),
      mSize_({100, 100}),
      mTitle_("Unspecified"),
      mVkHandle_() {

    if (mInitSDL3_() != Result::LFS_SUCCESS) throw std::runtime_error("SDL initialisation failed");
    if (mCreateWindow_() != Result::LFS_SUCCESS) throw std::runtime_error("Window creation failed");
    mInitVkHandle_();
}

Window::Window(const Vec2<i32_t> size, const cstr_t title)
    : mSDL3window_(nullptr),
      mIsOpen_(false),
      mSize_(size),
      mTitle_(title),
      mVkHandle_(title, &mSDL3window_) {

    if (mInitSDL3_() != Result::LFS_SUCCESS) throw std::runtime_error("GLFW initialisation failed");
    if (mCreateWindow_() != Result::LFS_SUCCESS) throw std::runtime_error("Window creation failed");
    mInitVkHandle_();
}

Window::~Window() noexcept {
    mVkHandle_.Quit();
    SDL_DestroyWindow(mSDL3window_);
    SDL_Quit();
}

void Window::Create(const Window &other) {
    mSize_ = other.mSize_;
    mTitle_ = other.mTitle_;
    if (mCreateWindow_() != Result::LFS_SUCCESS) throw std::runtime_error("Window creation failed");
    mInitVkHandle_();
}

void Window::Create(const Vec2<i32_t> size, const cstr_t title) {
    mSize_ = size;
    mTitle_ = title;
    if (mCreateWindow_() != Result::LFS_SUCCESS) throw std::runtime_error("Window creation failed");
    mInitVkHandle_();
}

void Window::Close() noexcept {
    mIsOpen_ = false;
}

void Window::EventHandling(const SDL_Event &e) noexcept {
    switch (e.type) {
        case SDL_EVENT_KEY_DOWN:
            if (e.key.key == SDLK_F11) {
                if (SDL_GetWindowFlags(mSDL3window_) & SDL_WINDOW_FULLSCREEN) {
                    SDL_SetWindowFullscreen(mSDL3window_, false);
                } else {
                    SDL_SetWindowFullscreen(mSDL3window_, true);
                }
            }
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            mVkHandle_.Resized();
            SDL_GetWindowSize(mSDL3window_, &mSize_.X, &mSize_.Y);
            break;
    }

}

f64_t Window::DeltaTime() noexcept {
    return mDeltaTime_;
}

SDL_Window *Window::GetWindow() noexcept {
    return mSDL3window_;
}

lle::Vec2<i32_t> Window::GetSize() noexcept {
    return mSize_;
}

lle::Vec2<f32_t> Window::GetMousePos() noexcept {
    lle::Vec2<f32_t> pos{0.0, 0.0};
    SDL_GetMouseState(&pos.X, &pos.Y);
    return pos;
}

void Window::Update() noexcept {

    currentTime = SDL_GetTicksNS();
    mDeltaTime_ = (currentTime - lastTime) / 1'000'000'000.0;
    lastTime = currentTime;

    if (!(SDL_GetWindowFlags(mSDL3window_) & SDL_WINDOW_MINIMIZED)) {
        mVkHandle_.StartFrame();

        mVkHandle_.RestartCommand("Main");

        for (auto shaderType = static_cast<u32_t>(ShaderTypes::SHADER_FIRST); shaderType < ShaderTypes::SHADER_COUNT; shaderType++) {
            auto &circles = mCircles_[shaderType];
            std::string circleBuffName = "Circle" + std::to_string(shaderType);
            if (circles.size() > 0) {
                mVkHandle_.WriteToBuffer(circleBuffName, circles.data(), sizeof(Window::CircleData) * circles.size());
                mVkHandle_.AddToCommand("Main", "Circle" + std::to_string(shaderType), circleBuffName, 6, circles.size());
                circles.clear();
            }

            auto &rects = mRects_[shaderType];
            std::string rectBuffName = "Rect" + std::to_string(shaderType);

            if (rects.size() > 0) {
                mVkHandle_.WriteToBuffer(rectBuffName, rects.data(), sizeof(Window::RectData) * rects.size());
                mVkHandle_.AddToCommand("Main", "Rect" + std::to_string(shaderType), rectBuffName, 6, rects.size());
                rects.clear();
            }
        }

        mVkHandle_.ExecuteCommand("Main");

        mVkHandle_.EndFrame();
    }
}

bool Window::isOpen() const noexcept {
    return mIsOpen_;
}

void Window::DrawCircle(lle::Vec2<f32_t> pos, f32_t rad, lle::Vec3<u8_t> col, ShaderTypes shaderType) {

    auto &circles = mCircles_[shaderType];

    circles.emplace_back(glm::vec2(pos.X / mSize_.X, pos.Y / mSize_.Y), rad / mSize_.Y, glm::vec3(col.R / 255.0, col.G / 255.0, col.B / 255.0));

    std::string buffName = "Circle" + std::to_string(shaderType);

    if (sizeof(Window::CircleData) * circles.size() > mVkHandle_.GetDataBufferSize(buffName)) {
        mVkHandle_.DestroyDataBuffer(buffName);
        if (Result res = mVkHandle_.CreateDataBuffer(buffName, sizeof(Window::CircleData) * circles.size()); res != Result::LFS_SUCCESS) throw lle::ResultEvaluation(res);
    }
}

void Window::DrawRectangle(lle::Vec2<f32_t> pos, lle::Vec2<f32_t> size, lle::Vec3<u8_t> color, ShaderTypes shaderType) {

    auto &rects = mRects_[shaderType];

    rects.emplace_back(glm::vec2(pos.X / mSize_.X, pos.Y / mSize_.Y), glm::vec2(size.X / mSize_.Y, size.Y / mSize_.Y), glm::vec3(color.R / 255.0, color.G / 255.0, color.B / 255.0));

    std::string buffName = "Rect" + std::to_string(shaderType);

    if (sizeof(Window::RectData) * rects.size() > mVkHandle_.GetDataBufferSize(buffName)) {
        mVkHandle_.DestroyDataBuffer(buffName);
        if (Result res = mVkHandle_.CreateDataBuffer(buffName, sizeof(Window::RectData) * rects.size()); res != Result::LFS_SUCCESS) throw lle::ResultEvaluation(res);
    }
}
