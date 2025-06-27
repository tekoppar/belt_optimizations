#pragma once

#include <utility>
#include <type_traits>

namespace expr
{
	template<typename type>
	inline constexpr bool is_even(type lhs) noexcept
		requires(std::is_integral_v<type> && std::is_unsigned_v<type>)
	{
		return (static_cast<unsigned long long>(lhs) % 2ull) == 0ull;
	};
	template<typename type>
	inline constexpr bool is_even(type lhs) noexcept
		requires(std::is_integral_v<type>&& std::is_signed_v<type>)
	{
		return (static_cast<long long>(lhs) % 2ll) == 0ll;
	};
	template<typename type>
	inline constexpr bool is_odd(type lhs) noexcept
		requires(std::is_integral_v<type>&& std::is_unsigned_v<type>)
	{
		return !expr::is_even<type>(lhs);
	};
	template<typename type>
	inline constexpr bool is_odd(type lhs) noexcept
		requires(std::is_integral_v<type>&& std::is_signed_v<type>)
	{
		return !expr::is_even<type>(lhs);
	};
	static_assert(is_even(13ll) == false);
	static_assert(!(is_even(15ll) == true));
	static_assert(is_even(12ll) == true);

	template<typename type>
	inline constexpr type abs(type f) noexcept
	{
		if (f == -0)
			return 0;

		return f < 0 ? -f : f;
	};
	template<typename type>
	inline constexpr type negate(type f) noexcept
	{
		if (f == 0)
			return -0;

		return f > 0 ? -f : f;
	};

	template<typename type>
	inline constexpr type max(type lhs, type rhs) noexcept
	{
		return lhs >= rhs ? lhs : rhs;
	};
	template<typename type>
	inline constexpr type max(type lhs, type rhs, type compare) noexcept
	{
		return lhs >= compare ? rhs : lhs;
	};
	static_assert(max(25, 5, 15) == 5, "no");

	template<typename type>
	inline constexpr type min(type lhs, type rhs) noexcept
	{
		return lhs < rhs ? lhs : rhs;
	};
	template<typename type>
	inline constexpr type min(type lhs, type rhs, type compare) noexcept
	{
		return lhs < compare ? rhs : lhs;
	};

	inline constexpr auto round_div(auto lhs, auto rhs) noexcept
		requires(std::is_integral_v<decltype(lhs)> && std::is_integral_v<decltype(rhs)>)
	{
		const double cast = static_cast<double>(lhs) / rhs;
		return cast > lhs / rhs + 0.5 ? static_cast<decltype(lhs)>(lhs / rhs + 1.0) : static_cast<decltype(lhs)>(lhs / rhs);
	};
	static_assert(round_div(258ll, 8ll) == 32ll);
	static_assert(round_div(261ll, 8ll) == 33ll);
	static_assert(round_div(260ll, 8ll) != 33ll);
	static_assert(round_div(260ll, 4ll) == 65ll);

	inline constexpr auto floor_div(auto lhs, auto rhs) noexcept
		requires(std::is_integral_v<decltype(lhs)>&& std::is_integral_v<decltype(rhs)>)
	{
		return static_cast<decltype(lhs)>(lhs / rhs);
	};
	static_assert(floor_div(64ll, 13ll) == 4ll);
	static_assert(floor_div(64ll, 12ll) == 5ll);

	inline constexpr auto ceil_div(auto lhs, auto rhs) noexcept
		requires(std::is_integral_v<decltype(lhs)>&& std::is_integral_v<decltype(rhs)>)
	{
		if (static_cast<decltype(lhs)>(lhs / rhs) == static_cast<double>(lhs) / static_cast<double>(rhs))
			return static_cast<decltype(lhs)>(lhs / rhs);
		else
			return static_cast<decltype(lhs)>(lhs / rhs) + 1ll;
	};
	static_assert(ceil_div(5ll, 2ll) == 3ll);
	static_assert(ceil_div(24ll, 2ll) == 12ll);

	inline constexpr auto ceil_div_power2(auto lhs) noexcept
		requires(std::is_integral_v<decltype(lhs)>)
	{
		return lhs + 1 >> 1;
	};
	static_assert(ceil_div_power2(5ll) == 3ll);
	static_assert(ceil_div_power2(24ll) == 12ll);

	template<typename type_size>
	struct div_rem
	{
		type_size div;
		type_size rem;
	};
	template<typename type_size>
	constexpr div_rem<type_size> divide_with_remainder(type_size i, type_size division)
		requires(std::is_integral_v<type_size>)
	{
		type_size tmp = i;
		type_size div = tmp >> (division / static_cast<type_size>(2));
		if (div * division == i) return { div };
		else return { div, i - (div * division) };
	};
	constexpr div_rem<long long> divide_with_remainder(long long i, const long long division)
	{
		const long long tmp = i;
		const long long tmp_div = division;
		const double double_div = (double)tmp / (double)tmp_div;
		const long long div = double_div == (long long)double_div ? double_div : (long long)double_div;
		if (div * tmp_div == i) return { div };
		else return { div, i - (div * tmp_div) };
	};
	template<long long i, long long division>
	constexpr div_rem<long long> divide_with_remainder()
	{
		constexpr long long tmp = i;
		constexpr long long shift = division / static_cast<long long>(2);
		constexpr long long div = shift % 2 == 0 ? tmp >> shift : tmp / division;
		if constexpr (div * division == i) return { div };
		else return { div, i - (div * division) };
	};
	static_assert(divide_with_remainder(50ll, 4ll).div + divide_with_remainder(50ll, 4ll).rem == 14ll);
	static_assert(divide_with_remainder(256ll, 4ll).div == 256ll / 4ll);

	constexpr div_rem<long long> multiply_division_with_remainder(const long long i, const long long multi, const long long division)
	{
		const long long tmp = i;
		const long long tmp_div = division;
		const long long tmp_mul = multi;
		const double double_div = (double)tmp / (double)tmp_div;
		const double double_mul = double_div * (double)tmp_mul;
		const long long div = double_div == (long long)double_div ? double_div : (long long)double_div;
		if (div * tmp_div * tmp_mul == double_mul) return { div * tmp_div };
		else return { div * tmp_div * tmp_mul, ((long long)tmp * tmp_mul) - (div * tmp_div * tmp_mul) };
	};
	static_assert(multiply_division_with_remainder(114102ll, 7ll, 4ll).div == 798700ll);
	static_assert(multiply_division_with_remainder(114102ll, 7ll, 4ll).rem == 14ll);
};