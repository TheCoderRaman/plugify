#pragma once

#include <array>
#include <plugify/vector.hpp>
#include <plugify/string.hpp>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#endif  // defined(__GNUC__)

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpedantic"
#pragma clang diagnostic ignored "-Wignored-qualifiers"
#pragma clang diagnostic ignored "-Wfloat-equal"
#endif  // defined(__clang__)

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4201 )
#endif // defined(_MSC_VER)

namespace plugify {
	extern "C" {
		/**
		* @struct Vector2
		* @brief Represents a 2D vector with x and y components.
		*/
		struct Vector2 {
			union {
				struct {
					float x; ///< x-component of the vector.
					float y; ///< y-component of the vector.
				};
				std::array<float, 2> data{}; ///< Array representation of the vector.
			};
		};

		/**
		* @struct Vector3
		* @brief Represents a 3D vector with x, y, and z components.
		*/
		struct Vector3 {
			union {
				struct {
					float x; ///< x-component of the vector.
					float y; ///< y-component of the vector.
					float z; ///< z-component of the vector.
				};
				std::array<float, 3> data{}; ///< Array representation of the vector.
			};
		};

		/**
		* @struct Vector4
		* @brief Represents a 4D vector with x, y, z, and w components.
		*/
		struct Vector4 {
			union {
				struct {
					float x; ///< x-component of the vector.
					float y; ///< y-component of the vector.
					float z; ///< z-component of the vector.
					float w; ///< w-component of the vector.
				};
				std::array<float, 4> data{}; ///< Array representation of the vector.
			};
		};

		/**
		* @struct Matrix4x4
		* @brief Represents a 4x4 matrix using Vector4 rows.
		*/
		struct Matrix4x4 {
			union {
				struct {
					float m00, m01, m02, m03;
					float m10, m11, m12, m13;
					float m20, m21, m22, m23;
					float m30, m31, m32, m33;
				};
				std::array<float, 16> data{}; ///< Array representation of the matrix.
			};
		};

		/**
		 * @struct Vector
		 * @brief Represents a plg::vector using C-compatible struct.
		 */
		struct Vector {
			[[maybe_unused]] uint8_t padding[sizeof(plg::vector<int>)]{};
		};

		/**
		 * @struct String
		 * @brief Represents a plg::string using C-compatible struct.
		 */
		struct String {
			[[maybe_unused]] uint8_t padding[sizeof(plg::string)]{};
		};
	}
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif  // defined(__clang__)

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif  // defined(__GNUC__)

#if defined(_MSC_VER)
#pragma warning( pop )
#endif // defined(_MSC_VER)
