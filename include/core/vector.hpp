////////////////////////////////////////////////////////////////////////
/// 
/// @file vector.hpp
///
/// @brief Vec<N, T> type
/// 
/// A basic vector type used 
/// throughout LifeLike Engine.
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
#ifndef VECTOR_HPP
#define VECTOR_HPP


////////////////////////////
// Headers
////////////////////////////
#include <ostream>


namespace lle {

    /// 
    /// @brief Forward declaration of Vec<N, T>, Vec<2, T>, Vec<3, T> and Vec<4, T>
    ///
    /// Used by _VecBase for accessing the 
    /// Vec's Components.
    ///
    /// @tparam N The size of the Vec
    ///
    /// @tparam T The type which should be stored
    /// 
    template<size_t N, typename T>
    struct Vec;
    
    /// 
    /// @brief Base for Vec<N, T>, Vec<2, T>, Vec<3, T> and Vec<4, T>
    ///
    /// Contains all functionality Vec should have.
    ///
    /// @tparam N The size of the derived Vec
    ///
    /// @tparam T The type which should be stored
    ///
    /// @note Member naming convention:
    /// - instance/static + public => PascalCase
    /// - instance + protected => M + camelCase + _
    /// - instance + private => m + PascalCase + _
    /// - static + protected => S + camelCase + _
    /// - static + private => s + PascalCase + _
    ///
    template<size_t N, typename T>
    struct _VecBase {
    private:

        /// 
        /// @brief Returns a pointer to the derived Vec 
        ///
        /// @return A Pointer to the derived Vec 
        /// struct instance
        ///
        /// @throws None
        ///
        [[nodiscard]] constexpr Vec<N, T>* mSelf_() noexcept;

    public:

        /// 
        /// @brief Operator overload for std::cout << Vec
        /// 
        /// Makes it possible to output the Vec 
        /// to the console using std::cout.
        /// 
        /// @tparam NVec The size of the Vec instance
        ///
        /// @tparam TVec The type which should be stored
        ///
        /// @param os Reference to a @ref std::ostream
        ///
        /// @param vec Reference to the Vec instance
        /// 
        /// @throws std::ios_base::failure
        /// If the stream is configured to throw on write failure
        ///
        /// @throws std::bad_alloc
        /// If memory allocation fails
        ///
        /// @throws std::runtime_error
        /// If another error occurs
        ///
        template<size_t NVec, typename TVec>
        constexpr friend std::ostream& operator<<(std::ostream &os, const Vec<NVec, TVec> &vec);

    };

    /// 
    /// @brief 2D vector
    ///
    /// A 2 dimensional vector 
    /// with functionality defined in 
    /// _VecBase.
    ///
    /// @tparam T The type which should be stored
    ///
    /// @note Member naming convention:
    /// - instance/static + public => PascalCase
    /// - instance + protected => M + camelCase + _
    /// - instance + private => m + PascalCase + _
    /// - static + protected => S + camelCase + _
    /// - static + private => s + PascalCase + _
    ///
    template<typename T>
    struct Vec<2, T> : public _VecBase<2, T> {

        union {

            struct {

                /// @var 
                /// The @ref Vec<2, T> "Vec"'s components
                T Components[2];
            };

            struct { 

                /// The @ref Vec<2, T> "Vec"'s X component 
                /// (alias to Components[0])
                T X; 

                /// The @ref Vec<2, T> "Vec"'s Y component 
                /// (alias to Components[1])
                T Y;
            };
        }; 

        /// 
        /// @brief Parameter pack constructor
        ///
        /// Constructs the @ref Vec<2, T> "Vec" from 
        /// a given number of arguments
        /// 
        /// @param comps Components which have to be of type 
        /// `T` and match the @ref Vec<2, T> "Vec"'s size
        /// 
        /// @throws std::bad_alloc
        /// If memory allocation fails
        ///
        /// @throws std::runtime_error
        /// If another error occurs
        ///
        template<typename... TArgs>
        Vec(const TArgs... comps);

        /// 
        /// @brief Default copy constructor
        ///
        /// @param other Reference to another 
        /// @ref Vec<2, T> "Vec"
        ///
        /// @throws std::bad_alloc
        /// If memory allocation fails
        ///
        /// @throws std::runtime_error
        /// If another error occurs
        ///
        Vec(const Vec &other) = default;

        /// 
        /// @brief Default destructor
        ///
        /// @throws None
        ///
        ~Vec() noexcept = default;
      
    };

    /// 
    /// @brief 3D vector
    ///
    /// A 3 dimensional vector 
    /// with functionality defined in 
    /// _VecBase.
    ///
    /// @tparam T The type which should be stored
    ///
    /// @note Member naming convention:
    /// - instance/static + public => PascalCase
    /// - instance + protected => M + camelCase + _
    /// - instance + private => m + PascalCase + _
    /// - static + protected => S + camelCase + _
    /// - static + private => s + PascalCase + _
    ///
    template<typename T>
    struct Vec<3, T> : public _VecBase<3, T> {

        union {

            struct {

                /// @var 
                /// The @ref Vec<3, T> "Vec"'s components
                T Components[3];
            };

            struct {

                /// The @ref Vec<3, T> "Vec"'s X component
                /// (alias to Components[0])
                T X;

                /// The @ref Vec<3, T> "Vec"'s Y component
                /// (alias to Components[1])
                T Y;

                /// The @ref Vec<3, T> "Vec"'s Z component
                /// (alias to Components[2])
                T Z;
            };
            struct {

                /// The @ref Vec<3, T> "Vec"'s Red channel component
                /// (alias to Components[0])
                T R;

                /// The @ref Vec<3, T> "Vec"'s Green channel component
                /// (alias to Components[1])
                T G;

                /// The @ref Vec<3, T> "Vec"'s Blue channel component
                /// (alias to Components[2])
                T B;
            };
        };

        /// 
        /// @brief Parameter pack constructor
        ///
        /// Constructs the @ref Vec<3, T> "Vec" from 
        /// a given number of arguments
        /// 
        /// @param comps Components which have to be of type 
        /// `T` and match the @ref Vec<3, T> "Vec"'s size
        /// 
        /// @throws std::bad_alloc
        /// If memory allocation fails
        ///
        /// @throws std::runtime_error
        /// If another error occurs
        ///
        template<typename... TArgs>
        Vec(const TArgs... comps);

        /// 
        /// @brief Default copy constructor
        ///
        /// @param other Reference to another 
        /// @ref Vec<3, T> "Vec"
        ///
        /// @throws std::bad_alloc
        /// If memory allocation fails
        ///
        /// @throws std::runtime_error
        /// If another error occurs
        ///
        Vec(const Vec &other) = default;

        /// 
        /// @brief Default destructor
        ///
        /// @throws None
        ///
        ~Vec() noexcept = default;

    };

    /// 
    /// @brief 4D vector
    ///
    /// A 4 dimensional vector 
    /// with functionality defined in 
    /// _VecBase.
    ///
    /// @tparam T The type which should be stored
    ///
    /// @note Member naming convention:
    /// - instance/static + public => PascalCase
    /// - instance + protected => M + camelCase + _
    /// - instance + private => m + PascalCase + _
    /// - static + protected => S + camelCase + _
    /// - static + private => s + PascalCase + _
    ///
    template<typename T>
    struct Vec<4, T> : public _VecBase<4, T> {

        union {

            struct {

                /// @var 
                /// The @ref Vec<4, T> "Vec"'s components
                T Components[4];
            };

            struct {

                /// The @ref Vec<4, T> "Vec"'s X component
                /// (alias to Components[0])
                T X;

                /// The @ref Vec<4, T> "Vec"'s Y component
                /// (alias to Components[1])
                T Y;

                /// The @ref Vec<4, T> "Vec"'s Z component
                /// (alias to Components[2])
                T Z;

                /// The @ref Vec<4, T> "Vec"'s W component
                /// (alias to Components[3])
                T W;
            };
            struct {

                /// The @ref Vec<4, T> "Vec"'s Red channel component
                /// (alias to Components[0])
                T R;

                /// The @ref Vec<4, T> "Vec"'s Green channel component
                /// (alias to Components[1])
                T G;

                /// The @ref Vec<4, T> "Vec"'s Blue channel component
                /// (alias to Components[2])
                T B;

                /// The @ref Vec<4, T> "Vec"'s Alpha channel component
                /// (alias to Components[3])
                T A;
            };
        }; 

        /// 
        /// @brief Parameter pack constructor
        ///
        /// Constructs the @ref Vec<4, T> "Vec" from 
        /// a given number of arguments
        /// 
        /// @param comps Components which have to be of type 
        /// `T` and match the @ref Vec<4, T> "Vec"'s size
        /// 
        /// @throws std::bad_alloc
        /// If memory allocation fails
        ///
        /// @throws std::runtime_error
        /// If another error occurs
        ///
        template<typename... TArgs>
        Vec(const TArgs... comps);

        /// 
        /// @brief Default copy constructor
        ///
        /// @param other Reference to another 
        /// @ref Vec<4, T> "Vec"
        ///
        /// @throws std::bad_alloc
        /// If memory allocation fails
        ///
        /// @throws std::runtime_error
        /// If another error occurs
        ///
        Vec(const Vec &other) = default;

        /// 
        /// @brief Default destructor
        ///
        /// @throws None
        ///
        ~Vec() noexcept = default;

    };

    /// 
    /// @brief Custom size vector
    ///
    /// A vector with a custom 
    /// but static amount of dimensions 
    /// and functionality defined in 
    /// _VecBase.
    /// 
    /// @tparam N The size of the @ref Vec<N, T> "Vec"
    ///
    /// @tparam T The type which should be stored
    ///
    /// @note Member naming convention:
    /// - instance/static + public => PascalCase
    /// - instance + protected => M + camelCase + _
    /// - instance + private => m + PascalCase + _
    /// - static + protected => S + camelCase + _
    /// - static + private => s + PascalCase + _
    ///
    template<size_t N, typename T>
    requires(N > 4)
    struct Vec<N, T> : public _VecBase<N, T> {

        /// @var The @ref Vec<N, T> "Vec"'s components
        T Components[N];

        /// 
        /// @brief Parameter pack constructor
        ///
        /// Constructs the @ref Vec<N, T> "Vec" from 
        /// a given number of arguments
        /// 
        /// @param comps Components which have to be of type 
        /// `T` and match the @ref Vec<N, T> "Vec"'s size
        /// 
        /// @throws std::bad_alloc
        /// If memory allocation fails
        ///
        /// @throws std::runtime_error
        /// If another error occurs
        ///
        template<typename... TArgs>
        Vec(const TArgs... comps);

        /// 
        /// @brief Default copy constructor
        ///
        /// @param other Reference to another 
        /// @ref Vec<N, T> "Vec"
        ///
        /// @throws std::bad_alloc
        /// If memory allocation fails
        ///
        /// @throws std::runtime_error
        /// If another error occurs
        ///
        Vec(const Vec &other) = default;

        /// 
        /// @brief Default destructor
        ///
        /// @throws None
        ///
        ~Vec() noexcept = default;

    };

    template<typename T>
    using Vec2 = Vec<2, T>; ///< 2D Vec alias

    template<typename T>
    using Vec3 = Vec<3, T>; ///< 3D Vec alias

    template<typename T>
    using Vec4 = Vec<4, T>; ///< 4D Vec alias
    
} // namespace lle

#include "vector.inl"

#endif // !VECTOR_HPP
