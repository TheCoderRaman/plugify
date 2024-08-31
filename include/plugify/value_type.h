#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <optional>
#include <string_view>

namespace plugify {
	/**
	 * @enum ValueType
	 * @brief Enumerates possible types of values in the reflection system.
	 */
	enum class ValueType : uint8_t {
		Invalid,

		// C types
		Void,
		Bool,
		Char8,
		Char16,
		Int8,
		Int16,
		Int32,
		Int64,
		UInt8,
		UInt16,
		UInt32,
		UInt64,
		Pointer,
		Float,
		Double,

		Function,

		// Objects
		String,

		ArrayBool,
		ArrayChar8,
		ArrayChar16,
		ArrayInt8,
		ArrayInt16,
		ArrayInt32,
		ArrayInt64,
		ArrayUInt8,
		ArrayUInt16,
		ArrayUInt32,
		ArrayUInt64,
		ArrayPointer,
		ArrayFloat,
		ArrayDouble,
		ArrayString,

		// Structs
		Vector2,
		Vector3,
		Vector4,

		Matrix4x4,
		//Matrix2x2,
		//Matrix2x3,
		//Matrix2x4,
		//Matrix3x2,
		//Matrix3x3,
		//Matrix3x4,
		//Matrix4x2,
		//Matrix4x3,

		//! Helpers

		_BaseStart = Void,
		_BaseEnd = Function,

		_FloatStart = Float,
		_FloatEnd = Double,

		_ObjectStart = String,
		_ObjectEnd = ArrayString,

		_ArrayStart = ArrayBool,
		_ArrayEnd = ArrayString,

		_StructStart = Vector2,
		_StructEnd = Matrix4x4,

		// First struct which return as 1st parameter
#if _WIN32
		_HiddenParamStart = Vector3,
#else
		_HiddenParamStart = Matrix4x4,
#endif
		_LastAssigned = Matrix4x4,
	};

	/**
	 * @brief Namespace containing string representations of ValueType enum values.
	 */
	namespace ValueName {
		static constexpr std::string_view Void = "void";
		static constexpr std::string_view Bool = "bool";
		static constexpr std::string_view Char8 = "char8";
		static constexpr std::string_view Char16 = "char16";
		static constexpr std::string_view Int8 = "int8";
		static constexpr std::string_view Int16 = "int16";
		static constexpr std::string_view Int32 = "int32";
		static constexpr std::string_view Int64 = "int64";
		static constexpr std::string_view UInt8 = "uint8";
		static constexpr std::string_view UInt16 = "uint16";
		static constexpr std::string_view UInt32 = "uint32";
		static constexpr std::string_view UInt64 = "uint64";
		static constexpr std::string_view Float = "float";
		static constexpr std::string_view Double = "double";
		static constexpr std::string_view Function = "function";
		static constexpr std::string_view String = "string";
		static constexpr std::string_view ArrayBool = "bool*";
		static constexpr std::string_view ArrayChar8 = "char8*";
		static constexpr std::string_view ArrayChar16 = "char16*";
		static constexpr std::string_view ArrayInt8 = "int8*";
		static constexpr std::string_view ArrayInt16 = "int16*";
		static constexpr std::string_view ArrayInt32 = "int32*";
		static constexpr std::string_view ArrayInt64 = "int64*";
		static constexpr std::string_view ArrayUInt8 = "uint8*";
		static constexpr std::string_view ArrayUInt16 = "uint16*";
		static constexpr std::string_view ArrayUInt32 = "uint32*";
		static constexpr std::string_view ArrayUInt64 = "uint64*";
		static constexpr std::string_view ArrayFloat = "float*";
		static constexpr std::string_view ArrayDouble = "double*";
		static constexpr std::string_view ArrayString = "string*";
		static constexpr std::string_view Vector2 = "vec2";
		static constexpr std::string_view Vector3 = "vec3";
		static constexpr std::string_view Vector4 = "vec4";
		static constexpr std::string_view Matrix4x4 = "mat4x4";
		static constexpr std::string_view Invalid = "invalid";
#if INTPTR_MAX == INT32_MAX
		static constexpr std::string_view Pointer = "ptr32";
		static constexpr std::string_view ArrayPointer = "ptr32*";
#elif INTPTR_MAX == INT64_MAX
		static constexpr std::string_view Pointer = "ptr64";
		static constexpr std::string_view ArrayPointer = "ptr64*";
#else
		#error "Environment not 32 or 64-bit."
#endif
	}

	/**
	 * @brief Namespace containing utility functions of ValueType enum.
	 */
	namespace ValueUtils {
		/**
		 * @brief Checks if a value is between two other values.
		 *
		 * @tparam T The type of the values.
		 * @param x The value to check.
		 * @param a The lower bound.
		 * @param b The upper bound.
		 * @return True if x is between a and b, inclusive. False otherwise.
		 */
		template<typename T>
		[[maybe_unused]] constexpr bool IsBetween(T x, T a, T b) noexcept {
			return x >= a && x <= b;
		}

		/**
		 * @brief Tests whether a given type is \ref ValueType::Void.
		 *
		 * @param type The type to test.
		 * @return True if type is ValueType::Void. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsVoid(ValueType type) noexcept { return type == ValueType::Void; }

		/**
		 * @brief Tests whether a given type is a valid non-void type.
		 *
		 * @param type The type to test.
		 * @return True if type is a valid non-void type. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsValid(ValueType type) noexcept { return IsBetween(type, ValueType::Void, ValueType::_LastAssigned); }

		/**
		 * @brief Tests whether a given type is scalar (has no vector part).
		 *
		 * @param type The type to test.
		 * @return True if type is scalar. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsScalar(ValueType type) noexcept { return IsBetween(type, ValueType::_BaseStart, ValueType::_BaseEnd); }

		/**
		 * @brief Tests whether a given type is a scalar floating point of any size.
		 *
		 * @param type The type to test.
		 * @return True if type is a scalar floating point. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsFloating(ValueType type) noexcept { return IsBetween(type, ValueType::_FloatStart, ValueType::_FloatEnd); }

		/**
		 * @brief Tests whether a given type is a 1-bit boolean.
		 *
		 * @param type The type to test.
		 * @return True if type is a 1-bit boolean. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsBool(ValueType type) noexcept { return type == ValueType::Bool; }

		/**
		 * @brief Tests whether a given type is an 8-bit character.
		 *
		 * @param type The type to test.
		 * @return True if type is an 8-bit character. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsChar8(ValueType type) noexcept { return type == ValueType::Char8; }

		/**
		 * @brief Tests whether a given type is a 16-bit character.
		 *
		 * @param type The type to test.
		 * @return True if type is a 16-bit character. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsChar16(ValueType type) noexcept { return type == ValueType::Char16; }

		/**
		 * @brief Tests whether a given type is an 8-bit integer.
		 *
		 * @param type The type to test.
		 * @return True if type is an 8-bit integer. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsInt8(ValueType type) noexcept { return type == ValueType::Int8; }

		/**
		 * @brief Tests whether a given type is an 8-bit unsigned integer.
		 *
		 * @param type The type to test.
		 * @return True if type is an 8-bit unsigned integer. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsUInt8(ValueType type) noexcept { return type == ValueType::UInt8; }

		/**
		 * @brief Tests whether a given type is a 16-bit integer.
		 *
		 * @param type The type to test.
		 * @return True if type is a 16-bit integer. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsInt16(ValueType type) noexcept { return type == ValueType::Int16; }

		/**
		 * @brief Tests whether a given type is a 16-bit unsigned integer.
		 *
		 * @param type The type to test.
		 * @return True if type is a 16-bit unsigned integer. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsUInt16(ValueType type) noexcept { return type == ValueType::UInt16; }

		/**
		 * @brief Tests whether a given type is a 32-bit integer.
		 *
		 * @param type The type to test.
		 * @return True if type is a 32-bit integer. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsInt32(ValueType type) noexcept { return type == ValueType::Int32; }

		/**
		 * @brief Tests whether a given type is a 32-bit unsigned integer.
		 *
		 * @param type The type to test.
		 * @return True if type is a 32-bit unsigned integer. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsUInt32(ValueType type) noexcept { return type == ValueType::UInt32; }

		/**
		 * @brief Tests whether a given type is a 64-bit integer.
		 *
		 * @param type The type to test.
		 * @return True if type is a 64-bit integer. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsInt64(ValueType type) noexcept { return type == ValueType::Int64; }

		/**
		 * @brief Tests whether a given type is a 64-bit unsigned integer.
		 *
		 * @param type The type to test.
		 * @return True if type is a 64-bit unsigned integer. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsUInt64(ValueType type) noexcept { return type == ValueType::UInt64; }

		/**
		 * @brief Tests whether a given type is a pointer.
		 *
		 * @param type The type to test.
		 * @return True if type is a pointer. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsPointer(ValueType type) noexcept { return type == ValueType::Pointer; }

		/**
		 * @brief Tests whether a given type is a float.
		 *
		 * @param type The type to test.
		 * @return True if type is a float. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsFloat(ValueType type) noexcept { return type == ValueType::Float; }

		/**
		 * @brief Tests whether a given type is a double.
		 *
		 * @param type The type to test.
		 * @return True if type is a double. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsDouble(ValueType type) noexcept { return type == ValueType::Double; }

		/**
		 * @brief Tests whether a given type is a C-function pointer.
		 *
		 * @param type The type to test.
		 * @return True if type is a C-function pointer. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsFunction(ValueType type) noexcept { return type == ValueType::Function; }

		/**
		 * @brief Tests whether a given type is a string.
		 *
		 * @param type The type to test.
		 * @return True if type is a string. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsString(ValueType type) noexcept { return type == ValueType::String; }

		/**
		 * @brief Tests whether a given type is an object of any size.
		 *
		 * @param type The type to test.
		 * @return True if type is an object. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsObject(ValueType type) noexcept { return IsBetween(type, ValueType::_ObjectStart, ValueType::_ObjectEnd); }

		/**
		 * @brief Tests whether a given type is an array of any size.
		 *
		 * @param type The type to test.
		 * @return True if type is an array. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsArray(ValueType type) noexcept { return IsBetween(type, ValueType::_ArrayStart, ValueType::_ArrayEnd); }

		/**
		 * @brief Tests whether a given type is a POD (plain old data) structure of any size.
		 *
		 * @param type The type to test.
		 * @return True if type is a POD structure. False otherwise.
		 */
		[[maybe_unused]] constexpr bool IsStruct(ValueType type) noexcept { return IsBetween(type, ValueType::_StructStart, ValueType::_StructEnd); }


		/**
		 * @brief Checks if a given ValueType is considered a hidden object parameter.
		 *
		 * @param type The ValueType to check.
		 * @return true if considered a hidden object parameter, false otherwise.
		 *
		 * This function determines if the provided ValueType is typically treated as a hidden object parameter.
		 * Hidden object parameters are those where the return argument is allocated by the caller function and
		 * passed as the first argument. This is often true for objects and large structs.
		 */
		[[maybe_unused]] constexpr bool IsHiddenParam(ValueType type) noexcept {
			return IsObject(type) || IsBetween(type, ValueType::_HiddenParamStart, ValueType::_StructEnd);
		}
		
		/**
		 * @brief Convert a ValueType enum value to its string representation.
		 * @param value The ValueType value to convert.
		 * @return The string representation of the ValueType.
		 */
		[[maybe_unused]] constexpr std::string_view ToString(ValueType value) noexcept {
			switch (value) {
				case ValueType::Void:          return ValueName::Void;
				case ValueType::Bool:          return ValueName::Bool;
				case ValueType::Char8:         return ValueName::Char8;
				case ValueType::Char16:        return ValueName::Char16;
				case ValueType::Int8:          return ValueName::Int8;
				case ValueType::Int16:         return ValueName::Int16;
				case ValueType::Int32:         return ValueName::Int32;
				case ValueType::Int64:         return ValueName::Int64;
				case ValueType::UInt8:         return ValueName::UInt8;
				case ValueType::UInt16:        return ValueName::UInt16;
				case ValueType::UInt32:        return ValueName::UInt32;
				case ValueType::UInt64:        return ValueName::UInt64;
				case ValueType::Pointer:       return ValueName::Pointer;
				case ValueType::Float:         return ValueName::Float;
				case ValueType::Double:        return ValueName::Double;
				case ValueType::Function:      return ValueName::Function;
				case ValueType::String:        return ValueName::String;
				case ValueType::ArrayBool:     return ValueName::ArrayBool;
				case ValueType::ArrayChar8:    return ValueName::ArrayChar8;
				case ValueType::ArrayChar16:   return ValueName::ArrayChar16;
				case ValueType::ArrayInt8:     return ValueName::ArrayInt8;
				case ValueType::ArrayInt16:    return ValueName::ArrayInt16;
				case ValueType::ArrayInt32:    return ValueName::ArrayInt32;
				case ValueType::ArrayInt64:    return ValueName::ArrayInt64;
				case ValueType::ArrayUInt8:    return ValueName::ArrayUInt8;
				case ValueType::ArrayUInt16:   return ValueName::ArrayUInt16;
				case ValueType::ArrayUInt32:   return ValueName::ArrayUInt32;
				case ValueType::ArrayUInt64:   return ValueName::ArrayUInt64;
				case ValueType::ArrayPointer:  return ValueName::ArrayPointer;
				case ValueType::ArrayFloat:    return ValueName::ArrayFloat;
				case ValueType::ArrayDouble:   return ValueName::ArrayDouble;
				case ValueType::ArrayString:   return ValueName::ArrayString;
				case ValueType::Vector2:       return ValueName::Vector2;
				case ValueType::Vector3:       return ValueName::Vector2;
				case ValueType::Vector4:       return ValueName::Vector3;
				case ValueType::Matrix4x4:     return ValueName::Vector4;
				default:                       return ValueName::Invalid;
			}
		}

		/**
		 * @brief Convert a string representation to a ValueType enum value.
		 * @param value The string representation of ValueType.
		 * @return The corresponding ValueType enum value.
		 */
		[[maybe_unused]] constexpr ValueType FromString(std::string_view value) noexcept {
			if (value == ValueName::Void) {
				return ValueType::Void;
			} else if (value == ValueName::Bool) {
				return ValueType::Bool;
			} else if (value == ValueName::Char8) {
				return ValueType::Char8;
			} else if (value == ValueName::Char16) {
				return ValueType::Char16;
			} else if (value == ValueName::Int8) {
				return ValueType::Int8;
			} else if (value == ValueName::Int16) {
				return ValueType::Int16;
			} else if (value == ValueName::Int32) {
				return ValueType::Int32;
			} else if (value == ValueName::Int64) {
				return ValueType::Int64;
			} else if (value == ValueName::UInt8) {
				return ValueType::UInt8;
			} else if (value == ValueName::UInt16) {
				return ValueType::UInt16;
			} else if (value == ValueName::UInt32) {
				return ValueType::UInt32;
			} else if (value == ValueName::UInt64) {
				return ValueType::UInt64;
			} else if (value == ValueName::Pointer) {
				return ValueType::Pointer;
			} else if (value == ValueName::Float) {
				return ValueType::Float;
			} else if (value == ValueName::Double) {
				return ValueType::Double;
			} else if (value == ValueName::Function) {
				return ValueType::Function;
			} else if (value == ValueName::String) {
				return ValueType::String;
			} else if (value == ValueName::ArrayBool) {
				return ValueType::ArrayBool;
			} else if (value == ValueName::ArrayChar8) {
				return ValueType::ArrayChar8;
			} else if (value == ValueName::ArrayChar16) {
				return ValueType::ArrayChar16;
			} else if (value == ValueName::ArrayInt8) {
				return ValueType::ArrayInt8;
			} else if (value == ValueName::ArrayInt16) {
				return ValueType::ArrayInt16;
			} else if (value == ValueName::ArrayInt32) {
				return ValueType::ArrayInt32;
			} else if (value == ValueName::ArrayInt64) {
				return ValueType::ArrayInt64;
			} else if (value == ValueName::ArrayUInt8) {
				return ValueType::ArrayUInt8;
			} else if (value == ValueName::ArrayUInt16) {
				return ValueType::ArrayUInt16;
			} else if (value == ValueName::ArrayUInt32) {
				return ValueType::ArrayUInt32;
			} else if (value == ValueName::ArrayUInt64) {
				return ValueType::ArrayUInt64;
			} else if (value == ValueName::ArrayPointer) {
				return ValueType::ArrayPointer;
			} else if (value == ValueName::ArrayFloat) {
				return ValueType::ArrayFloat;
			} else if (value == ValueName::ArrayDouble) {
				return ValueType::ArrayDouble;
			} else if (value == ValueName::ArrayString) {
				return ValueType::ArrayString;
			} else if (value == ValueName::Vector2) {
				return ValueType::Vector2;
			} else if (value == ValueName::Vector3) {
				return ValueType::Vector3;
			} else if (value == ValueName::Vector4) {
				return ValueType::Vector4;
			} else if (value == ValueName::Matrix4x4) {
				return ValueType::Matrix4x4;
			}
			return ValueType::Invalid;
		}
	} // namespace ValueUtils

} // namespace plugify