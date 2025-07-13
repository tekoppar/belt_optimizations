#pragma once

#include "vectors.h"

namespace belt_utility
{
	enum class belt_direction
	{
		null = -1,
		left_right,
		right_left,
		top_bottom,
		bottom_top
	};
	constexpr static vec2 belt_direction_multi[4]{ vec2{1.0f, 0.0f}, vec2{-1.0f, 0.0f}, vec2{0.0f, 1.0f}, vec2{0.0f, -1.0f} };

	/*
	//	0	1	2
	//	3		4
	//	5	6	7
	*/
	enum class belt_neighbour
	{
		top_l,
		top_m,
		top_r,
		left,
		right,
		bot_l,
		bot_m,
		bot_r
	};

	enum class inserter_fits_results
	{
		no_fit,
		before,
		after,
		inbetween
	};

	enum class belt_update_mode : unsigned char
	{
		free,
		first_stuck,
		all_stuck
	};

	struct neighbour_to_direction_value
	{
		belt_neighbour x;
		belt_direction y;
	};

	constexpr neighbour_to_direction_value neighbour_to_direction[8]{
		{belt_neighbour::top_l, belt_direction::null }, {belt_neighbour::top_m, belt_direction::bottom_top }, {belt_neighbour::top_r, belt_direction::null },
		{belt_neighbour::left, belt_direction::right_left },													{belt_neighbour::right, belt_direction::left_right },
		{belt_neighbour::bot_l, belt_direction::null }, {belt_neighbour::bot_l, belt_direction::top_bottom }, {belt_neighbour::bot_l, belt_direction::null }
	};

	struct direction_to_neighbour_value
	{
		belt_direction x;
		belt_neighbour y;
	};

	constexpr direction_to_neighbour_value direction_to_neighbour[4]{
		{belt_direction::bottom_top, belt_neighbour::top_m }, {belt_direction::right_left, belt_neighbour::left }, {belt_direction::left_right, belt_neighbour::right }, {belt_direction::top_bottom, belt_neighbour::bot_l }
	};

	constexpr belt_direction get_neighbour_to_direction(const belt_neighbour& val)
	{
		constexpr long long l = 4;
		for (long long i = 0; i < l; ++i)
		{
			if (direction_to_neighbour[i].y == val) return direction_to_neighbour[i].x;
		}
	};

	//pixels per frame
	enum belt_speed
	{
		slow = 1,
		normal = 2,
		fast = 3
	};

	enum class belt_color
	{
		gray = 1,
		yellow = 2,
		red = 3
	};

	enum class belt_side
	{
		top,
		bottom
	};

	struct reserved_item_data
	{
		short reserved_index{ -1 };
		short reserved_item_count{ -1 };
	};

	enum class find_closest_item_group_return_result
	{
		insert_into_group,
		new_group_before_iter,
		new_group_after_iter,
		invalid_value
	};

	template<typename object>
	struct find_closest_item_group_result
	{
		find_closest_item_group_return_result scan{ find_closest_item_group_return_result::invalid_value };
		object result = object{};
	};

	enum class find_closest_active_mode_return_result
	{
		inbetween,
		new_active_mode_before_iter,
		new_active_mode_after_iter,
		invalid_value
	};

	template<typename object>
	struct find_closest_active_mode_result
	{
		find_closest_active_mode_return_result scan{ find_closest_active_mode_return_result::invalid_value };
		object result = object{};
	};

	enum class distance_comparison
	{
		null = -1,
		distance_is_before = 0,
		distance_is_after = 1,
		distance_is_inside = 2
	};
	enum class need_new_slot_result
	{
		update_pointer_to_new_index = 0,
		object_is_between_slots = 1,
		need_new_slot = 2
	};

	enum class item_update_state
	{
		adding,
		removing
	};

	struct add_inserter_return_indexes
	{
		long long index{ 0 };
		long long nested_index{ 0 };
	};

	enum class distance_slot_inserted_position
	{
		new_item_after_last_goal = 0,
		new_item_after_iter = 1,
		new_item_before_iter = 2
	};
};