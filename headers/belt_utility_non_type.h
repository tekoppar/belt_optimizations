#pragma once

#include <vectors.h>
#include <belt_utility_data.h>

namespace belt_utility
{
	template<typename lambda, typename... params>
	concept is_lambda = requires(lambda l, params... values)
	{
		{
			l(values...)
		};
	};

	template<typename _vector, typename U, typename compare_lambda>
	constexpr bool contains(_vector& v, U* p, const compare_lambda& comp) noexcept
		requires(is_lambda<compare_lambda, U*, U*>)
	{
		const long long l(v.size());
		for (long long i = 0; i < l; i++)
		{
			if (comp(v[i], p)) return true;
		}
		return false;
	};
	template<typename _vector, typename U, typename compare_lambda>
	constexpr long long find_index(_vector& v, U* p, const compare_lambda& comp) noexcept
		requires(is_lambda<compare_lambda, U*, U*>)
	{
		const long long l(v.size());
		for (long long i = 0; i < l; i++) if (comp(v[i], p)) return i;
		return -1ll;
	};

	constexpr belt_neighbour get_neighbour_to_direction(const belt_direction& val)
	{
		constexpr long long l = 8;
		for (long long i = 0; i < l; ++i)
		{
			if (neighbour_to_direction[i].y == val) return neighbour_to_direction[i].x;
		}
	};

	template<belt_direction direction>
	inline static constexpr long long get_direction_position(vec2_int64 pos) noexcept
	{
		if constexpr (belt_direction::left_right == direction) return pos.x;
		if constexpr (belt_direction::right_left == direction) return pos.x;
		if constexpr (belt_direction::top_bottom == direction) return pos.y;
		if constexpr (belt_direction::bottom_top == direction) return pos.y;
	};
	template<belt_direction direction>
	inline static constexpr long long get_offset_position(vec2_int64 pos) noexcept
	{
		if constexpr (belt_direction::left_right == direction) return pos.y;
		if constexpr (belt_direction::right_left == direction) return pos.y;
		if constexpr (belt_direction::top_bottom == direction) return pos.x;
		if constexpr (belt_direction::bottom_top == direction) return pos.x;
	};
	inline static constexpr long long get_direction_position(const belt_direction direction, vec2_int64 pos) noexcept
	{
		if (belt_direction::left_right == direction) return pos.x;
		if (belt_direction::right_left == direction) return pos.x;
		if (belt_direction::top_bottom == direction) return pos.y;
		if (belt_direction::bottom_top == direction) return pos.y;
	};

	static consteval belt_direction direction_from_positions(vec2_int64 start, vec2_int64 end) noexcept
	{
		if (start.x != end.x)
		{
			if (start.x < end.x) return belt_direction::left_right;
			else return belt_direction::right_left;
		}
		else
		{
			if (start.y < end.y) return belt_direction::top_bottom;
			else return belt_direction::bottom_top;
		}
	};
};