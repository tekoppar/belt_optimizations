#pragma once

#include <type_traits>
#include <utility>

#include <item_settings.h>
#include "vectors.h"
#include "item.h"
#include "belt_utility_data.h"
#include "belt_utility_non_type.h"
#include "shared_classes.h"
#include <cstring>
#include <index_inserter_settings.h>

class index_inserter
{
	friend class belt_segment_index_inserter;

public:
	static inline long long grabbed_items{ 0ll };

private:
	vec2_int64 position{ 0, 0 };
	item_type item_type_{ item_type::pink_square };
	item_type item_need_types[index_inserter_settings::item_need_types_size]{ item_type::pink_square, item_type::pink_square, item_type::pink_square, item_type::pink_square };
	//item_uint item;
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
	constexpr index_inserter(const vec2_int64 pos, const item_type i_type, item_type const* const i_need_types, const long long pos_plus, const long long pos_minus) noexcept :
		position{ pos },
		item_type_{ i_type },
		distance_position_plus{ pos_plus },
		distance_position_minus{ pos_minus }
	{
		if (std::is_constant_evaluated())
		{
			constexpr long long l = index_inserter_settings::item_need_types_size;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = i_need_types[i];
			}
		}
		else std::memcpy(&item_need_types[0], &i_need_types[0], index_inserter_settings::item_need_types_size * 2ll);
	};
	constexpr ~index_inserter() noexcept
	{};

	constexpr index_inserter(const index_inserter& o) noexcept :
		position{ o.position },
		item_type_{ o.item_type_ },
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
			constexpr long long l = index_inserter_settings::item_need_types_size;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = o.item_need_types[i];
			}
		}
		else std::memcpy(&item_need_types[0], &o.item_need_types[0], index_inserter_settings::item_need_types_size * 2ll);
	};
	constexpr index_inserter(index_inserter&& o) noexcept :
		position{ std::exchange(o.position, vec2_int64{}) },
		item_type_{ std::exchange(o.item_type_, item_type::pink_square) },
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
			constexpr long long l = index_inserter_settings::item_need_types_size;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = o.item_need_types[i];
			}
		}
		else std::memcpy(&item_need_types[0], &o.item_need_types[0], index_inserter_settings::item_need_types_size * 2ll);
	};
	constexpr index_inserter& operator=(const index_inserter& o) noexcept
	{
		position = o.position;
		item_type_ = o.item_type_;
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
			constexpr long long l = index_inserter_settings::item_need_types_size;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = o.item_need_types[i];
			}
		}
		else std::memcpy(&item_need_types[0], &o.item_need_types[0], index_inserter_settings::item_need_types_size * 2ll);

		return *this;
	};
	constexpr index_inserter& operator=(index_inserter&& o) noexcept
	{
		position = std::exchange(o.position, vec2_int64{});
		item_type_ = std::exchange(o.item_type_, item_type::pink_square);
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
			constexpr long long l = index_inserter_settings::item_need_types_size;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = std::move(o.item_need_types[i]);
			}
		}
		else std::memcpy(&item_need_types[0], &o.item_need_types[0], index_inserter_settings::item_need_types_size * 2ll);

		return *this;
	};

	friend constexpr bool operator==(const index_inserter& lhs, const index_inserter& rhs) noexcept
	{
		return lhs.position == rhs.position && lhs.item_type_ == rhs.item_type_;
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
			constexpr long long l = index_inserter_settings::item_need_types_size;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = types[i];
			}
		}
		else std::memcpy(&item_need_types[0], &types[0], index_inserter_settings::item_need_types_size * 2ll);
	};
	constexpr bool check_if_need_type(item_type type) const noexcept
	{
		constexpr long long l = index_inserter_settings::item_need_types_size;
		for (long long i = 0; i < l; ++i)
		{
			if (item_need_types[i] == type) return true;
		}

		return false;
	};

public:
	inline constexpr item_type get_item_type() const noexcept
	{
		return item_type_;
	};
	inline constexpr void grab_item(item_type&& t) noexcept
	{
		item_type_ = std::move(t);
	};
	inline constexpr item_uint get_item() const noexcept
	{
		return item_uint{ item_type_, position };
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

class belt_segment_index_inserter
{
public:
	static inline long long grabbed_items{ 0ll };

private:
	int offset_position_from_belt{ -1 };
	item_type item_type_{ item_type::pink_square };
	long long distance_position_plus{ -1ll };
	long long distance_position_minus{ -1ll };
	item_type item_need_types[index_inserter_settings::item_need_types_size]{ item_type::pink_square, item_type::pink_square, item_type::pink_square, item_type::pink_square };
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
	constexpr belt_segment_index_inserter() noexcept
	{};
	constexpr belt_segment_index_inserter(int offset_position_from_belt_, long long distance_position_plus) noexcept :
		offset_position_from_belt{ offset_position_from_belt_ },
		distance_position_plus{ distance_position_plus },
		distance_position_minus{ distance_position_plus - item_settings::belt_item_size }
	{};
	constexpr belt_segment_index_inserter(int offset_position_from_belt_, long long distance_position_plus, index_inserter inserter) noexcept :
		offset_position_from_belt{ offset_position_from_belt_ },
		item_type_{ inserter.get_item_type() },
		distance_position_plus{ distance_position_plus },
		distance_position_minus{ distance_position_plus - item_settings::belt_item_size }
	{
		if (std::is_constant_evaluated())
		{
			constexpr long long l = index_inserter_settings::item_need_types_size;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = inserter.item_need_types[i];
			}
		}
		else std::memcpy(&item_need_types[0], &inserter.item_need_types[0], index_inserter_settings::item_need_types_size * 2ll);
	};
	constexpr ~belt_segment_index_inserter() noexcept
	{};

	constexpr belt_segment_index_inserter(const belt_segment_index_inserter& o) noexcept :
		offset_position_from_belt{ o.offset_position_from_belt },
		item_type_{ o.item_type_ },
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
			constexpr long long l = index_inserter_settings::item_need_types_size;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = o.item_need_types[i];
			}
		}
		else std::memcpy(&item_need_types[0], &o.item_need_types[0], index_inserter_settings::item_need_types_size * 2ll);
	};
	constexpr belt_segment_index_inserter(belt_segment_index_inserter&& o) noexcept :
		offset_position_from_belt{ std::exchange(o.offset_position_from_belt, -1) },
		item_type_{ std::exchange(o.item_type_, item_type::pink_square) },
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
			constexpr long long l = index_inserter_settings::item_need_types_size;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = o.item_need_types[i];
			}
		}
		else std::memcpy(&item_need_types[0], &o.item_need_types[0], index_inserter_settings::item_need_types_size * 2ll);
	};
	constexpr belt_segment_index_inserter& operator=(const belt_segment_index_inserter& o) noexcept
	{
		offset_position_from_belt = o.offset_position_from_belt;
		item_type_ = o.item_type_;
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
			constexpr long long l = index_inserter_settings::item_need_types_size;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = o.item_need_types[i];
			}
		}
		else std::memcpy(&item_need_types[0], &o.item_need_types[0], index_inserter_settings::item_need_types_size * 2ll);

		return *this;
	};
	constexpr belt_segment_index_inserter& operator=(belt_segment_index_inserter&& o) noexcept
	{
		offset_position_from_belt = std::exchange(o.offset_position_from_belt, -1);
		item_type_ = std::exchange(o.item_type_, item_type::pink_square);
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
			constexpr long long l = index_inserter_settings::item_need_types_size;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = std::move(o.item_need_types[i]);
			}
		}
		else std::memcpy(&item_need_types[0], &o.item_need_types[0], index_inserter_settings::item_need_types_size * 2ll);

		return *this;
	};

	explicit constexpr operator index_inserter() const noexcept
	{
		return { vec2_int64{ -1ll, -1ll }, item_type_, &item_need_types[0], distance_position_plus, distance_position_minus };
	};

	friend constexpr bool operator==(const belt_segment_index_inserter& lhs, const belt_segment_index_inserter& rhs) noexcept
	{
		return lhs.offset_position_from_belt == rhs.offset_position_from_belt && lhs.item_type_ == rhs.item_type_;
	};

	template<belt_utility::belt_direction direction>
	inline constexpr vec2_int64 get_position(const long long end_distance, const long long offset_position) const noexcept
	{
		if constexpr (belt_utility::belt_direction::left_right == direction || belt_utility::belt_direction::right_left == direction)
			return vec2_int64{ distance_position_plus + end_distance, offset_position_from_belt + offset_position };
		if constexpr (belt_utility::belt_direction::top_bottom == direction || belt_utility::belt_direction::bottom_top == direction)
			return vec2_int64{ offset_position_from_belt + offset_position, distance_position_plus + end_distance };
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
			constexpr long long l = index_inserter_settings::item_need_types_size;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = types[i];
			}
		}
		else std::memcpy(&item_need_types[0], &types[0], index_inserter_settings::item_need_types_size * 2ll);
	};
	constexpr bool check_if_need_type(item_type type) const noexcept
	{
		constexpr long long l = index_inserter_settings::item_need_types_size;
		for (long long i = 0; i < l; ++i)
		{
			if (item_need_types[i] == type) return true;
		}

		return false;
	};

public:
	inline constexpr void grab_item(item_type&& t) noexcept
	{
		item_type_ = std::move(t);
	};
	template<belt_utility::belt_direction direction>
	inline constexpr item_uint get_item(const long long end_distance, const long long offset_position) const noexcept
	{
		return item_uint{ item_type_, get_position<direction>(end_distance, offset_position) };
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