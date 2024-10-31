#pragma once

// Just in case, because we can't ignore some warnings from `-Wpedantic` (about zero size arrays and anonymous structs when gnu extensions are disabled) on gcc
#if defined(__clang__)
#  pragma clang system_header
#elif defined(__GNUC__)
#  pragma GCC system_header
#endif

#include <initializer_list>  // for std::initializer_list
#include <string_view>       // for std::basic_string_view
#include <type_traits>       // for std::is_constant_evaluated, std::declval, std::false_type
#include <algorithm>         // for std::ranges::min, std::max
#include <concepts>          // for std::unsigned_integral, std::signed_integral
#include <iterator>          // for std::ranges::distance, std::ranges::next, std::iterator_traits, std::input_iterator
#include <utility>           // for std::move, std::hash
#include <compare>           // for std::strong_ordering
#include <memory>            // for std::allocator, std::swap, std::allocator_traits
#include <limits>            // for std::numeric_limits
#include <charconv>          // for std::to_chars

#include <cstdint>
#include <cstddef>
#include <cstdarg>

#ifndef PLUGIFY_STRING_STD_HASH
#  define PLUGIFY_STRING_STD_HASH 1
#endif

#ifndef PLUGIFY_STRING_STD_FORMAT
#  define PLUGIFY_STRING_STD_FORMAT 1
#  include <plugify/compat_format.hpp>
#endif

#ifndef PLUGIFY_STRING_NUMERIC_CONVERSIONS
#  define PLUGIFY_STRING_NUMERIC_CONVERSIONS 1
#endif

#if PLUGIFY_STRING_NUMERIC_CONVERSIONS && !__has_include(<cstdlib>)
#  undef PLUGIFY_STRING_NUMERIC_CONVERSIONS
#  define PLUGIFY_STRING_NUMERIC_CONVERSIONS 0
#endif

#if PLUGIFY_STRING_NUMERIC_CONVERSIONS
#  include <cstdlib>
#endif

#ifndef PLUGIFY_STRING_FLOAT
#  define PLUGIFY_STRING_FLOAT 1
#endif

#if PLUGIFY_STRING_FLOAT && !PLUGIFY_STRING_NUMERIC_CONVERSIONS
#  undef PLUGIFY_STRING_FLOAT
#  define PLUGIFY_STRING_FLOAT 0
#endif

#ifndef PLUGIFY_STRING_LONG_DOUBLE
#  define PLUGIFY_STRING_LONG_DOUBLE 1
#endif

#if PLUGIFY_STRING_LONG_DOUBLE && !PLUGIFY_STRING_FLOAT
#  undef PLUGIFY_STRING_LONG_DOUBLE
#  define PLUGIFY_STRING_LONG_DOUBLE 0
#endif

#ifndef PLUGIFY_STRING_CONTAINERS_RANGES
#  define PLUGIFY_STRING_CONTAINERS_RANGES 1
#endif

#if PLUGIFY_STRING_CONTAINERS_RANGES && (__cplusplus <= 202002L || !__has_include(<ranges>) || !defined(__cpp_lib_containers_ranges))
#  undef PLUGIFY_STRING_CONTAINERS_RANGES
#  define PLUGIFY_STRING_CONTAINERS_RANGES 0
#endif

#if PLUGIFY_STRING_CONTAINERS_RANGES
#  include <ranges>
#endif

#define _PLUGIFY_STRING_HAS_EXCEPTIONS (__cpp_exceptions || __EXCEPTIONS || _HAS_EXCEPTIONS)

#ifndef PLUGIFY_STRING_EXCEPTIONS
#  if _PLUGIFY_STRING_HAS_EXCEPTIONS
#    define PLUGIFY_STRING_EXCEPTIONS 1
#  else
#    define PLUGIFY_STRING_EXCEPTIONS 0
#  endif
#endif

#if PLUGIFY_STRING_EXCEPTIONS && (!_PLUGIFY_STRING_HAS_EXCEPTIONS || !__has_include(<stdexcept>))
#  undef PLUGIFY_STRING_EXCEPTIONS
#  define PLUGIFY_STRING_EXCEPTIONS 0
#endif

#ifndef PLUGIFY_STRING_FALLBACK_ASSERT
#  define PLUGIFY_STRING_FALLBACK_ASSERT 1
#endif

#if PLUGIFY_STRING_FALLBACK_ASSERT && !__has_include(<cassert>)
#  undef PLUGIFY_STRING_FALLBACK_ASSERT
#  define PLUGIFY_STRING_FALLBACK_ASSERT 0
#endif

#ifndef PLUGIFY_STRING_FALLBACK_ABORT
#  define PLUGIFY_STRING_FALLBACK_ABORT 1
#endif

#if PLUGIFY_STRING_FALLBACK_ABORT && !__has_include(<cstdlib>)
#  undef PLUGIFY_STRING_FALLBACK_ABORT
#  define PLUGIFY_STRING_FALLBACK_ABORT 0
#endif

#ifndef PLUGIFY_STRING_FALLBACK_ABORT_FUNCTION
#  define PLUGIFY_STRING_FALLBACK_ABORT_FUNCTION [] (auto) { }
#endif

#if PLUGIFY_STRING_EXCEPTIONS
#  include <stdexcept>
#  define _PLUGIFY_STRING_ASSERT(x, str, e) do { if (!(x)) [[unlikely]] throw e(str); } while (0)
#elif PLUGIFY_STRING_FALLBACK_ASSERT
#  include <cassert>
#  define _PLUGIFY_STRING_ASSERT(x, str, ...) assert(x && str)
#elif PLUGIFY_STRING_FALLBACK_ABORT
#  if !PLUGIFY_STRING_NUMERIC_CONVERSIONS
#    include <cstdlib>
#  endif
#  define _PLUGIFY_STRING_ASSERT(x, ...) do { if (!(x)) [[unlikely]] { std::abort(); } } while (0)
#else
#  define _PLUGIFY_STRING_ASSERT(x, str, ...) do { if (!(x)) [[unlikely]] { PLUGIFY_STRING_FALLBACK_ABORT_FUNCTION (str); { while (true) { [] { } (); } } } } while (0)
#endif

#define _PLUGIFY_STRING_PRAGMA_IMPL(x) _Pragma(#x)
#define _PLUGIFY_STRING_PRAGMA(x) _PLUGIFY_STRING_PRAGMA_IMPL(x)

#if defined(__clang__)
#  define _PLUGIFY_STRING_PRAGMA_DIAG_PREFIX clang
#elif defined(__GNUC__)
#  define _PLUGIFY_STRING_PRAGMA_DIAG_PREFIX GCC
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define _PLUGIFY_STRING_WARN_PUSH() _PLUGIFY_STRING_PRAGMA(_PLUGIFY_STRING_PRAGMA_DIAG_PREFIX diagnostic push)
#  define _PLUGIFY_STRING_WARN_IGNORE(wrn) _PLUGIFY_STRING_PRAGMA(_PLUGIFY_STRING_PRAGMA_DIAG_PREFIX diagnostic ignored wrn)
#  define _PLUGIFY_STRING_WARN_POP() _PLUGIFY_STRING_PRAGMA(_PLUGIFY_STRING_PRAGMA_DIAG_PREFIX diagnostic pop)
#elif defined(_MSC_VER)
#  define _PLUGIFY_STRING_WARN_PUSH()	__pragma(warning(push))
#  define _PLUGIFY_STRING_WARN_IGNORE(wrn) __pragma(warning(disable: wrn))
#  define _PLUGIFY_STRING_WARN_POP() __pragma(warning(pop))
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define _PLUGIFY_STRING_PACK(decl) decl __attribute__((__packed__))
#elif defined(_MSC_VER)
#  define _PLUGIFY_STRING_PACK(decl) __pragma(pack(push, 1)) decl __pragma(pack(pop))
#else
#  define _PLUGIFY_STRING_PACK(decl) decl
#endif

#if defined(__GNUC__) || defined(__clang__)
#  define _PLUGIFY_STRING_ALWAYS_INLINE __attribute__((always_inline)) inline
#  define _PLUGIFY_STRING_ALWAYS_RESTRICT __restrict__
#elif defined(_MSC_VER)
#  define _PLUGIFY_STRING_ALWAYS_INLINE __forceinline
#  define _PLUGIFY_STRING_ALWAYS_RESTRICT __restrict
#else
#  define _PLUGIFY_STRING_ALWAYS_INLINE inline
#  define _PLUGIFY_STRING_ALWAYS_RESTRICT
#endif

#if defined(_MSC_VER)
#  define _PLUGIFY_STRING_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#else
#  define _PLUGIFY_STRING_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

namespace plg {
	namespace detail {
		template<typename Allocator, typename = void>
		struct is_allocator : std::false_type {};

		template<typename Allocator>
		struct is_allocator<Allocator, std::void_t<typename Allocator::value_type, decltype(std::declval<Allocator&>().allocate(std::size_t{}))>> : std::true_type {};

		template<typename Allocator>
		constexpr inline bool is_allocator_v = is_allocator<Allocator>::value;

		struct uninitialized_size_tag {};

		template<typename>
		constexpr bool dependent_false = false;

#if PLUGIFY_STRING_CONTAINERS_RANGES
		template<typename Range, typename Type>
		concept container_compatible_range = std::ranges::input_range<Range> && std::convertible_to<std::ranges::range_reference_t<Range>, Type>;
#endif
	}// namespace detail

	// basic_string
	// based on implementations from libc++, libstdc++ and Microsoft STL
	template<typename Char, typename Traits = std::char_traits<Char>, typename Allocator = std::allocator<Char>> requires(detail::is_allocator_v<Allocator>)
	class basic_string {
	private:
		using allocator_traits = std::allocator_traits<Allocator>;

	public:
		using traits_type = Traits;
		using value_type = typename traits_type::char_type;
		using allocator_type = Allocator;
		using size_type = typename allocator_traits::size_type;
		using difference_type = typename allocator_traits::difference_type;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = typename allocator_traits::pointer;
		using const_pointer = typename allocator_traits::const_pointer;
		using iterator = value_type*;
		using const_iterator = const value_type*;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using sview_type = std::basic_string_view<Char, Traits>;

		constexpr static size_type npos = static_cast<size_t>(-1);

	private:
		constexpr static auto _terminator = value_type();

		_PLUGIFY_STRING_NO_UNIQUE_ADDRESS
		allocator_type _allocator;

		_PLUGIFY_STRING_WARN_PUSH()

#if defined(__clang__)
		_PLUGIFY_STRING_WARN_IGNORE("-Wgnu-anonymous-struct")
		_PLUGIFY_STRING_WARN_IGNORE("-Wzero-length-array")
#elif defined(__GNUC__)
		_PLUGIFY_STRING_WARN_IGNORE("-Wpedantic")// this doesn't work
#elif defined(_MSC_VER)
		_PLUGIFY_STRING_WARN_IGNORE(4201)
		_PLUGIFY_STRING_WARN_IGNORE(4200)
#endif

		template<typename CharT, size_t = sizeof(CharT)>
		struct padding {
			[[maybe_unused]] uint8_t padding[sizeof(CharT) - 1];
		};

		template<typename CharT>
		struct padding<CharT, 1> {
			// template specialization to remove the padding structure to avoid warnings on zero length arrays
			// also, this allows us to take advantage of the empty-base-class optimization.
		};

		// size must correspond to the last byte of long_data.cap, so we don't want the compiler to insert
		// padding after size if sizeof(value_type) != 1; Also ensures both layouts are the same size.
		struct sso_size : padding<value_type> {
			_PLUGIFY_STRING_PACK(struct {
				uint8_t spare_size : 7;
				uint8_t is_long : 1;
			});
		};

		static constexpr int char_bit = std::numeric_limits<uint8_t>::digits + std::numeric_limits<uint8_t>::is_signed;

		static_assert(char_bit == 8, "assumes an 8 bit byte.");

		struct long_data {
			pointer data;
			size_type size;
			_PLUGIFY_STRING_PACK(struct {
				 size_type cap : sizeof(size_type) * char_bit - 1;
				 size_type is_long : 1;
			});
		};

		static constexpr size_type min_cap = (sizeof(long_data) - sizeof(uint8_t)) / sizeof(value_type) > 2 ? (sizeof(long_data) - sizeof(uint8_t)) / sizeof(value_type) : 2;

		struct short_data {
			value_type data[min_cap];
			sso_size size;
		};

		_PLUGIFY_STRING_WARN_POP()

		static_assert(sizeof(short_data) == (sizeof(value_type) * (min_cap + sizeof(uint8_t))), "short has an unexpected size.");
		static_assert(sizeof(short_data) == sizeof(long_data), "short and long layout structures must be the same size");

		union {
			long_data _long;
			short_data _short{};
		} _storage;

		[[nodiscard]] constexpr static bool fits_in_sso(size_type size)
		{
			return size < min_cap;
		}

		constexpr void long_init() {
			is_long(true);
			set_long_data(nullptr);
			set_long_size(0);
			set_long_cap(0);
		}

		constexpr void short_init() {
			is_long(false);
			set_short_size(0);
		}

		constexpr void default_init(size_type size)
		{
			if (fits_in_sso(size))
				short_init();
			else
				long_init();
		}

		[[nodiscard]] constexpr auto& get_long_data() noexcept
		{
			return _storage._long.data;
		}

		[[nodiscard]] constexpr const auto& get_long_data() const noexcept
		{
			return _storage._long.data;
		}

		[[nodiscard]] constexpr auto& get_short_data() noexcept
		{
			return _storage._short.data;
		}

		[[nodiscard]] constexpr const auto& get_short_data() const noexcept
		{
			return _storage._short.data;
		}

		constexpr void set_short_size(size_type size) noexcept
		{
			_storage._short.size.spare_size = min_cap - (size & 0x7F);
		}

		[[nodiscard]] constexpr size_type get_short_size() const noexcept
		{
			return min_cap - _storage._short.size.spare_size;
		}

		constexpr void set_long_size(size_type size) noexcept
		{
			_storage._long.size = size;
		}

		[[nodiscard]] constexpr size_type get_long_size() const noexcept
		{
			return _storage._long.size;
		}

		constexpr void set_long_cap(size_type cap) noexcept
		{
			_storage._long.cap = (cap & 0x7FFFFFFFFFFFFFFF);
		}

		[[nodiscard]] constexpr size_type get_long_cap() const noexcept
		{
			return _storage._long.cap;
		}

		constexpr void set_long_data(value_type* data)
		{
			_storage._long.data = data;
		}

		constexpr void is_long(bool l) noexcept
		{
			_storage._long.is_long = l;
		}

		[[nodiscard]] constexpr bool is_long() const noexcept
		{
			return _storage._long.is_long == true;
		}

		[[nodiscard]] constexpr pointer get_data() noexcept
		{
			return is_long() ? get_long_data() : get_short_data();
		}

		[[nodiscard]] constexpr const_pointer get_data() const noexcept
		{
			return is_long() ? get_long_data() : get_short_data();
		}

		[[nodiscard]] constexpr size_type get_size() const noexcept
		{
			return is_long() ? get_long_size() : get_short_size();
		}

		constexpr void set_size(size_type size) noexcept
		{
			if (is_long())
				set_long_size(size);
			else
				set_short_size(size);
		}

		[[nodiscard]] constexpr size_type get_cap() const noexcept
		{
			if (is_long())
				return get_long_cap();
			else
				return min_cap;
		}

		[[nodiscard]] constexpr sview_type get_view() const noexcept
		{
			return sview_type(get_data(), get_size());
		}

		constexpr void reallocate(std::size_t new_cap, bool copy_old) {
			
			if (new_cap == get_long_cap())
				return;

			auto old_len = get_long_size();
			auto old_cap = get_long_cap();
			auto& old_buffer = get_long_data();

			auto new_len = std::ranges::min(new_cap, old_len);
			auto new_data = _allocator.allocate(new_cap + 1);

			if (old_buffer != nullptr) {
				if (old_len != 0 && copy_old)
					Traits::copy(new_data, old_buffer, new_len);
				_allocator.deallocate(old_buffer, old_cap + 1);
			}

			set_long_data(new_data);
			set_long_size(new_len);
			set_long_cap(new_cap);
		}

		constexpr void deallocate() 
		{
			if (is_long()) {
				if (auto& buffer = get_long_data(); buffer != nullptr) {
					_allocator.deallocate(buffer, get_long_cap() + 1);
					buffer = nullptr;
				}
			}
		}

		constexpr void grow_to(size_type new_cap) 
		{
			if (is_long() == true) {
				reallocate(new_cap, true);
				return;
			}

			auto buffer = _allocator.allocate(new_cap + 1);
			auto len = get_short_size();

			Traits::copy(buffer, get_short_data(), len);
			Traits::assign(buffer[len], _terminator);

			long_init();
			set_long_data(buffer);
			set_long_size(len);
			set_long_cap(new_cap);
		}

		constexpr void null_terminate() noexcept
		{
			auto buffer = get_data();
			if (buffer == nullptr) [[unlikely]]
				return;
			Traits::assign(buffer[get_size()], _terminator);
		}

		constexpr bool addr_in_range(const_pointer ptr) const
		{
			if (std::is_constant_evaluated())
				return false;
			return get_data() <= ptr && ptr <= get_data() + get_size();
		}

		constexpr void internal_replace_impl(auto func, size_type pos, size_type oldcount, size_type count)
		{
			auto cap = get_cap();
			auto sz = get_size();

			auto rsz = sz - oldcount + count;

			if (cap < rsz)
				grow_to(rsz);

			if (oldcount != count)
				Traits::move(get_data() + pos + count, get_data() + pos + oldcount, sz - pos - oldcount);

			func();

			set_size(rsz);
			null_terminate();
		}

		constexpr void internal_replace(size_type pos, const_pointer str, size_type oldcount, size_type count) 
		{
			if (addr_in_range(str)) {
				basic_string rstr(str, count);
				internal_replace_impl([&]() { Traits::copy(get_data() + pos, rstr.data(), count); }, pos, oldcount, count);
			} else
				internal_replace_impl([&]() { Traits::copy(get_data() + pos, str, count); }, pos, oldcount, count);
		}

		constexpr void internal_replace(size_type pos, value_type ch, size_type oldcount, size_type count) 
		{
			internal_replace_impl([&]() { Traits::assign(get_data() + pos, count, ch); }, pos, oldcount, count);
		}

		constexpr void internal_insert_impl(auto func, size_type pos, size_type size)
		{
			if (size == 0) [[unlikely]]
				return;

			auto cap = get_cap();
			auto sz = get_size();
			auto rsz = sz + size;

			if (cap < rsz)
				grow_to(rsz);

			Traits::move(get_data() + pos + size, get_data() + pos, sz - pos);
			func();

			set_size(rsz);
			null_terminate();
		}

		constexpr void internal_insert(size_type pos, const_pointer str, size_type count) 
		{
			if (addr_in_range(str)) {
				basic_string rstr(str, count);
				internal_insert_impl([&]() { Traits::copy(get_data() + pos, rstr.data(), count); }, pos, count);
			} else
				internal_insert_impl([&]() { Traits::copy(get_data() + pos, str, count); }, pos, count);
		}

		constexpr void internal_insert(size_type pos, value_type ch, size_type count) 
		{
			internal_insert_impl([&]() { Traits::assign(get_data() + pos, count, ch); }, pos, count);
		}

		constexpr void internal_append_impl(auto func, size_type size)
		{
			if (size == 0) [[unlikely]]
				return;

			auto cap = get_cap();
			auto sz = get_size();
			auto rsz = sz + size;

			if (cap < rsz)
				grow_to(rsz);

			func(sz);
			set_size(rsz);
			null_terminate();
		}

		constexpr void internal_append(const_pointer str, size_type count) 
		{
			if (addr_in_range(str)) {
				basic_string rstr(str, count);
				internal_append_impl([&](size_type pos) { Traits::copy(get_data() + pos, rstr.data(), count); }, count);
			} else
				internal_append_impl([&](size_type pos) { Traits::copy(get_data() + pos, str, count); }, count);
		}

		constexpr void internal_append(value_type ch, size_type count) 
		{
			internal_append_impl([&](size_type pos) { Traits::assign(get_data() + pos, count, ch); }, count);
		}

		constexpr void internal_assign_impl(auto func, size_type size, bool copy_old)
		{
			if (fits_in_sso(size)) {
				if (is_long() == true) {
					deallocate();
					short_init();
				}

				set_short_size(size);
				func(get_short_data());
				null_terminate();
			} else {
				if (is_long() == false)
					long_init();
				if (get_long_cap() < size)
					reallocate(size, copy_old);

				func(get_long_data());
				set_long_size(size);
				null_terminate();
			}
		}

		constexpr void internal_assign(const_pointer str, size_type size, bool copy_old = false) 
		{
			if (addr_in_range(str)) {
				basic_string rstr(str, size);
				internal_assign_impl([&](auto data) { Traits::copy(data, rstr.data(), size); }, size, copy_old);
			} else
				internal_assign_impl([&](auto data) { Traits::copy(data, str, size); }, size, copy_old);
		}

		constexpr void internal_assign(value_type ch, size_type count, bool copy_old = false) 
		{
			internal_assign_impl([&](auto data) { Traits::assign(data, count, ch); }, count, copy_old);
		}

	public:
		explicit constexpr basic_string(detail::uninitialized_size_tag, size_type size, const allocator_type& alloc)
			: _allocator(alloc)
		{
			_PLUGIFY_STRING_ASSERT(size <= max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			if (fits_in_sso(size))
				short_init();
			else {
				long_init();
				reallocate(size, false);
			}
			set_size(size);
		}

		constexpr basic_string() noexcept(std::is_nothrow_default_constructible<allocator_type>::value)
			: basic_string(allocator_type())
		{}

		explicit constexpr basic_string(const allocator_type& alloc) noexcept
			: _allocator(alloc)
		{
			short_init();
		}

		constexpr basic_string(size_type count, value_type ch, const allocator_type& alloc = allocator_type())
			: _allocator(alloc)
		{
			_PLUGIFY_STRING_ASSERT(count <= max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			internal_assign(ch, count);
		}

		constexpr basic_string(const basic_string& str, size_type pos, size_type count, const allocator_type& alloc = allocator_type())
			: _allocator(alloc)
		{
			_PLUGIFY_STRING_ASSERT(pos >= 0 && pos <= str.get_size(), "plg::basic_string::basic_string(): pos out of range", std::out_of_range);
			auto len = std::ranges::min(count, str.get_size() - pos);
			_PLUGIFY_STRING_ASSERT(len <= max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			internal_assign(str.data() + pos, len);
		}
		constexpr basic_string(const basic_string& str, size_type pos, const allocator_type& alloc = allocator_type())
			: basic_string(str, pos, npos, alloc)
		{}

		constexpr basic_string(const value_type* str, size_type count, const allocator_type& alloc = allocator_type())
			: _allocator(alloc)
		{
			_PLUGIFY_STRING_ASSERT(count <= max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			internal_assign(str, count);
		}

		constexpr basic_string(const value_type* str, const allocator_type& alloc = allocator_type())
			: basic_string(str, Traits::length(str), alloc)
		{}

		template<std::input_iterator InputIterator>
		constexpr basic_string(InputIterator first, InputIterator last, const allocator_type& alloc = allocator_type())
			: _allocator(alloc)
		{
			auto len = static_cast<size_type>(std::ranges::distance(first, last));
			_PLUGIFY_STRING_ASSERT(len <= max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			internal_assign(const_pointer(first), len);
		}

		constexpr basic_string(const basic_string& str, const allocator_type& alloc)
			: _allocator(alloc)
		{
			auto len = str.length();
			_PLUGIFY_STRING_ASSERT(len <= max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			internal_assign(str.data(), len);
		}
		constexpr basic_string(const basic_string& str)
			: basic_string(str, allocator_type())
		{}

		constexpr basic_string(basic_string&& str, const allocator_type& alloc)
			: _allocator(alloc)
		{
			assign(std::move(str));
		}
		constexpr basic_string(basic_string&& str) noexcept(std::is_nothrow_move_constructible<allocator_type>::value)
			: _allocator(std::move(str._allocator)), _storage(std::move(str._storage))
		{
			str.short_init();
		}

		constexpr basic_string(std::initializer_list<value_type> list, const allocator_type& alloc = allocator_type())
			: _allocator(alloc)
		{
			auto len = list.size();
			_PLUGIFY_STRING_ASSERT(len <= max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			internal_assign(const_pointer(list.begin()), len);
		}

		template<typename Type> requires(std::is_convertible_v<const Type&, sview_type>)
		constexpr basic_string(const Type& t, size_type pos, size_type count, const allocator_type& alloc = allocator_type())
			: _allocator(alloc)
		{
			auto sv = sview_type(t);
			_PLUGIFY_STRING_ASSERT(pos >= 0 && pos <= sv.length(), "plg::basic_string::basic_string(): pos out of range", std::out_of_range);
			auto ssv = sv.substr(pos, count);
			auto len = ssv.length();
			_PLUGIFY_STRING_ASSERT(len <= max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			internal_assign(ssv.data(), len);
		}

		template<typename Type>
			requires(std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string(const Type& t, const allocator_type& alloc = allocator_type())
			: _allocator(alloc)
		{
			sview_type sv(t);
			auto len = sv.length();
			_PLUGIFY_STRING_ASSERT(len <= max_size(), "plg::basic_string::basic_string(): constructed string size would exceed max_size()", std::length_error);
			internal_assign(sv.data(), len);
		}

		constexpr basic_string(basic_string&& str, size_type pos, size_type count, const allocator_type& alloc = allocator_type())
			: basic_string(std::move(str), alloc)
		{
			_PLUGIFY_STRING_ASSERT(pos >= 0 && pos <= str.get_size(), "plg::basic_string::basic_string(): pos out of range", std::out_of_range);
			erase(pos, count);
        }

		constexpr basic_string(basic_string&& str, size_type pos, const allocator_type& alloc = allocator_type())
			: basic_string(std::move(str), pos, npos, alloc)
		{}

#if __cplusplus > 202002L
		basic_string(std::nullptr_t) = delete;
#endif

#if PLUGIFY_STRING_CONTAINERS_RANGES
		template<detail::container_compatible_range<Char> Range>
		constexpr basic_string(std::from_range_t, Range&& range, const Allocator& a = Allocator()) : basic_string(std::ranges::begin(range), std::ranges::end(range), a)
		{}
#endif

		constexpr ~basic_string()
		{
			deallocate();
		}

		constexpr basic_string& operator=(const basic_string& str)
		{
			return assign(str);
		}

		constexpr basic_string& operator=(basic_string&& str) noexcept(
				allocator_traits::propagate_on_container_move_assignment::value ||
				allocator_traits::is_always_equal::value) 
		{
			return assign(std::move(str));
		}

		constexpr basic_string& operator=(const value_type* str) 
		{
			return assign(str, Traits::length(str));
		}

		constexpr basic_string& operator=(value_type ch) 
		{
			return assign(std::addressof(ch), 1);
		}

		constexpr basic_string& operator=(std::initializer_list<value_type> list)
		{
			return assign(list.begin(), list.size());
		}

		template<typename Type>
			requires(std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& operator=(const Type& t) 
		{
			sview_type sv(t);
			return assign(sv);
		}

#if __cplusplus > 202002L
		constexpr basic_string& operator=(std::nullptr_t) = delete;
#endif

		constexpr basic_string& assign(size_type count, value_type ch) 
		{
			_PLUGIFY_STRING_ASSERT(count <= max_size(), "plg::basic_string::assign(): resulted string size would exceed max_size()", std::length_error);
			internal_assign(ch, count);
			return *this;
		}

		constexpr basic_string& assign(const basic_string& str) 
		{
			if (this == &str) [[unlikely]]
				return *this;
			internal_assign(str.data(), str.size());
			return *this;
		}

		constexpr basic_string& assign(const basic_string& str, size_type pos, size_type count = npos) 
		{
			_PLUGIFY_STRING_ASSERT(pos >= 0 && pos <= str.get_size(), "plg::basic_string::assign(): pos out of range", std::out_of_range);
			internal_assign(str.data(), std::ranges::min(count, str.size() - pos));
			return *this;
		}

		constexpr basic_string& assign(basic_string&& str) noexcept(
				allocator_traits::propagate_on_container_move_assignment::value ||
				allocator_traits::is_always_equal::value) 
		{
			if (this == &str) [[unlikely]]
				return *this;
			if (str.is_long() && _allocator != str._allocator) {
				auto len = str.get_long_size();
				internal_assign(str.get_long_data(), len);
			} else {
				std::ranges::swap(_storage , str._storage);
				str.deallocate();
				str.short_init();
			}
			return *this;
		}

		constexpr basic_string& assign(const value_type* str, size_type count) 
		{
			_PLUGIFY_STRING_ASSERT(count <= max_size(), "plg::basic_string::assign(): resulted string size would exceed max_size()", std::length_error);
			internal_assign(str, count);
			return *this;
		}

		constexpr basic_string& assign(const value_type* str)
		{
			return assign(str, Traits::length(str));
		}

		template<std::input_iterator InputIterator>
		constexpr basic_string& assign(InputIterator first, InputIterator last)
		{
			auto len = static_cast<size_type>(std::ranges::distance(first, last));
			_PLUGIFY_STRING_ASSERT(len <= max_size(), "plg::basic_string::assign(): resulted string size would exceed max_size()", std::length_error);
			internal_assign(const_pointer(first), len);
			return *this;
		}

		constexpr basic_string& assign(std::initializer_list<value_type> list)
		{
			auto len = list.size();
			_PLUGIFY_STRING_ASSERT(len <= max_size(), "plg::basic_string::assign(): resulted string size would exceed max_size()", std::length_error);
			internal_assign(const_pointer(list.begin()), len);
			return *this;
		}

		template<typename Type>
			requires(std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& assign(const Type& t) 
		{
			sview_type sv(t);
			return assign(sv.data(), sv.length());
		}

		template<typename Type>
			requires(std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& assign(const Type& t, size_type pos, size_type count = npos) 
		{
			auto sv = sview_type(t).substr(pos, count);
			auto len = sv.length();
			_PLUGIFY_STRING_ASSERT(len <= max_size(), "plg::basic_string::assign(): resulted string size would exceed max_size()", std::length_error);
			return assign(sv.data(), len);
		}

#if PLUGIFY_STRING_CONTAINERS_RANGES
		template<detail::container_compatible_range<Char> Range>
		constexpr basic_string& assign_range(Range&& range) 
		{
			auto str = basic_string(std::from_range, std::forward<Range>(range), _allocator);
			_PLUGIFY_STRING_ASSERT(str.get_size() <= max_size(), "plg::basic_string::assign_range(): resulted string size would exceed max_size()", std::length_error);
			return assign();
		}
#endif
		[[nodiscard]] constexpr allocator_type get_allocator() const noexcept 
		{
			return _allocator;
		}

		[[nodiscard]] constexpr reference operator[](size_type pos) 
		{
			return get_data()[pos];
		}

		[[nodiscard]] constexpr const_reference operator[](size_type pos) const 
		{
			return get_data()[pos];
		}

		[[nodiscard]] constexpr reference at(size_type pos) 
		{
			_PLUGIFY_STRING_ASSERT(pos >= 0 && pos <= get_size(), "plg::basic_string::at(): pos out of range", std::out_of_range);
			return get_data()[pos];
		}

		[[nodiscard]] constexpr const_reference at(size_type pos) const 
		{
			_PLUGIFY_STRING_ASSERT(pos >= 0 && pos <= get_size(), "plg::basic_string::at(): pos out of range", std::out_of_range);
			return get_data()[pos];
		}

		[[nodiscard]] constexpr reference front() 
		{
			return get_data()[0];
		}

		[[nodiscard]] constexpr const_reference front() const 
		{
			return get_data()[0];
		}

		[[nodiscard]] constexpr reference back() 
		{
			return get_data()[get_size() - 1];
		}

		[[nodiscard]] constexpr const_reference back() const 
		{
			return get_data()[get_size() - 1];
		}

		[[nodiscard]] constexpr const value_type* data() const noexcept 
		{
			return get_data();
		}

		[[nodiscard]] constexpr value_type* data() noexcept 
		{
			return get_data();
		}

		[[nodiscard]] constexpr const value_type* c_str() const noexcept 
		{
			return get_data();
		}

		[[nodiscard]] constexpr operator sview_type() const noexcept 
		{
			return get_view();
		}

		[[nodiscard]] constexpr iterator begin() noexcept 
		{
			return get_data();
		}

		[[nodiscard]] constexpr const_iterator begin() const noexcept 
		{
			return get_data();
		}

		[[nodiscard]] constexpr const_iterator cbegin() const noexcept 
		{
			return get_data();
		}

		[[nodiscard]] constexpr iterator end() noexcept
		{
			return get_data() + get_size();
		}

		[[nodiscard]] constexpr const_iterator end() const noexcept 
		{
			return get_data() + get_size();
		}

		[[nodiscard]] constexpr const_iterator cend() const noexcept 
		{
			return get_data() + get_size();
		}

		[[nodiscard]] constexpr reverse_iterator rbegin() noexcept 
		{
			return reverse_iterator(end());
		}

		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept 
		{
			return const_reverse_iterator(end());
		}

		[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept
		{
			return const_reverse_iterator(cend());
		}

		[[nodiscard]] constexpr reverse_iterator rend() noexcept 
		{
			return reverse_iterator(begin());
		}

		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept 
		{
			return const_reverse_iterator(begin());
		}

		[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept 
		{
			return const_reverse_iterator(cbegin());
		}

		[[nodiscard]] constexpr bool empty() const noexcept 
		{
			return get_size() == 0;
		}

		[[nodiscard]] constexpr size_type size() const noexcept 
		{
			return get_size();
		}

		[[nodiscard]] constexpr size_type length() const noexcept 
		{
			return get_size();
		}

		[[nodiscard]] constexpr size_type max_size() const noexcept 
		{
			// size_type m = allocator_traits::max_size(_allocator);

			// if (m <= numeric_limits<size_type>::max() / 2)
			//     return m - alignment;
			// else
			//     return (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) ? m - alignment : (m / 2) - alignment;
			return (allocator_traits::max_size(allocator_type()) - 1) / 2;
		}

		[[nodiscard]] constexpr size_type capacity() const noexcept
		{
			return get_cap();
		}

		constexpr void reserve(size_type cap) 
		{
			_PLUGIFY_STRING_ASSERT(cap <= max_size(), "plg::basic_string::reserve(): allocated memory size would exceed max_size()", std::length_error);
			if (cap <= get_cap()) [[unlikely]]
				return;

			auto new_cap = std::ranges::max(cap, get_size());
			if (new_cap == get_cap()) [[unlikely]]
				return;

			grow_to(new_cap);
		}

		void reserve() 
		{
			shrink_to_fit();
		}

		constexpr void shrink_to_fit()
		{
			if (is_long() == false)
				return;

			reallocate(get_size(), true);
		}

		constexpr void clear() noexcept
		{
			set_size(0);
		}

		constexpr basic_string& insert(size_type pos, size_type count, value_type ch) 
		{
			_PLUGIFY_STRING_ASSERT(get_size() + count <= max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			_PLUGIFY_STRING_ASSERT(pos >= 0 && pos <= get_size(), "plg::basic_string::insert(): pos out of range", std::out_of_range);
			insert(std::ranges::next(cbegin(), pos), count, ch);
			return *this;
		}

		constexpr basic_string& insert(size_type pos, const value_type* str) 
		{
			_PLUGIFY_STRING_ASSERT(pos >= 0 && pos <= get_size(), "plg::basic_string::insert(): pos out of range", std::out_of_range);
			auto len = Traits::length(str);
			_PLUGIFY_STRING_ASSERT(get_size() + len <= max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			internal_insert(pos, str, len);
			return *this;
		}

		constexpr basic_string& insert(size_type pos, const value_type* str, size_type count) 
		{
			_PLUGIFY_STRING_ASSERT(get_size() + count <= max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			_PLUGIFY_STRING_ASSERT(pos >= 0 && pos <= get_size(), "plg::basic_string::insert(): pos out of range", std::out_of_range);
			internal_insert(pos, str, count);
			return *this;
		}

		constexpr basic_string& insert(size_type pos, const basic_string& str) 
		{
			_PLUGIFY_STRING_ASSERT(get_size() + str.get_size() <= max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			_PLUGIFY_STRING_ASSERT(pos >= 0 && pos <= get_size(), "plg::basic_string::insert(): pos out of range", std::out_of_range);
			internal_insert(pos, const_pointer(str.get_data()), str.get_size());
			return *this;
		}

		constexpr basic_string& insert(size_type pos, const basic_string& str, size_type pos_str, size_type count = npos) 
		{
			_PLUGIFY_STRING_ASSERT(pos <= get_size() && pos_str <= str.get_size(), "plg::basic_string::insert(): pos or pos_str out of range", std::out_of_range);
			count = std::ranges::min(count, str.length() - pos_str);
			_PLUGIFY_STRING_ASSERT(get_size() + count <= max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			return insert(pos, str.data() + pos_str, count);
		}

		constexpr iterator insert(const_iterator pos, value_type ch) 
		{
			return insert(pos, 1, ch);
		}

		constexpr iterator insert(const_iterator pos, size_type count, value_type ch) 
		{
			_PLUGIFY_STRING_ASSERT(get_size() + count <= max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			auto spos = std::ranges::distance(cbegin(), pos);
			internal_insert(spos, ch, count);
			return std::ranges::next(begin(), spos);
		}

		template<std::input_iterator InputIterator>
		constexpr iterator insert(const_iterator pos, InputIterator first, InputIterator last) 
		{
			auto spos = std::ranges::distance(cbegin(), pos);
			auto len = static_cast<size_type>(std::ranges::distance(first, last));
			_PLUGIFY_STRING_ASSERT(get_size() + len <= max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			internal_insert(spos, const_pointer(first), len);
			return std::ranges::next(begin(), spos);
		}

		constexpr iterator insert(const_iterator pos, std::initializer_list<value_type> list)
		{
			_PLUGIFY_STRING_ASSERT(get_size() + list.size() <= max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			auto spos = std::ranges::distance(cbegin(), pos);
			internal_insert(spos, const_pointer(list.begin()), list.size());
			return std::ranges::next(begin(), spos);
		}

		template<typename Type>
			requires(std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& insert(size_type pos, const Type& t) 
		{
			_PLUGIFY_STRING_ASSERT(pos >= 0 && pos <= get_size(), "plg::basic_string::insert(): pos out of range", std::out_of_range);
			sview_type sv(t);
			_PLUGIFY_STRING_ASSERT(get_size() + sv.length() <= max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			internal_insert(pos, const_pointer(sv.data()), sv.length());
			return *this;
		}

		template<typename Type>
			requires(std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& insert(size_type pos, const Type& t, size_type pos_str, size_type count = npos) 
		{
			auto sv = sview_type(t);
			_PLUGIFY_STRING_ASSERT(pos <= get_size() && pos_str <= sv.length(), "plg::basic_string::insert(): pos or pos_str out of range", std::out_of_range);
			auto ssv = sv.substr(pos_str, count);
			_PLUGIFY_STRING_ASSERT(get_size() + ssv.length() <= max_size(), "plg::basic_string::insert(): resulted string size would exceed max_size()", std::length_error);
			internal_insert(pos, const_pointer(ssv.data()), ssv.length());
			return *this;
		}

#if PLUGIFY_STRING_CONTAINERS_RANGES
		template<detail::container_compatible_range<Char> Range>
		constexpr iterator insert_range(const_iterator pos, Range&& range) 
		{
			auto str = basic_string(std::from_range, std::forward<Range>(range), _allocator);
			_PLUGIFY_STRING_ASSERT(get_size() + str.get_size() <= max_size(), "plg::basic_string::insert_range(): resulted string size would exceed max_size()", std::length_error);
			return insert(pos - begin(), str);
		}
#endif

		constexpr basic_string& erase(size_type pos = 0, size_type count = npos) 
		{
			auto sz = get_size();
			auto buffer = get_data();

			_PLUGIFY_STRING_ASSERT(pos >= 0 && pos <= sz, "plg::basic_string::erase(): pos out of range", std::out_of_range);

			count = std::ranges::min(count, sz - pos);

			auto left = sz - (pos + count);
			if (left != 0)
				Traits::move(buffer + pos, buffer + pos + count, left);

			auto new_size = pos + left;
			set_size(new_size);
			null_terminate();

			return *this;
		}

		constexpr iterator erase(const_iterator position) 
		{
			auto pos = std::ranges::distance(cbegin(), position);
			erase(pos, 1);
			return begin() + pos;
		}

		constexpr iterator erase(const_iterator first, const_iterator last) 
		{
			auto pos = std::ranges::distance(cbegin(), first);
			auto len = std::ranges::distance(first, last);
			erase(pos, len);
			return begin() + pos;
		}

		constexpr void push_back(value_type ch) 
		{
			_PLUGIFY_STRING_ASSERT(get_size() + 1 <= max_size(), "plg::basic_string::push_back(): resulted string size would exceed max_size()", std::length_error);
			append(1, ch);
		}

		constexpr void pop_back() 
		{
			erase(end() - 1);
		}

		constexpr basic_string& append(size_type count, value_type ch) 
		{
			_PLUGIFY_STRING_ASSERT(get_size() + count <= max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			internal_append(ch, count);
			return *this;
		}

		constexpr basic_string& append(const basic_string& str) 
		{
			_PLUGIFY_STRING_ASSERT(get_size() + str.get_size() <= max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			internal_append(str.get_data(), str.get_size());
			return *this;
		}

		constexpr basic_string& append(const basic_string& str, size_type pos, size_type count = npos) 
		{
			_PLUGIFY_STRING_ASSERT(pos >= 0 && pos <= str.get_size(), "plg::basic_string::append(): pos out of range", std::out_of_range);
			auto ssv = sview_type(str).substr(pos, count);
			_PLUGIFY_STRING_ASSERT(get_size() + ssv.length() <= max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			internal_append(ssv.data(), ssv.length());
			return *this;
		}

		constexpr basic_string& append(const value_type* str, size_type count) 
		{
			_PLUGIFY_STRING_ASSERT(get_size() + count <= max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			internal_append(str, count);
			return *this;
		}

		constexpr basic_string& append(const value_type* str) 
		{
			auto len = Traits::length(str);
			_PLUGIFY_STRING_ASSERT(get_size() + len <= max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			return append(str, len);
		}

		template<std::input_iterator InputIterator>
		constexpr basic_string& append(InputIterator first, InputIterator last) 
		{
			auto len = static_cast<size_type>(std::ranges::distance(first, last));
			_PLUGIFY_STRING_ASSERT(get_size() + len <= max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			internal_append(const_pointer(first), len);
			return *this;
		}

		constexpr basic_string& append(std::initializer_list<value_type> list)
		{
			_PLUGIFY_STRING_ASSERT(get_size() + list.size() <= max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			internal_append(const_pointer(list.begin()), list.size());
			return *this;
		}

		template<typename Type>
			requires(std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& append(const Type& t) 
		{
			sview_type sv(t);
			_PLUGIFY_STRING_ASSERT(get_size() + sv.length() <= max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			internal_append(sv.data(), sv.size());
			return *this;
		}

		template<typename Type>
			requires(std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& append(const Type& t, size_type pos, size_type count = npos) 
		{
			sview_type sv(t);
			_PLUGIFY_STRING_ASSERT(pos >= 0 && pos <= sv.length(), "plg::basic_string::append(): pos out of range", std::out_of_range);
			auto ssv = sv.substr(pos, count);
			_PLUGIFY_STRING_ASSERT(get_size() + ssv.length() <= max_size(), "plg::basic_string::append(): resulted string size would exceed max_size()", std::length_error);
			internal_append(ssv.data(), ssv.length());
			return *this;
		}

#if PLUGIFY_STRING_CONTAINERS_RANGES
		template<detail::container_compatible_range<Char> Range>
		constexpr basic_string& append_range(Range&& range)
		{
			auto str = basic_string(std::from_range, std::forward<Range>(range), _allocator);
			_PLUGIFY_STRING_ASSERT(get_size() + str.get_size() <= max_size(), "plg::basic_string::insert_range(): resulted string size would exceed max_size()", std::length_error);
			return append(str);
		}
#endif

		constexpr basic_string& operator+=(const basic_string& str) 
		{
			return append(str);
		}

		constexpr basic_string& operator+=(value_type ch) 
		{
			push_back(ch);
			return *this;
		}

		constexpr basic_string& operator+=(const value_type* str) 
		{
			return append(str);
		}

		constexpr basic_string& operator+=(std::initializer_list<value_type> list)
		{
			return append(list);
		}

		template<typename Type>
			requires(std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& operator+=(const Type& t) 
		{
			return append(sview_type(t));
		}

		[[nodiscard]] constexpr int compare(const basic_string& str) const noexcept 
		{
			return get_view().compare(str.get_view());
		}

		[[nodiscard]] constexpr int compare(size_type pos1, size_type count1, const basic_string& str) const
		{
			return get_view().compare(pos1, count1, str.get_view());
		}

		[[nodiscard]] constexpr int compare(size_type pos1, size_type count1, const basic_string& str, size_type pos2, size_type count2 = npos) const 
		{
			return get_view().compare(pos1, count1, str.get_view(), pos2, count2);
		}

		[[nodiscard]] constexpr int compare(const value_type* str) const 
		{
			return get_view().compare(str);
		}

		[[nodiscard]] constexpr int compare(size_type pos1, size_type count1, const value_type* str) const 
		{
			return get_view().compare(pos1, count1, str);
		}

		[[nodiscard]] constexpr int compare(size_type pos1, size_type count1, const value_type* str, size_type count2) const
		{
			return get_view().compare(pos1, count1, str, count2);
		}

		template<typename Type>
			requires(std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		[[nodiscard]] constexpr int compare(const Type& t) const noexcept(noexcept(std::is_nothrow_convertible_v<const Type&, sview_type>)) 
		{
			return get_view().compare(sview_type(t));
		}

		template<typename Type>
			requires(std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		[[nodiscard]] constexpr int compare(size_type pos1, size_type count1, const Type& t) const 
		{
			return get_view().compare(pos1, count1, sview_type(t));
		}

		template<typename Type>
			requires(std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		[[nodiscard]] constexpr int compare(size_type pos1, size_type count1, const Type& t, size_type pos2, size_type count2 = npos) const 
		{
			return get_view().compare(pos1, count1, sview_type(t), pos2, count2);
		}

		[[nodiscard]] constexpr bool starts_with(sview_type sv) const noexcept 
		{
			return get_view().starts_with(sv);
		}

		[[nodiscard]] constexpr bool starts_with(Char ch) const noexcept 
		{
			return get_view().starts_with(ch);
		}

		[[nodiscard]] constexpr bool starts_with(const Char* str) const 
		{
			return get_view().starts_with(str);
		}

		[[nodiscard]] constexpr bool ends_with(sview_type sv) const noexcept 
		{
			return get_view().ends_with(sv);
		}

		[[nodiscard]] constexpr bool ends_with(Char ch) const noexcept 
		{
			return get_view().ends_with(ch);
		}

		[[nodiscard]] constexpr bool ends_with(const Char* str) const 
		{
			return get_view().ends_with(str);
		}

		[[nodiscard]] constexpr bool contains(sview_type sv) const noexcept 
		{
			return get_view().contains(sv);
		}

		[[nodiscard]] constexpr bool contains(Char ch) const noexcept 
		{
			return get_view().contains(ch);
		}

		[[nodiscard]] constexpr bool contains(const Char* str) const 
		{
			return get_view().contains(str);
		}

		constexpr basic_string& replace(size_type pos, size_type count, const basic_string& str) 
		{
			_PLUGIFY_STRING_ASSERT(pos >= 0 && pos <= get_size(), "plg::basic_string::replace(): pos out of range", std::out_of_range);
			return replace(pos, count, str, 0, str.length());
		}

		constexpr basic_string& replace(const_iterator first, const_iterator last, const basic_string& str)
		{
			auto pos = std::ranges::distance(cbegin(), first);
			auto count = std::ranges::distance(first, last);
			return replace(pos, count, str, 0, str.length());
		}

		constexpr basic_string& replace(size_type pos, size_type count, const basic_string& str, size_type pos2, size_type count2 = npos) 
		{
			_PLUGIFY_STRING_ASSERT(pos <= get_size() && pos2 <= str.get_size(), "plg::basic_string::replace(): pos or pos_str out of range", std::out_of_range);
			count2 = std::ranges::min(count2, str.length() - pos2);
			auto ssv = sview_type(str).substr(pos2, count2);
			return replace(pos, count, ssv.data(), ssv.length());
		}

		template<std::input_iterator InputIterator>
		constexpr basic_string& replace(const_iterator first, const_iterator last, InputIterator first2, InputIterator last2) 
		{
			return replace(first, last, const_pointer(first2), std::ranges::distance(first2, last2));
		}

		constexpr basic_string& replace(size_type pos, size_type count, const value_type* str, size_type count2) 
		{
			_PLUGIFY_STRING_ASSERT(pos >= 0 && pos <= get_size(), "plg::basic_string::replace(): pos out of range", std::out_of_range);
			count = std::ranges::min(count, length() - pos);
			_PLUGIFY_STRING_ASSERT(get_size() - count + count2 <= max_size(), "plg::basic_string::replace(): resulted string size would exceed max_size()", std::length_error);
			internal_replace(pos, const_pointer(str), count, count2);
			return *this;
		}

		constexpr basic_string& replace(const_iterator first, const_iterator last, const value_type* str, size_type count2) 
		{
			size_type pos = std::ranges::distance(cbegin(), first);
			size_type count = std::ranges::distance(first, last);

			return replace(pos, count, str, count2);
		}

		constexpr basic_string& replace(size_type pos, size_type count, const value_type* str) 
		{
			return replace(pos, count, str, Traits::length(str));
		}

		constexpr basic_string& replace(const_iterator first, const_iterator last, const value_type* str)
		{
			return replace(first, last, str, Traits::length(str));
		}

		constexpr basic_string& replace(size_type pos, size_type count, size_type count2, value_type ch) 
		{
			_PLUGIFY_STRING_ASSERT(pos >= 0 && pos <= get_size(), "plg::basic_string::replace(): pos out of range", std::out_of_range);
			count = std::ranges::min(count, length() - pos);
			_PLUGIFY_STRING_ASSERT(get_size() - count + count2 <= max_size(), "plg::basic_string::replace(): resulted string size would exceed max_size()", std::length_error);
			internal_replace(pos, ch, count, count2);
			return *this;
		}

		constexpr basic_string& replace(const_iterator first, const_iterator last, size_type count2, value_type ch) 
		{
			auto pos = std::ranges::distance(cbegin(), first);
			auto count = std::ranges::distance(first, last);

			_PLUGIFY_STRING_ASSERT(get_size() - count + count2 <= max_size(), "plg::basic_string::replace(): resulted string size would exceed max_size()", std::length_error);
			_PLUGIFY_STRING_ASSERT(pos >= 0 && pos <= get_size(), "plg::basic_string::replace(): pos out of range", std::out_of_range);
			internal_replace(pos, ch, count, count2);
			return *this;
		}

		constexpr basic_string& replace(const_iterator first, const_iterator last, std::initializer_list<value_type> list)
		{
			return replace(first, last, const_pointer(list.begin()), list.size());
		}

		template<typename Type>
			requires(std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& replace(size_type pos, size_type count, const Type& t) 
		{
			_PLUGIFY_STRING_ASSERT(pos >= 0 && pos <= get_size(), "plg::basic_string::replace(): pos out of range", std::out_of_range);
			sview_type sv(t);
			return replace(pos, count, sv.data(), sv.length());
		}

		template<typename Type>
			requires(std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& replace(const_iterator first, const_iterator last, const Type& t) 
		{
			sview_type sv(t);
			return replace(first, last, sv.data(), sv.length());
		}

		template<typename Type>
			requires(std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		constexpr basic_string& replace(size_type pos, size_type count, const Type& t, size_type pos2, size_type count2 = npos) 
		{
			_PLUGIFY_STRING_ASSERT(pos >= 0 && pos <= get_size(), "plg::basic_string::replace(): pos out of range", std::out_of_range);
			auto sv = sview_type(t).substr(pos2, count2);
			return replace(pos, count, sv.data(), sv.length());
		}

#if PLUGIFY_STRING_CONTAINERS_RANGES
		template<detail::container_compatible_range<Char> Range>
		constexpr iterator replace_with_range(const_iterator first, const_iterator last, Range&& range) 
		{
			auto str = basic_string(std::from_range, std::forward<Range>(range), _allocator);
			return replace(first, last, str);// replace checks for max_size()
		}
#endif

		[[nodiscard]] constexpr basic_string substr(size_type pos = 0, size_type count = npos) const 
		{
			_PLUGIFY_STRING_ASSERT(pos >= 0 && pos <= get_size(), "plg::basic_string::substr(): pos out of range", std::out_of_range);
			return basic_string(*this, pos, count);
		}

		constexpr size_type copy(value_type* str, size_type count, size_type pos = 0) const 
		{
			_PLUGIFY_STRING_ASSERT(pos >= 0 && pos <= get_size(), "plg::basic_string::copy(): pos out of range", std::out_of_range);
			return get_view().copy(str, count, pos);
		}

		constexpr void resize(size_type count, value_type ch) 
		{
			_PLUGIFY_STRING_ASSERT(get_size() + count <= max_size(), "plg::basic_string::resize(): resulted string size would exceed max_size()", std::length_error);
			auto cap = get_cap();
			auto sz = get_size();
			auto rsz = count + sz;

			if (sz < rsz) {
				if (cap < rsz)
					grow_to(rsz);
				Traits::assign(get_data() + sz, count, ch);
			}
			set_size(rsz);
			null_terminate();
		}

		constexpr void resize(size_type count) 
		{
			resize(count, _terminator);
		}

		template<typename Operation>
		constexpr void resize_and_overwrite(size_type, Operation) 
		{
			static_assert(detail::dependent_false<Char>, "plg::basic_string::resize_and_overwrite(count, op) not implemented!");
		}

		constexpr void swap(basic_string& other) noexcept(allocator_traits::propagate_on_container_swap::value || allocator_traits::is_always_equal::value) 
		{
			using std::ranges::swap;
			if constexpr (allocator_traits::propagate_on_container_swap::value) {
				swap(_allocator, other._allocator);
			}
			swap(_storage, other._storage);
		}

		[[nodiscard]] constexpr size_type find(const basic_string& str, size_type pos = 0) const noexcept 
		{
			return get_view().find(sview_type(str), pos);
		}

		[[nodiscard]] constexpr size_type find(const value_type* str, size_type pos, size_type count) const noexcept 
		{
			return get_view().find(str, pos, count);
		}

		[[nodiscard]] constexpr size_type find(const value_type* str, size_type pos = 0) const noexcept 
		{
			return get_view().find(str, pos);
		}

		[[nodiscard]] constexpr size_type find(value_type ch, size_type pos = 0) const noexcept 
		{
			return get_view().find(ch, pos);
		}

		template<typename Type>
			requires(std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		[[nodiscard]] constexpr size_type find(const Type& t, size_type pos = 0) const noexcept(std::is_nothrow_convertible_v<const Type&, sview_type>) 
		{
			return get_view().find(sview_type(t), pos);
		}

		[[nodiscard]] constexpr size_type rfind(const basic_string& str, size_type pos = npos) const noexcept 
		{
			return get_view().rfind(sview_type(str), pos);
		}

		[[nodiscard]] constexpr size_type rfind(const value_type* str, size_type pos, size_type count) const noexcept 
		{
			return get_view().rfind(str, pos, count);
		}

		[[nodiscard]] constexpr size_type rfind(const value_type* str, size_type pos = npos) const noexcept 
		{
			return get_view().rfind(str, pos);
		}

		[[nodiscard]] constexpr size_type rfind(value_type ch, size_type pos = npos) const noexcept
		{
			return get_view().rfind(ch, pos);
		}

		template<typename Type>
			requires(std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		[[nodiscard]] constexpr size_type rfind(const Type& t, size_type pos = npos) const noexcept(std::is_nothrow_convertible_v<const Type&, sview_type>)
		{
			return get_view().rfind(sview_type(t), pos);
		}

		[[nodiscard]] constexpr size_type find_first_of(const basic_string& str, size_type pos = 0) const noexcept 
		{
			return get_view().find_first_of(sview_type(str), pos);
		}

		[[nodiscard]] constexpr size_type find_first_of(const value_type* str, size_type pos, size_type count) const noexcept 
		{
			return get_view().find_first_of(str, pos, count);
		}

		[[nodiscard]] constexpr size_type find_first_of(const value_type* str, size_type pos = 0) const noexcept 
		{
			return get_view().find_first_of(str, pos);
		}

		[[nodiscard]] constexpr size_type find_first_of(value_type ch, size_type pos = 0) const noexcept 
		{
			return get_view().find_first_of(ch, pos);
		}

		template<typename Type>
			requires(std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		[[nodiscard]] constexpr size_type find_first_of(const Type& t, size_type pos = 0) const noexcept(std::is_nothrow_convertible_v<const Type&, sview_type>) 
		{
			return get_view().find_first_of(sview_type(t), pos);
		}

		[[nodiscard]] constexpr size_type find_first_not_of(const basic_string& str, size_type pos = 0) const noexcept 
		{
			return get_view().find_last_not_of(sview_type(str), pos);
		}

		[[nodiscard]] constexpr size_type find_first_not_of(const value_type* str, size_type pos, size_type count) const noexcept 
		{
			return get_view().find_last_not_of(str, pos, count);
		}

		[[nodiscard]] constexpr size_type find_first_not_of(const value_type* str, size_type pos = 0) const noexcept 
		{
			return get_view().find_last_not_of(str, pos);
		}

		[[nodiscard]] constexpr size_type find_first_not_of(value_type ch, size_type pos = 0) const noexcept 
		{
			return get_view().find_first_not_of(ch, pos);
		}

		template<typename Type>
			requires(std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		[[nodiscard]] constexpr size_type find_first_not_of(const Type& t, size_type pos = 0) const noexcept(std::is_nothrow_convertible_v<const Type&, sview_type>) 
		{
			return get_view().find_first_not_of(sview_type(t), pos);
		}

		[[nodiscard]] constexpr size_type find_last_of(const basic_string& str, size_type pos = npos) const noexcept 
		{
			return get_view().find_last_of(sview_type(str), pos);
		}

		[[nodiscard]] constexpr size_type find_last_of(const value_type* str, size_type pos, size_type count) const noexcept 
		{
			return get_view().find_last_of(str, pos, count);
		}

		[[nodiscard]] constexpr size_type find_last_of(const value_type* str, size_type pos = npos) const noexcept
		{
			return get_view().find_last_of(str, pos);
		}

		[[nodiscard]] constexpr size_type find_last_of(value_type ch, size_type pos = npos) const noexcept 
		{
			return get_view().find_last_of(ch, pos);
		}

		template<typename Type>
			requires(std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		[[nodiscard]] constexpr size_type find_last_of(const Type& t, size_type pos = npos) const noexcept(std::is_nothrow_convertible_v<const Type&, sview_type>) 
		{
			return get_view().find_last_of(sview_type(t), pos);
		}

		[[nodiscard]] constexpr size_type find_last_not_of(const basic_string& str, size_type pos = npos) const noexcept 
		{
			return get_view().find_last_not_of(sview_type(str), pos);
		}

		[[nodiscard]] constexpr size_type find_last_not_of(const value_type* str, size_type pos, size_type count) const noexcept 
		{
			return get_view().find_last_not_of(str, pos, count);
		}

		[[nodiscard]] constexpr size_type find_last_not_of(const value_type* str, size_type pos = npos) const noexcept 
		{
			return get_view().find_last_not_of(str, pos);
		}

		[[nodiscard]] constexpr size_type find_last_not_of(value_type ch, size_type pos = npos) const noexcept 
		{
			return get_view().find_last_not_of(ch, pos);
		}

		template<typename Type>
			requires(std::is_convertible_v<const Type&, sview_type> &&
					!std::is_convertible_v<const Type&, const Char*>)
		[[nodiscard]] constexpr size_type find_last_not_of(const Type& t, size_type pos = npos) const noexcept(std::is_nothrow_convertible_v<const Type&, sview_type>)
		{
			return get_view().find_last_not_of(sview_type(t), pos);
		}

		[[nodiscard]] friend constexpr basic_string operator+(const basic_string& lhs, const basic_string& rhs) 
		{
			auto lhs_sz = lhs.size();
			auto rhs_sz = rhs.size();
			basic_string ret(detail::uninitialized_size_tag(), lhs_sz + rhs_sz, basic_string::allocator_traits::select_on_container_copy_construction(lhs._allocator));
			auto buffer = ret.get_data();
			Traits::copy(buffer, lhs.data(), lhs_sz);
			Traits::copy(buffer + lhs_sz, rhs.data(), rhs_sz);
			ret.null_terminate();
			return ret;
		}

		[[nodiscard]] friend constexpr basic_string operator+(basic_string&& lhs, const basic_string& rhs)
		{
			return std::move(lhs.append(rhs));
		}

		[[nodiscard]] friend constexpr basic_string operator+(const basic_string& lhs, basic_string&& rhs)
		{
			return std::move(rhs.insert(0, lhs));
		}

		[[nodiscard]] friend constexpr basic_string operator+(basic_string&& lhs, basic_string&& rhs) 
		{
			return std::move(lhs.append(rhs));
		}

		[[nodiscard]] friend constexpr basic_string operator+(const Char* lhs, const basic_string& rhs) 
		{
			auto lhs_sz = Traits::length(lhs);
			auto rhs_sz = rhs.size();
			basic_string ret(detail::uninitialized_size_tag(), lhs_sz + rhs_sz, basic_string::allocator_traits::select_on_container_copy_construction(rhs._allocator));
			auto buffer = ret.get_data();
			Traits::copy(buffer, lhs, lhs_sz);
			Traits::copy(buffer + lhs_sz, rhs.data(), rhs_sz);
			ret.null_terminate();
			return ret;
		}

		[[nodiscard]] friend constexpr basic_string operator+(const Char* lhs, basic_string&& rhs)
		{
			return std::move(rhs.insert(0, lhs));
		}

		[[nodiscard]] friend constexpr basic_string operator+(Char lhs, const basic_string& rhs)
		{
			auto rhs_sz = rhs.size();
			basic_string ret(detail::uninitialized_size_tag(), rhs_sz + 1, basic_string::allocator_traits::select_on_container_copy_construction(rhs._allocator));
			auto buffer = ret.get_data();
			Traits::assign(buffer, 1, lhs);
			Traits::copy(buffer + 1, rhs.data(), rhs_sz);
			ret.null_terminate();
			return ret;
		}

		
		[[nodiscard]] friend constexpr basic_string operator+(Char lhs, basic_string&& rhs) 
		{
			rhs.insert(rhs.begin(), lhs);
			return std::move(rhs);
		}

		[[nodiscard]] friend constexpr basic_string operator+(const basic_string& lhs, const Char* rhs)
		{
			auto lhs_sz = lhs.size();
			auto rhs_sz = Traits::length(rhs);
			basic_string ret(detail::uninitialized_size_tag(), lhs_sz + rhs_sz, basic_string::allocator_traits::select_on_container_copy_construction(lhs._allocator));
			auto buffer = ret.get_data();
			Traits::copy(buffer, lhs.data(), lhs_sz);
			Traits::copy(buffer + lhs_sz, rhs, rhs_sz);
			ret.null_terminate();
			return ret;
		}

		[[nodiscard]] friend constexpr basic_string operator+(basic_string&& lhs, const Char* rhs) 
		{
			return std::move(lhs.append(rhs));
		}

		[[nodiscard]] friend constexpr basic_string operator+(const basic_string& lhs, Char rhs) 
		{
			auto lhs_sz = lhs.size();
			basic_string ret(detail::uninitialized_size_tag(), lhs_sz + 1, basic_string::allocator_traits::select_on_container_copy_construction(lhs._allocator));
			auto buffer = ret.get_data();
			Traits::copy(buffer, lhs.data(), lhs_sz);
			Traits::assign(buffer + lhs_sz, 1, rhs);
			ret.null_terminate();
			return ret;
		}

		[[nodiscard]] friend constexpr basic_string operator+(basic_string&& lhs, Char rhs)
		{
			lhs.push_back(rhs);
			return std::move(lhs);
		}
	};

	template<typename Char, typename Traits, typename Allocator>
	[[nodiscard]] constexpr bool operator==(const basic_string<Char, Traits, Allocator>& lhs, const basic_string<Char, Traits, Allocator>& rhs) noexcept 
	{
		return lhs.compare(rhs) == 0;
	}

	template<typename Char, typename Traits, typename Allocator>
	[[nodiscard]] constexpr bool operator==(const basic_string<Char, Traits, Allocator>& lhs, const Char* rhs)
	{
		return lhs.compare(rhs) == 0;
	}

	template<typename Char, typename Traits, typename Allocator>
	[[nodiscard]] constexpr std::strong_ordering operator<=>(const basic_string<Char, Traits, Allocator>& lhs, const basic_string<Char, Traits, Allocator>& rhs) noexcept 
	{
		return lhs.compare(rhs) <=> 0;
	}

	template<typename Char, typename Traits, typename Allocator>
	[[nodiscard]] constexpr std::strong_ordering operator<=>(const basic_string<Char, Traits, Allocator>& lhs, const Char* rhs) 
	{
		return lhs.compare(rhs) <=> 0;
	}

	// swap
	template<typename Char, typename Traits, typename Allocator>
	constexpr void swap(basic_string<Char, Traits, Allocator>& lhs, basic_string<Char, Traits, Allocator>& rhs) noexcept(noexcept(lhs.swap(rhs)))
	{
		lhs.swap(rhs);
	}

	// erasure
	template<typename Char, typename Traits, typename Allocator, typename U>
	constexpr typename basic_string<Char, Traits, Allocator>::size_type erase(basic_string<Char, Traits, Allocator>& c, const U& value) 
	{
		auto it = std::remove(c.begin(), c.end(), value);
		auto r = std::distance(it, c.end());
		c.erase(it, c.end());
		return r;
	}

	template<typename Char, typename Traits, typename Allocator, typename Pred>
	constexpr typename basic_string<Char, Traits, Allocator>::size_type erase_if(basic_string<Char, Traits, Allocator>& c, Pred pred) 
	{
		auto it = std::remove_if(c.begin(), c.end(), pred);
		auto r = std::distance(it, c.end());
		c.erase(it, c.end());
		return r;
	}

	// deduction guides
	template<typename InputIterator, typename Allocator = std::allocator<typename std::iterator_traits<InputIterator>::value_type>>
	basic_string(InputIterator, InputIterator, Allocator = Allocator()) -> basic_string<typename std::iterator_traits<InputIterator>::value_type, std::char_traits<typename std::iterator_traits<InputIterator>::value_type>, Allocator>;

	template<typename Char, typename Traits, typename Allocator = std::allocator<Char>>
	explicit basic_string(std::basic_string_view<Char, Traits>, const Allocator& = Allocator()) -> basic_string<Char, Traits, Allocator>;

	template<typename Char, typename Traits, typename Allocator = std::allocator<Char>>
	basic_string(std::basic_string_view<Char, Traits>, typename basic_string<Char, Traits, Allocator>::size_type, typename basic_string<Char, Traits, Allocator>::size_type, const Allocator& = Allocator()) -> basic_string<Char, Traits, Allocator>;

#if PLUGIFY_STRING_CONTAINERS_RANGES
	template<std::ranges::input_range Range, typename Allocator = std::allocator<std::ranges::range_value_t<Range>>>
	basic_string(std::from_range_t, Range&&, Allocator = Allocator()) -> basic_string<std::ranges::range_value_t<Range>, std::char_traits<std::ranges::range_value_t<Range>>, Allocator>;
#endif

	// basic_string typedef-names
	using string = basic_string<char>;
	using u8string = basic_string<char8_t>;
	using u16string = basic_string<char16_t>;
	using u32string = basic_string<char32_t>;
	using wstring = basic_string<wchar_t>;

#if PLUGIFY_STRING_NUMERIC_CONVERSIONS
	// numeric conversions
	[[nodiscard]] inline int stoi(const string& str, std::size_t* pos = nullptr, int base = 10) 
	{
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtol(cstr, &ptr, base);
		if (pos != nullptr)
			*pos = static_cast<size_t>(cstr - ptr);

		return static_cast<int>(ret);
	}

	[[nodiscard]] inline long stol(const string& str, std::size_t* pos = nullptr, int base = 10) 
	{
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtol(cstr, &ptr, base);
		if (pos != nullptr)
			*pos = static_cast<size_t>(cstr - ptr);

		return ret;
	}

	[[nodiscard]] inline long long stoll(const string& str, std::size_t* pos = nullptr, int base = 10)
	{
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtoll(cstr, &ptr, base);
		if (pos != nullptr)
			*pos = static_cast<size_t>(cstr - ptr);

		return ret;
	}

	[[nodiscard]] inline unsigned long stoul(const string& str, std::size_t* pos = nullptr, int base = 10)
	{
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtoul(cstr, &ptr, base);
		if (pos != nullptr)
			*pos = static_cast<size_t>(cstr - ptr);

		return ret;
	}

	[[nodiscard]] inline unsigned long long stoull(const string& str, std::size_t* pos = nullptr, int base = 10) 
	{
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtoull(cstr, &ptr, base);
		if (pos != nullptr)
			*pos = static_cast<size_t>(cstr - ptr);

		return ret;
	}

#if PLUGIFY_STRING_FLOAT
	[[nodiscard]] inline float stof(const string& str, std::size_t* pos = nullptr)
	{
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtof(cstr, &ptr);
		if (pos != nullptr)
			*pos = static_cast<size_t>(cstr - ptr);

		return ret;
	}

	[[nodiscard]] inline double stod(const string& str, std::size_t* pos = nullptr)
	{
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtod(cstr, &ptr);
		if (pos != nullptr)
			*pos = static_cast<size_t>(cstr - ptr);

		return ret;
	}

#if PLUGIFY_STRING_LONG_DOUBLE
	[[nodiscard]] inline long double stold(const string& str, std::size_t* pos = nullptr) 
	{
		auto cstr = str.c_str();
		char* ptr = const_cast<char*>(cstr);

		auto ret = strtold(cstr, &ptr);
		if (pos != nullptr)
			*pos = static_cast<size_t>(cstr - ptr);

		return ret;
	}
#endif
#endif

	namespace detail {
		template<typename S, typename V>
		constexpr _PLUGIFY_STRING_ALWAYS_INLINE S to_string(V v)
		{
			//  numeric_limits::digits10 returns value less on 1 than desired for unsigned numbers.
			//  For example, for 1-byte unsigned value digits10 is 2 (999 can not be represented),
			//  so we need +1 here.
			constexpr size_t bufSize = std::numeric_limits<V>::digits10 + 2; // +1 for minus, +1 for digits10
			char buf[bufSize];
			const auto res = std::to_chars(buf, buf + bufSize, v);
			return S(buf, res.ptr);
		}

		typedef int (*wide_printf)(wchar_t* __restrict, size_t, const wchar_t* __restrict, ...);

#if defined(_MSC_VER)
		inline int truncate_snwprintf(wchar_t* __restrict buffer, size_t count, const wchar_t* __restrict format, ...) 
		{
			int r;
			va_list args;
			va_start(args, format);
			r = _vsnwprintf_s(buffer, count, _TRUNCATE, format, args);
			va_end(args);
			return r;
		}
#endif

		constexpr _PLUGIFY_STRING_ALWAYS_INLINE wide_printf get_swprintf() noexcept
		{
#if defined(_MSC_VER)
			return static_cast<int(__cdecl*)(wchar_t* __restrict, size_t, const wchar_t* __restrict, ...)>(truncate_snwprintf);
#else
			return swprintf;
#endif
		}

		template<typename S, typename P, typename V>
		constexpr _PLUGIFY_STRING_ALWAYS_INLINE S as_string(P sprintf_like, const typename S::value_type* fmt, V v)
		{
			typedef typename S::size_type size_type;
			S s;
			s.resize(s.capacity());
			size_type available = s.size();
			while (true) {
				int status = sprintf_like(&s[0], available + 1, fmt, v);
				if (status >= 0) {
					auto used = static_cast<size_type>(status);
					if (used <= available) {
						s.resize(used);
						break;
					}
					available = used; // Assume this is advice of how much space we need.
				} else {
					available = available * 2 + 1;
				}
				s.resize(available);
			}
			return s;
		}
	}// namespace detail

	[[nodiscard]] inline string to_string(int val) { return detail::to_string<string>(val); }
	[[nodiscard]] inline string to_string(unsigned val) { return detail::to_string<string>(val); }
	[[nodiscard]] inline string to_string(long val) { return detail::to_string<string>(val); }
	[[nodiscard]] inline string to_string(unsigned long val) { return detail::to_string<string>(val); }
	[[nodiscard]] inline string to_string(long long val) { return detail::to_string<string>(val); }
	[[nodiscard]] inline string to_string(unsigned long long val) { return detail::to_string<string>(val); }

#if PLUGIFY_STRING_FLOAT
	[[nodiscard]] inline string to_string(float val) { return detail::as_string<string>(snprintf, "%f", val); }
	[[nodiscard]] inline string to_string(double val) { return detail::as_string<string>(snprintf, "%f", val); }

#if PLUGIFY_STRING_LONG_DOUBLE
	[[nodiscard]] inline string to_string(long double val) { return detail::as_string<string>(snprintf, "%Lf", val); }
#endif
#endif

	[[nodiscard]] inline wstring to_wstring(int val) { return detail::to_string<wstring>(val); }
	[[nodiscard]] inline wstring to_wstring(unsigned val) { return detail::to_string<wstring>(val); }
	[[nodiscard]] inline wstring to_wstring(long val) { return detail::to_string<wstring>(val); }
	[[nodiscard]] inline wstring to_wstring(unsigned long val) { return detail::to_string<wstring>(val); }
	[[nodiscard]] inline wstring to_wstring(long long val) { return detail::to_string<wstring>(val); }
	[[nodiscard]] inline wstring to_wstring(unsigned long long val) { return detail::to_string<wstring>(val); }

#if PLUGIFY_STRING_FLOAT
	[[nodiscard]] inline wstring to_wstring(float val) { return detail::as_string<wstring>(detail::get_swprintf(), L"%f", val); }
	[[nodiscard]] inline wstring to_wstring(double val) { return detail::as_string<wstring>(detail::get_swprintf(), L"%f", val); }

#if PLUGIFY_STRING_LONG_DOUBLE
	[[nodiscard]] inline wstring to_wstring(long double val) { return detail::as_string<wstring>(detail::get_swprintf(), L"%Lf", val); }
#endif
#endif
#endif

#if PLUGIFY_STRING_STD_HASH
	// hash support
	namespace detail {
		template<typename Char, typename Allocator, typename String = basic_string<Char, std::char_traits<Char>, Allocator>>
		struct string_hash_base {
			[[nodiscard]] constexpr std::size_t operator()(const String& str) const noexcept
			{
				return std::hash<typename String::sview_type>{}(typename String::sview_type(str));
			}
		};
	}// namespace detail
#endif

#if PLUGIFY_STRING_STD_FORMAT
	// format support
	namespace detail {
		template<typename Char>
		static constexpr const Char* format_string() {
			if constexpr (std::is_same_v<Char, char> || std::is_same_v<Char, char8_t>)
				return "{}";
			if constexpr (std::is_same_v<Char, wchar_t>)
				return L"{}";
			if constexpr (std::is_same_v<Char, char16_t>)
				return u"{}";
			if constexpr (std::is_same_v<Char, char32_t>)
				return U"{}";
		}

		template<typename Char, typename Allocator, typename String = basic_string<Char, std::char_traits<Char>, Allocator>>
		struct string_formatter_base {
			constexpr auto parse(std::format_parse_context& ctx)
			{
				return ctx.begin();
			}

			template<class FormatContext>
			auto format(const String& str, FormatContext& ctx) const
			{
				return std::format_to(ctx.out(), format_string<Char>(), str.c_str());
			}
		};
	}
#endif

	inline namespace literals {
		inline namespace string_literals {
			_PLUGIFY_STRING_WARN_PUSH()

#if defined(__clang__)
			_PLUGIFY_STRING_WARN_IGNORE("-Wuser-defined-literals")
#elif defined(__GNUC__)
			_PLUGIFY_STRING_WARN_IGNORE("-Wliteral-suffix")
#elif defined(_MSC_VER)
			_PLUGIFY_STRING_WARN_IGNORE(4455)
#endif
			// suffix for basic_string literals
			[[nodiscard]] constexpr string operator""s(const char* str, std::size_t len) { return string{str, len}; }
			[[nodiscard]] constexpr u8string operator""s(const char8_t* str, std::size_t len) { return u8string{str, len}; }
			[[nodiscard]] constexpr u16string operator""s(const char16_t* str, std::size_t len) { return u16string{str, len}; }
			[[nodiscard]] constexpr u32string operator""s(const char32_t* str, std::size_t len) { return u32string{str, len}; }
			[[nodiscard]] constexpr wstring operator""s(const wchar_t* str, std::size_t len) { return wstring{str, len}; }

			_PLUGIFY_STRING_WARN_POP()
		}// namespace string_literals
	}// namespace literals
}// namespace plg

#if PLUGIFY_STRING_STD_HASH
// hash support
namespace std {
	template<typename Allocator>
	struct hash<plg::basic_string<char, std::char_traits<char>, Allocator>> : plg::detail::string_hash_base<char, Allocator> {};

	template<typename Allocator>
	struct hash<plg::basic_string<char8_t, std::char_traits<char8_t>, Allocator>> : plg::detail::string_hash_base<char8_t, Allocator> {};

	template<typename Allocator>
	struct hash<plg::basic_string<char16_t, std::char_traits<char16_t>, Allocator>> : plg::detail::string_hash_base<char16_t, Allocator> {};

	template<typename Allocator>
	struct hash<plg::basic_string<char32_t, std::char_traits<char32_t>, Allocator>> : plg::detail::string_hash_base<char32_t, Allocator> {};

	template<typename Allocator>
	struct hash<plg::basic_string<wchar_t, std::char_traits<wchar_t>, Allocator>> : plg::detail::string_hash_base<wchar_t, Allocator> {};
}// namespace std
#endif

#if PLUGIFY_STRING_STD_FORMAT
// format support
#ifdef FMT_HEADER_ONLY
namespace fmt {
#else
namespace std {
#endif
	template<typename Allocator>
	struct formatter<plg::basic_string<char, std::char_traits<char>, Allocator>> : plg::detail::string_formatter_base<char, Allocator> {};

	template<typename Allocator>
	struct formatter<plg::basic_string<char8_t, std::char_traits<char8_t>, Allocator>> : plg::detail::string_formatter_base<char8_t, Allocator> {};

	template<typename Allocator>
	struct formatter<plg::basic_string<char16_t, std::char_traits<char16_t>, Allocator>> : plg::detail::string_formatter_base<char16_t, Allocator> {};

	template<typename Allocator>
	struct formatter<plg::basic_string<char32_t, std::char_traits<char32_t>, Allocator>> : plg::detail::string_formatter_base<char32_t, Allocator> {};

	template<typename Allocator>
	struct formatter<plg::basic_string<wchar_t, std::char_traits<wchar_t>, Allocator>> : plg::detail::string_formatter_base<wchar_t, Allocator> {};
}// namespace std
#endif
