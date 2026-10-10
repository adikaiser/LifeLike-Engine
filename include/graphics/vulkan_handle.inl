////////////////////////////////////////////////////////////////////////
//
// LifeLike Engine
// 
// MIT License
// 
// Copyright (c) 2025 Jonathan Goth
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
////////////////////////////////////////////////////////////////////////


////////////////////////////
// Headers
////////////////////////////
#include "vulkan_handle.hpp"
#include "types.hpp"

#include <iostream>


namespace lle {

    constexpr Version::Version(const u16_t major, const u16_t minor, const u16_t patch)
        : mMajor_(major),
          mMinor_(minor),
          mPatch_(patch) {
        if (major > 1023) throw std::runtime_error("Major mVersion must be in the range [0, 1023]");
        if (minor > 1023) throw std::runtime_error("Minor mVersion must be in the range [0, 1023]");
        if (patch > 1023) throw std::runtime_error("Patch mVersion must be in the range [0, 4095]");
    }

    constexpr u16_t Version::Major() const noexcept { return mMajor_; }

    constexpr u16_t Version::Minor() const noexcept { return mMinor_; }

    constexpr u16_t Version::Patch() const noexcept { return mPatch_; }

    constexpr u32_t Version::ToVulkan() const noexcept { return VK_MAKE_VERSION(mMajor_, mMinor_, mPatch_); }
    
    inline
    std::ostream& operator<<(std::ostream &os, const Version inst) {
            return os << inst.mMajor_ << '.' << inst.mMinor_ << '.' << inst.mPatch_;
        }
} // namespace lle
