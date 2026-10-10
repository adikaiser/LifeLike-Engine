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
#include "vector.hpp"


namespace lle {

    template<size_t N, typename T>
    constexpr inline Vec<N, T>* _VecBase<N, T>::mSelf_() noexcept {
            return static_cast<Vec<N, T>*>(this);
    }

    template<size_t NVec, typename TVec>
    constexpr inline std::ostream& operator<<(std::ostream &os, const Vec<NVec, TVec> &vec) {
        os << '[';
        for (size_t i = 0; i < NVec - 1; ++i) {
            os << vec.Components[i] << ", ";
        }
        os << vec.Components[NVec - 1] << ']';
        return os;
    }



    template<size_t N, typename T>
    requires(N > 4)
    template<typename... TArgs>
    Vec<N, T>::Vec(const TArgs... comps) : Components( comps... ) {
        static_assert(sizeof...(TArgs) == N, "Number of components has to match the Vec's size");
    }

    template<typename T>
    template<typename... TArgs>
    Vec<2, T>::Vec(const TArgs... comps) : Components( comps... ) {
        static_assert(sizeof...(TArgs) == 2, "Number of components has to match the Vec's size");
    }

    template<typename T>
    template<typename... TArgs>
    Vec<3, T>::Vec(const TArgs... comps) : Components( comps... ) {
        static_assert(sizeof...(TArgs) == 3, "Number of components has to match the Vec's size");
    }

    template<typename T>
    template<typename... TArgs>
    Vec<4, T>::Vec(const TArgs... comps) : Components( comps... ) {
        static_assert(sizeof...(TArgs) == 4, "Number of components has to match the Vec's size");
    }

} // namespace lle
