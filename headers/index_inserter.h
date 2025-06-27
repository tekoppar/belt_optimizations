#pragma once

#include <type_traits>
#include <utility>

#include "const_data.h"

#include "vectors.h"
#include "index_iterator.h"
#include "item.h"
#include "belt_utility_data.h"
#include "shared_classes.h"
#include "belt_utility.h"

class belt_segment;

class index_inserter
{
public:
	static inline constexpr long long inserter_grid_size{ 32ll };
	static inline long long grabbed_items{ 0ll };

private:
	vec2_int64 position{ 0, 0 };
	item_type item_need_types[8]{ item_type::pink_square, item_type::pink_square, item_type::pink_square, item_type::pink_square,
	item_type::pink_square, item_type::pink_square, item_type::pink_square, item_type::pink_square };
	__declspec(align(32)) item_uint item;
	long long distance_position_plus{ -1ll };
	long long distance_position_minus{ -1ll };
#ifdef _DEBUG
public:
	long long loop_count = 0ll;
	long long missed_grabs = 0ll;
	long long local_grabbed_items = 0ll;
	long long no_item_found = 0ll;
	long long wrong_goal_pointer_frame_count = 0ll;
	long long wrong_goal_pointer_updated = 0ll;
#endif

public:
	constexpr index_inserter() noexcept
	{};
	constexpr index_inserter(vec2_int64 pos) noexcept :
		position{ pos }
	{};
	constexpr ~index_inserter() noexcept
	{};

	constexpr index_inserter(const index_inserter& o) noexcept :
		position{ o.position },
		item{ o.item },
		distance_position_plus{ o.distance_position_plus },
		distance_position_minus{ o.distance_position_minus }
#ifdef _DEBUG
		,
		loop_count{ o.loop_count },
		missed_grabs{ o.missed_grabs },
		local_grabbed_items{ o.local_grabbed_items },
		no_item_found{ o.no_item_found },
		wrong_goal_pointer_frame_count{ o.wrong_goal_pointer_frame_count },
		wrong_goal_pointer_updated{ o.wrong_goal_pointer_updated }
#endif
	{
		if (std::is_constant_evaluated())
		{
			constexpr long long l = 8;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = o.item_need_types[i];
			}
		}
		else std::memcpy(&item_need_types[0], &o.item_need_types[0], 16);
	};
	constexpr index_inserter(index_inserter&& o) noexcept :
		position{ std::exchange(o.position, vec2_int64{}) },
		item{ std::exchange(o.item, item_uint{}) },
		distance_position_plus{ std::exchange(o.distance_position_plus, -1ll) },
		distance_position_minus{ std::exchange(o.distance_position_minus, -1ll) }
#ifdef _DEBUG
		,
		loop_count{ o.loop_count },
		missed_grabs{ o.missed_grabs },
		local_grabbed_items{ o.local_grabbed_items },
		no_item_found{ o.no_item_found },
		wrong_goal_pointer_frame_count{ o.wrong_goal_pointer_frame_count },
		wrong_goal_pointer_updated{ o.wrong_goal_pointer_updated }
#endif
	{
		if (std::is_constant_evaluated())
		{
			constexpr long long l = 8;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = o.item_need_types[i];
			}
		}
		else std::memcpy(&item_need_types[0], &o.item_need_types[0], 16);
	};
	constexpr index_inserter& operator=(const index_inserter& o) noexcept
	{
		position = o.position;
		item = o.item;
		distance_position_plus = o.distance_position_plus;
		distance_position_minus = o.distance_position_minus;
#ifdef _DEBUG
		loop_count = o.loop_count;
		missed_grabs = o.missed_grabs;
		local_grabbed_items = o.local_grabbed_items;
		no_item_found = o.no_item_found;
		wrong_goal_pointer_frame_count = o.wrong_goal_pointer_frame_count;
		wrong_goal_pointer_updated = o.wrong_goal_pointer_updated;
#endif

		if (std::is_constant_evaluated())
		{
			constexpr long long l = 8;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = o.item_need_types[i];
			}
		}
		else std::memcpy(&item_need_types[0], &o.item_need_types[0], 16);

		return *this;
	};
	constexpr index_inserter& operator=(index_inserter&& o) noexcept
	{
		position = std::exchange(o.position, vec2_int64{});
		item = std::exchange(o.item, item_uint{});
		distance_position_plus = std::exchange(o.distance_position_plus, -1ll);
		distance_position_minus = std::exchange(o.distance_position_minus, -1ll);
#ifdef _DEBUG
		loop_count = o.loop_count;
		missed_grabs = o.missed_grabs;
		local_grabbed_items = o.local_grabbed_items;
		no_item_found = o.no_item_found;
		wrong_goal_pointer_frame_count = o.wrong_goal_pointer_frame_count;
		wrong_goal_pointer_updated = o.wrong_goal_pointer_updated;
#endif

		if (std::is_constant_evaluated())
		{
			constexpr long long l = 8;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = std::move(o.item_need_types[i]);
			}
		}
		else std::memcpy(&item_need_types[0], &o.item_need_types[0], 16);

		return *this;
	};

	friend constexpr bool operator==(const index_inserter& lhs, const index_inserter& rhs) noexcept
	{
		return lhs.position == rhs.position && lhs.item == rhs.item;
	};

	inline constexpr vec2_int64 get_position() const noexcept
	{
		return position;
	};
	inline constexpr item_type get_item_type(short index) const noexcept
	{
		return item_need_types[index];
	};
	inline constexpr void set_item_type(item_type type) noexcept
	{
		item_need_types[0] = type;
	};
	constexpr void set_item_types(item_type* types) noexcept
	{
		if (std::is_constant_evaluated())
		{
			constexpr long long l = 8;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = types[i];
			}
		}
		else std::memcpy(&item_need_types[0], &types[0], 16);
	};
	constexpr bool check_if_need_type(item_type type) const noexcept
	{
		constexpr long long l = 8;
		for (long long i = 0; i < l; ++i)
		{
			if (item_need_types[i] == type) return true;
		}

		return false;
	};

public:
	inline constexpr void grab_item(const item_uint& _item) noexcept
	{
		item = _item;
	};
	inline constexpr item_uint& get_item() noexcept
	{
		return item;
	};
	inline constexpr void set_distance_position_plus(long long distance) noexcept
	{
		distance_position_plus = distance;
	};
	inline constexpr void set_distance_position_minus(long long distance) noexcept
	{
		distance_position_minus = distance;
	};
	inline constexpr long long get_distance_position_plus() const noexcept
	{
		return distance_position_plus;
	};
	inline constexpr long long get_distance_position_minus() const noexcept
	{
		return distance_position_minus;
	};
};