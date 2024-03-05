#pragma once

#include <array>

namespace plugify {
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

		/**
		 * @brief Assignment operator for Vector2.
		 * @param other The Vector2 to assign values from.
		 * @return Reference to the modified Vector2.
		 */
		Vector2& operator=(const Vector2& other) = default;
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

		/**
		 * @brief Assignment operator for Vector3.
		 * @param other The Vector3 to assign values from.
		 * @return Reference to the modified Vector3.
		 */
		Vector3& operator=(const Vector3& other) = default;
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

		/**
		 * @brief Assignment operator for Vector4.
		 * @param other The Vector4 to assign values from.
		 * @return Reference to the modified Vector4.
		 */
		Vector4& operator=(const Vector4& other) = default;
	};

	/**
	 * @struct Matrix4x4
	 * @brief Represents a 4x4 matrix using Vector4 rows.
	 */
	struct Matrix4x4 {
		union {
			struct {
				float m11, m12, m13, m14;
				float m21, m22, m23, m24;
				float m31, m32, m33, m34;
				float m41, m42, m43, m44;
			};
			Vector4 rows[4]; ///< Rows of the matrix represented by Vector4.
			std::array<float, 16> data; ///< Array representation of the matrix.
		};

		/**
		 * @brief Default constructor. Initializes the matrix to the identity matrix.
		 */
		Matrix4x4() : m11{1.0f}, m12{0.0f}, m13{0.0f}, m14{0.0f},
					  m21{0.0f}, m22{1.0f}, m23{0.0f}, m24{0.0f},
					  m31{0.0f}, m32{0.0f}, m33{1.0f}, m34{0.0f},
					  m41{0.0f}, m42{0.0f}, m43{0.0f}, m44{1.0f} {}

		/**
		 * @brief Assignment operator for Matrix4x4.
		 * @param other The Matrix4x4 to assign values from.
		 * @return Reference to the modified Matrix4x4.
		 */
		Matrix4x4& operator=(const Matrix4x4& other) = default;
	};

	/**
	 * @struct Matrix3x2
	 * @brief Represents a 3x2 matrix using Vector3 rows.
	 */
	struct Matrix3x2 {
		union {
			struct {
				float m11, m12;
				float m21, m22;
				float m31, m32;
			};
			Vector2 rows[3]; ///< Rows of the matrix represented by Vector3.
			std::array<float, 6> data; ///< Array representation of the matrix.
		};

		/**
		 * @brief Default constructor. Initializes the matrix to the identity matrix.
		 */
		Matrix3x2() : m11{1.0f}, m12{0.0f},
					  m21{0.0f}, m22{1.0f},
					  m31{0.0f}, m32{0.0f} {}

		/**
		 * @brief Assignment operator for Matrix3x2.
		 * @param other The Matrix3x2 to assign values from.
		 * @return Reference to the modified Matrix3x2.
		 */
		Matrix3x2& operator=(const Matrix3x2& other) = default;
	};
}