////////////////////////////////////////////////////////////////////////
/// 
/// @file types.hpp
///
/// @brief Fundamental types and Maros
/// 
/// - Macros for detecting the 
/// Operating System, Compiler 
/// and wether Debug is on
/// - type aliases for 
/// fundamental types
/// - Basic costom types
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
////////////////////////////////////////////////////////////////////////

// Header include guard
#ifndef TYPES_HPP
#define TYPES_HPP

////////////////////////////
// Headers
////////////////////////////
#include <cstddef>


// Look for Window API
#if defined(_WIN32)
    #define OS_WINDOWS
    // Check if Windows is 64 bit
    #if defined(_WIN64)
        #define OS_WINDOWS_64 1 ///< OS is 64 Bit Windows
        #define LLP64 1         ///< Data model is LLP64
    #else
        #define OS_WINDOWS_32 1 ///< OS is 32 Bit Windows
        #define ILP32 1         ///< Data model is ILP32
    #endif

// Look for Linux API
#elif defined(__linux__) || defined(__linux)
    #define OS_LINUX
    // Check if Linux is 64 bit
    #if defined(__LP64__) || defined(__x86_64__) 
        #define OS_LINUX_64 1   ///< OS is 64 Bit Linux
        #define LP64 1          ///< Data model is LP64
    #else
        #define OS_LINUX_32 1   ///< OS is 32 Bit Linux
        #define ILP32 1         ///< Data model is ILP32
    #endif

// Error if the operating system is not Windows or Linux
#else
    #error "Operating system not (yet) supported"
#endif


// Look for Clang
#if defined(__clang__)
    #define COMPILER_CLANG 1    ///< Compiler is `Clang`
    #define COMPILER "Clang"    ///< Compiler is `Clang`

// Look for GCC
#elif defined(__GNUC__)
    #define COMPILER_GCC 1      ///< Compiler is `GCC`
    #define COMPILER "gcc"      ///< Compiler is `GCC`

// Look for MSVC
#elif defined(_MSC_VER)
    #define COMPILER_MSVC 1     ///< Compiler is `MSVC`
    #define COMPILER "MSVC"     ///< Compiler is `MSVC`

// Other compilers
#else
    #define COMPILER_UNKNOWN    ///< Compiler is Unknown
    #define COMPILER "Unknown"  ///< Compiler is Unknown
#endif

// Check if debug is enabled
#ifndef NDEBUG
#define DEBUG true              ///< Debug mode is on
#else
#define DEBUG false             ///< Debug mode is off
#endif

/// 
/// @def DEBUG_BLOCK(CODE)
///
/// @brief Code executed only in Debug mode
///
/// @param CODE Code block that is to 
/// be executed
///
#if (DEBUG)
#define DEBUG_BLOCK(CODE)       \
        do {                    \
            CODE                \
        } while(0);             
#else
#define DEBUG_BLOCK(CODE)
#endif


// Integral types
using i8_t  =       char;               ///< 8 Bit signed integer
using u8_t  =       unsigned char;      ///< 8 Bit unsigned integer
using i16_t =       short;              ///< 16 Bit signed integer
using u16_t =       unsigned short;     ///< 16 Bit unsigned integer
#ifdef LP32
using i32_t =       long;               ///< 32 Bit signed integer
using u32_t =       unsigned long;      ///< 32 Bit unsigned integer
#else
using i32_t =       int;                ///< 32 Bit signed integer
using u32_t =       unsigned int;       ///< 32 Bit unsigned integer
#endif
using i64_t =       long long;          ///< 64 Bit signed integer
using u64_t =       unsigned long long; ///< 64 Bit unsigned integer

using size_t=       std::size_t;        ///< Max size of an object of any type

// Floating point types
using f32_t =       float;              ///< 32 Bit float
using f64_t =       double;             ///< 64 Bit float

// Bool type
using b8_t  =       bool;               ///< 8 Bit boolean

// C-style string type
using cstr_t=       const char*;        ///< C-style string

#endif // !TYPES_HPP
