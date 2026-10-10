////////////////////////////////////////////////////////////////////////
/// 
/// @file window.hpp
///
/// @brief Everything needed to create a window
/// 
/// Contains everything needed for 
/// instantiating and handling
/// an SDL window.
///
/// # LifeLike Engine
/// 
/// MIT License
/// 
/// Copyright (c) 2025 Jonathan Goth
/// 
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in all
/// copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
/// SOFTWARE.
/// 
////////////////////////////////////////////////////////////////////////

// Header include guard
#ifndef WIONDW_HPP
#define WIONDW_HPP


////////////////////////////
// Headers
////////////////////////////
#include "types.hpp"
#include "result.hpp"
#include "vector.hpp"
#include "vulkan_handle.hpp"

#include "SDL3/SDL.h"
#include "glm/glm.hpp"

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_video.h>
#include <glm/ext/vector_float3.hpp>
#include <string>


using std::string;
                                                        
///
/// @brief LifeLike Engine
/// 
/// The main namespace for the LifeLike Engine 
/// containing every class, struct and enum
///
namespace lle {

    ///
    /// @brief Available shader types
    ///
    enum ShaderTypes {
        SHADER_DEFAULT = 0,
        SHADER_FIRST = 0,
        FILL = 0,
        GRADIENT = 1,
        OUTLINE = 2,
        SHADER_COUNT = 3,
    };

    /// 
    /// @brief Creates and manages a window
    ///
    /// Creates and manages a Window with SDL
    /// and Vulkan. 
    /// 
    /// @note Member naming convention:
    /// - instance/static + public => PascalCase
    /// - instance + protected => M + camelCase + _
    /// - instance + private => m + PascalCase + _
    /// - static + protected => S + camelCase + _
    /// - static + private => s + PascalCase + _
    ///
    class Window {

        /// Pointer to an SDL3 window
        SDL_Window *mSDL3window_;

        /// The deltatime (time since last frame)
        f64_t mDeltaTime_;

        /// Tracks wether the window is open
        bool mIsOpen_;

        /// Size of the Window
        Vec2<i32_t> mSize_;

        /// Title of the Window
        string mTitle_;

        /// Vulkan Handle instance
        VulkanHandle mVkHandle_;

        /// Keeps track of all available shaders
        std::vector<std::string> mCircleShaders_;

        ///
        /// @brief Data of a single circle
        ///
        /// Used by the shader to draw one
        /// circle
        ///
        struct CircleData {

            /// The circle's position on screen from [0, 0] (upper left corner) to [1, 1] (lower right corner)
            alignas(4) glm::vec2 pos;

            /// The circle's radius as a fraction of the screen height
            alignas(4) float radius;

            /// The circle's color form [0.0, 0.0, 0.0] to [1.0, 1.0, 1.0]
            alignas(4) glm::vec3 color;
        };

        /// Circles that will be drawn on the next Update() call
        std::vector<CircleData> mCircles_[ShaderTypes::SHADER_COUNT];

        ///
        /// @brief Data of a single rectangle
        ///
        /// Used by the shader to draw one rectangle
        ///
        struct RectData {
            /// The rect's upper left corner position on screen from [0, 0] (upper left corner) to [1, 1] (lower right corner)
            alignas(4) glm::vec2 pos;

            /// The rect's width and height ad a fraction of the screens height
            alignas(4) glm::vec2 size;

            /// The rect's color from [0.0, 0.0, 0.0] to [1.0, 1.0, 1.0]
            alignas(4) glm::vec3 color;
        };

        /// Rectangles that will be drawn on the next Update() call
        std::vector<RectData> mRects_[ShaderTypes::SHADER_COUNT];

        /// 
        /// @brief Initializes SDL3
        ///
        /// @return A Result which can be SUCCESS
        /// or an error valueERROR
        ///
        /// @throws None
        ///
        Result mInitSDL3_() const noexcept;

        /// 
        /// @brief Creates an SDL window
        ///
        /// Creates an SDL window and stores
        /// it in `mSDL3window_`
        ///
        /// @return A Result which can be SUCCESS
        /// or an error valueERROR
        ///
        /// @throws None
        ///
        Result mCreateWindow_() noexcept;

        /// 
        /// @brief Initializes the Vulkan Handle
        ///
        /// Initializes `mVkHandle_` to be used 
        /// by the Window
        ///
        /// @throws Result if vulkan fails
        ///
        void mInitVkHandle_();

    public:

        ///
        /// @brief Default Constructor
        ///
        /// Creates a new Window with 
        /// the following default parameters:
        /// - size = `[100, 100]`
        /// - title = `Unspecified`
        /// 
        /// @throws std::runtime_error A runtime 
        /// error if any of the initialisation 
        /// fails
        ///
        Window();

        ///
        /// @brief Custom Constructor
        ///
        /// Creates a new Window according 
        /// to the given values.
        ///
        /// @param size A `lle::Vec2<T>` for the Window's 
        /// size: @c size.X -> width, @c size.Y -> height
        /// 
        /// @param title A C-style string for the Title 
        /// of the Window
        /// 
        /// @throws std::runtime_error A runtime 
        /// error if any of the initialisation 
        /// fails
        ///
        Window(const Vec2<i32_t> size, const cstr_t title);

        ///
        /// @brief Copy Constructor
        ///
        /// Creates a new Window from 
        /// a given Window.
        ///
        /// @param other Another Window instance 
        /// 
        /// @throws std::runtime_error A runtime 
        /// error if any of the initialisation 
        /// fails
        /// 
        Window(const Window &other) = delete;

        ///
        /// @brief Default Destructor
        ///
        /// Destroys the Window and all 
        /// members.
        ///
        /// @throws None
        ///
        ~Window() noexcept;

        ///
        /// @brief Recreates the Window
        /// 
        /// @param size A lle::Vec2 for the Window's 
        /// size: @p size.X -> width, @p size.Y -> height
        /// 
        /// @param title A @ref cstr_t "C-style string" for the Title 
        /// of the window
        /// 
        /// Example usage:
        /// @code
        /// lle::Window window();
        /// window.Create({800, 600}, "LifeLike Engine")
        /// @endcode
        ///
        /// @throws std::runtime_error A runtime 
        /// error if the Window recreation fails
        ///
        void Create(const Vec2<i32_t> size, const cstr_t title);

        ///
        /// @brief Recreates a Window
        /// 
        /// Recreates the Window with 
        /// another one.
        ///
        /// @throws std::runtime_error A runtime 
        /// error if the Window recreation fails
        ///
        void Create(const Window &other);

        ///
        /// @brief Closes the Window
        ///
        /// @throws None
        ///
        void Close() noexcept;

        ///
        /// @brief internal event-dependent functionality
        ///
        /// @param e the SDL event to be evaluated
        ///
        /// @throws None
        ///
        void EventHandling(const SDL_Event &e) noexcept;

        ///
        /// @brief Returns the Delta Time
        ///
        /// Returns the time that has passed since 
        /// the last frame.
        ///
        /// @throws None
        ///
        f64_t DeltaTime() noexcept;

        ///
        /// @brief Returns the SDL3 window pointer
        ///
        /// @throws None
        ///
        SDL_Window *GetWindow() noexcept;

        ///
        /// @brief Returns the window's size
        ///
        /// @throws None
        ///
        lle::Vec2<i32_t> GetSize() noexcept;

        ///
        /// @brief Returns the Mouse's position 
        /// in a range from 0.0 to 1.0.
        ///
        /// @throws None
        ///
        lle::Vec2<f32_t> GetMousePos() noexcept;

        ///
        /// @brief Displays the current frame
        ///
        /// Updates the Window to display everything 
        /// in the current frame.
        ///
        /// @throws None
        ///
        void Update() noexcept;

        ///
        /// @brief Returns wether the window 
        /// is open
        /// 
        /// @return b8_t A boolean value which 
        /// is set to @c true if the Window is 
        /// open and @c false if not
        ///
        /// @throws None
        ///
        b8_t isOpen() const noexcept;

        ///
        /// @brief Draw a circle on screen
        ///
        /// Adds a circle to the
        /// list of circles drawn at the
        /// next Update() call
        /// 
        /// @throws lle::Result if there is an ERROR
        ///
        void DrawCircle(lle::Vec2<f32_t> pos, f32_t rad, lle::Vec3<u8_t> col, ShaderTypes shaderType = SHADER_DEFAULT);


        ///
        /// @brief Draw a rectangle on screen
        ///
        /// Adds a rectangle to the
        /// list of rectangles drawn at the
        /// next Update() call
        /// 
        /// @throws lle::Result if there is an ERROR
        ///
        void DrawRectangle(lle::Vec2<f32_t> pos, lle::Vec2<f32_t> size, lle::Vec3<u8_t> color, ShaderTypes shaderType = SHADER_DEFAULT);

    };

} // lle

#endif // !WIONDW_HPP
