#pragma once

#include <stdexcept>
#include <type_traits>

#include "math_utility.h"
#include "mem_vector.h"

#include "const_data.h"
#include "belt_utility_data.h"
#include "belt_utility.h"
#include "vectors.h"
#include "index_iterator.h"
#include "item.h"
#include "index_inserter.h"
#include "item_32.h"
#include "shared_classes.h"
#include "macros.h"
#include <intrin.h>
#include <utility>
#include <limits>

using closest_item_group_result = belt_utility::find_closest_item_group_result<_vector::iterator>;

constexpr static size_t get_tick_time_until_event(size_t tick_count, long long distance, size_t travel_per_tick) noexcept
{
	const auto ticks_left = std::numeric_limits<size_t>::max() - distance;
	const auto ticks_to_travel = distance / travel_per_tick;
	if (ticks_to_travel > ticks_left)
	{
		return ticks_to_travel - ticks_left;
	}
	else
	{
		return tick_count + ticks_to_travel;
	}
};

struct belt_segment_correct_t
{
	explicit belt_segment_correct_t() = default;
}; inline constexpr belt_segment_correct_t belt_segment_correct_v{};

struct event_tick_data
{
	size_t tick_time{ 0ull };
	size_t start_tick_time{ 0ull };

	inline constexpr friend bool operator==(const event_tick_data& lhs, const event_tick_data& rhs)
	{
		return lhs.tick_time == rhs.tick_time && lhs.start_tick_time == rhs.start_tick_time;
	};
	inline constexpr friend bool operator!=(const event_tick_data& lhs, const event_tick_data& rhs)
	{
		return !(lhs == rhs);
	};
};

class belt_segment
{
public:
	constexpr static long long travel_distance_per_tick{ 1ll };
	constexpr inline const static long long groups_to_update_begin_index{ 1ll };

	vec2_int64 start_of_segment{ 0, 0 };
	vec2_int64 end_of_segment{ 0, 0 };

	_data_vector item_groups_data{ 1024 };
	_vector item_groups{ 1024 };
	//contains the distance between item_groups for all item_groups
	//the value is the distance to the end of the belt segment
	_vector_distance item_groups_distance_between{ 64 };
	mem::vector<size_t, mem::Allocating_Type::ALIGNED_NEW> item_groups_goal_item_count{ 64 };

	//contains the item_groups_goal distance of the first item_group
	_vector_item_groups_head item_groups_heads{ 64 };
	mem::vector<event_tick_data, mem::Allocating_Type::ALIGNED_NEW> item_groups_goal_distance_event_data{ 64 };
	mem::vector<mem::vector<event_tick_data*, mem::Allocating_Type::ALIGNED_NEW>, mem::Allocating_Type::ALIGNED_NEW> groups_to_update{ 8 };

	_vector_inserters inserters{ 32 };
	mem::vector<double_index_iterator<index_inserter, _vector_inserters>, mem::Allocating_Type::ALIGNED_NEW, mem::allocator<double_index_iterator<index_inserter, _vector_inserters>, mem::Allocating_Type::ALIGNED_NEW>, mem::use_memcpy::force_checks_off> inserter_slots{ 1024 };
	mem::vector<_vector_item_groups_head::iterator, mem::Allocating_Type::ALIGNED_NEW> remove_iterators{ 8 };

	// belt_segments that this segment moves items onto
	mem::vector<belt_segment*> segment_end_points{ 8 };
	// belt_segments that will move items onto this segment
	mem::vector<belt_segment*> connected_segments{ 8 };

	long long first_group_to_update{ 1ll };
	size_t first_group_to_update_tick{ 0ull };
	const belt_utility::belt_direction segment_direction{ belt_utility::belt_direction::left_right };
	bool item_was_removed = false;
	size_t tick_count{ 0ull };
#ifdef _DEBUG	
	size_t removed_count{ 0ull };
#endif

	constexpr belt_utility::belt_direction direction_construct(vec2_int64 start, vec2_int64 end) const noexcept
	{
		if (start.x != end.x)
		{
			if (start.x < end.x) return belt_utility::belt_direction::left_right;
			else return belt_utility::belt_direction::right_left;
		}
		else
		{
			if (start.y < end.y) return belt_utility::belt_direction::top_bottom;
			else return belt_utility::belt_direction::bottom_top;
		}
	};

public:
	constexpr belt_segment() noexcept
	{};

	constexpr belt_segment(vec2_int64 start, vec2_int64 end) noexcept :
		start_of_segment{ start },
		end_of_segment{ end },
		segment_direction{ direction_construct(start, end) }
	{
		groups_to_update.emplace_back();
	};

	constexpr belt_segment(vec2_int64 start, vec2_int64 end, belt_segment_correct_t) noexcept :
		start_of_segment{ start },
		end_of_segment{ end }
	{
		groups_to_update.emplace_back();
	};

	constexpr ~belt_segment() noexcept
	{};

	constexpr belt_segment(const belt_segment& o) noexcept :
		start_of_segment{ o.start_of_segment },
		end_of_segment{ o.end_of_segment },
		item_groups_data{ o.item_groups_data },
		item_groups{ o.item_groups },
		item_groups_distance_between{ o.item_groups_distance_between },
		item_groups_goal_item_count{ o.item_groups_goal_item_count },
		item_groups_heads{ o.item_groups_heads },
		item_groups_goal_distance_event_data{ o.item_groups_goal_distance_event_data },
		groups_to_update{ o.groups_to_update },
		inserters{ o.inserters },
		inserter_slots{ o.inserter_slots },
		remove_iterators{ o.remove_iterators },
		segment_end_points{ o.segment_end_points },
		connected_segments{ o.connected_segments },
		first_group_to_update{ o.first_group_to_update },
		first_group_to_update_tick{ o.first_group_to_update_tick },
		segment_direction{ o.segment_direction },
		item_was_removed{ o.item_was_removed },
		tick_count{ o.tick_count }
	{};

	constexpr belt_segment(belt_segment&& o) noexcept :
		start_of_segment{ std::exchange(o.start_of_segment, vec2_int64{}) },
		end_of_segment{ std::exchange(o.end_of_segment, vec2_int64{}) },
		item_groups_data{ std::exchange(o.item_groups_data, decltype(item_groups_data){}) },
		item_groups{ std::exchange(o.item_groups, decltype(item_groups){}) },
		item_groups_distance_between{ std::exchange(o.item_groups_distance_between, decltype(item_groups_distance_between){}) },
		item_groups_goal_item_count{ std::exchange(o.item_groups_goal_item_count, decltype(item_groups_goal_item_count){}) },
		item_groups_heads{ std::exchange(o.item_groups_heads, decltype(item_groups_heads){}) },
		item_groups_goal_distance_event_data{ std::exchange(o.item_groups_goal_distance_event_data, decltype(item_groups_goal_distance_event_data){}) },
		groups_to_update{ std::exchange(o.groups_to_update, decltype(groups_to_update){}) },
		inserters{ std::exchange(o.inserters, decltype(inserters){}) },
		inserter_slots{ std::exchange(o.inserter_slots, decltype(inserter_slots){}) },
		remove_iterators{ std::move(o.remove_iterators) },
		segment_end_points{ std::exchange(o.segment_end_points, decltype(segment_end_points){}) },
		connected_segments{ std::exchange(o.connected_segments, decltype(connected_segments){}) },
		first_group_to_update{ std::exchange(o.first_group_to_update, 1ll) },
		first_group_to_update_tick{ std::exchange(o.first_group_to_update_tick, 0ull) },
		item_was_removed{ o.item_was_removed },
		tick_count{ std::exchange(o.tick_count, 0ull) }
	{};

	constexpr belt_segment& operator=(const belt_segment& o) noexcept
	{
		start_of_segment = o.start_of_segment;
		end_of_segment = o.end_of_segment;
		item_groups_data = o.item_groups_data;
		item_groups = o.item_groups;
		item_groups_distance_between = o.item_groups_distance_between;
		item_groups_goal_item_count = o.item_groups_goal_item_count;
		item_groups_heads = o.item_groups_heads;
		item_groups_goal_distance_event_data = o.item_groups_goal_distance_event_data;
		groups_to_update = o.groups_to_update;
		inserters = o.inserters;
		inserter_slots = o.inserter_slots;
		remove_iterators = o.remove_iterators;
		segment_end_points = o.segment_end_points;
		connected_segments = o.connected_segments;
		first_group_to_update = o.first_group_to_update;
		first_group_to_update_tick = o.first_group_to_update_tick;
		item_was_removed = o.item_was_removed;
		tick_count = o.tick_count;

		return *this;
	};

	constexpr belt_segment& operator=(belt_segment&& o) noexcept
	{
		start_of_segment = std::move(o.start_of_segment);
		end_of_segment = std::move(o.end_of_segment);
		item_groups_data = std::move(o.item_groups_data);
		item_groups = std::move(o.item_groups);
		item_groups_distance_between = std::move(o.item_groups_distance_between);
		item_groups_goal_item_count = std::move(o.item_groups_goal_item_count);
		item_groups_heads = std::move(o.item_groups_heads);
		item_groups_goal_distance_event_data = std::move(o.item_groups_goal_distance_event_data);
		groups_to_update = std::move(o.groups_to_update);
		inserters = std::move(o.inserters);
		inserter_slots = std::move(o.inserter_slots);
		remove_iterators = std::move(o.remove_iterators);
		segment_end_points = std::move(o.segment_end_points);
		connected_segments = std::move(o.connected_segments);
		first_group_to_update = std::move(o.first_group_to_update);
		first_group_to_update_tick = std::move(o.first_group_to_update_tick);
		item_was_removed = o.item_was_removed;
		tick_count = std::move(tick_count);

		return *this;
	};

	struct resize_check
	{
		long long s_item_groups_data{ 0ll };
		long long s_item_groups{ 0ll };
		long long s_item_groups_goal_distance{ 0ll };
		long long s_item_groups_distance_between{ 0ll };
	};
	constexpr inline const resize_check get_vector_resize() const noexcept
	{
		return { item_groups_data.size(), item_groups.size(), item_groups_heads.size(), item_groups_distance_between.size() };
	};

#ifndef ENABLE_CPP_EXCEPTION_THROW
	constexpr inline bool validate_vector_resize(const resize_check) const noexcept
	{
		const auto current_state = get_vector_resize();
		return true;
	};
#else
	constexpr inline bool validate_vector_resize(const resize_check& old_values) const
	{
		const auto current_state = get_vector_resize();
		if (old_values.s_item_groups_data != current_state.s_item_groups_data) throw std::runtime_error("");
		if (old_values.s_item_groups != current_state.s_item_groups) throw std::runtime_error("");
		if (old_values.s_item_groups_goal_distance != current_state.s_item_groups_goal_distance) throw std::runtime_error("");
		if (old_values.s_item_groups_distance_between != current_state.s_item_groups_distance_between) throw std::runtime_error("");

		return true;
	};
#endif

	constexpr inline bool validate_vectors() const
	{
#ifdef ENABLE_CPP_EXCEPTION_THROW
		if (item_groups_data.is_dead) throw std::runtime_error("");
		if (item_groups.is_dead) throw std::runtime_error("");
		if (item_groups_heads.is_dead) throw std::runtime_error("");
		if (item_groups_distance_between.is_dead) throw std::runtime_error("");
		if (item_groups_data.size() == 0ll) throw std::runtime_error("");
		if (item_groups.size() == 0ll) throw std::runtime_error("");
		if (item_groups_heads.size() == 0ll) throw std::runtime_error("");
		if (item_groups_distance_between.size() == 0ll) throw std::runtime_error("");
#endif

		return true;
	};
private:
	constexpr _vector_item_groups_head_type::iterator get_head_item_belongs_too(long long item_distance) const noexcept
	{
		if (item_groups_heads.empty()) return item_groups_heads.last();
		if ((item_groups_heads.last() - 1ll)->distance > item_distance) return item_groups_heads.last() - 1ll;
		if (item_groups_heads.begin()->distance < item_distance) return item_groups_heads.begin();

		auto half_size = item_groups_heads.size();
		if (half_size == 1) return item_groups_heads.begin();
		if (half_size == 2)
		{
			if ((item_groups_heads[0]).distance > item_distance) return item_groups_heads.begin() + 1;
			return item_groups_heads.begin();
		}

		auto left_goal_iter = item_groups_heads.begin();
		auto right_goal_iter = item_groups_heads.last() - 1ll;

		while (half_size > 1ll)
		{
			half_size = expr::ceil_div_power2(right_goal_iter - left_goal_iter); // return (lhs + 1) >> 1;
			auto temp_half_goal_iter = left_goal_iter + half_size;

			if ((*temp_half_goal_iter).distance < item_distance) right_goal_iter = temp_half_goal_iter;
			else left_goal_iter = temp_half_goal_iter;
		}

		if ((*left_goal_iter).distance < item_distance) return left_goal_iter;
		else return right_goal_iter;
	};

	template<belt_utility::item_update_state state = belt_utility::item_update_state::adding>
	inline constexpr void update_distance_between_for_item_changes(long long item_index, long long* const previous_goal_distance_ptr, long long item_distance, item_groups_type* item_group, const _vector_goal_distance::iterator goal_object) noexcept
	{
		if (item_groups_distance_between.size() > 1ll)
		{
			//if were removing the last item so distance_between values needs to have the the item distance value added to them
			//if were removing the first item so we need to subtract item_groups_type::belt_item_size from the distance_between values
			//else if it's in the middle, we don't care since it doesn't effect the distance between
			if (item_group->count() == item_index + 1)
			{
				if constexpr (belt_utility::item_update_state::adding == state) item_distance = expr::negate(item_distance);
				else item_distance = expr::abs(item_distance);
			}
			else if (item_index == 0)
			{
				if (item_group->count() == 0) item_distance = 0ll;
				else
				{
					if constexpr (belt_utility::item_update_state::adding == state) item_distance = item_groups_type::belt_item_size;
					else item_distance = -item_groups_type::belt_item_size;
				}
			}
			else item_distance = 0;

			auto prev_dist = goal_object->get_offset_ptr(-1ll);
			if (prev_dist != nullptr && previous_goal_distance_ptr != prev_dist) prev_dist.add_goal_distance(item_distance);
		}
	};

	__forceinline constexpr void advance_item_group_head(_vector_item_groups_head::iterator item_group_head) noexcept
	{
		if (item_group_head->next_item_group_index >= 0 && item_group_head->next_item_group_index < item_groups_distance_between.size() &&
			item_groups_distance_between[item_group_head->next_item_group_index] != -1ll)
		{
			item_group_head->distance += item_groups_distance_between[item_group_head->next_item_group_index];
			item_groups_distance_between[item_group_head->next_item_group_index] = -1ll;
			item_group_head->item_group = std::move(item_groups[item_group_head->next_item_group_index]);
			item_group_head->item_group_data = std::move(item_groups_data[item_group_head->next_item_group_index]);
			--item_group_head->next_item_group_index;
		}
	};

public:
	template<belt_utility::belt_direction direction>
	inline constexpr item_uint get_item(long long item_group, long long i) noexcept
	{
		ASSERT_NOT_CONSTEXPR<_BOUNDS_CHECKING_>(i < item_groups.size());
		return item_groups[item_group].get(get_end_distance_direction<direction>(), get_direction_y_value<direction>(), item_groups_distance_between[item_group], item_groups_data[item_group], i);
	};

	template<belt_utility::belt_direction direction>
	inline constexpr item_uint remove_item(_vector_item_groups_head::iterator item_group_head) noexcept
	{
		const auto found_index = item_group_head->item_to_grab;
		item_uint return_item{ item_group_head->item_group_data.items[item_group_head->item_to_grab].type, vec2_int64{ get_end_distance_direction<direction>() - item_group_head->item_group.get_item_position<direction>(item_group_head->distance, item_group_head->item_group_data, item_group_head->item_to_grab), get_direction_y_value<direction>()} };

		if (item_groups_type::item_removal_result::item_removed_zero_remains == item_group_head->item_group.remove_item(&item_group_head->distance, item_group_head->item_group_data, found_index))
			remove_iterators.emplace_back(item_group_head);

		--item_groups_goal_item_count[(item_group_head - item_groups_heads.begin())];
		return return_item;
	};
	template<belt_utility::belt_direction direction>
	inline constexpr item_uint remove_item(item_groups_type* item_group, const long long item_index) noexcept
	{
		const auto index_ptr = item_group - item_groups.begin();
		const auto data_group = item_groups_data.begin() + index_ptr;
		const auto distance_group = item_groups_distance_between.begin() + index_ptr;
		const auto goal_object = belt_utility::get_goal_object_index_binary(index_ptr, item_groups_heads, item_groups_distance_between);

		auto return_item = (*item_group).get(get_end_distance_direction<direction>(), get_direction_y_value<direction>(), (*goal_object).distance, *data_group, item_index);
		if (item_groups_type::item_removal_result::item_removed_zero_remains == (*item_group).remove_item(distance_group.operator->(), *data_group, item_index)) item_group_has_zero_count(item_groups.begin() + index_ptr, data_group);

		--item_groups_goal_item_count[(goal_object - item_groups_heads.begin())];

		return return_item;
	};
	template<belt_utility::belt_direction direction>
	inline constexpr item_uint remove_item(const long long item_group_index, const long long item_index) noexcept
	{
		return remove_item<direction>(&item_groups[item_group_index], item_index);
	};

	inline constexpr item_groups_type& get_item_group(long long i) noexcept
	{
		ASSERT_NOT_CONSTEXPR<_BOUNDS_CHECKING_>(i < item_groups.size());
		return item_groups[i];
	};

	inline constexpr const item_groups_type& get_item_group(long long i) const noexcept
	{
		ASSERT_NOT_CONSTEXPR<_BOUNDS_CHECKING_>(i < item_groups.size());
		return item_groups[i];
	};

	inline constexpr item_groups_data_type& get_item_data_group(long long i) noexcept
	{
		ASSERT_NOT_CONSTEXPR<_BOUNDS_CHECKING_>(i < item_groups.size());
		return item_groups_data[i];
	};

	inline constexpr const item_groups_data_type& get_item_data_group(long long i) const noexcept
	{
		ASSERT_NOT_CONSTEXPR<_BOUNDS_CHECKING_>(i < item_groups.size());
		return item_groups_data[i];
	};

	inline constexpr _vector_distance::iterator get_distance_between_item_groups_iterator(long long i) const noexcept
	{
		return item_groups_distance_between.begin() + i;
	};

	inline constexpr index_inserter& get_inserter(belt_utility::add_inserter_return_indexes indexes) noexcept
	{
		if constexpr (ENABLE_CPP_EXCEPTION_THROW)
		{
			if (indexes.index >= inserters.size()) throw std::out_of_range("");
			if (indexes.nested_index >= inserters[indexes.index].size()) throw std::out_of_range("");
		}

		return inserters[indexes.index][indexes.nested_index];

		if constexpr (ENABLE_CPP_EXCEPTION_THROW) throw std::runtime_error("");  //this shouldn't occur as long as the index is valid and not human entered
	};

	inline constexpr index_inserter& get_inserter(long long i) noexcept
	{
		long long count_index = 0ull;
		for (auto begin_iter = inserters.begin(); begin_iter != inserters.last(); ++begin_iter)
		{
			if (count_index == i) return begin_iter->operator[](0);
			const auto inserter_size = begin_iter->size();
			if (count_index <= i && count_index + inserter_size >= i + 1ll) return begin_iter->operator[](i - count_index);
			count_index += inserter_size;
		}

		if constexpr (ENABLE_CPP_EXCEPTION_THROW)
		{
			if (count_index == i) throw std::runtime_error("");
			throw std::runtime_error("");  //this shouldn't occur as long as the index is valid and not human entered
		}

		return inserters.begin()->operator[](0);
	};

	inline constexpr const index_inserter& get_inserter(belt_utility::add_inserter_return_indexes indexes) const noexcept
	{
		if constexpr (ENABLE_CPP_EXCEPTION_THROW)
		{
			if (indexes.index >= inserters.size()) throw std::out_of_range("");
			if (indexes.nested_index >= inserters[indexes.index].size()) throw std::out_of_range("");
		}

		return inserters[indexes.index][indexes.nested_index];

		if constexpr (ENABLE_CPP_EXCEPTION_THROW) throw std::runtime_error("");  //this shouldn't occur as long as the index is valid and not human entered
	};

	inline constexpr const index_inserter& get_inserter(long long i) const noexcept
	{
		long long count_index = 0ull;
		for (auto begin_iter = inserters.begin(); begin_iter != inserters.last(); ++begin_iter)
		{
			if (count_index + 1 == i) return begin_iter->operator[](0);
			const auto inserter_size = begin_iter->size();
			if (count_index <= i && count_index + inserter_size >= i) return begin_iter->operator[](i - count_index + 1);
			count_index += inserter_size;
		}

		if constexpr (ENABLE_CPP_EXCEPTION_THROW) throw std::runtime_error("");  //this shouldn't occur as long as the index is valid and not human entered
	};

	inline constexpr long long get_new_item_distance(long long i) const noexcept
	{
		return item_groups_distance_between[i];
	};

	template<belt_utility::belt_direction direction>
	inline constexpr long long get_new_item_position(long long i) const noexcept
	{
		if constexpr (ENABLE_CPP_EXCEPTION_THROW)
		{
			if (i >= item_groups_heads.size()) throw std::runtime_error("");
			if (i >= item_groups_distance_between.size()) throw std::runtime_error("");
		}

		const auto goal_object = belt_utility::get_goal_object_index(i, item_groups_heads, item_groups_distance_between);

		if constexpr (_BOUNDS_CHECKING_) if (item_groups_heads.last() == goal_object) return -1ll;
		if constexpr (ENABLE_CPP_EXCEPTION_THROW) if ((*goal_object).get_index_ptr() == nullptr) throw std::runtime_error("");

		const auto index_from_ptr = ((*goal_object).get_index_from_ptr(item_groups_distance_between.begin().operator->()));
		return get_end_distance_direction<direction>() - belt_utility::get_distances_from_to2(item_groups_distance_between.begin() + index_from_ptr, item_groups_distance_between.begin() + i) + 32;
	};

	inline constexpr long long count_item_groups() const noexcept
	{
		return item_groups.size();
	};

	inline constexpr long long count_items_in_group(long long i) const noexcept
	{
		ASSERT_NOT_CONSTEXPR<_BOUNDS_CHECKING_>(i < item_groups.size());
		return item_groups[i].count();
	};

	inline constexpr long long count_all_items() const noexcept
	{
		long long total = 0ll;
		const long long l = item_groups.size();
		long long i = 0;
		for (; i < l; ++i)
		{
			ASSERT_NOT_CONSTEXPR<_BOUNDS_CHECKING_>(i < item_groups.size());

			const auto count = item_groups[i].count();
			total += count;

			if constexpr (__DEBUG_BUILD) if (total < 0ll) __debugbreak();
		}
		return total;
	};

	inline constexpr long long goal_distance_in_group(long long i) const noexcept
	{
		ASSERT_NOT_CONSTEXPR<_BOUNDS_CHECKING_>(i < item_groups_distance_between.size());
		return item_groups_distance_between[i];
	};

	inline constexpr long long get_item_groups_goal_distance_size() const noexcept
	{
		return item_groups_heads.size();
	};

	inline constexpr goal_distance get_goal_distance(long long i) noexcept
	{
		ASSERT_NOT_CONSTEXPR<_BOUNDS_CHECKING_>(i < item_groups_heads.size());
		return &item_groups_heads[i].distance;
	};

	inline constexpr long long goal_distance_in_destinations(long long i) const noexcept
	{
		ASSERT_NOT_CONSTEXPR<_BOUNDS_CHECKING_>(i < item_groups_heads.size());
		return item_groups_heads[i].distance;
	};

	inline constexpr long long count_inserters() const noexcept
	{
		long long count_index = 0ull;
		for (auto begin_iter = inserters.begin(); begin_iter != inserters.last(); ++begin_iter)
		{
			count_index += begin_iter->size();
		}
		return count_index;
	};

	template<belt_utility::belt_direction direction>
	inline constexpr long long get_direction_y_value() const noexcept
	{
		using enum belt_utility::belt_direction;
		if constexpr (left_right == direction) return start_of_segment.y;
		if constexpr (right_left == direction) return start_of_segment.y;
		if constexpr (top_bottom == direction) return start_of_segment.x;
		if constexpr (bottom_top == direction) return start_of_segment.x;
	};

	template<belt_utility::belt_direction direction>
	inline constexpr long long get_start_direction_value() const noexcept
	{
		using enum belt_utility::belt_direction;
		if constexpr (left_right == direction) return start_of_segment.x;
		if constexpr (right_left == direction) return start_of_segment.x;
		if constexpr (top_bottom == direction) return start_of_segment.y;
		if constexpr (bottom_top == direction) return start_of_segment.y;
	};

	template<belt_utility::belt_direction direction>
	inline constexpr long long get_end_distance() const noexcept
	{
		using enum belt_utility::belt_direction;
		if constexpr (left_right == direction) return end_of_segment.x - start_of_segment.x;
		if constexpr (right_left == direction) return start_of_segment.x - end_of_segment.x;
		if constexpr (top_bottom == direction) return end_of_segment.y - start_of_segment.y;
		if constexpr (bottom_top == direction) return start_of_segment.y - end_of_segment.y;
	};

	template<belt_utility::belt_direction direction>
	inline constexpr long long get_end_distance_direction() const noexcept
	{
		using enum belt_utility::belt_direction;
		if constexpr (left_right == direction) return end_of_segment.x;
		if constexpr (right_left == direction) return end_of_segment.x;
		if constexpr (top_bottom == direction) return end_of_segment.y;
		if constexpr (bottom_top == direction) return end_of_segment.y;
	};

	template<belt_utility::belt_direction direction>
	inline constexpr item_groups_type::index_item_position_return get_closest_item_inserter_can_grab(const long long inserter_index, const long long cur_dist, const item_groups_type& item_groups_ref, const item_groups_data_type& item_data_ref) const noexcept
	{
		const long long item_count = item_groups_ref.count();
		if (item_count == 0ll) return item_groups_type::index_item_position_return{};

		auto& nested_inserter_vector = inserters[inserter_index];
		const auto nested_ins_last = nested_inserter_vector.last();
		const long long temp_last_item_position = cur_dist + item_data_ref.item_distance[item_count - 1ll];

		//long long triggered_inserter_index = 0ll; //++triggered_inserter_index
		for (auto nested_ins_iter = nested_inserter_vector.begin(); nested_ins_iter != nested_ins_last; ++nested_ins_iter)
		{
			const long long inserter_pos_minus = nested_ins_iter->get_distance_position_minus();
			const item_type inserter_item_type = nested_ins_iter->get_item_type(0);
			if (temp_last_item_position >= inserter_pos_minus)
			{
				for (long long i = 0ll; i < item_count; ++i)
				{
					const auto item_position = item_groups_ref.get_item_position<direction>(cur_dist, item_data_ref, i);
					if (item_data_ref.items[i].type == inserter_item_type && item_position >= inserter_pos_minus)
						return item_groups_type::index_item_position_return{ i, item_position }; //triggered_inserter_index
				}
			}
		}

		return item_groups_type::index_item_position_return{};
	};

	template<belt_utility::belt_direction direction>
	constexpr void add_event_tick(event_tick_data* current_tick_data) noexcept
	{
		const auto current_tick_data_index = current_tick_data - item_groups_goal_distance_event_data.begin().operator->();
		long long distance = this->item_groups_heads[current_tick_data_index].distance;//get_end_distance<direction>();
		if (inserters.size() > current_tick_data_index)
		{
			const auto item_group_head = (item_groups_heads.begin() + current_tick_data_index);
			const item_groups_type::index_item_position_return can_grab = get_closest_item_inserter_can_grab<direction>(current_tick_data_index, item_group_head->distance, item_group_head->item_group, item_group_head->item_group_data);
			if (can_grab.found_index != -1ll)
			{
				item_group_head->item_to_grab = can_grab.found_index;
				//item_group_head->event_trigger_index = can_grab.event_trigger_index;
				distance = can_grab.item_distance_position - (inserters[current_tick_data_index].last() - 1ll)->get_distance_position_plus();

				if (distance < 0 && distance >= -item_groups_type::belt_item_size)
					distance = 0;
			}
		}

		if (distance == 0) current_tick_data->tick_time = tick_count + 1;
		else current_tick_data->tick_time = get_tick_time_until_event(tick_count, distance, travel_distance_per_tick);

		current_tick_data->start_tick_time = tick_count;

		for (auto begin_iter = groups_to_update.begin() + 1; begin_iter != groups_to_update.last(); ++begin_iter)
		{
			if ((*begin_iter)[0]->tick_time == current_tick_data->tick_time)
			{
				begin_iter->emplace_back(current_tick_data);
				return;
			}
		}

		mem::vector<event_tick_data*, mem::Allocating_Type::ALIGNED_NEW> new_group{ 4 };
		new_group.emplace_back(current_tick_data);
		groups_to_update.emplace_back(new_group);
	};

	constexpr void add_event_tick_zero_update_group(event_tick_data* current_tick_data) noexcept
	{
		groups_to_update[0].emplace_back(current_tick_data);
	};
	constexpr void add_event_tick_no_update_skip(event_tick_data* current_tick_data) noexcept
	{
		const long long length = groups_to_update.size();
		for (long long i = 1; i < length; ++i)
		{
			if (groups_to_update[i][0] != current_tick_data && groups_to_update[i][0]->tick_time == current_tick_data->tick_time)
			{
				groups_to_update[i].emplace_back(current_tick_data);
				return;
			}
		}

		mem::vector<event_tick_data*, mem::Allocating_Type::ALIGNED_NEW> new_group{ 4 };
		new_group.emplace_back(current_tick_data);
		groups_to_update.emplace_back(new_group);
	};
	constexpr void add_event_tick_no_update(event_tick_data* current_tick_data) noexcept
	{
		for (auto begin_iter = groups_to_update.begin() + 1; begin_iter != groups_to_update.last(); ++begin_iter)
		{
			if ((*begin_iter)[0] != current_tick_data && (*begin_iter)[0]->tick_time == current_tick_data->tick_time)
			{
				begin_iter->emplace_back(current_tick_data);
				return;
			}
		}

		mem::vector<event_tick_data*, mem::Allocating_Type::ALIGNED_NEW> new_group{ 4 };
		new_group.emplace_back(current_tick_data);
		groups_to_update.emplace_back(new_group);
	};

	template<belt_utility::belt_direction direction>
	constexpr void update_event_tick_index(event_tick_data* current_tick_data) noexcept
	{
		const auto current_tick_data_index = current_tick_data - item_groups_goal_distance_event_data.begin().operator->();
		long long distance = this->item_groups_heads[current_tick_data_index].distance;
		if (inserters.size() > current_tick_data_index)
		{
			const _vector_item_groups_head::iterator item_groups_head = (item_groups_heads.begin() + current_tick_data_index);
			const item_groups_type::index_item_position_return can_grab = get_closest_item_inserter_can_grab<direction>(current_tick_data_index, item_groups_head->distance, item_groups_head->item_group, item_groups_head->item_group_data);
			if (can_grab.found_index != -1ll)
			{
				item_groups_head->item_to_grab = can_grab.found_index;
				//item_groups_head->event_trigger_index = can_grab.event_trigger_index;
				distance = can_grab.item_distance_position - (inserters[current_tick_data_index].last() - 1ll)->get_distance_position_plus();

				if (distance < 0 && distance >= -item_groups_type::belt_item_size)
					distance = 0;
			}
		}

		if (distance == 0) current_tick_data->tick_time = tick_count + 1;
		else current_tick_data->tick_time = get_tick_time_until_event(tick_count, distance, travel_distance_per_tick);

		current_tick_data->start_tick_time = tick_count;
	}

	constexpr void remove_event_tick(mem::vector<event_tick_data>::iterator current_tick_data) noexcept
	{
		for (auto groups_to_update_data_iter = groups_to_update.begin() + 1; groups_to_update_data_iter != groups_to_update.last(); ++groups_to_update_data_iter)
		{
			for (auto begin_iter = groups_to_update_data_iter->begin(); begin_iter != groups_to_update_data_iter->last(); ++begin_iter)
			{
				if ((*begin_iter) == current_tick_data.operator->())
				{
					groups_to_update_data_iter->remove(current_tick_data - (*groups_to_update_data_iter->begin()));

					if (groups_to_update.begin() != groups_to_update_data_iter && groups_to_update_data_iter->empty())
					{
						const auto remove_group_index = groups_to_update_data_iter - groups_to_update.begin();
						groups_to_update.remove_unsafe(remove_group_index);
					}
					return;
				}
			}
		}
	};
	constexpr void remove_event_tick(const event_tick_data* current_tick_data) noexcept
	{
		for (auto groups_to_update_data_iter = groups_to_update.begin() + 1; groups_to_update_data_iter != groups_to_update.last(); ++groups_to_update_data_iter)
		{
			for (auto begin_iter = groups_to_update_data_iter->begin(); begin_iter != groups_to_update_data_iter->last(); ++begin_iter)
			{
				if ((*begin_iter) == current_tick_data)
				{
					groups_to_update_data_iter->remove_unsafe(current_tick_data - (*(*groups_to_update_data_iter).begin()));

					if (groups_to_update.begin() != groups_to_update_data_iter && groups_to_update_data_iter->empty())
					{
						const auto remove_group_index = groups_to_update_data_iter - groups_to_update.begin();
						groups_to_update.remove_unsafe(remove_group_index);
					}
					return;
				}
			}
		}
	};

	enum class update_event_tick_state : char
	{
		updated_tick_data,
		no_update,
		removed_container
	};
	template<belt_utility::belt_direction direction>
	__declspec(noinline) constexpr void update_event_tick(event_tick_data* current_tick_data, const size_t current_group_tick_time) noexcept
	{
		const ptrdiff_t distance_goal_index = (current_tick_data - item_groups_goal_distance_event_data.begin());
		long long distance = this->item_groups_heads[distance_goal_index].distance;
		if (inserters.size() > static_cast<long long>(distance_goal_index))
		{
			const auto item_group_head = item_groups_heads.begin() + distance_goal_index;
			const auto can_grab = get_closest_item_inserter_can_grab<direction>(distance_goal_index, item_group_head->distance, item_group_head->item_group, item_group_head->item_group_data);
			if (can_grab.found_index != -1ll)
			{
				item_group_head->item_to_grab = static_cast<char>(can_grab.found_index);
				//item_group_head->event_trigger_index = can_grab.event_trigger_index;
				distance = can_grab.item_distance_position - (inserters[distance_goal_index].last() - 1ll)->get_distance_position_plus();

				if (distance < 0 && distance >= -item_groups_type::belt_item_size)
					distance = 0;
			}
		}

		if (distance == 0) current_tick_data->tick_time = tick_count == 0 ? 0 : tick_count + 1;
		else current_tick_data->tick_time = get_tick_time_until_event(tick_count, distance, travel_distance_per_tick);

		current_tick_data->start_tick_time = tick_count;

		if (current_group_tick_time != (std::numeric_limits<size_t>::max)() && current_group_tick_time != current_tick_data->tick_time) add_event_tick_no_update_skip(current_tick_data);
	};
	template<belt_utility::belt_direction direction>
	constexpr bool update_event_tick(long long distance_goal_index) noexcept
	{
		const auto current_tick_data = item_groups_goal_distance_event_data.begin() + distance_goal_index;
		{
			{
				{
					long long distance = this->item_groups_heads[distance_goal_index].distance;
					if (inserters.size() > distance_goal_index)
					{
						const _vector_item_groups_head::iterator item_group_head = (item_groups_heads.begin() + distance_goal_index);

						const item_groups_type::index_item_position_return can_grab = get_closest_item_inserter_can_grab<direction>(distance_goal_index, item_group_head->distance, item_group_head->item_group, item_group_head->item_group_data);
						if (can_grab.found_index != -1ll)
						{
							item_group_head->item_to_grab = static_cast<char>(can_grab.found_index);
							//item_group_head->event_trigger_index = can_grab.event_trigger_index;
							distance = can_grab.item_distance_position - (inserters[distance_goal_index].last() - 1ll)->get_distance_position_plus();

							if (distance < 0 && distance >= -item_groups_type::belt_item_size)
								distance = 0;
						}
					}

					const size_t old_tick_time = current_tick_data->tick_time;
					if (distance == 0)
						current_tick_data->tick_time = tick_count == 0 ? tick_count : tick_count + 1;
					else
						current_tick_data->tick_time = get_tick_time_until_event(tick_count, distance, travel_distance_per_tick);

					current_tick_data->start_tick_time = tick_count;

					if (old_tick_time != current_tick_data->tick_time)
					{
						remove_event_tick(current_tick_data);
						add_event_tick_no_update(current_tick_data.operator->());
						return true;
					}

					return false;
				}
			}
		}
	};

	inline constexpr mem::vector<long long> get_current_goal_distance_values() const noexcept
	{
		mem::vector<long long> old_distances;
		const auto e_iter = item_groups_heads.last();
		for (auto b_iter = item_groups_heads.begin(); b_iter != e_iter; ++b_iter)
		{
			old_distances.push_back(b_iter->distance);
		}
		return old_distances;
	};

	inline constexpr mem::vector<long long> get_current_goal_distance_pointers_item_groups_removed() const noexcept
	{
		mem::vector<long long> old_distances;
		const auto e_iter = item_groups_heads.last();
		for (auto b_iter = item_groups_heads.begin(); b_iter != e_iter; ++b_iter)
		{
			old_distances.emplace_back(b_iter->distance);
		}

		return old_distances;
	};
	inline constexpr mem::vector<long long> get_current_goal_distance_pointers_item_groups_removed(decltype(remove_iterators)& future_removes) const noexcept
	{
		mem::vector<long long> old_distances;
		auto begin_future = future_removes.begin();
		const auto e_iter = item_groups_heads.last();
		for (auto b_iter = item_groups_heads.begin(); b_iter != e_iter; ++b_iter)
		{
			if (begin_future != future_removes.last())
			{
				if ((*begin_future)->distance == b_iter->distance)
				{
					old_distances.emplace_back(0ll);
					++begin_future;
				}
				else old_distances.emplace_back(b_iter->distance);

			}
			else old_distances.emplace_back(b_iter->distance);
		}
		return old_distances;
	};
#ifdef _DEBUG
	inline constexpr void validate_goal_pointers(const std::vector<long long>& old_distance_values, const std::vector<long long>& new_distance_values) noexcept
#else
	inline constexpr void validate_goal_pointers(const mem::vector<long long>& old_distance_values) noexcept
#endif
	{
#ifdef _DEBUG
		if (old_distance_values.size() != new_distance_values.size()) throw std::runtime_error("");
#endif
		const long long l = old_distance_values.size();
		for (long long i = 0; i < l; ++i)
		{
			if (old_distance_values[i] != 0ll)
			{
#ifdef _DEBUG
				if (old_distance_values[i] != new_distance_values[i]) throw std::runtime_error("");
#endif
			}
		}
	};

	inline constexpr _vector_item_groups_head::iterator get_goal_distance_iterator(_vector_item_groups_head::iterator dist_iter) const noexcept
	{
		const auto e_iter = item_groups_heads.last();
		for (auto b_iter = item_groups_heads.begin(); b_iter != e_iter; ++b_iter)
		{
			if (b_iter->distance == dist_iter->distance) return b_iter;
		}

		return item_groups_heads.last();
	};

	inline constexpr void item_group_has_zero_count(_vector_item_groups_head::iterator item_group_head) noexcept
	{
		if constexpr (ENABLE_CPP_EXCEPTION_THROW) if (item_group_head == item_groups_heads.last().operator->()) throw std::runtime_error("");

		remove_iterators.emplace_back(item_group_head);
	};

	template<belt_utility::belt_direction direction>
	__declspec(noinline) constexpr void item_group_has_reached_goal(_vector_item_groups_head::iterator item_group_head) noexcept
	{
		if (segment_end_points.size() > 0ull)
		{
			const auto end_distance_direction = get_end_distance_direction<direction>();
			const auto direction_y_value = get_direction_y_value<direction>();
			for (long long i2 = 0; i2 < segment_end_points.size(); ++i2)
			{
				ASSERT_NOT_CONSTEXPR<_BOUNDS_CHECKING_>(i2 < segment_end_points.size());

				auto segment_ptr = segment_end_points[i2];
				const auto ptr_position = item_group_head->item_group.get_position(end_distance_direction, direction_y_value, item_group_head->distance);

				if (segment_ptr->start_of_segment != ptr_position) continue;

				if (segment_ptr->add_item(item_group_head->item_group.get_first_item(end_distance_direction, direction_y_value, item_group_head->distance, item_group_head->item_group_data)))
				{
					if (item_group_head->item_group.count() == 1ll)
					{
						item_group_head->item_group.remove_last_item(item_group_head->item_group_data);
						item_group_has_zero_count(item_group_head);
					}
					else
					{
						if (item_groups_type::item_removal_result::item_removed_zero_remains == item_group_head->item_group.remove_first_item(&item_group_head->distance, item_group_head->item_group_data)) item_group_has_zero_count(item_group_head);
					}

					item_was_removed = true;
					return;
				}
			}
		}
	};

	template<belt_utility::belt_direction direction>
	__declspec(noinline) constexpr void item_groups_removal() noexcept
	{
		if (remove_iterators.size() > 0ll)
		{
#ifdef _DEBUG
			auto old_goal_values = get_current_goal_distance_pointers_item_groups_removed(remove_iterators);
			const auto old_size = item_groups.size();
#endif
			auto begin_remove_iter = remove_iterators.begin();
			const auto begin_object = *item_groups_distance_between.begin();
			const auto remove_iter_last = remove_iterators.last();

			for (; begin_remove_iter != remove_iter_last; ++begin_remove_iter)
			{
				const _vector_item_groups_head::iterator item_group_head = *begin_remove_iter;
				if (item_group_head->distance == begin_object)
					item_group_head->distance = -1ll;
				else
					advance_item_group_head(item_group_head);
			}

			remove_iterators.clear();
			return;
		}
	};

public:
	template<belt_utility::belt_direction direction>
	constexpr void event_update_item(decltype(groups_to_update[0])& updates) noexcept
	{
		//const long long end_distance = get_end_distance_direction<direction>();
		//const long long end_y_direction = get_direction_y_value<direction>();

		auto const* const event_data_begin_pointer = (item_groups_goal_distance_event_data.begin().operator->());
		const auto goal_count_begin_iter = item_groups_goal_item_count.begin();

		for (auto const* const update : updates)
		{
			const long long ticks_for_group = static_cast<long long>((update)->tick_time - (update)->start_tick_time);

			const long long event_data_index = update - event_data_begin_pointer;
			const auto begin_iter = item_groups_heads.begin() + event_data_index;

			begin_iter->distance -= (ticks_for_group * travel_distance_per_tick);
			if (begin_iter->distance > 0ll) [[likely]]
			{
				auto begin_goal_count_iter = goal_count_begin_iter + event_data_index;
				if (std::is_constant_evaluated() == false) item_groups_type::items_moved_per_frame += *begin_goal_count_iter + (ticks_for_group * (*begin_goal_count_iter));

				//auto& nested_inserter = inserters[event_data_index];
				for (auto& nested_inserter : inserters[event_data_index])
				{
					const char found_index = begin_iter->item_to_grab;
					//const auto trigger_index = begin_iter->event_trigger_index;
					item_type item_type = begin_iter->item_group_data.items[found_index].type;

					if (item_groups_type::item_removal_result::item_removed_zero_remains == begin_iter->item_group.remove_item(&begin_iter->distance, begin_iter->item_group_data, found_index))
						remove_iterators.push_back_unchecked(begin_iter);

					--(*begin_goal_count_iter);
					nested_inserter.grab_item(std::move(item_type)); //[trigger_index]
#ifdef _DEBUG
					++removed_count;
					//++nested_ins_iter->local_grabbed_items;
					//nested_ins_iter->loop_count = 0;
#endif
				}
			}
			else [[unlikely]]
				begin_iter->item_group.items_stuck_update(begin_iter->item_group_data);
		}
	};

	constexpr void update() noexcept
	{
		using enum belt_utility::belt_direction;
		switch (segment_direction)
		{
			case null:
			case left_right: update<left_right>(); break;
			case right_left: update<right_left>(); break;
			case top_bottom: update<top_bottom>(); break;
			case bottom_top: update<bottom_top>(); break;
		}
	};
private:
	template<belt_utility::belt_direction direction>
	inline constexpr void update_zero_event_tick_group() noexcept
	{
		constexpr size_t max_sub_1 = std::numeric_limits<size_t>::max() - 1;
		const long long l = groups_to_update[0ll].size();
		for (long long i = 0; i < l; ++i)
			update_event_tick<direction>(groups_to_update[0ll][i], max_sub_1);

		groups_to_update[0ll].decrease_size(0ll);
	};

	template<belt_utility::belt_direction direction>
	__declspec(noinline) constexpr void update() noexcept
	{
		if (!groups_to_update[0ll].empty())
			update_zero_event_tick_group<direction>();

		const long long group_update_index = first_group_to_update;
		if (groups_to_update.size() > 0ll && tick_count == first_group_to_update_tick) [[unlikely]]
		{
			event_update_item<direction>(groups_to_update[group_update_index]);

			if (!item_groups_heads.empty())
			{
				if ((item_groups_heads.begin()->distance) == 0ll)
					item_group_has_reached_goal<direction>(item_groups_heads.begin());
			}

			if (!remove_iterators.empty())
				item_groups_removal<direction>();

			++tick_count;

			update_event_tick<direction>(groups_to_update[group_update_index][0ll], std::numeric_limits<size_t>::max());
			const size_t current_tick_time = groups_to_update[group_update_index][0ll]->tick_time;
			const long long length = groups_to_update[group_update_index].size();
			long long same_group_index = 1ll;
			for (long long i = groups_to_update_begin_index; i < length; ++i)
			{
				update_event_tick<direction>(groups_to_update[group_update_index][i], current_tick_time);
				if (groups_to_update[group_update_index][i]->tick_time == current_tick_time)
				{
					groups_to_update[group_update_index][same_group_index] = groups_to_update[group_update_index][i];
					++same_group_index;
				}
			}

			groups_to_update[group_update_index].decrease_size(same_group_index);

			size_t closest_tick{ std::numeric_limits<size_t>::max() };
			const long long l_groups = groups_to_update.size();
			for (long long i = groups_to_update_begin_index; i < l_groups; ++i)
			{
				if (groups_to_update[i][0ll]->tick_time < closest_tick)
				{
					closest_tick = groups_to_update[i][0ll]->tick_time;
					first_group_to_update = i;
					first_group_to_update_tick = closest_tick;
				}
			}
		}
		else [[likely]] ++tick_count;
	};

	constexpr bool is_end_goal_destination(auto goal) const noexcept
	{
		if ((*goal).distance == (*(item_groups_heads.last() - 1ll)).distance) return true;

		return false;
	};

	constexpr void recalculate_distances_between_from_to(auto old_goal_distance_ptr, auto start, auto end) noexcept
	{
#ifdef _DEBUG
		if (start == end) return;
		if (start.operator->() == old_goal_distance_ptr) return;
#endif
		long long new_distance = *start;
		--start;
		if (start.operator->() == old_goal_distance_ptr)
		{
			*start = *start - new_distance;
			if (start == end) return;
			--start;
		}

		while (start != end)
		{
			--start;
		}
	};

	constexpr auto new_goal_before_distance_loop(long long distance, _vector_item_groups_head::iterator begin_goal, _vector_distance::iterator first_distance_between) noexcept
	{
		long long current_distance = 0ll;
		do
		{
			current_distance += begin_goal->distance;
			if (current_distance == distance)
			{
				begin_goal->distance = current_distance;

				const bool goal_distance_needs_resize = item_groups_heads.needs_resize();
				mem::vector<mem::vector<long long>> index_container;
				if (goal_distance_needs_resize)
				{
					index_container = mem::utility::validate_pointers_get_indexes(item_groups_goal_distance_event_data, groups_to_update);
					remove_iterators.resize(item_groups_heads.size() * 2ll);
				}

				const long long item_group_head_index = begin_goal - item_groups_heads.begin();
				item_groups_heads.emplace(begin_goal, begin_goal->distance, begin_goal->next_item_group_index);
				item_groups_goal_distance_event_data.emplace(item_groups_goal_distance_event_data.begin() + item_group_head_index);
				item_groups_goal_item_count.emplace(item_groups_goal_item_count.begin() + (begin_goal - item_groups_heads.begin()), 0ull);

				if (goal_distance_needs_resize)
					mem::utility::validate_pointers_from_indexes(item_groups_goal_distance_event_data, index_container, groups_to_update);

				return true;
			}
			else --begin_goal->distance;
		} while (*first_distance_between != begin_goal->distance);

		return false;
	};

	constexpr auto insert_new_goal_before_distance(long long distance) noexcept
	{
		if (item_groups_heads.size() > 0ll)
		{
			if (new_goal_before_distance_loop(distance, item_groups_heads.begin(), item_groups_distance_between.begin())) return true;

			const auto last_goal = item_groups_heads.last();
			for (auto begin_goal = item_groups_heads.begin(), next_goal = begin_goal + 1ll; next_goal != last_goal; ++next_goal)
			{
				if ((*next_goal).distance < distance) //means begin_goal is in front of distance
				{
					const auto index_ptr = (*begin_goal).next_item_group_index + 1ll;
					const auto first_distance = item_groups_distance_between.begin() + index_ptr;
					auto goal_distance = item_groups_distance_between.begin() + (*next_goal).next_item_group_index;

					long long current_distance = 0ll;
					do
					{
						current_distance += *goal_distance;
						if (current_distance == distance)
						{
							*goal_distance = current_distance;

							const bool goal_distance_needs_resize = item_groups_heads.needs_resize();
							mem::vector<mem::vector<long long>> index_container;
							if (goal_distance_needs_resize)
							{
								index_container = mem::utility::validate_pointers_get_indexes(item_groups_goal_distance_event_data, groups_to_update);
								remove_iterators.resize(item_groups_heads.size() * 2ll);
							}

							const long long item_group_head_index = next_goal - item_groups_heads.begin();
							item_groups_heads.emplace(next_goal, *goal_distance, (*next_goal).next_item_group_index);
							item_groups_goal_distance_event_data.emplace(item_groups_goal_distance_event_data.begin() + item_group_head_index);
							item_groups_goal_item_count.emplace(item_groups_goal_item_count.begin() + (next_goal - item_groups_heads.begin()), 0ull);

							if (goal_distance_needs_resize)
								mem::utility::validate_pointers_from_indexes(item_groups_goal_distance_event_data, index_container, groups_to_update);

							return true;
						}
						else --goal_distance;
					} while (first_distance != goal_distance);
				}

				begin_goal = next_goal;
			}
		}

		return false;
	};

public:
	template<belt_utility::belt_direction direction, belt_utility::distance_slot_inserted_position position_setting>
	constexpr belt_utility::need_new_slot_result check_if_new_head_slot_is_needed(long long distance, _vector_item_groups_head_type::iterator item_group_head, long long distance_between_inserted_index) const noexcept
	{
		//TODO need to fix so that it handles adding new slots correctly when the previous distance - item_size is from the previous inserter slot
		const auto end_distance = get_end_distance_direction<direction>();
		const long long item_group_head_index = item_group_head - item_groups_heads.begin();

		if (item_group_head != item_groups_heads.last())
		{
			if (item_group_head_index >= inserters.size())
				return belt_utility::need_new_slot_result::update_pointer_to_new_index;

			const decltype(inserters)::iterator begin_inserter_iter = inserters.begin() + item_group_head_index;
			const auto distance_comparison = belt_utility::get_distance_comparison<direction>(get_end_distance_direction<direction>(), distance, begin_inserter_iter, begin_inserter_iter, decltype(item_groups)::value_type::belt_item_size);

			switch (distance_comparison)
			{
				case belt_utility::distance_comparison::null:
				case belt_utility::distance_comparison::distance_is_before: return belt_utility::need_new_slot_result::update_pointer_to_new_index;
				case belt_utility::distance_comparison::distance_is_after: return belt_utility::need_new_slot_result::need_new_slot;
				case belt_utility::distance_comparison::distance_is_inside: return belt_utility::need_new_slot_result::object_is_between_slots;
			}
		}

		decltype(inserters)::iterator begin_inserter_iter;
		if constexpr (belt_utility::distance_slot_inserted_position::new_item_after_last_goal != position_setting) begin_inserter_iter = inserters.begin();

		if constexpr (belt_utility::distance_slot_inserted_position::new_item_after_last_goal == position_setting)
		{
			if (item_group_head != item_groups_heads.last()) begin_inserter_iter = inserters.begin() + item_group_head_index;
			else
			{
				const auto inserter_size = (item_groups_heads.size() - 1ll);
				if (inserter_size < inserters.size()) begin_inserter_iter = inserters.begin() + inserter_size;
				else begin_inserter_iter = inserters.begin();
			}
		}

		auto distance_comparison = belt_utility::get_distance_comparison<direction>(get_end_distance_direction<direction>(), distance, begin_inserter_iter, begin_inserter_iter, decltype(item_groups)::value_type::belt_item_size);
		if (belt_utility::distance_comparison::distance_is_after == distance_comparison)
		{
			do
			{
				++begin_inserter_iter;
				if (begin_inserter_iter != inserters.last()) distance_comparison = belt_utility::get_distance_comparison<direction>(get_end_distance_direction<direction>(), distance, begin_inserter_iter, begin_inserter_iter, decltype(item_groups)::value_type::belt_item_size);
			} while (begin_inserter_iter != inserters.last() && belt_utility::distance_comparison::distance_is_after == distance_comparison);
		}
		if (belt_utility::distance_comparison::distance_is_before == distance_comparison)
		{
			if (item_group_head_index == begin_inserter_iter - inserters.begin())
				return belt_utility::need_new_slot_result::update_pointer_to_new_index;
		}

		if (begin_inserter_iter != inserters.begin() && begin_inserter_iter != inserters.last())
		{
			auto previous_inserter_iter = begin_inserter_iter;

			const auto inserter_distance = end_distance - belt_utility::get_direction_position<direction>(begin_inserter_iter->operator[](0).get_position());
			if (inserter_distance < distance) previous_inserter_iter = begin_inserter_iter - 1ll;

			decltype(distance_comparison) previous_distance_comp = decltype(distance_comparison)::null;
			decltype(distance_comparison) prev_previous_distance_comp = decltype(distance_comparison)::null;

			previous_distance_comp = belt_utility::get_distance_comparison<direction>(get_end_distance_direction<direction>(), distance, previous_inserter_iter, previous_inserter_iter, decltype(item_groups)::value_type::belt_item_size);
			prev_previous_distance_comp = belt_utility::get_distance_comparison<direction>(get_end_distance_direction<direction>(), *(item_groups_distance_between.begin() + distance_between_inserted_index - 1ll).operator->(), previous_inserter_iter, previous_inserter_iter, decltype(item_groups)::value_type::belt_item_size);

			{
				using enum belt_utility::distance_comparison; //just to avoid belt_utility::distance_comparison::
				if (!(distance_is_inside == previous_distance_comp && distance_is_inside == prev_previous_distance_comp) &&
					!(distance_is_inside == previous_distance_comp && distance_is_before == prev_previous_distance_comp) &&
					distance_is_before != previous_distance_comp && distance_is_after != prev_previous_distance_comp)
					return belt_utility::need_new_slot_result::update_pointer_to_new_index;
			}
		}
		else if (begin_inserter_iter == inserters.last() && item_groups_heads.size() == inserters.size()) return belt_utility::need_new_slot_result::update_pointer_to_new_index;

		return belt_utility::need_new_slot_result::need_new_slot;
	};

	template<belt_utility::belt_direction direction, belt_utility::distance_slot_inserted_position position_setting>
	constexpr belt_utility::need_new_slot_result need_new_goal_distance_slot(long long distance, long long distance_between_inserted_index) noexcept
	{
		if constexpr (ENABLE_CPP_EXCEPTION_THROW)
		{
			if (distance_between_inserted_index - 1ll < 0) throw std::runtime_error("negative index");
			if (distance_between_inserted_index - 1ll >= item_groups_distance_between.size()) throw std::runtime_error("index is larger than the vector size");
		}

		if (inserters.empty())
		{
			if (item_groups_heads.size() == 1ll)
			{
				item_groups_heads[0].distance = distance;
				return belt_utility::need_new_slot_result::update_pointer_to_new_index;
			}

			for (auto begin_iter = item_groups_heads.begin(), next_iter = begin_iter + 1ll, end_iter = item_groups_heads.last(); next_iter != end_iter; ++begin_iter, ++next_iter)
			{
				//object belongs to next_iter
				if (begin_iter->distance > distance && distance < next_iter->distance) return belt_utility::need_new_slot_result::object_is_between_slots;

				//we should update the pointer to the new index
				if (next_iter == end_iter - 1ll)
				{
					next_iter->distance = distance;
					return belt_utility::need_new_slot_result::update_pointer_to_new_index;
				}
			};
		}
		else
		{
			bool skip_loop = false;

			//TODO need to fix so that it handles adding new slots correctly when the previous distance - item_size is from the previous inserter slot
			const auto end_distance = get_end_distance_direction<direction>();
			decltype(inserters)::iterator begin_inserter_iter;
			if constexpr (belt_utility::distance_slot_inserted_position::new_item_after_last_goal != position_setting) begin_inserter_iter = inserters.begin();

			if constexpr (belt_utility::distance_slot_inserted_position::new_item_after_last_goal == position_setting)
			{
				const auto inserter_size = (item_groups_heads.size() - 1ll);
				if (inserter_size < inserters.size()) begin_inserter_iter = inserters.begin() + inserter_size;
				else begin_inserter_iter = inserters.begin();
			}

			auto distance_comparison = belt_utility::get_distance_comparison<direction>(get_end_distance_direction<direction>(), distance, begin_inserter_iter, begin_inserter_iter, decltype(item_groups)::value_type::belt_item_size);
			if (belt_utility::distance_comparison::distance_is_after == distance_comparison)
			{
				do
				{
					++begin_inserter_iter;
					if (begin_inserter_iter != inserters.last()) distance_comparison = belt_utility::get_distance_comparison<direction>(get_end_distance_direction<direction>(), distance, begin_inserter_iter, begin_inserter_iter, decltype(item_groups)::value_type::belt_item_size);
				} while (begin_inserter_iter != inserters.last() && belt_utility::distance_comparison::distance_is_after == distance_comparison);
			}

			if (begin_inserter_iter != inserters.begin() && begin_inserter_iter != inserters.last())
			{
				auto previous_inserter_iter = begin_inserter_iter;

				const auto inserter_distance = end_distance - belt_utility::get_direction_position<direction>(begin_inserter_iter->operator[](0).get_position());
				if (inserter_distance < distance) previous_inserter_iter = begin_inserter_iter - 1ll;

				decltype(distance_comparison) previous_distance_comp = decltype(distance_comparison)::null;
				decltype(distance_comparison) prev_previous_distance_comp = decltype(distance_comparison)::null;

				previous_distance_comp = belt_utility::get_distance_comparison<direction>(get_end_distance_direction<direction>(), distance, previous_inserter_iter, previous_inserter_iter, decltype(item_groups)::value_type::belt_item_size);
				prev_previous_distance_comp = belt_utility::get_distance_comparison<direction>(get_end_distance_direction<direction>(), *(item_groups_distance_between.begin() + distance_between_inserted_index - 1ll).operator->(), previous_inserter_iter, previous_inserter_iter, decltype(item_groups)::value_type::belt_item_size);

				{
					using enum belt_utility::distance_comparison; //just to avoid belt_utility::distance_comparison::
					if (!(distance_is_inside == previous_distance_comp && distance_is_inside == prev_previous_distance_comp) &&
						!(distance_is_inside == previous_distance_comp && distance_is_before == prev_previous_distance_comp) &&
						distance_is_before != previous_distance_comp && distance_is_after != prev_previous_distance_comp)
						skip_loop = true;
				}
			}
			else if (begin_inserter_iter == inserters.last() && item_groups_heads.size() == inserters.size()) skip_loop = true;

			if (item_groups_heads.size() == inserters.size() && belt_utility::distance_comparison::distance_is_after == distance_comparison) skip_loop = true;

			if (skip_loop == false)
			{
				auto begin_iter = item_groups_heads.begin();
				auto next_iter = begin_iter + 1ll;
				const auto end_iter = item_groups_heads.last();
				for (; begin_iter != end_iter; ++begin_iter, ++next_iter)
				{
					if ((*begin_iter).distance == *(item_groups_distance_between.begin() + (distance_between_inserted_index - 1ll)))
					{
						const auto iter_index = begin_iter - item_groups_heads.begin();
						begin_iter->distance = distance;

						auto previous_iter = (item_groups_distance_between.begin() + begin_iter->next_item_group_index) - 1ll;
						*previous_iter = item_groups_type::belt_item_size;

						update_event_tick<direction>(iter_index);

						return belt_utility::need_new_slot_result::update_pointer_to_new_index;
					}
				}
			}
		}

		return belt_utility::need_new_slot_result::need_new_slot;
	};

	constexpr long long calculate_distance_(_vector_distance::iterator from, _vector_item_groups_head_type::iterator too) const noexcept
	{
		long long calc_dist = 0ll;
		while (*from != too->distance)
		{
			calc_dist += *from;
			++from;
		}

		return calc_dist;
	};

private:
	template<belt_utility::belt_direction direction>
	constexpr bool add_item_before(const item_uint& new_item, closest_item_group_result iter, _vector_item_groups_head::iterator item_group_head, long long index_ptr_temp) noexcept
	{
		auto& new_data_group = item_groups_data.emplace_back();
		const auto new_iter_group = item_groups.emplace(iter.result);

		auto new_distance_value = get_end_distance_direction<direction>() - belt_utility::get_direction_position<direction>(new_item.position);

		if (item_group_head != item_groups_heads.last()) new_distance_value = belt_utility::get_direction_position<direction>(new_item.position) - calculate_distance_(item_groups_distance_between.begin() + index_ptr_temp, item_group_head);

		const auto new_distance_iter = item_groups_distance_between.emplace(item_groups_distance_between.begin() + index_ptr_temp, new_distance_value);

		if (new_iter_group != item_groups.last()) new_iter_group->add_item(get_end_distance_direction<direction>(), new_distance_iter.operator->(), new_data_group, new_item, new_item.position);
		return true;
	};

	template<belt_utility::belt_direction direction>
	constexpr bool add_item_after_last(const item_uint& new_item, _vector_item_groups_head::iterator item_group_head) noexcept
	{
		const long long item_distance_position = belt_utility::get_direction_position<direction>(new_item.position);
		long long new_distance_value = get_end_distance_direction<direction>() - item_distance_position;

		const auto new_slot_result = check_if_new_head_slot_is_needed<direction, belt_utility::distance_slot_inserted_position::new_item_after_last_goal>(new_distance_value, item_group_head, item_groups_distance_between.size() - 1ll);
		if (belt_utility::need_new_slot_result::need_new_slot == new_slot_result)
		{
			const bool goal_distance_needs_resize = item_groups_heads.needs_resize();
			mem::vector<mem::vector<long long>> index_container;
			if (goal_distance_needs_resize)
			{
				index_container = mem::utility::validate_pointers_get_indexes(item_groups_goal_distance_event_data, groups_to_update);
				remove_iterators.resize(item_groups_heads.size() * 2ll);
			}

			auto& new_item_group_head = item_groups_heads.emplace_back(new_distance_value, item_groups.size());
			event_tick_data& inserted_event_data = item_groups_goal_distance_event_data.emplace_back();
			item_groups_goal_item_count.emplace_back(1ull);

			if (goal_distance_needs_resize)
				mem::utility::validate_pointers_from_indexes(item_groups_goal_distance_event_data, index_container, groups_to_update);

			add_event_tick_zero_update_group(&inserted_event_data);

			new_item_group_head.item_group.add_item(get_end_distance_direction<direction>(), &new_item_group_head.distance, new_item_group_head.item_group_data, new_item, new_item.position);
		}
		else
		{
			item_groups_data.emplace_back(std::move(item_group_head->item_group_data));
			item_groups.emplace_back(std::move(item_group_head->item_group));
			item_groups_distance_between.emplace_back(item_group_head->distance - new_distance_value);
			++item_group_head->next_item_group_index;
			item_group_head->distance = new_distance_value;

			item_group_head->item_group.add_item(get_end_distance_direction<direction>(), &item_group_head->distance, item_group_head->item_group_data, new_item, new_item.position);
			++item_groups_goal_item_count[item_group_head - item_groups_heads.begin()];
		}

		if constexpr (ENABLE_CPP_EXCEPTION_THROW) if (!inserters.empty() && item_groups_heads.size() > inserters.size() + 1) throw std::runtime_error("");
		return true;
	};

	template<belt_utility::belt_direction direction>
	constexpr bool add_item_after(const item_uint& new_item, closest_item_group_result iter, _vector_item_groups_head::iterator item_group_head, long long index_ptr_temp) noexcept
	{
		auto new_data_group = item_groups_data.emplace(item_groups_data.begin() + (iter.result - item_groups.begin()));
		const auto new_iter_group = item_groups.emplace(iter.result);

		auto new_distance_value = get_end_distance_direction<direction>() - belt_utility::get_direction_position<direction>(new_item.position);
		const auto goal_object = belt_utility::get_goal_object_index_binary(index_ptr_temp, item_groups_heads, item_groups_distance_between);
		if (goal_object != item_groups_heads.last() && item_groups_heads.size() > 1ll) new_distance_value = belt_utility::get_direction_position<direction>(new_item.position) - calculate_distance_(item_groups_distance_between.begin() + (index_ptr_temp + 1), goal_object);

		const auto new_distance_iter = item_groups_distance_between.emplace(item_groups_distance_between.begin() + (index_ptr_temp + 1), new_distance_value);

		const auto new_slot_result = need_new_goal_distance_slot<direction, belt_utility::distance_slot_inserted_position::new_item_after_iter>(*new_distance_iter, new_distance_iter - item_groups_distance_between.begin());
		if (belt_utility::need_new_slot_result::need_new_slot == new_slot_result)
		{
			const bool goal_distance_needs_resize = item_groups_heads.needs_resize();
			mem::vector<mem::vector<long long>> index_container;
			if (goal_distance_needs_resize)
			{
				index_container = mem::utility::validate_pointers_get_indexes(item_groups_goal_distance_event_data, groups_to_update);
				remove_iterators.resize(item_groups_heads.size() * 2ll);
			}

			item_groups_heads.emplace_back(*new_distance_iter, new_distance_iter - item_groups_distance_between.begin());
			auto& inserted_event_data = item_groups_goal_distance_event_data.emplace_back();
			item_groups_goal_item_count.emplace_back(1ull);

			if (goal_distance_needs_resize)
				mem::utility::validate_pointers_from_indexes(item_groups_goal_distance_event_data, index_container, groups_to_update);

			add_event_tick_zero_update_group(&inserted_event_data);
		}

		if constexpr (ENABLE_CPP_EXCEPTION_THROW) if (!inserters.empty() && item_groups_heads.size() > inserters.size() + 1) throw std::runtime_error("");

		if (new_iter_group != item_groups.last()) new_iter_group->add_item(get_end_distance_direction<direction>(), new_distance_iter.operator->(), *new_data_group, new_item, new_item.position);
		return true;
	};

	template<belt_utility::belt_direction direction>
	constexpr bool split_item_group(const item_uint& new_item, closest_item_group_result iter, _vector_item_groups_head::iterator item_group_head, long long index_ptr_temp) noexcept
	{
		const auto end_distance_direction = get_end_distance_direction<direction>();
		const auto index = iter.result->get_first_item_before_position<direction>(
			end_distance_direction,
			item_groups_distance_between[index_ptr_temp],
			item_groups_data[index_ptr_temp],
			belt_utility::get_direction_position<direction>(new_item.position)
		);

		const auto goal_object = belt_utility::get_goal_object_index_binary(index_ptr_temp, item_groups_heads, item_groups_distance_between);
		bool is_not_goal_object = true;
		if (goal_object->distance == item_groups_distance_between[index_ptr_temp])
		{
			//goal_object->update_pointers_without_checks(1ll);
			is_not_goal_object = false;
		}

		auto split_group = item_groups[index_ptr_temp].split_from_index(index);
		const auto split_data_result = item_data_utility::split_from_index(item_groups_data[index_ptr_temp], index);
		const auto new_distance = item_groups_data[index_ptr_temp].item_distance[item_groups[index_ptr_temp].count() - 1ll] + split_data_result.missing_distance;

		const auto inserted_group = item_groups.insert(item_groups.begin() + index_ptr_temp, split_group);
		const auto inserted_data = item_groups_data.insert(item_groups_data.begin() + index_ptr_temp, split_data_result.data);
		const auto inserted_distance = item_groups_distance_between.insert(item_groups_distance_between.begin() + index_ptr_temp, new_distance);

		long long added_index = -1;
		if (is_not_goal_object == false)
		{
			const auto calc_dist = belt_utility::get_distances_from_to2(item_groups_distance_between.begin() + goal_object->next_item_group_index, inserted_distance);
			added_index = inserted_group->add_item(end_distance_direction, calc_dist, inserted_distance.operator->(), *inserted_data, new_item, new_item.position);
		}
		else added_index = inserted_group->add_item(end_distance_direction, inserted_distance.operator->(), *inserted_data, new_item, new_item.position);

		return added_index != -1;
	};

public:
	template<belt_utility::belt_direction direction>
	constexpr void update_all_event_ticks() noexcept
	{
		const long long l = item_groups_heads.size();
		for (long long i = 0; i < l; ++i)
			update_event_tick<direction>(i);
	};

	constexpr bool add_item(const item_uint& new_item, const bool update_event_tick_ = true) noexcept
	{
		using enum belt_utility::belt_direction;
		switch (segment_direction)
		{
			default:
			case null:
			case left_right: return add_item<left_right>(new_item, update_event_tick_); break;
			case right_left: return add_item<right_left>(new_item, update_event_tick_); break;
			case top_bottom: return add_item<top_bottom>(new_item, update_event_tick_); break;
			case bottom_top: return add_item<bottom_top>(new_item, update_event_tick_); break;
		}
	};

private:
	template<belt_utility::belt_direction direction>
	constexpr bool add_item(const item_uint& new_item, const bool update_event_tick_) noexcept
	{
		//TODO Need to account for when items are added but the groups_to_update assosicated with the group we're about to add in
		//hasn't been ticked, meaning it's "behind" in time. So take current tick and subtract the assosicated groups_to_update start tick
		//use that to offset the new_item's position to rewind it to offset the time dilation going on
		//meaning it will appear to move backward in distance, but once it ticks again will be corrected

		const auto new_goal_distance = get_end_distance_direction<direction>() - belt_utility::get_direction_position<direction>(new_item.position);

		if (item_groups_heads.size() == 0)
		{
			const bool goal_distance_needs_resize = item_groups_heads.needs_resize();
			mem::vector<mem::vector<long long>> index_container;
			if (goal_distance_needs_resize)
			{
				index_container = mem::utility::validate_pointers_get_indexes(item_groups_goal_distance_event_data, groups_to_update);
				remove_iterators.resize(item_groups_heads.size() * 2ll);
			}

			item_groups_head_t& item_groups_head = item_groups_heads.emplace_back(new_goal_distance);
			auto& inserted_event_data = item_groups_goal_distance_event_data.emplace_back();
			item_groups_goal_item_count.emplace_back(1ull);

			if (goal_distance_needs_resize)
				mem::utility::validate_pointers_from_indexes(item_groups_goal_distance_event_data, index_container, groups_to_update);

			const long long added_index = item_groups_head.item_group.add_item(get_end_distance_direction<direction>(), &item_groups_head.distance, item_groups_head.item_group_data, new_item, new_item.position);
			if (added_index == -1) return false;

			add_event_tick_zero_update_group(&inserted_event_data);
			return true;
		}
		else
		{
			const _vector_item_groups_head::iterator item_group_head = get_head_item_belongs_too(new_goal_distance);
			if (item_group_head == item_groups_heads.last()) return false;

			const long long head_index = item_group_head - item_groups_heads.begin();
			const closest_item_group_result iter = belt_utility::find_closest_item_group<direction>(
				get_end_distance_direction<direction>(),
				item_groups_data,
				item_groups,
				item_groups_distance_between,
				item_group_head,
				head_index,
				item_groups_heads.size(),
				belt_utility::get_direction_position<direction>(new_item.position),
				inserters
			);
			const ptrdiff_t index_ptr_temp = iter.result - item_groups.begin();

			switch (iter.scan)
			{
				case belt_utility::find_closest_item_group_return_result::invalid_value: return false;
				case belt_utility::find_closest_item_group_return_result::insert_into_group:
				{
					if (iter.result == item_groups.last())
					{
						if (item_group_head->item_group.count() >= item_groups_type::max_item_count) return split_item_group<direction>(new_item, iter, item_group_head, item_group_head->next_item_group_index + 1ll);
						else
						{
							const long long added_index = item_group_head->item_group.add_item(get_end_distance_direction<direction>(), &item_group_head->distance, item_group_head->item_group_data, new_item, new_item.position);
							if (added_index != -1)
							{
								++item_groups_goal_item_count[head_index];
								if (update_event_tick_ == true) update_event_tick<direction>(head_index);
								return true;
							}
						}
					}
					else
					{
						if (iter.result->count() >= item_groups_type::max_item_count) return split_item_group<direction>(new_item, iter, item_group_head, index_ptr_temp);
						else
						{
							const long long added_index = iter.result->add_item(get_end_distance_direction<direction>(), &item_groups_distance_between[index_ptr_temp], item_groups_data[index_ptr_temp], new_item, new_item.position);
							if (added_index != -1)
							{
								++item_groups_goal_item_count[head_index];
								if (update_event_tick_ == true) update_event_tick<direction>(head_index);
								return true;
							}
						}
					}

					return false;
				}
				case belt_utility::find_closest_item_group_return_result::new_group_before_iter: return add_item_before<direction>(new_item, iter, item_group_head, index_ptr_temp);
				case belt_utility::find_closest_item_group_return_result::new_group_after_iter:
				{
					if (iter.result == item_groups.last())
						return add_item_after_last<direction>(new_item, item_group_head);
					else
						return add_item_after<direction>(new_item, iter, item_group_head, index_ptr_temp);
				}
			}
		}

		return false;
	};

public:
	template<belt_utility::belt_direction direction>
	constexpr long long get_inserter_group_index(auto& group_inserters, index_inserter object) const noexcept
	{
		const long long l = group_inserters.size();
		if (l == 1ll)
		{
			if (belt_utility::inserter_fits_results::before == belt_utility::is_inserter_before_or_after<direction>(group_inserters[0], object)) return 0;
			else return 1;
		}

		for (long long i = 0, i2 = 1; i2 < l; ++i, ++i2)
		{
			const auto test_result = belt_utility::is_inserter_before_or_after<direction>(group_inserters[i], object);
			if (belt_utility::inserter_fits_results::before == test_result) return i;
			else if (belt_utility::inserter_fits_results::after == test_result)
			{
				const auto second_test_result = belt_utility::is_inserter_before_or_after<direction>(group_inserters[i2], object);
				if (belt_utility::inserter_fits_results::before == second_test_result) return i;
			}
		}

		if (belt_utility::inserter_fits_results::before == belt_utility::is_inserter_before_or_after<direction>(group_inserters.back(), object)) return group_inserters.size() - 1ll;
		else return group_inserters.size();
	};

	constexpr belt_utility::add_inserter_return_indexes add_inserter(index_inserter object) noexcept
	{
		using enum belt_utility::belt_direction;
		switch (segment_direction)
		{
			default:
			case null:
			case left_right: return add_inserter<left_right>(object); break;
			case right_left: return add_inserter<right_left>(object); break;
			case top_bottom: return add_inserter<top_bottom>(object); break;
			case bottom_top: return add_inserter<bottom_top>(object); break;
		}
	};

private:
	template<belt_utility::belt_direction direction>
	constexpr belt_utility::add_inserter_return_indexes add_inserter(index_inserter object) noexcept
	{
		constexpr const auto max_inserter_distance = item_groups_type::belt_item_size * item_groups_type::max_item_count * 4; //TODO magic numba,, what even is this 4
		const auto object_direction_position = belt_utility::get_direction_position<direction>(object.get_position());

		if (inserters.size() == 0)
		{
			inserters.emplace_back();
			object.set_distance_position_plus(get_end_distance_direction<direction>() - object_direction_position);
			object.set_distance_position_minus(get_end_distance_direction<direction>() - object_direction_position - item_groups_type::belt_item_size);
			inserters[0].emplace_back(object);

			inserter_slots.emplace_back(0ull, 0ull, &inserters);
			return { 0ll, 0ll };
		}

		long long can_fit_index = -1ll;
		long long can_fit_nested_index = -1ll;
		auto insert_results = belt_utility::inserter_fits_results::no_fit;
		if (inserters.size() == 1ll && inserters[0].size() == 1ll)
		{
			if (object_direction_position - belt_utility::get_direction_position<direction>(inserters[0ll][0ll].get_position()) >= max_inserter_distance &&
				object_direction_position - belt_utility::get_direction_position<direction>(inserters[0ll].back().get_position()) >= max_inserter_distance)
			{
				insert_results = belt_utility::inserter_fits_results::no_fit;
				can_fit_index = inserters.size();
				can_fit_nested_index = 0ll;
			}
			else
			{
				can_fit_index = 0ll;
				can_fit_nested_index = get_inserter_group_index<direction>(inserters[0ll], object);
			}
		}
		else
		{
			const auto inserter_first_distance = object_direction_position - belt_utility::get_direction_position<direction>(inserters[0][0].get_position());
			const auto inserter_first_last_distance = object_direction_position - belt_utility::get_direction_position<direction>(inserters[0].back().get_position());
			const auto inserter_last_distance = object_direction_position - belt_utility::get_direction_position<direction>(inserters.back()[0].get_position());
			const auto inserter_last_last_distance = object_direction_position - belt_utility::get_direction_position<direction>(inserters.back().back().get_position());

			if (inserter_last_distance < 0ll && inserter_last_distance <= max_inserter_distance && inserter_last_last_distance <= max_inserter_distance)
			{
				insert_results = belt_utility::inserter_fits_results::inbetween;
				can_fit_index = inserters.size() - 1ll;
				can_fit_nested_index = get_inserter_group_index<direction>(inserters.back(), object);
			}
			else if (inserter_first_distance < 0ll && inserter_first_distance <= max_inserter_distance && inserter_first_last_distance <= max_inserter_distance)
			{
				insert_results = belt_utility::inserter_fits_results::inbetween;
				can_fit_index = 0ll;
				can_fit_nested_index = get_inserter_group_index<direction>(inserters[0], object);
			}
			else if (inserter_first_distance >= max_inserter_distance && inserter_first_last_distance >= max_inserter_distance)
			{
				insert_results = belt_utility::inserter_fits_results::no_fit;
				can_fit_index = inserters.size();
				can_fit_nested_index = 0ll;
			}
			else if (inserter_last_distance >= max_inserter_distance && inserter_last_last_distance >= max_inserter_distance)
			{
				insert_results = belt_utility::inserter_fits_results::no_fit;
				can_fit_index = inserters.size();
				can_fit_nested_index = 0ll;
			}
			//first check if it's before the first inserter
			else if (belt_utility::inserter_fits_results::before == belt_utility::is_inserter_before_or_after<direction>(inserters[0][0], object))
			{
				insert_results = belt_utility::inserter_fits_results::before;
				can_fit_index = 0ll;
				can_fit_nested_index = get_inserter_group_index<direction>(inserters[0ll], object);
			}
			//second check if it's after the last inserter
			else if (belt_utility::inserter_fits_results::after == belt_utility::is_inserter_before_or_after<direction>(inserters.back()[0], object))
			{
				insert_results = belt_utility::inserter_fits_results::after;
				can_fit_index = inserters.size() - 1ll;
				can_fit_nested_index = get_inserter_group_index<direction>(inserters.back(), object);
			}
			else
			{
				//check if we can place it in between inserters
				const long long l = inserters.size();
				for (long long i = 0ll, i2 = 1ll; i2 < l; ++i, ++i2)
				{
					if (object_direction_position - belt_utility::get_direction_position<direction>(inserters[i][0].get_position()) >= max_inserter_distance &&
						object_direction_position - belt_utility::get_direction_position<direction>(inserters[i].back().get_position()) >= max_inserter_distance)
					{
						insert_results = belt_utility::inserter_fits_results::no_fit;
						can_fit_index = l;
						can_fit_nested_index = get_inserter_group_index<direction>(inserters[i2], object);
						break;
					}
					else
					{
						if (belt_utility::inserter_fits_results::inbetween == belt_utility::is_inserter_between<direction>(inserters[i][0], inserters[i2][0], object))
						{
							insert_results = belt_utility::inserter_fits_results::inbetween;
							can_fit_index = i2;
							can_fit_nested_index = get_inserter_group_index<direction>(inserters[i2], object);
						}
					}
				}
			}
		}

		if (can_fit_index != -1ll)
		{
			if (belt_utility::inserter_fits_results::no_fit == insert_results && can_fit_index == inserters.size())
			{
				auto& group = inserters.emplace_back();
				object.set_distance_position_plus(get_end_distance_direction<direction>() - object_direction_position);
				object.set_distance_position_minus(get_end_distance_direction<direction>() - object_direction_position - item_groups_type::belt_item_size);
				group.emplace_back(object);

				inserter_slots.emplace_back(static_cast<std::size_t>(can_fit_index), 0ull, &inserters);
			}
			else
			{
				inserters[can_fit_index].insert(inserters[can_fit_index].begin() + can_fit_nested_index, object);

				(inserters[can_fit_index].begin() + can_fit_nested_index)->set_distance_position_plus(get_end_distance_direction<direction>() - object_direction_position);
				(inserters[can_fit_index].begin() + can_fit_nested_index)->set_distance_position_minus(get_end_distance_direction<direction>() - object_direction_position - item_groups_type::belt_item_size);
			}

			return { can_fit_index, can_fit_nested_index };
		}

		return { -1ll, -1ll };
	};

public:
	inline constexpr void add_end_segment_section(belt_segment* ptr) noexcept
	{
		segment_end_points.push_back(ptr);
	};
};