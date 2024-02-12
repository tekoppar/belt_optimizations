#pragma once

#include <utility>

namespace expr
{
	template<typename type>
	inline constexpr type abs(type f) noexcept
	{
		if (f == -0)
			return 0;

		return f < 0 ? -f : f;
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
	constexpr div_rem<long long> divide_with_remainder(long long i, long long division)
	{
		long long tmp = i;
		long long shift = division / static_cast<long long>(2);
		long long div = shift % 2 == 0 ? tmp >> shift : tmp / division;
		if (div * division == i) return { div };
		else return { div, i - (div * division) };
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
	constexpr auto tmp = divide_with_remainder<2048ll, 3ll>();
};