#pragma once

#include <stdexcept>
#include <vector>
//#include <corecrt_terminate.h>
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
#include <exception>
#include <limits>

struct linked_access_wrapper
{
	const mem::vector<int> a{ 4 };
	const mem::vector<float> b{ 4 };
	const mem::vector<void*> c{ 4 };
};

constexpr static size_t get_tick_time_until_event(size_t tick_count, size_t distance, size_t travel_per_tick) noexcept
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

template<belt_utility::belt_direction segment_direction>
class b_test
{
	mem::vector<b_test*> vector;
};

using _simple_inserter_vector = mem::vector<index_inserter, mem::Allocating_Type::ALIGNED_NEW, mem::allocator<index_inserter, mem::Allocating_Type::ALIGNED_NEW>, mem::use_memcpy::force_checks_off>;

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

	//protected:
	vec2_int64 start_of_segment{ 0, 0 };
	vec2_int64 end_of_segment{ 0, 0 };

	_data_vector item_groups_data{ 4 };
	_vector item_groups{ 4 };
	//contains the distance between item_groups for all item_groups
	//the value is the distance to the end of the belt segment
	_vector_distance item_groups_distance_between{ 4 };
	mem::vector<size_t, mem::Allocating_Type::ALIGNED_NEW> item_groups_goal_item_count{ 4 };

	//contains the item_groups_goal distance of the first item_group
	_vector_goal_distance item_groups_goal_distance{ 4 };
	mem::vector<event_tick_data, mem::Allocating_Type::ALIGNED_NEW> item_groups_goal_distance_event_data{ 4 };
	mem::vector<mem::vector<event_tick_data*, mem::Allocating_Type::ALIGNED_NEW>, mem::Allocating_Type::ALIGNED_NEW> groups_to_update{ 2 };

	//mem::vector<long long, mem::Allocating_Type::ALIGNED_NEW> groups_to_update_indexes{ 4 };

	mem::vector<_simple_inserter_vector, mem::Allocating_Type::ALIGNED_NEW, mem::allocator<_simple_inserter_vector, mem::Allocating_Type::ALIGNED_NEW>, mem::use_memcpy::force_checks_off> inserters{ 4 };
	mem::vector<double_index_iterator<index_inserter, decltype(belt_segment::inserters)>, mem::Allocating_Type::ALIGNED_NEW, mem::allocator<double_index_iterator<index_inserter, decltype(belt_segment::inserters)>, mem::Allocating_Type::ALIGNED_NEW>, mem::use_memcpy::force_checks_off> inserter_slots{ 4 };
	mem::vector<remove_iterators_, mem::Allocating_Type::ALIGNED_NEW> remove_iterators{ 2 };

	// belt_segments that this segment moves items onto
	mem::vector<belt_segment*> segment_end_points{ 4 };
	// belt_segments that will move items onto this segment
	mem::vector<belt_segment*> connected_segments{ 4 };

	long long first_group_to_update{ 0ll };
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
	{};

	constexpr belt_segment(vec2_int64 start, vec2_int64 end, belt_segment_correct_t) noexcept :
		start_of_segment{ start },
		end_of_segment{ end }
	{};

	constexpr ~belt_segment() noexcept
	{};

	constexpr belt_segment(const belt_segment& o) noexcept :
		start_of_segment{ o.start_of_segment },
		end_of_segment{ o.end_of_segment },
		item_groups_data{ o.item_groups_data },
		item_groups{ o.item_groups },
		item_groups_goal_distance{ o.item_groups_goal_distance },
		item_groups_goal_distance_event_data{ o.item_groups_goal_distance_event_data },
		groups_to_update{ o.groups_to_update },
		item_groups_distance_between{ o.item_groups_distance_between },
		item_groups_goal_item_count{ o.item_groups_goal_item_count },
		inserters{ o.inserters },
		inserter_slots{ o.inserter_slots },
		segment_end_points{ o.segment_end_points },
		connected_segments{ o.connected_segments },
		remove_iterators{ o.remove_iterators },
		segment_direction{ o.segment_direction },
		item_was_removed{ o.item_was_removed }
	{};

	constexpr belt_segment(belt_segment&& o) noexcept :
		start_of_segment{ std::exchange(o.start_of_segment, vec2_int64{}) },
		end_of_segment{ std::exchange(o.end_of_segment, vec2_int64{}) },
		item_groups_data{ std::exchange(o.item_groups_data, decltype(item_groups_data){}) },
		item_groups{ std::exchange(o.item_groups, decltype(item_groups){}) },
		item_groups_goal_distance{ std::exchange(o.item_groups_goal_distance, decltype(item_groups_goal_distance){}) },
		item_groups_goal_distance_event_data{ std::exchange(o.item_groups_goal_distance_event_data, decltype(item_groups_goal_distance_event_data){}) },
		groups_to_update{ std::exchange(o.groups_to_update, decltype(groups_to_update){}) },
		item_groups_distance_between{ std::exchange(o.item_groups_distance_between, decltype(item_groups_distance_between){}) },
		item_groups_goal_item_count{ std::exchange(o.item_groups_goal_item_count, decltype(item_groups_goal_item_count){}) },
		inserters{ std::exchange(o.inserters, decltype(inserters){}) },
		inserter_slots{ std::exchange(o.inserter_slots, decltype(inserter_slots){}) },
		segment_end_points{ std::exchange(o.segment_end_points, decltype(segment_end_points){}) },
		connected_segments{ std::exchange(o.connected_segments, decltype(connected_segments){}) },
		remove_iterators{ std::move(o.remove_iterators) },
		segment_direction{ std::move(o.segment_direction) },
		item_was_removed{ o.item_was_removed }
	{};

	constexpr belt_segment& operator=(const belt_segment& o) noexcept
	{
		start_of_segment = o.start_of_segment;
		end_of_segment = o.end_of_segment;
		item_groups_data = o.item_groups_data;
		item_groups = o.item_groups;
		item_groups_goal_distance = o.item_groups_goal_distance;
		item_groups_goal_distance_event_data = o.item_groups_goal_distance_event_data;
		groups_to_update = o.groups_to_update;
		item_groups_distance_between = o.item_groups_distance_between;
		item_groups_goal_item_count = o.item_groups_goal_item_count;
		inserters = o.inserters;
		inserter_slots = o.inserter_slots;
		segment_end_points = o.segment_end_points;
		connected_segments = o.connected_segments;
		remove_iterators = o.remove_iterators;
		item_was_removed = o.item_was_removed;

		return *this;
	};

	constexpr belt_segment& operator=(belt_segment&& o) noexcept
	{
		start_of_segment = std::move(o.start_of_segment);
		end_of_segment = std::move(o.end_of_segment);
		item_groups_data = std::move(o.item_groups_data);
		item_groups = std::move(o.item_groups);
		item_groups_goal_distance = std::move(o.item_groups_goal_distance);
		item_groups_goal_distance_event_data = std::move(o.item_groups_goal_distance_event_data);
		groups_to_update = std::move(o.groups_to_update);
		item_groups_distance_between = std::move(o.item_groups_distance_between);
		item_groups_goal_item_count = std::move(o.item_groups_goal_item_count);
		inserters = std::move(o.inserters);
		inserter_slots = std::move(o.inserter_slots);
		segment_end_points = std::move(o.segment_end_points);
		connected_segments = std::move(o.connected_segments);
		remove_iterators = std::move(o.remove_iterators);
		item_was_removed = o.item_was_removed;

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
		return { item_groups_data.size(), item_groups.size(), item_groups_goal_distance.size(), item_groups_distance_between.size() };
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
		if (item_groups_goal_distance.is_dead) throw std::runtime_error("");
		if (item_groups_distance_between.is_dead) throw std::runtime_error("");
		if (item_groups_data.size() == 0ll) throw std::runtime_error("");
		if (item_groups.size() == 0ll) throw std::runtime_error("");
		if (item_groups_goal_distance.size() == 0ll) throw std::runtime_error("");
		if (item_groups_distance_between.size() == 0ll) throw std::runtime_error("");
#endif

		return true;
	};
private:
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

public:
	template<belt_utility::belt_direction direction>
	inline constexpr item_uint get_item(long long item_group, long long i) noexcept
	{
		ASSERT_NOT_CONSTEXPR<_BOUNDS_CHECKING_>(i < item_groups.size());
		return item_groups[item_group].get(get_end_distance_direction<direction>(), get_direction_y_value<direction>(), item_groups_distance_between[item_group], item_groups_data[item_group], i);
	};

	template<belt_utility::belt_direction direction>
	inline constexpr item_uint remove_item(item_groups_type& item_group, item_groups_data_type& data_group, decltype(item_groups_goal_distance)::iterator goal_object, const long long item_index) noexcept
	{
		auto return_item = item_group.get(get_end_distance_direction<direction>(), get_direction_y_value<direction>(), (*goal_object).get_distance(), data_group, item_index);
		if (item_groups_type::item_removal_result::item_removed_zero_remains == item_group.remove_item(goal_object->get_unsafe_index_ptr(), data_group, item_index)) item_group_has_zero_count(item_group, data_group);

		--item_groups_goal_item_count[(goal_object - item_groups_goal_distance.begin())];

		return return_item;
	};
	template<belt_utility::belt_direction direction>
	inline constexpr item_uint remove_item(const long long index_ptr, item_groups_type* item_group, item_groups_data_type* data_group, decltype(item_groups_goal_distance)::iterator goal_object, const item_groups_type::index_item_position_return& found_item) noexcept
	{
		auto return_item = (*item_group).get(get_end_distance_direction<direction>(), get_direction_y_value<direction>(), (*goal_object).get_distance(), *data_group, found_item);
		if (item_groups_type::item_removal_result::item_removed_zero_remains == (*item_group).remove_item(goal_object->get_unsafe_index_ptr(), *data_group, found_item.found_index))
			remove_iterators.emplace_back(_vector::iterator{ item_group }, _data_vector::iterator{ data_group }, (item_groups_distance_between.begin() + index_ptr), goal_object);
		//item_group_has_zero_count(item_group, data_group, goal_object);

		--item_groups_goal_item_count[(goal_object - item_groups_goal_distance.begin())];
		return return_item;
	};
	template<belt_utility::belt_direction direction>
	inline constexpr item_uint remove_item(item_groups_type* item_group, const long long item_index) noexcept
	{
		const auto index_ptr = item_group - item_groups.begin();
		const auto data_group = item_groups_data.begin() + index_ptr;
		const auto distance_group = item_groups_distance_between.begin() + index_ptr;
		const auto goal_object = belt_utility::get_goal_object_index_binary(index_ptr, item_groups_goal_distance, item_groups_distance_between);

		auto return_item = (*item_group).get(get_end_distance_direction<direction>(), get_direction_y_value<direction>(), (*goal_object).get_distance(), *data_group, item_index);
		if (item_groups_type::item_removal_result::item_removed_zero_remains == (*item_group).remove_item(distance_group.operator->(), *data_group, item_index)) item_group_has_zero_count(item_groups.begin() + index_ptr, data_group);

		--item_groups_goal_item_count[(goal_object - item_groups_goal_distance.begin())];

		//update_event_tick<direction>(find_event_tick_from_index<direction>(goal_object - item_groups_goal_distance.begin()));

		/*if (item_groups_distance_between.size() > 1ll)
		{
			//if were removing the last item so distance_between values needs to have the the item distance value added to them
			//if were removing the first item so we need to subtract item_groups_type::belt_item_size from the distance_between values
			//else if it's in the middle, we don't care since it doesn't effect the distance between
			if (item_index == 0) item_distance = -item_groups_type::belt_item_size;
			else if (item_group->count() < item_index + 1) item_distance = 0;

			auto prev_dist = (goal_object->get_index_ptr() - 1ll);
			*prev_dist += item_distance;
		}*/

		return return_item;
	};
	template<belt_utility::belt_direction direction>
	inline constexpr item_uint remove_item(const long long item_group_index, const long long item_index) noexcept
	{
		return remove_item<direction>(&item_groups[item_group_index], item_index);
	};

	/*inline constexpr const item_uint get_item(std::size_t item_group, std::size_t i) const noexcept
	{
	#ifdef _BOUNDS_CHECKING_
			ASSERT_NOT_CONSTEXPR(i < item_groups.size());
	#endif
			return item_groups[item_group].get(get_end_distance_direction(), get_direction_y_value(), item_groups_data[item_group], i);
		};*/
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

	inline constexpr index_inserter& get_inserter(std::size_t i) noexcept
	{
		std::size_t count_index = 0ull;
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

	inline constexpr const index_inserter& get_inserter(std::size_t i) const noexcept
	{
		std::size_t count_index = 0ull;
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
			if (i >= item_groups_goal_distance.size()) throw std::runtime_error("");
			if (i >= item_groups_distance_between.size()) throw std::runtime_error("");
		}

		const auto goal_object = belt_utility::get_goal_object_index(i, item_groups_goal_distance, item_groups_distance_between);

		if constexpr (_BOUNDS_CHECKING_) if (item_groups_goal_distance.last() == goal_object) return -1ll;

		// || (*goal_object).get_index_ptr() < item_groups_distance_between.begin().operator->() || (*goal_object).get_index_ptr() >= item_groups_distance_between.last().operator->())
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
		return item_groups_goal_distance.size();
	};

	inline constexpr goal_distance get_goal_distance(long long i) noexcept
	{
		ASSERT_NOT_CONSTEXPR<_BOUNDS_CHECKING_>(i < item_groups_goal_distance.size());
		return item_groups_goal_distance[i];
	};

	inline constexpr long long goal_distance_in_destinations(long long i) noexcept
	{
		ASSERT_NOT_CONSTEXPR<_BOUNDS_CHECKING_>(i < item_groups_goal_distance.size());
		return *item_groups_goal_distance[i].get_index_ptr();
	};

	inline constexpr long long count_inserters() const noexcept
	{
		std::size_t count_index = 0ull;
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
	constexpr auto get_closest_item_inserter_can_grab(long long inserter_index, long long cur_dist, item_groups_type& item_groups_ref, item_groups_data_type& item_data_ref) noexcept
	{
		if (item_groups_ref.count() == 0) return item_groups_type::index_item_position_return{};

		auto& nested_inserter_vector = inserters[inserter_index];
		const auto nested_ins_last = nested_inserter_vector.last();

		const auto temp_last_item_position = cur_dist + item_groups_ref.get_distance_to_last_item(item_data_ref);
		const auto end_distance = get_end_distance_direction<direction>();

		for (auto nested_ins_iter = nested_inserter_vector.begin(); nested_ins_iter != nested_ins_last; ++nested_ins_iter)
		{
			//const auto inserter_distance_position = end_distance - inserter_position;
			if (temp_last_item_position >= nested_ins_iter->get_distance_position_minus()) //cur_dist <= nested_ins_iter->get_distance_position_plus() &&
			{
				//const auto inserter_position = belt_utility::get_direction_position<direction>(nested_ins_iter->get_position());
				//const auto found_data = item_groups_ref.get_first_item_of_type_before_position<direction>(end_distance, cur_dist, item_data_ref, nested_ins_iter->get_item_type(0), inserter_position);

				const auto found_data_new = item_groups_ref.get_first_item_of_type_before_position_fast<direction>(cur_dist, item_data_ref, nested_ins_iter->get_item_type(0), nested_ins_iter->get_distance_position_minus());
				//TODO REPLACE FOUND_DATA WITH FOUND_DATA_NEW
				//if (found_data_new.found_index != found_data.found_index) __debugbreak();
				if (found_data_new.found_index != -1ll)
				{
					//bool temp_test = found_data_new.item_distance_position >= nested_ins_iter->get_distance_position_minus();
					//bool temp_old_test = found_data.item_distance_position <= inserter_position + item_groups_type::belt_item_size;
					//if (temp_test != temp_old_test) __debugbreak();
					//if (found_data.item_distance_position <= inserter_position + item_groups_type::belt_item_size)
					if (found_data_new.item_distance_position >= nested_ins_iter->get_distance_position_minus())
						return found_data_new;
				}
			}
		}

		return item_groups_type::index_item_position_return{};
	};

	template<belt_utility::belt_direction direction>
	constexpr void add_event_tick(event_tick_data* current_tick_data) noexcept
	{
		size_t insert_after_index = 0ull;
		const auto current_tick_data_index = current_tick_data - item_groups_goal_distance_event_data.begin().operator->();
		long long distance = this->item_groups_goal_distance[current_tick_data_index].get_distance();//get_end_distance<direction>();
		if (inserters.size() > current_tick_data_index)
		{
			const auto index_ptr = (item_groups_goal_distance.begin() + current_tick_data_index)->get_index_from_ptr(item_groups_distance_between.begin().operator->());
			auto& item_data_ref = item_groups_data[index_ptr];
			auto& item_groups_ref = item_groups[index_ptr];

			const auto can_grab = get_closest_item_inserter_can_grab<direction>(current_tick_data_index, item_groups_goal_distance[current_tick_data_index].get_distance(), item_groups_ref, item_data_ref);
			if (can_grab.found_index != -1ll)
			{
				distance = can_grab.item_distance_position - (inserters[current_tick_data_index].last() - 1ll)->get_distance_position_plus();
				if (distance < 0 && distance >= -32)
					distance = 0;
			}
		}

		if (distance == 0) current_tick_data->tick_time = tick_count + 1;
		else current_tick_data->tick_time = get_tick_time_until_event(tick_count, distance, travel_distance_per_tick);

		current_tick_data->start_tick_time = tick_count;

		for (auto begin_iter = groups_to_update.begin(); begin_iter != groups_to_update.last(); ++begin_iter)
		{
			if ((*begin_iter)[0]->tick_time == current_tick_data->tick_time)
			{
				begin_iter->emplace_back(current_tick_data);
				return;
			}
			else if ((*begin_iter)[0]->tick_time < current_tick_data->tick_time) ++insert_after_index;
		}

		mem::vector<event_tick_data*, mem::Allocating_Type::ALIGNED_NEW> new_group{ 4 };
		new_group.emplace_back(current_tick_data);
		const long long group_size = groups_to_update.size();
		if (insert_after_index < group_size)
			groups_to_update.insert(groups_to_update.begin() + insert_after_index, new_group);
		else
		{
			groups_to_update.emplace_back(new_group);
			//groups_to_update_indexes.emplace_back(group_size);
		}
	};

	constexpr void add_event_tick_no_update_skip(event_tick_data* current_tick_data) noexcept
	{
		size_t insert_after_index = 1ull;
		const size_t length = groups_to_update.size();
		for (size_t i = 1; i < length; ++i)
		{
			if (groups_to_update[i][0] != current_tick_data && groups_to_update[i][0]->tick_time == current_tick_data->tick_time)
			{
				groups_to_update[i].emplace_back(current_tick_data);
				return;
			}
			else if (groups_to_update[i][0]->tick_time < current_tick_data->tick_time) ++insert_after_index;
			else if (groups_to_update[i][0] == current_tick_data) ++insert_after_index;
		}

		mem::vector<event_tick_data*, mem::Allocating_Type::ALIGNED_NEW> new_group{ 4 };
		new_group.emplace_back(current_tick_data);
		const long long group_size = groups_to_update.size();
		if (insert_after_index < group_size)
		{
			groups_to_update.insert(groups_to_update.begin() + insert_after_index, new_group);
			//groups_to_update_indexes.emplace_back(group_size);
		}
		else
		{
			groups_to_update.emplace_back(new_group);
			//groups_to_update_indexes.emplace_back(group_size);
		}
	};
	constexpr void add_event_tick_no_update(event_tick_data* current_tick_data) noexcept
	{
		size_t insert_after_index = 0ull;
		for (auto begin_iter = groups_to_update.begin(); begin_iter != groups_to_update.last(); ++begin_iter)
		{
			if ((*begin_iter)[0] != current_tick_data && (*begin_iter)[0]->tick_time == current_tick_data->tick_time)
			{
				begin_iter->emplace_back(current_tick_data);
				return;
			}
			else if ((*begin_iter)[0]->tick_time < current_tick_data->tick_time) ++insert_after_index;
			else if ((*begin_iter)[0] == current_tick_data) ++insert_after_index;
		}

		mem::vector<event_tick_data*, mem::Allocating_Type::ALIGNED_NEW> new_group{ 4 };
		new_group.emplace_back(current_tick_data);
		const long long group_size = groups_to_update.size();
		if (insert_after_index < group_size)
		{
			groups_to_update.insert(groups_to_update.begin() + insert_after_index, new_group);
			//groups_to_update_indexes.emplace_back(group_size);
		}
		else
		{
			groups_to_update.emplace_back(new_group);
			//groups_to_update_indexes.emplace_back(group_size);
		}
	};

	template<belt_utility::belt_direction direction>
	constexpr void update_event_tick_index(event_tick_data* current_tick_data) noexcept
	{
		//size_t insert_after_index = 0ull;
		const auto current_tick_data_index = current_tick_data - item_groups_goal_distance_event_data.begin().operator->();
		long long distance = this->item_groups_goal_distance[current_tick_data_index].get_distance();//get_end_distance<direction>();
		if (inserters.size() > static_cast<long long>(current_tick_data_index))
		{
			const auto index_ptr = (item_groups_goal_distance.begin() + current_tick_data_index)->get_index_from_ptr(item_groups_distance_between.begin().operator->());
			auto& item_data_ref = item_groups_data[index_ptr];
			auto& item_groups_ref = item_groups[index_ptr];

			const auto can_grab = get_closest_item_inserter_can_grab<direction>(current_tick_data_index, item_groups_goal_distance[current_tick_data_index].get_distance(), item_groups_ref, item_data_ref);
			if (can_grab.found_index != -1ll)
			{
				distance = can_grab.item_distance_position - (inserters[current_tick_data_index].last() - 1ll)->get_distance_position_plus();
				if (distance < 0 && distance >= -32)
					distance = 0;

				/*const auto inserter_position = belt_utility::get_direction_position<direction>((inserters[current_tick_data_index].last() - 1ll)->get_position());
				distance = inserter_position - can_grab.item_distance_position;

				if (distance < 0ll && distance <= item_groups_type::belt_item_size)
				{
					//current_tick_data.item_data = can_grab;
					distance = 0;
				}*/
			}
		}

		if (distance == 0) current_tick_data->tick_time = tick_count + 1;
		else current_tick_data->tick_time = get_tick_time_until_event(tick_count, distance, travel_distance_per_tick);

		current_tick_data->start_tick_time = tick_count;
	}

	/*constexpr void fix_removed_index_missmatch(size_t index) noexcept
	{
		for (long long i = 0, l = groups_to_update_indexes.size(); i < l; ++i)
		{
			if (groups_to_update_indexes[i] > index)
				--groups_to_update_indexes[i];
		}
	};*/
	constexpr void remove_event_tick(mem::vector<event_tick_data>::iterator current_tick_data) noexcept
	{
		for (auto groups_to_update_data_iter = groups_to_update.begin(); groups_to_update_data_iter != groups_to_update.last(); ++groups_to_update_data_iter)
		{
			for (auto begin_iter = groups_to_update_data_iter->begin(); begin_iter != groups_to_update_data_iter->last(); ++begin_iter)
			{
				if ((*begin_iter) == current_tick_data.operator->())
				{
					groups_to_update_data_iter->remove(current_tick_data - (*groups_to_update_data_iter->begin()));

					if (groups_to_update_data_iter->empty())
					{
						const auto remove_group_index = groups_to_update_data_iter - groups_to_update.begin();
						//groups_to_update_indexes.remove_unsafe(remove_group_index);
						groups_to_update.remove_unsafe(remove_group_index); //groups_to_update.remove_unsafe(groups_to_update_data_iter - groups_to_update.begin());
						//fix_removed_index_missmatch(remove_group_index);
					}
					return;
				}
			}
		}
	};
	constexpr void remove_event_tick(const event_tick_data* current_tick_data) noexcept
	{
		for (auto groups_to_update_data_iter = groups_to_update.begin(); groups_to_update_data_iter != groups_to_update.last(); ++groups_to_update_data_iter)
		{
			for (auto begin_iter = groups_to_update_data_iter->begin(); begin_iter != groups_to_update_data_iter->last(); ++begin_iter)
			{
				if ((*begin_iter) == current_tick_data)
				{
					groups_to_update_data_iter->remove_unsafe(current_tick_data - (*(*groups_to_update_data_iter).begin()));

					if (groups_to_update_data_iter->empty())
					{
						const auto remove_group_index = groups_to_update_data_iter - groups_to_update.begin();
						//groups_to_update_indexes.remove_unsafe(remove_group_index);
						groups_to_update.remove_unsafe(remove_group_index); //groups_to_update.remove_unsafe(groups_to_update_data_iter - groups_to_update.begin());
						//fix_removed_index_missmatch(remove_group_index);
					}
					return;
				}
			}
		}
	};

	enum update_event_tick_state : char
	{
		updated_tick_data,
		no_update,
		removed_container
	};
	template<belt_utility::belt_direction direction>
	__declspec(noinline) constexpr void update_event_tick(event_tick_data* current_tick_data, const size_t current_group_tick_time) noexcept
	{
		const ptrdiff_t distance_goal_index = (current_tick_data - item_groups_goal_distance_event_data.begin());
		long long distance = this->item_groups_goal_distance[distance_goal_index].get_distance();// get_end_distance<direction>();
		if (inserters.size() > static_cast<long long>(distance_goal_index))
		{
			const long long index_ptr = (item_groups_goal_distance.begin() + distance_goal_index)->get_index_from_ptr(item_groups_distance_between.begin().operator->());
			auto& item_data_ref = item_groups_data[index_ptr];
			auto& item_groups_ref = item_groups[index_ptr];

			const auto can_grab = get_closest_item_inserter_can_grab<direction>(distance_goal_index, item_groups_goal_distance[distance_goal_index].get_distance(), item_groups_ref, item_data_ref);
			if (can_grab.found_index != -1ll)
			{
				distance = can_grab.item_distance_position - (inserters[distance_goal_index].last() - 1ll)->get_distance_position_plus();
				if (distance < 0 && distance >= -32)
					distance = 0;

				/*const auto inserter_position = belt_utility::get_direction_position<direction>((inserters[distance_goal_index].last() - 1ll)->get_position());
				distance = inserter_position - can_grab.item_distance_position;

				if (distance < 0ll && distance <= item_groups_type::belt_item_size)
					distance = 0;*/
			}
		}

		const size_t old_tick_time = current_tick_data->tick_time;
		if (distance == 0) current_tick_data->tick_time = tick_count == 0 ? 0 : tick_count + 1;
		else current_tick_data->tick_time = get_tick_time_until_event(tick_count, distance, travel_distance_per_tick);

		current_tick_data->start_tick_time = tick_count;

		if (current_group_tick_time != (std::numeric_limits<size_t>::max)() && current_group_tick_time != current_tick_data->tick_time) add_event_tick_no_update_skip(current_tick_data);

	};
	template<belt_utility::belt_direction direction>
	constexpr bool update_event_tick(long long distance_goal_index) noexcept
	{
		auto current_tick_data = item_groups_goal_distance_event_data.begin() + distance_goal_index;
		//for (auto begin_iter = groups_to_update.begin(); begin_iter != groups_to_update.last(); ++begin_iter)
		{
			//for (auto nested_begin_iter = begin_iter->begin(); nested_begin_iter != begin_iter->last(); ++nested_begin_iter)
			{
				//if (*(*nested_begin_iter) == (*current_tick_data))
				{
					//event_tick_data* current_tick_data = *nested_begin_iter;

					//update the tick time first
					long long distance = this->item_groups_goal_distance[distance_goal_index].get_distance();// get_end_distance<direction>();
					if (inserters.size() > distance_goal_index)
					{
						//TODO 0x0 nullptr in item_groups_goal_distance, design issue somewhere find it looser
						const auto index_ptr = (item_groups_goal_distance.begin() + distance_goal_index)->get_index_from_ptr(item_groups_distance_between.begin().operator->());
						auto& item_data_ref = item_groups_data[index_ptr];
						auto& item_groups_ref = item_groups[index_ptr];

						const auto can_grab = get_closest_item_inserter_can_grab<direction>(distance_goal_index, item_groups_goal_distance[distance_goal_index].get_distance(), item_groups_ref, item_data_ref);
						if (can_grab.found_index != -1ll)
						{
							distance = can_grab.item_distance_position - (inserters[distance_goal_index].last() - 1ll)->get_distance_position_plus();
							if (distance < 0 && distance >= -32)
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

	inline constexpr std::vector<long long> get_current_goal_distance_values() const noexcept
	{
		std::vector<long long> old_distances;
		const auto e_iter = item_groups_goal_distance.last();
		for (auto b_iter = item_groups_goal_distance.begin(); b_iter != e_iter; ++b_iter)
		{
			old_distances.push_back(*(*b_iter).get_index_ptr());
		}
		return old_distances;
	};

	inline constexpr void update_old_goal_distance_pointers(const std::vector<long long>& old_distance_values) const noexcept
	{
		if (old_distance_values.size() > 0ull)
		{
			auto old_dist_iter = old_distance_values.begin();
			auto goal_iter = item_groups_goal_distance.begin();
			long long index_count = 0ll;
			const auto e_iter = item_groups_distance_between.last();
			for (auto b_iter = item_groups_distance_between.begin(); b_iter != e_iter; ++b_iter)
			{
				if (b_iter.operator*() == *old_dist_iter) //same distance value
				{
#ifdef ENABLE_CPP_EXCEPTION_THROW
					if (goal_iter->get_index_ptr() == b_iter.operator->()) throw std::runtime_error("");
#else
					if (goal_iter->get_index_ptr() == b_iter.operator->()) [[unlikely]] std::terminate();
#endif
					goal_iter->set_index_ptr(b_iter.operator->());
					++goal_iter;
					++old_dist_iter;
					++index_count;
				}
				if (old_dist_iter == old_distance_values.end() || goal_iter == item_groups_goal_distance.last()) break;
			}

#ifdef ENABLE_CPP_EXCEPTION_THROW
			//if (old_distance_values.size() != index_count) throw std::runtime_error("");
#endif
		}
	};
	inline constexpr std::vector<long long> get_current_goal_distance_pointers_item_groups_removed() const noexcept
	{
		std::vector<long long> old_distances;
		const auto e_iter = item_groups_goal_distance.last();
		for (auto b_iter = item_groups_goal_distance.begin(); b_iter != e_iter; ++b_iter)
		{
			old_distances.emplace_back(b_iter->get_distance());
		}

		return old_distances;
	};
	inline constexpr std::vector<long long> get_current_goal_distance_pointers_item_groups_removed(const mem::vector<remove_iterators_, mem::Allocating_Type::ALIGNED_NEW>& future_removes) const noexcept
	{
		std::vector<long long> old_distances;
		auto begin_future = future_removes.begin();
		const auto e_iter = item_groups_goal_distance.last();
		for (auto b_iter = item_groups_goal_distance.begin(); b_iter != e_iter; ++b_iter)
		{
			if (begin_future != future_removes.last())
			{
				if ((*(begin_future->item_groups_goal_dist_iter)).get_distance() == b_iter->get_distance())
				{
					old_distances.emplace_back(0ll);
					++begin_future;
				}
				else old_distances.emplace_back(b_iter->get_distance());

			}
			else old_distances.emplace_back(b_iter->get_distance());
		}
		return old_distances;
	};
	inline constexpr void update_old_goal_distance_pointers_item_groups_removed(const std::vector<long long>& old_distance_values) noexcept
	{
		if (old_distance_values.size() > 0ull)
		{
			long long offset_value = 0ll;
			long long index = 0ll;
			auto old_dist_iter = item_groups_goal_distance.begin();
			const auto e_iter = old_distance_values.end();
			for (auto b_iter = old_distance_values.begin(); b_iter != e_iter; ++b_iter)
			{
				if (*b_iter == 0ll) ++offset_value; //same distance value
				else
				{
					(*old_dist_iter).offset_ptr(offset_value);
					item_groups_goal_distance[index] = *old_dist_iter;
					//TODO add event_tick calculations here
					++index;
					++old_dist_iter;
				}
			}
		}
	};
#ifdef _DEBUG
	inline constexpr void validate_goal_pointers(const std::vector<long long>& old_distance_values, const std::vector<long long>& new_distance_values) noexcept
#else
	inline constexpr void validate_goal_pointers(const std::vector<long long>& old_distance_values) noexcept
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

	inline constexpr _vector_goal_distance::iterator get_goal_distance_iterator(_vector_distance::iterator dist_iter) const noexcept
	{
		const auto e_iter = item_groups_goal_distance.last();
		for (auto b_iter = item_groups_goal_distance.begin(); b_iter != e_iter; ++b_iter)
		{
			if (b_iter->get_distance() == *dist_iter) return b_iter;
		}

		return item_groups_goal_distance.last();
	};

	inline constexpr void item_group_has_zero_count(item_groups_type* ptr, item_groups_data_type* data_ptr, decltype(item_groups_goal_distance)::iterator goal_object) noexcept
	{
		const auto index_ptr = ptr - item_groups.begin().operator->();
		//const auto goal_dist_iter = belt_utility::get_goal_object_index_binary(index_ptr, item_groups_goal_distance, item_groups_distance_between);
		const auto dist_iter = item_groups_distance_between.begin() + index_ptr;

		if constexpr (ENABLE_CPP_EXCEPTION_THROW) if (goal_object == item_groups_goal_distance.last().operator->()) throw std::runtime_error("");

		remove_iterators.emplace_back(_vector::iterator{ ptr }, _data_vector::iterator{ data_ptr }, dist_iter, goal_object);
		//remove_iterators.push_back({ _vector::iterator{ ptr }, _data_vector::iterator{ data_ptr }, dist_iter, goal_object });
	};
	inline constexpr void item_group_has_zero_count(item_groups_type* ptr, item_groups_data_type* data_ptr) noexcept
	{
		const auto index_ptr = ptr - item_groups.begin().operator->();
		const auto goal_dist_iter = belt_utility::get_goal_object_index_binary(index_ptr, item_groups_goal_distance, item_groups_distance_between);
		const auto dist_iter = item_groups_distance_between.begin() + index_ptr;

		if constexpr (ENABLE_CPP_EXCEPTION_THROW) if (goal_dist_iter == item_groups_goal_distance.last()) throw std::runtime_error("");

		remove_iterators.push_back({ _vector::iterator{ ptr }, _data_vector::iterator{ data_ptr }, dist_iter, goal_dist_iter });
	};

	inline constexpr void item_group_has_zero_count(_vector::iterator ptr, _data_vector::iterator data_ptr) noexcept
	{
		const auto index_ptr = ptr - item_groups.begin();
		auto goal_dist_iter = belt_utility::get_goal_object_index_binary(index_ptr, item_groups_goal_distance, item_groups_distance_between);
		auto dist_iter = item_groups_distance_between.begin() + index_ptr;

		if constexpr (ENABLE_CPP_EXCEPTION_THROW) if (goal_dist_iter == item_groups_goal_distance.last()) throw std::runtime_error("");

		remove_iterators.emplace_back(ptr, data_ptr, dist_iter, goal_dist_iter);
	};

	template<belt_utility::belt_direction direction>
	__declspec(noinline) constexpr void item_group_has_reached_goal(_vector::iterator ptr, _data_vector::iterator item_data, _vector_goal_distance::iterator goal_distance) noexcept
	{
		if (segment_end_points.size() > 0ull)
		{
			const auto end_distance_direction = get_end_distance_direction<direction>();
			const auto direction_y_value = get_direction_y_value<direction>();
			for (long long i2 = 0; i2 < segment_end_points.size(); ++i2)
			{
				ASSERT_NOT_CONSTEXPR<_BOUNDS_CHECKING_>(i2 < segment_end_points.size());

				auto segment_ptr = segment_end_points[i2];
				const auto ptr_position = ptr->get_position(end_distance_direction, direction_y_value, goal_distance->get_distance());

				if (segment_ptr->start_of_segment != ptr_position) continue;

				if (segment_ptr->add_item(ptr->get_first_item(end_distance_direction, direction_y_value, goal_distance->get_distance(), *item_data)))
				{
					if (ptr->count() == 1ll)
					{
						ptr->remove_last_item(*item_data);
						item_group_has_zero_count(ptr, item_data);
					}
					else
					{
						if (item_groups_type::item_removal_result::item_removed_zero_remains == ptr->remove_first_item((*goal_distance).get_unsafe_index_ptr(), *item_data)) item_group_has_zero_count(ptr, item_data);
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
			for (; begin_remove_iter != remove_iterators.last(); ++begin_remove_iter)
			{
				auto goal_dist_writer_iter = begin_remove_iter->item_groups_goal_dist_iter;
				if ((*goal_dist_writer_iter).get_index_ptr() == item_groups_distance_between.begin().operator->())
					(*goal_dist_writer_iter).set_index_ptr(nullptr);
				else if ((goal_dist_writer_iter - item_groups_goal_distance.begin()) > 0ll)
				{
					if ((*(goal_dist_writer_iter - 1ll)).get_index_ptr() == (*goal_dist_writer_iter).get_offset_ptr(-1ll))
					{
						auto tmp_begin_iter = item_groups_goal_distance.begin();
						if ((*goal_dist_writer_iter).get_index_ptr() == *tmp_begin_iter)
							(*goal_dist_writer_iter).set_index_ptr(nullptr);
						else
							(*goal_dist_writer_iter).set_checked_index_ptr(nullptr);
					}
				}
				if ((*goal_dist_writer_iter) != nullptr)
				{
					(*goal_dist_writer_iter).update_pointer_and_values(item_groups_distance_between.begin().operator->(), 1ll, goal_distance_dead_object_v);

					//update_event_tick<direction>((goal_dist_writer_iter - item_groups_goal_distance.begin()));
				}
			}

			remove_iterators.clear();
			return;
		}
	};

public:
	template<belt_utility::belt_direction direction>
	__declspec(noinline) constexpr void event_update_item(decltype(groups_to_update[0])& updates) noexcept
	{
		const long long updates_size{ updates.size() };
		const auto end_distance = get_end_distance_direction<direction>();

#define ALMOST_BRANCHLESS_UPDATES

		const auto last_iter = item_groups_goal_distance.last();
		const auto last_goal_destination = inserters.empty() ? last_iter : last_iter - 1ll;
		const auto* begin_pointer = item_groups_goal_distance.values.first;

		/*const __m256i sa_item_groups_goal_distance_event_data = _mm256_set1_epi64x((size_t)item_groups_goal_distance_event_data.begin().operator->());
		const __m256i sa_item_groups_goal_distance = _mm256_set1_epi64x((size_t)item_groups_goal_distance.begin().operator->());
		const __m256i sa_item_groups_goal_distance_nested_indexes = _mm256_set1_epi64x((size_t)item_groups_goal_distance.begin().operator->()->get_index_ptr());
		const __m256i sa_item_groups_goal_item_count = _mm256_set1_epi64x((size_t)item_groups_goal_item_count.begin().operator->());

		const __m256i sa_inserter_slots = _mm256_set1_epi64x((size_t)inserter_slots.begin().operator->());
		const __m256i sa_distance_between = _mm256_set1_epi64x((size_t)item_groups_distance_between.begin().operator->());

		const __m256i sa_item_groups = _mm256_set1_epi64x((size_t)item_groups.begin().operator->());
		const __m256i sa_item_groups_data = _mm256_set1_epi64x((size_t)item_groups_data.begin().operator->());

		using goal_distance_iter_type = decltype(item_groups_goal_distance.begin());
		using goal_item_count_iter_type = decltype(item_groups_goal_item_count.begin());
		using inserter_slots_iter_type = decltype(inserter_slots.begin());

		const __m256i index_start_tick_time_4x = _mm256_set_epi64x(8, 8, 8, 8);

		const expr::div_rem<long long> div_rem = expr::divide_with_remainder(updates_size, 4);
		updates_size = div_rem.rem;
		for (size_t i = 0; i < div_rem.div; ++i)
		{
			const __m256i update_addresses = _mm256_load_si256((__m256i*)(updates.values.first + (i * 4)));
			const __m256i start_indexes_bytes = _mm256_sub_epi64(update_addresses, sa_item_groups_goal_distance_event_data);
			const __m256i start_indexes = _mm256_srli_epi64(start_indexes_bytes, 4);
			const __m256i start_indexes_8byte = _mm256_srli_epi64(start_indexes_bytes, 1);

			const __m256i updates_index_start_tick_time_4x = _mm256_add_epi64(start_indexes_bytes, index_start_tick_time_4x);
			const __m256i tick_time_4x = _mm256_i64gather_epi64((const long long*)item_groups_goal_distance_event_data.begin().operator->(), start_indexes_bytes, 1);
			const __m256i start_tick_time_4x = _mm256_i64gather_epi64((const long long*)item_groups_goal_distance_event_data.begin().operator->(), updates_index_start_tick_time_4x, 1);
			const __m256i ticks_for_groups_4x = _mm256_sub_epi64(tick_time_4x, start_tick_time_4x);

			const __m256i begin_iter_4x = _mm256_add_epi64(sa_item_groups_goal_distance, start_indexes_8byte);
			const __m256i begin_iter_nested_4x = _mm256_i64gather_epi64((const long long*)item_groups_goal_distance.begin().operator->(), start_indexes, 8);

			const __m256i begin_goal_count_iter_4x = _mm256_add_epi64(sa_item_groups_goal_item_count, start_indexes_8byte);
			const __m256i begin_inserter_goal_iter_4x = _mm256_add_epi64(sa_inserter_slots, _mm256_mul_epu32(start_indexes, _mm256_set1_epi64x((long long)sizeof(decltype(inserter_slots)::value_type))));
			const __m256i index_ptr_4x = _mm256_srli_epi64(_mm256_sub_epi64(begin_iter_nested_4x, sa_distance_between), 3);
			const __m256i item_groups_4x = _mm256_add_epi64(sa_item_groups, index_ptr_4x);
			const __m256i item_groups_data_4x = _mm256_add_epi64(sa_item_groups_data, _mm256_mul_epu32(index_ptr_4x, _mm256_set1_epi64x((long long)sizeof(decltype(item_groups_data)::value_type))));

			for (size_t i4 = 0; i4 < 4; ++i4)
			{
				decltype(item_groups_goal_distance)::pointer goal_object = (decltype(item_groups_goal_distance)::pointer)begin_iter_4x.m256i_i64[i4];
				decltype(item_groups_goal_item_count)::pointer goal_count = (decltype(item_groups_goal_item_count)::pointer)begin_goal_count_iter_4x.m256i_i64[i4];
				decltype(inserter_slots)::pointer inserter_slot = (decltype(inserter_slots)::pointer)begin_inserter_goal_iter_4x.m256i_i64[i4];
				decltype(item_groups)::pointer item_group = (decltype(item_groups)::pointer)item_groups_4x.m256i_i64[i4];
				decltype(item_groups_data)::pointer item_group_data = (decltype(item_groups_data)::pointer)item_groups_data_4x.m256i_i64[i4];

				const long long ticks_for_group = ticks_for_groups_4x.m256i_i64[i4];
				goal_object->subtract_goal_distance(ticks_for_group * travel_distance_per_tick);

				const long long cur_dist = goal_object->get_distance();
				if (cur_dist == -1ll) [[unlikely]]
					item_group->items_stuck_update(item_group_data); //add method for pointer type
				else [[likely]]
				{
					if (std::is_constant_evaluated() == false) item_groups_type::items_moved_per_frame += ticks_for_group * (*goal_count);

					auto& nested_inserter_vector = inserters[inserter_slot->get_index()];
					const auto nested_ins_last = nested_inserter_vector.last();
					for (auto nested_ins_iter = nested_inserter_vector.begin(); nested_ins_iter != nested_ins_last; ++nested_ins_iter)
					{
						const auto found_data = item_group->get_first_item_of_type_before_position_fast<direction>(cur_dist, item_group_data, nested_ins_iter->get_item_type(0), nested_ins_iter->get_distance_position_minus());
						nested_ins_iter->grab_item(remove_item<direction>(item_group, item_group_data, goal_object, found_data.found_index));
					}
				}
			}
		}*/

		const auto next_inserter_goal_iter = inserter_slots.last();
		const auto end_inserter_goal_iter = inserter_slots.last();
		//event_tick_data** update = updates.values.first;

		//for (; update != updates.values.last; ++update)
			//for (size_t i = div_rem.div * 4; i < div_rem.div * 4 + updates_size; ++i)
		for (long long i = 0; i < updates_size; ++i)
		{
			const auto* update = updates[i];
			const auto event_data_index = update - (item_groups_goal_distance_event_data.begin().operator->());
			const auto begin_iter = item_groups_goal_distance.begin() + event_data_index;
			auto begin_goal_count_iter = item_groups_goal_item_count.begin() + event_data_index;

			const auto begin_inserter_goal_iter = inserter_slots.begin() + event_data_index;

#ifndef ALMOST_BRANCHLESS_UPDATES
			if (inserters.size() <= 1) throw 0;
			if (inserters.size() > 1) next_inserter_goal_iter = begin_inserter_goal_iter + 1;
#endif

			const auto ticks_for_group = ((update)->tick_time - (update)->start_tick_time);
			begin_iter->subtract_goal_distance(ticks_for_group * travel_distance_per_tick);

			const auto cur_dist = begin_iter->get_distance();
			//if (cur_dist <= 0) throw 0;
			if (cur_dist > 0ll)
			{
				if (std::is_constant_evaluated() == false) item_groups_type::items_moved_per_frame += ticks_for_group * (*begin_goal_count_iter);

#ifndef ALMOST_BRANCHLESS_UPDATES
				if (begin_inserter_goal_iter == end_inserter_goal_iter) throw 0;
				if (begin_inserter_goal_iter != end_inserter_goal_iter)
#endif
				{
					const auto index_ptr = begin_iter->get_index_from_ptr(item_groups_distance_between.begin().operator->());
					//auto& item_data_ref = item_groups_data[index_ptr];
					//auto& item_groups_ref = item_groups[index_ptr];

					auto& nested_inserter_vector = inserters[begin_inserter_goal_iter->get_index()];
					const auto nested_ins_last = nested_inserter_vector.last();
					for (auto nested_ins_iter = nested_inserter_vector.begin(); nested_ins_iter != nested_ins_last; ++nested_ins_iter)
					{
						//const auto inserter_distance_position = end_distance - inserter_position;
#ifndef ALMOST_BRANCHLESS_UPDATES
						const auto temp_last_item_position = cur_dist + item_groups_ref.get_distance_to_last_item(item_data_ref);
						if (cur_dist > nested_ins_iter->get_distance_position_plus()) throw 0;
						if (temp_last_item_position < nested_ins_iter->get_distance_position_minus()) throw 0;
						if (cur_dist <= nested_ins_iter->get_distance_position_plus() && temp_last_item_position >= nested_ins_iter->get_distance_position_minus())
#endif
						{
							//const auto inserter_position = belt_utility::get_direction_position<direction>(nested_ins_iter->get_position());
							//const auto found_data = item_groups_ref.get_first_item_of_type_before_position<direction>(end_distance, cur_dist, item_data_ref, nested_ins_iter->get_item_type(0), inserter_position);

							//const auto found_data_new = item_groups_ref.get_first_item_of_type_before_position_fast<direction>(cur_dist, item_data_ref, nested_ins_iter->get_item_type(0), nested_ins_iter->get_distance_position_plus());
							//const auto found_data = item_groups_ref.get_first_item_of_type_before_position_fast<direction>(cur_dist, item_data_ref, nested_ins_iter->get_item_type(0), nested_ins_iter->get_distance_position_minus());

							const auto found_data = item_groups[index_ptr].get_first_item_of_type_before_position_fast<direction>(cur_dist, item_groups_data[index_ptr], nested_ins_iter->get_item_type(0), nested_ins_iter->get_distance_position_minus());
							//if (found_data_new.found_index != found_data.found_index) __debugbreak();
#ifndef ALMOST_BRANCHLESS_UPDATES
							if (found_data.found_index < 0) throw 0;
							if (found_data.item_distance_position < inserter_position) throw 0;
							if (found_data.item_distance_position > inserter_position + item_groups_type::belt_item_size) throw 0;
							if (found_data.found_index >= 0 && found_data.item_distance_position >= inserter_position && found_data.item_distance_position <= inserter_position + item_groups_type::belt_item_size)
#endif
							{
								nested_ins_iter->grab_item(remove_item<direction>(index_ptr, &item_groups[index_ptr], &item_groups_data[index_ptr], begin_iter, found_data));// found_data.found_index));
								//nested_ins_iter->grab_item(remove_item<direction>(&item_groups_ref, found_data.found_index));
#ifdef _DEBUG
								++removed_count;
								++nested_ins_iter->local_grabbed_items;
								nested_ins_iter->loop_count = 0;
#endif
							}
#ifdef _DEBUG
							//else
								//++nested_ins_iter->missed_grabs;
#endif
						}
#ifdef _DEBUG
						//else
							//++nested_ins_iter->missed_grabs;
#endif
#ifdef _DEBUG
						//++nested_ins_iter->loop_count;
#endif
					}
				}
			}
			else if (cur_dist != -1ll)
			{
				const auto index_ptr = begin_iter->get_index_from_ptr(item_groups_distance_between.begin().operator->());
				item_groups[index_ptr].items_stuck_update(item_groups_data[index_ptr]);
			}
		}
	};

	template<belt_utility::belt_direction direction>
	constexpr void tmp_update(decltype(groups_to_update)& updates) noexcept
	{
		const size_t updates_size{ updates[0].usize() };
		size_t smallest_tick_value{ std::numeric_limits<size_t>::max() };
		for (size_t i = 0; i < updates_size; ++i)
		{
			update_event_tick_index<direction>(updates[0][i]);
			if (smallest_tick_value > updates[0][i]->tick_time) smallest_tick_value = updates[0][i]->tick_time;
			//update_event_tick<direction>(updates[0][i]);
		}

		size_t smallest_current_index{ 0ull };
		for (size_t i = 0; i < updates_size; ++i)
		{
			if (smallest_tick_value == updates[0][i]->tick_time)
			{
				updates[0][smallest_current_index] = updates[0][i];
				++smallest_current_index;
			}
			else
			{
				add_event_tick_no_update(updates[0][i]);
			}
		}

		updates[0].decrease_size(smallest_current_index);

		long long move_group_index{ 0ull };
		for (long long i = 1; i < updates.size(); ++i)
		{
			if (updates[i][0]->tick_time == updates[0][0]->tick_time)
			{
				updates[i].insert(updates[i].last(), updates[0].begin(), updates[0].last());
				updates.remove(0);
				return;
			}
			if (updates[i][0]->tick_time > updates[0][0]->tick_time)
				break;
			if (updates[i][0]->tick_time < updates[0][0]->tick_time)
				move_group_index = i;
		}

		if (move_group_index != 0)
		{
			updates.insert(updates.begin() + move_group_index + 1, updates[0]);
			updates.remove(0);
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
	__declspec(noinline) constexpr void update() noexcept
	{
		const long long group_update_index = first_group_to_update;
		if (groups_to_update.size() > 0 && tick_count == groups_to_update[group_update_index][0]->tick_time)
		{
			event_update_item<direction>(groups_to_update[group_update_index]);

			//update_item<direction>();
			if (!item_groups_goal_distance.empty())
			{
				if ((item_groups_goal_distance.begin()->get_distance()) == 0ll)
					item_group_has_reached_goal<direction>(item_groups.begin(), item_groups_data.begin(), item_groups_goal_distance.begin());
			}

			if (!remove_iterators.empty())
				item_groups_removal<direction>();

			if (std::numeric_limits<unsigned long long>::max() == tick_count) tick_count = 0ull;
			else ++tick_count;

			update_event_tick<direction>(groups_to_update[group_update_index][0], -1);
			const size_t current_tick_time = groups_to_update[group_update_index][0]->tick_time;
			const size_t length = groups_to_update[group_update_index].size();
			long long same_group_index = 1;
			for (size_t i = 1; i < length; ++i)
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
			for (long long i = 0, l = groups_to_update.size(); i < l; ++i)
			{
				if (groups_to_update[i][0]->tick_time < closest_tick)
				{
					closest_tick = groups_to_update[i][0]->tick_time;
					first_group_to_update = i;
				}
			}
		}
		else
		{
			if (std::numeric_limits<unsigned long long>::max() == tick_count) tick_count = 0ull;
			else ++tick_count;
		}
	};

	constexpr bool is_end_goal_destination(auto goal) noexcept
	{
		if ((*goal).get_distance() == (*(item_groups_goal_distance.last() - 1)).get_distance()) return true;

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

	constexpr auto new_goal_before_distance_loop(long long distance, auto begin_goal, auto first_distance_between) noexcept
	{
		auto goal_distance = item_groups_distance_between.begin() + ((*begin_goal).get_index_from_ptr(item_groups_distance_between.begin().operator->()));
		long long current_distance = 0ll;
		do
		{
			current_distance += *goal_distance;
			if (current_distance == distance)
			{
				*goal_distance = current_distance;

				const bool goal_distance_needs_resize = item_groups_goal_distance.needs_resize();
				mem::vector<mem::vector<long long>> index_container;
				if (goal_distance_needs_resize)
					index_container = mem::utility::validate_pointers_get_indexes(item_groups_goal_distance_event_data, groups_to_update);

				item_groups_goal_distance.emplace(begin_goal, goal_distance.operator->());
				item_groups_goal_distance_event_data.emplace(item_groups_goal_distance_event_data.begin() + begin_goal->get_index_from_ptr((*item_groups_goal_distance.begin()).get_index_ptr()));
				item_groups_goal_item_count.emplace(item_groups_goal_item_count.begin() + (begin_goal - item_groups_goal_distance.begin()), 0ull);

				if (goal_distance_needs_resize)
					mem::utility::validate_pointers_from_indexes(item_groups_goal_distance_event_data, index_container, groups_to_update);

				recalculate_distances_between_from_to((*begin_goal).get_unsafe_index_ptr(), goal_distance, first_distance_between);
				return true;
			}
			else --goal_distance;
		} while (first_distance_between != goal_distance);

		return false;
	};

	constexpr auto insert_new_goal_before_distance(long long distance) noexcept
	{
		if (item_groups_goal_distance.size() > 0ll)
		{
			if (new_goal_before_distance_loop(distance, item_groups_goal_distance.begin(), item_groups_distance_between.begin())) return true;

			const auto last_goal = item_groups_goal_distance.last();
			for (auto begin_goal = item_groups_goal_distance.begin(), next_goal = begin_goal + 1ll; next_goal != last_goal; ++next_goal)
			{
				if ((*next_goal).get_distance() < distance) //means begin_goal is in front of distance
				{
					const auto index_ptr = ((*begin_goal).get_index_from_ptr(item_groups_distance_between.begin().operator->())) + 1ll;
					const auto first_distance = item_groups_distance_between.begin() + index_ptr;
					auto goal_distance = item_groups_distance_between.begin() + ((*next_goal).get_index_from_ptr(item_groups_distance_between.begin().operator->()));

					long long current_distance = 0ll;
					do
					{
						current_distance += *goal_distance;
						if (current_distance == distance)
						{
							*goal_distance = current_distance;

							const bool goal_distance_needs_resize = item_groups_goal_distance.needs_resize();
							mem::vector<mem::vector<long long>> index_container;
							if (goal_distance_needs_resize)
								index_container = mem::utility::validate_pointers_get_indexes(item_groups_goal_distance_event_data, groups_to_update);

							item_groups_goal_distance.emplace(next_goal, goal_distance.operator->());
							item_groups_goal_distance_event_data.emplace(item_groups_goal_distance_event_data.begin() + next_goal->get_index_from_ptr((*item_groups_goal_distance.begin()).get_index_ptr()));
							item_groups_goal_item_count.emplace(item_groups_goal_item_count.begin() + (next_goal - item_groups_goal_distance.begin()), 0ull);

							if (goal_distance_needs_resize)
								mem::utility::validate_pointers_from_indexes(item_groups_goal_distance_event_data, index_container, groups_to_update);

							recalculate_distances_between_from_to((*begin_goal).get_unsafe_index_ptr(), goal_distance, first_distance);
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
	constexpr belt_utility::need_new_slot_result need_new_goal_distance_slot(long long* distance, long long distance_between_inserted_index) noexcept
	{
		if constexpr (ENABLE_CPP_EXCEPTION_THROW)
		{
			if (distance_between_inserted_index - 1ll < 0) throw std::runtime_error("negative index");
			if (distance_between_inserted_index - 1ll >= item_groups_distance_between.size()) throw std::runtime_error("index is larger than the vector size");
		}

		if (inserters.empty())
		{
			if (item_groups_goal_distance.size() == 1ll)
			{
				auto begin_iter = item_groups_goal_distance.begin();
				auto old_goal_distance = (*begin_iter).get_index_ptr();
				item_groups_goal_distance[0].set_index_ptr(distance);
				const auto recalc_start_iter = item_groups_distance_between.begin() + ((*begin_iter).get_index_from_ptr(item_groups_distance_between.begin().operator->()));
				const auto recalc_end_iter = item_groups_distance_between.begin();
				recalculate_distances_between_from_to(old_goal_distance, recalc_start_iter, recalc_end_iter);
				return belt_utility::need_new_slot_result::update_pointer_to_new_index;
			}

			for (auto begin_iter = item_groups_goal_distance.begin(), next_iter = begin_iter + 1ll, end_iter = item_groups_goal_distance.last(); next_iter != end_iter; ++begin_iter, ++next_iter)
			{
				//object belongs to next_iter
				if (begin_iter->get_distance() > *distance && *distance < next_iter->get_distance()) return belt_utility::need_new_slot_result::object_is_between_slots;

				//we should update the pointer to the new index
				if (next_iter == end_iter - 1ll)
				{
					auto old_goal_distance = (*next_iter).get_index_ptr();
					next_iter->set_index_ptr(distance);
					const auto recalc_start_iter = item_groups_distance_between.begin() + ((*next_iter).get_index_from_ptr(item_groups_distance_between.begin().operator->()));
					const auto recalc_end_iter = item_groups_distance_between.begin() + ((*begin_iter).get_offset_ptr(1ll).get_index_from_ptr(item_groups_distance_between.begin().operator->()));
					recalculate_distances_between_from_to(old_goal_distance, recalc_start_iter, recalc_end_iter);
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
				const auto inserter_size = (item_groups_goal_distance.size() - 1ll);
				if (inserter_size < inserters.size()) begin_inserter_iter = inserters.begin() + inserter_size;
				else begin_inserter_iter = inserters.begin();
			}

			auto distance_comparison = belt_utility::get_distance_comparison<direction>(get_end_distance_direction<direction>(), *distance, begin_inserter_iter, begin_inserter_iter, decltype(item_groups)::value_type::belt_item_size);
			if (belt_utility::distance_comparison::distance_is_after == distance_comparison)
			{
				do
				{
					++begin_inserter_iter;
					if (begin_inserter_iter != inserters.last()) distance_comparison = belt_utility::get_distance_comparison<direction>(get_end_distance_direction<direction>(), *distance, begin_inserter_iter, begin_inserter_iter, decltype(item_groups)::value_type::belt_item_size);
				} while (begin_inserter_iter != inserters.last() && belt_utility::distance_comparison::distance_is_after == distance_comparison);
			}

			if (begin_inserter_iter != inserters.begin() && begin_inserter_iter != inserters.last())
			{
				auto previous_inserter_iter = begin_inserter_iter;

				const auto inserter_distance = end_distance - belt_utility::get_direction_position<direction>(begin_inserter_iter->operator[](0).get_position());
				if (inserter_distance < *distance) previous_inserter_iter = begin_inserter_iter - 1ll;

				decltype(distance_comparison) previous_distance_comp = decltype(distance_comparison)::null;
				decltype(distance_comparison) prev_previous_distance_comp = decltype(distance_comparison)::null;

				previous_distance_comp = belt_utility::get_distance_comparison<direction>(get_end_distance_direction<direction>(), *distance, previous_inserter_iter, previous_inserter_iter, decltype(item_groups)::value_type::belt_item_size);
				prev_previous_distance_comp = belt_utility::get_distance_comparison<direction>(get_end_distance_direction<direction>(), *(item_groups_distance_between.begin() + distance_between_inserted_index - 1ll).operator->(), previous_inserter_iter, previous_inserter_iter, decltype(item_groups)::value_type::belt_item_size);

				{
					using enum belt_utility::distance_comparison; //just to avoid belt_utility::distance_comparison::
					if (!(distance_is_inside == previous_distance_comp && distance_is_inside == prev_previous_distance_comp) &&
						!(distance_is_inside == previous_distance_comp && distance_is_before == prev_previous_distance_comp) &&
						distance_is_before != previous_distance_comp && distance_is_after != prev_previous_distance_comp)
						skip_loop = true;
				}
			}
			else if (begin_inserter_iter == inserters.last() && item_groups_goal_distance.size() == inserters.size()) skip_loop = true;

			if (item_groups_goal_distance.size() == inserters.size() && belt_utility::distance_comparison::distance_is_after == distance_comparison) skip_loop = true;

			if (skip_loop == false)
			{
				auto begin_iter = item_groups_goal_distance.begin();
				auto next_iter = begin_iter + 1ll;
				const auto end_iter = item_groups_goal_distance.last();
				for (; begin_iter != end_iter; ++begin_iter, ++next_iter)
				{
					if ((*begin_iter) == (item_groups_distance_between.begin() + (distance_between_inserted_index - 1ll)).operator->())
					{
						const auto iter_index = begin_iter - item_groups_goal_distance.begin();
						begin_iter->set_index_ptr(distance);

						auto previous_iter = (item_groups_distance_between.begin() + ((*begin_iter).get_index_from_ptr(item_groups_distance_between.begin().operator->()))) - 1ll;
						*previous_iter = 32ll;

						update_event_tick<direction>(iter_index);

						return belt_utility::need_new_slot_result::update_pointer_to_new_index;
					}
				}
			}
		}

		return belt_utility::need_new_slot_result::need_new_slot;
	};

	constexpr long long calculate_distance_(auto from, auto too) noexcept
	{
		long long calc_dist = 0ll;
		while (from.operator->() != *too)
		{
			calc_dist += *from;
			++from;
		}

		return calc_dist;
	};

private:
	template<belt_utility::belt_direction direction>
	constexpr bool add_item_before(const item_uint& new_item, auto iter, long long index_ptr_temp) noexcept
	{
		auto& new_data_group = item_groups_data.emplace_back();
		const auto new_iter_group = item_groups.emplace(iter.result);

		std::vector<long long> old_distances;
		if (item_groups_distance_between.needs_resize()) old_distances = get_current_goal_distance_values();
		auto new_distance_value = get_end_distance_direction<direction>() - belt_utility::get_direction_position<direction>(new_item.position);
		const auto goal_object = belt_utility::get_goal_object_index(index_ptr_temp, item_groups_goal_distance, item_groups_distance_between);

		if (goal_object != item_groups_goal_distance.last()) new_distance_value = belt_utility::get_direction_position<direction>(new_item.position) - calculate_distance_(item_groups_distance_between.begin() + index_ptr_temp, goal_object);

		const auto new_distance_iter = item_groups_distance_between.emplace(item_groups_distance_between.begin() + index_ptr_temp, new_distance_value);
		if (!old_distances.empty()) update_old_goal_distance_pointers(old_distances);

#ifdef __BELT_SEGMENT_VECTOR_TYPE__
		if (new_iter_group != item_groups.last()) new_iter_group->add_item(get_end_distance_direction<direction>(), new_distance_iter.operator->(), new_data_group, new_item, new_item.position);
#else
		if (new_iter_group != item_groups.end()) new_iter_group->add_item(new_item, new_item.position);
#endif
		return true;
	};

	template<belt_utility::belt_direction direction>
	constexpr bool add_item_after_last(const item_uint& new_item, long long index_ptr_temp) noexcept
	{
		auto& new_data_group = item_groups_data.emplace_back();
		auto& new_iter_group = item_groups.emplace_back();

		std::vector<long long> old_distances;
		if (item_groups_distance_between.needs_resize()) old_distances = get_current_goal_distance_values();

		const auto item_distance_position = belt_utility::get_direction_position<direction>(new_item.position);
		auto new_distance_value = get_end_distance_direction<direction>() - item_distance_position;
		auto& new_distance = item_groups_distance_between.emplace_back(new_distance_value);
		if (!old_distances.empty()) update_old_goal_distance_pointers(old_distances);

		const auto new_slot_result = need_new_goal_distance_slot<direction, belt_utility::distance_slot_inserted_position::new_item_after_last_goal>(&new_distance, item_groups_distance_between.size() - 1ll);
		if (belt_utility::need_new_slot_result::need_new_slot == new_slot_result)
		{
			const bool goal_distance_needs_resize = item_groups_goal_distance.needs_resize();
			mem::vector<mem::vector<long long>> index_container;
			if (goal_distance_needs_resize)
				index_container = mem::utility::validate_pointers_get_indexes(item_groups_goal_distance_event_data, groups_to_update);

			item_groups_goal_distance.emplace_back(&new_distance);
			auto& inserted_event_data = item_groups_goal_distance_event_data.emplace_back();
			item_groups_goal_item_count.emplace_back(1ull);

			if (goal_distance_needs_resize)
				mem::utility::validate_pointers_from_indexes(item_groups_goal_distance_event_data, index_container, groups_to_update);

			add_event_tick<direction>(&inserted_event_data);
		}

		if constexpr (ENABLE_CPP_EXCEPTION_THROW) if (!inserters.empty() && item_groups_goal_distance.size() > inserters.size() + 1) throw std::runtime_error("");

		new_iter_group.add_item(get_end_distance_direction<direction>(), &new_distance, new_data_group, new_item, new_item.position);
		const auto goal_object = belt_utility::get_goal_object_index_binary(index_ptr_temp, item_groups_goal_distance, item_groups_distance_between);
		const auto goal_object_index = goal_object - item_groups_goal_distance.begin();

		if (belt_utility::need_new_slot_result::need_new_slot != new_slot_result) ++item_groups_goal_item_count[goal_object_index];
		return true;
	};

	template<belt_utility::belt_direction direction>
	constexpr bool add_item_after(const item_uint& new_item, auto iter, long long index_ptr_temp) noexcept
	{
		auto new_data_group = item_groups_data.emplace(item_groups_data.begin() + (iter.result - item_groups.begin()));
		const auto new_iter_group = item_groups.emplace(iter.result);

		std::vector<long long> old_distances;
		if (item_groups_distance_between.needs_resize()) old_distances = get_current_goal_distance_values();

		auto new_distance_value = get_end_distance_direction<direction>() - belt_utility::get_direction_position<direction>(new_item.position);
		const auto goal_object = belt_utility::get_goal_object_index_binary(index_ptr_temp, item_groups_goal_distance, item_groups_distance_between);
		if (goal_object != item_groups_goal_distance.last() && item_groups_goal_distance.size() > 1ll) new_distance_value = belt_utility::get_direction_position<direction>(new_item.position) - calculate_distance_(item_groups_distance_between.begin() + (index_ptr_temp + 1), goal_object);

		const auto new_distance_iter = item_groups_distance_between.emplace(item_groups_distance_between.begin() + (index_ptr_temp + 1), new_distance_value);
		if (!old_distances.empty()) update_old_goal_distance_pointers(old_distances);

		const auto new_slot_result = need_new_goal_distance_slot<direction, belt_utility::distance_slot_inserted_position::new_item_after_iter>(new_distance_iter.operator->(), new_distance_iter - item_groups_distance_between.begin());
		if (belt_utility::need_new_slot_result::need_new_slot == new_slot_result)
		{
			const bool goal_distance_needs_resize = item_groups_goal_distance.needs_resize();
			mem::vector<mem::vector<long long>> index_container;
			if (goal_distance_needs_resize)
				index_container = mem::utility::validate_pointers_get_indexes(item_groups_goal_distance_event_data, groups_to_update);

			item_groups_goal_distance.emplace_back(new_distance_iter.operator->());
			auto& inserted_event_data = item_groups_goal_distance_event_data.emplace_back();
			item_groups_goal_item_count.emplace_back(1ull);

			if (goal_distance_needs_resize)
				mem::utility::validate_pointers_from_indexes(item_groups_goal_distance_event_data, index_container, groups_to_update);

			add_event_tick<direction>(&inserted_event_data);
		}

		if constexpr (ENABLE_CPP_EXCEPTION_THROW) if (!inserters.empty() && item_groups_goal_distance.size() > inserters.size() + 1) throw std::runtime_error("");
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
		if (new_iter_group != item_groups.last()) new_iter_group->add_item(get_end_distance_direction<direction>(), new_distance_iter.operator->(), *new_data_group, new_item, new_item.position);
#else
		if (new_iter_group != item_groups.end()) new_iter_group->add_item(new_item, new_item.position);
#endif
		return true;
	};

	template<belt_utility::belt_direction direction>
	constexpr bool split_item_group(const item_uint& new_item, auto iter, long long index_ptr_temp) noexcept
	{
		const auto end_distance_direction = get_end_distance_direction<direction>();
		const auto index = iter.result->get_first_item_before_position<direction>(
			end_distance_direction,
			item_groups_distance_between[index_ptr_temp],
			item_groups_data[index_ptr_temp],
			belt_utility::get_direction_position<direction>(new_item.position)
		);

		auto goal_object = belt_utility::get_goal_object_index_binary(index_ptr_temp, item_groups_goal_distance, item_groups_distance_between);
		bool is_not_goal_object = true;
		if (goal_object->get_index_ptr() == &item_groups_distance_between[index_ptr_temp])
		{
			goal_object->update_pointers_without_checks(1ll);
			is_not_goal_object = false;
		}

		auto split_group = item_groups[index_ptr_temp].split_from_index(index);
		const auto split_data_result = item_data_utility::split_from_index(item_groups_data[index_ptr_temp], index);
		const auto new_distance = item_groups_data[index_ptr_temp].item_distance[item_groups[index_ptr_temp].count() - 1ll] + split_data_result.missing_distance;

		const auto inserted_group = item_groups.insert(item_groups.begin() + index_ptr_temp, split_group);
		const auto inserted_data = item_groups_data.insert(item_groups_data.begin() + index_ptr_temp, split_data_result.data);
		const auto inserted_distance = item_groups_distance_between.insert(item_groups_distance_between.begin() + index_ptr_temp, new_distance);

		short added_index = -1;
		if (is_not_goal_object == false)
		{
			const auto calc_dist = belt_utility::get_distances_from_to2(item_groups_distance_between.begin() + ((*goal_object).get_index_from_ptr(item_groups_distance_between.begin().operator->())), inserted_distance);
			added_index = inserted_group->add_item(end_distance_direction, calc_dist, inserted_distance.operator->(), *inserted_data, new_item, new_item.position);
		}
		else added_index = inserted_group->add_item(end_distance_direction, inserted_distance.operator->(), *inserted_data, new_item, new_item.position);

		return added_index != -1;
	};

public:
	constexpr bool add_item(const item_uint& new_item) noexcept
	{
		using enum belt_utility::belt_direction;
		switch (segment_direction)
		{
			default:
			case null:
			case left_right: return add_item<left_right>(new_item); break;
			case right_left: return add_item<right_left>(new_item); break;
			case top_bottom: return add_item<top_bottom>(new_item); break;
			case bottom_top: return add_item<bottom_top>(new_item); break;
		}
	};

private:
	template<belt_utility::belt_direction direction>
	constexpr bool add_item(const item_uint& new_item) noexcept
	{
		//TODO Need to account for when items are added but the groups_to_update assosicated with the group we're about to add in
		//hasn't been ticked, meaning it's "behind" in time. So take current tick and subtract the assosicated groups_to_update start tick
		//use that to offset the new_item's position to rewind it to offset the time dilation going on
		//meaning it will appear to move backward in distance, but once it ticks again will be corrected

		if (item_groups.size() == 0)
		{
			auto& new_data_group = item_groups_data.emplace_back();
			auto& new_item_group = item_groups.emplace_back();
			auto& new_goal_distance = item_groups_distance_between.emplace_back(get_end_distance_direction<direction>() - belt_utility::get_direction_position<direction>(new_item.position)); //get_end_distance_direction() - belt_utility::get_direction_position(segment_direction, new_item.position)

			const bool goal_distance_needs_resize = item_groups_goal_distance.needs_resize();
			mem::vector<mem::vector<long long>> index_container;
			if (goal_distance_needs_resize)
				index_container = mem::utility::validate_pointers_get_indexes(item_groups_goal_distance_event_data, groups_to_update);

			item_groups_goal_distance.emplace_back(&new_goal_distance);
			auto& inserted_event_data = item_groups_goal_distance_event_data.emplace_back();
			item_groups_goal_item_count.emplace_back(1ull);

			if (goal_distance_needs_resize)
				mem::utility::validate_pointers_from_indexes(item_groups_goal_distance_event_data, index_container, groups_to_update);

			ASSERT_NOT_CONSTEXPR<_BOUNDS_CHECKING_>(0ll < item_groups.size());

			const short added_index = new_item_group.add_item(get_end_distance_direction<direction>(), &item_groups_distance_between[0], new_data_group, new_item, new_item.position);

			add_event_tick<direction>(&inserted_event_data);

			if (added_index != -1) return true;
			return false;
		}
		else
		{
			//TODO need to check if the new item fits in an existing destination
			// destinations are things like segment ends points, inserters and the like
			// we only need to care about the first item before a destination as the others just moves along behind it
			// once we remove the leader we make the previous one the new leader and just keep going
			// if it's before the current destination need to swap them out and calculate the new updated distance values for the item behind it, and the one before it
			// if it's after the current destination we just add it in and calculate the new updated distance values for the item behind it, and the one before it
			// if there's no existing destination we create a new one

			//const auto item_distance = belt_utility::get_direction_position<direction>(new_item.position);
			auto iter = belt_utility::find_closest_item_group<direction>(get_end_distance_direction<direction>(), item_groups_data, item_groups, item_groups_distance_between, item_groups_goal_distance, belt_utility::get_direction_position<direction>(new_item.position), inserters);
			const auto index_ptr_temp = iter.result - item_groups.begin();

			ASSERT_NOT_CONSTEXPR<_BOUNDS_CHECKING_>(iter.result != item_groups.end());

#ifdef __BELT_SEGMENT_VECTOR_TYPE__
			if (belt_utility::find_closest_item_group_return_result::invalid_value == iter.scan) return false;
			if ((iter.result != item_groups.last() && !iter.result->can_add_item(get_end_distance_direction<direction>(), &item_groups_distance_between[index_ptr_temp], item_groups_data[index_ptr_temp], new_item.position))) return false;
#else
			if (belt_utility::find_closest_item_group_return_result::invalid_value == iter.scan || iter.result == item_groups.end() || (iter.result != item_groups.end() && !iter.result->can_add_item(new_item.position))) return false;
#endif
			if (belt_utility::find_closest_item_group_return_result::insert_into_group == iter.scan)
			{
				if (iter.result->count() >= 32) return split_item_group<direction>(new_item, iter, index_ptr_temp);
				else
				{
					const short added_index = iter.result->add_item(get_end_distance_direction<direction>(), &item_groups_distance_between[index_ptr_temp], item_groups_data[index_ptr_temp], new_item, new_item.position);
					if (added_index != -1)
					{
						auto goal_object = belt_utility::get_goal_object_index_binary(index_ptr_temp, item_groups_goal_distance, item_groups_distance_between);
						++item_groups_goal_item_count[(goal_object - item_groups_goal_distance.begin())];

						update_event_tick<direction>((goal_object - item_groups_goal_distance.begin()));

						if (goal_object->get_index_ptr() == &item_groups_distance_between[index_ptr_temp])
						{
							long long* previous_goal_ptr_index = nullptr;
							if (goal_object != item_groups_goal_distance.begin()) previous_goal_ptr_index = (*(goal_object - 1ll)).get_unsafe_index_ptr();
						}

						return true;
					}
				}
				return false;
			}
			else if (belt_utility::find_closest_item_group_return_result::new_group_before_iter == iter.scan) return add_item_before<direction>(new_item, iter, index_ptr_temp); //TODO does this add towards the start of end of the belt segment?
			else if (belt_utility::find_closest_item_group_return_result::new_group_after_iter == iter.scan) //TODO does this add towards the start of end of the belt segment?
			{
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
				if (iter.result + 1 == item_groups.last())
#else
				if (iter.result + 1 == item_groups.end())
#endif
					return add_item_after_last<direction>(new_item, index_ptr_temp);
				else return add_item_after<direction>(new_item, iter, index_ptr_temp);
			}
		}

		return false;
	};

public:
	template<belt_utility::belt_direction direction>
	constexpr long long get_inserter_group_index(auto& group_inserters, index_inserter object) noexcept
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
		constexpr const auto max_inserter_distance = item_groups_type::belt_item_size * item_groups_type::max_item_count * 4;
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