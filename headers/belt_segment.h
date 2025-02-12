#pragma once

#include <stdexcept>
#include <vector>
#include <corecrt_terminate.h>
#include <type_traits>

#include "type_conversion.h"
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

class belt_segment
{
public:
	//protected:
	vec2_uint start_of_segment{ 0, 0 };
	vec2_uint end_of_segment{ 0, 0 };

	_data_vector item_groups_data{ 4 };
	_vector item_groups{ 4 };
	//contains the item_groups_goal distance of the first item_group
	_vector_goal_distance item_groups_goal_distance{ 4 };
	//contains the distance between item_groups for all item_groups
	//the value is the distance to the end of the belt segment
	_vector_distance item_groups_distance_between{ 4 };
	mem::vector<size_t> item_groups_goal_item_count{ 4 };

	mem::vector<_simple_inserter_vector, mem::Allocating_Type::ALIGNED_NEW, mem::allocator<_simple_inserter_vector, mem::Allocating_Type::ALIGNED_NEW>, mem::use_memcpy::force_checks_off> inserters{ 4 };
	mem::vector<double_index_iterator<index_inserter, decltype(belt_segment::inserters)>, mem::Allocating_Type::ALIGNED_NEW, mem::allocator<double_index_iterator<index_inserter, decltype(belt_segment::inserters)>, mem::Allocating_Type::ALIGNED_NEW>, mem::use_memcpy::force_checks_off> inserter_slots{ 4 };

	// belt_segments that this segment moves items onto
	std::vector<belt_segment*> segment_end_points;
	// belt_segments that will move items onto this segment
	std::vector<belt_segment*> connected_segments;

	std::vector<remove_iterators_> remove_iterators;
	belt_utility::belt_direction segment_direction{ belt_utility::belt_direction::left_right };
	bool item_was_removed = false;

	constexpr belt_utility::belt_direction direction_construct(vec2_uint start, vec2_uint end) const noexcept
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
	constexpr belt_segment(vec2_uint start, vec2_uint end) noexcept :
		start_of_segment{ start },
		end_of_segment{ end },
		segment_direction{ direction_construct(start, end) }
	{};
	constexpr belt_segment(vec2_uint start, vec2_uint end, belt_segment_correct_t) noexcept :
		start_of_segment{ start },
		end_of_segment{ end }
	{};

	constexpr belt_segment(const belt_segment& o) noexcept :
		start_of_segment{ o.start_of_segment },
		end_of_segment{ o.end_of_segment },
		item_groups_data{ o.item_groups_data },
		item_groups{ o.item_groups },
		item_groups_goal_distance{ o.item_groups_goal_distance },
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
		start_of_segment{ std::exchange(o.start_of_segment, vec2_uint{}) },
		end_of_segment{ std::exchange(o.end_of_segment, vec2_uint{}) },
		item_groups_data{ std::exchange(o.item_groups_data, decltype(item_groups_data){}) },
		item_groups{ std::exchange(o.item_groups, decltype(item_groups){}) },
		item_groups_goal_distance{ std::exchange(o.item_groups_goal_distance, decltype(item_groups_goal_distance){}) },
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
		item_groups_distance_between = o.item_groups_distance_between;
		item_groups_goal_item_count = o.item_groups_goal_item_count;
		inserters = o.inserters;
		inserter_slots = o.inserter_slots;
		segment_end_points = o.segment_end_points;
		connected_segments = o.connected_segments;
		remove_iterators = o.remove_iterators;
		segment_direction = o.segment_direction;
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
		item_groups_distance_between = std::move(o.item_groups_distance_between);
		item_groups_goal_item_count = std::move(o.item_groups_goal_item_count);
		inserters = std::move(o.inserters);
		inserter_slots = std::move(o.inserter_slots);
		segment_end_points = std::move(o.segment_end_points);
		connected_segments = std::move(o.connected_segments);
		remove_iterators = std::move(o.remove_iterators);
		segment_direction = std::move(o.segment_direction);
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
	inline constexpr item_uint get_item(std::size_t item_group, std::size_t i) noexcept
	{
#ifdef _BOUNDS_CHECKING_
		ASSERT_NOT_CONSTEXPR(i < item_groups.size())
#endif
			return item_groups[item_group].get(get_end_distance_direction<direction>(), get_direction_y_value<direction>(), item_groups_distance_between[item_group], item_groups_data[item_group], i);
	};

	template<belt_utility::belt_direction direction>
	inline constexpr item_uint remove_item(item_groups_type* item_group, const long long item_index) noexcept
	{
		const auto index_ptr = item_group - item_groups.begin();
		const auto data_group = item_groups_data.begin() + index_ptr;
		const auto distance_group = item_groups_distance_between.begin() + index_ptr;
		const auto goal_object = belt_utility::find_which_goal_object_index_belongs_too_binary(index_ptr, item_groups_goal_distance, item_groups_distance_between);

		auto return_item = (*item_group).get(get_end_distance_direction<direction>(), get_direction_y_value<direction>(), (*goal_object).get_distance(), *data_group, item_index);
		if (item_groups_type::item_removal_result::item_removed_zero_remains == (*item_group).remove_item(distance_group.operator->(), *data_group, item_index))
			item_group_has_zero_count(item_groups.begin() + index_ptr, data_group);

		--item_groups_goal_item_count[(goal_object - item_groups_goal_distance.begin())];

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
	inline constexpr item_groups_type& get_item_group(std::size_t i) noexcept
	{
#ifdef _BOUNDS_CHECKING_
		ASSERT_NOT_CONSTEXPR(i < item_groups.size())
#endif
			return item_groups[i];
	};

	inline constexpr const item_groups_type& get_item_group(std::size_t i) const noexcept
	{
#ifdef _BOUNDS_CHECKING_
		ASSERT_NOT_CONSTEXPR(i < item_groups.size());
#endif
		return item_groups[i];
	};

	inline constexpr item_groups_data_type& get_item_data_group(std::size_t i) noexcept
	{
#ifdef _BOUNDS_CHECKING_
		ASSERT_NOT_CONSTEXPR(i < item_groups.size())
#endif
			return item_groups_data[i];
	};

	inline constexpr const item_groups_data_type& get_item_data_group(std::size_t i) const noexcept
	{
#ifdef _BOUNDS_CHECKING_
		ASSERT_NOT_CONSTEXPR(i < item_groups.size());
#endif
		return item_groups_data[i];
	};

	inline constexpr _vector_distance::iterator get_distance_between_item_groups_iterator(std::size_t i) const noexcept
	{
		return item_groups_distance_between.begin() + i;
	};

	inline constexpr index_inserter& get_inserter(belt_utility::add_inserter_return_indexes indexes) noexcept
	{
#ifdef ENABLE_CPP_EXCEPTION_THROW
		if (indexes.index >= inserters.size()) throw std::out_of_range("");
		if (indexes.nested_index >= inserters[indexes.index].size()) throw std::out_of_range("");
#endif
		return inserters[indexes.index][indexes.nested_index];

#ifdef ENABLE_CPP_EXCEPTION_THROW
		throw std::runtime_error(""); //this shouldn't occur as long as the index is valid and not human entered
#endif
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

#ifdef ENABLE_CPP_EXCEPTION_THROW
		if (count_index == i) throw std::runtime_error("");
#endif

#ifdef ENABLE_CPP_EXCEPTION_THROW
		throw std::runtime_error(""); //this shouldn't occur as long as the index is valid and not human entered
#endif

		return inserters.begin()->operator[](0);
	};

	inline constexpr const index_inserter& get_inserter(belt_utility::add_inserter_return_indexes indexes) const noexcept
	{
#ifdef ENABLE_CPP_EXCEPTION_THROW
		if (indexes.index >= inserters.size()) throw std::out_of_range("");
		if (indexes.nested_index >= inserters[indexes.index].size()) throw std::out_of_range("");
#endif
		return inserters[indexes.index][indexes.nested_index];

#ifdef ENABLE_CPP_EXCEPTION_THROW
		throw std::runtime_error(""); //this shouldn't occur as long as the index is valid and not human entered
#endif
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

#ifdef ENABLE_CPP_EXCEPTION_THROW
		throw std::runtime_error(""); //this shouldn't occur as long as the index is valid and not human entered
#endif
	};

	inline constexpr long long get_new_item_distance(std::size_t i) const noexcept
	{
		return item_groups_distance_between[i];
	};
	template<belt_utility::belt_direction direction>
	inline constexpr long long get_new_item_position(std::size_t i) const noexcept
	{
#ifdef ENABLE_CPP_EXCEPTION_THROW
		if (i >= item_groups_goal_distance.size())
			throw std::runtime_error("");
		if (i >= item_groups_distance_between.size())
			throw std::runtime_error("");
#endif
		const auto goal_object = belt_utility::find_which_goal_object_index_belongs_too(i, item_groups_goal_distance, item_groups_distance_between);
#ifdef _BOUNDS_CHECKING_
		if (item_groups_goal_distance.last() == goal_object) return -1ll;
#endif
#ifdef ENABLE_CPP_EXCEPTION_THROW
		if ((*goal_object).get_index_ptr() == nullptr)// || (*goal_object).get_index_ptr() < item_groups_distance_between.begin().operator->() || (*goal_object).get_index_ptr() >= item_groups_distance_between.last().operator->())
			throw std::runtime_error("");
#endif
		auto index_from_ptr = ((*goal_object).get_index_from_ptr(item_groups_distance_between.begin().operator->()));
		return get_end_distance_direction<direction>() - belt_utility::get_distances_from_to2(item_groups_distance_between.begin() + index_from_ptr, item_groups_distance_between.begin() + i) + 32;
	};

	inline constexpr long long count_item_groups() const noexcept
	{
		return item_groups.size();
	};

	inline constexpr long long count_items_in_group(long long i) const noexcept
	{
#ifdef _BOUNDS_CHECKING_
		ASSERT_NOT_CONSTEXPR(i < item_groups.size());
#endif
		return item_groups[i].count();
	};

	inline constexpr long long count_all_items() const noexcept
	{
		long long total = 0ll;
		const long long l = item_groups.size();
		long long i = 0;
		for (; i < l; ++i)
		{
#ifdef _BOUNDS_CHECKING_
			ASSERT_NOT_CONSTEXPR(i < item_groups.size());
#endif
			const auto count = item_groups[i].count();
			total += count;
#ifdef _DEBUG
			if (total < 0ll) __debugbreak();
#endif
		}
		return total;
	};

	inline constexpr long long goal_distance_in_group(long long i) const noexcept
	{
#ifdef _BOUNDS_CHECKING_
		ASSERT_NOT_CONSTEXPR(i < item_groups_distance_between.size());
#endif
		return item_groups_distance_between[i];
	};
	inline constexpr long long get_item_groups_goal_distance_size() const noexcept
	{
		return item_groups_goal_distance.size();
	};
	inline constexpr goal_distance get_goal_distance(long long i) noexcept
	{
#ifdef _BOUNDS_CHECKING_
		ASSERT_NOT_CONSTEXPR(i < item_groups_goal_distance.size());
#endif
		return item_groups_goal_distance[i];
	};
	inline constexpr long long goal_distance_in_destinations(long long i) noexcept
	{
#ifdef _BOUNDS_CHECKING_
		ASSERT_NOT_CONSTEXPR(i < item_groups_goal_distance.size());
#endif
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

#ifndef __TEMPLATED_DIRECTION__
	inline constexpr long long get_direction_y_value() const noexcept
	{
		switch (segment_direction)
		{
			default:
			case belt_utility::belt_direction::left_right: return start_of_segment.y;
			case belt_utility::belt_direction::right_left: return start_of_segment.y;
			case belt_utility::belt_direction::top_bottom: return start_of_segment.x;
			case belt_utility::belt_direction::bottom_top: return start_of_segment.x;
			case belt_utility::belt_direction::null: return 0;
		}
	};
	inline constexpr long long get_start_direction_value() const noexcept
	{
		switch (segment_direction)
		{
			default:
			case belt_utility::belt_direction::left_right: return start_of_segment.x;
			case belt_utility::belt_direction::right_left: return start_of_segment.x;
			case belt_utility::belt_direction::top_bottom: return start_of_segment.y;
			case belt_utility::belt_direction::bottom_top: return start_of_segment.y;
			case belt_utility::belt_direction::null: return 0;
		}
	};
	inline constexpr long long get_end_distance() const noexcept
	{
		switch (segment_direction)
		{
			default:
			case belt_utility::belt_direction::left_right: return end_of_segment.x - start_of_segment.x;
			case belt_utility::belt_direction::right_left: return start_of_segment.x - end_of_segment.x;
			case belt_utility::belt_direction::top_bottom: return end_of_segment.y - start_of_segment.y;
			case belt_utility::belt_direction::bottom_top: return start_of_segment.y - end_of_segment.y;
			case belt_utility::belt_direction::null: return 0;
		}
	};
	inline constexpr long long get_end_distance_direction() const noexcept
	{
		switch (segment_direction)
		{
			case belt_utility::belt_direction::left_right: return end_of_segment.x;
			case belt_utility::belt_direction::right_left: return end_of_segment.x;
			case belt_utility::belt_direction::top_bottom: return end_of_segment.y;
			case belt_utility::belt_direction::bottom_top: return end_of_segment.y;
			default:
			case belt_utility::belt_direction::null: return 0;
		}
	};
#else
	template<belt_utility::belt_direction direction>
	inline constexpr long long get_direction_y_value() const noexcept
	{
		if constexpr (belt_utility::belt_direction::left_right == direction) return start_of_segment.y;
		if constexpr (belt_utility::belt_direction::right_left == direction) return start_of_segment.y;
		if constexpr (belt_utility::belt_direction::top_bottom == direction) return start_of_segment.x;
		if constexpr (belt_utility::belt_direction::bottom_top == direction) return start_of_segment.x;

		return 0ll;
	};
	template<belt_utility::belt_direction direction>
	inline constexpr long long get_start_direction_value() const noexcept
	{
		if constexpr (belt_utility::belt_direction::left_right == direction) return start_of_segment.x;
		if constexpr (belt_utility::belt_direction::right_left == direction) return start_of_segment.x;
		if constexpr (belt_utility::belt_direction::top_bottom == direction) return start_of_segment.y;
		if constexpr (belt_utility::belt_direction::bottom_top == direction) return start_of_segment.y;

		return 0ll;
	};
	template<belt_utility::belt_direction direction>
	inline constexpr long long get_end_distance() const noexcept
	{
		if constexpr (belt_utility::belt_direction::left_right == direction) return end_of_segment.x - start_of_segment.x;
		if constexpr (belt_utility::belt_direction::right_left == direction) return start_of_segment.x - end_of_segment.x;
		if constexpr (belt_utility::belt_direction::top_bottom == direction) return end_of_segment.y - start_of_segment.y;
		if constexpr (belt_utility::belt_direction::bottom_top == direction) return start_of_segment.y - end_of_segment.y;

		return 0ll;
	};
	template<belt_utility::belt_direction direction>
	inline constexpr long long get_end_distance_direction() const noexcept
	{
		if constexpr (belt_utility::belt_direction::left_right == direction) return end_of_segment.x;
		if constexpr (belt_utility::belt_direction::right_left == direction) return end_of_segment.x;
		if constexpr (belt_utility::belt_direction::top_bottom == direction) return end_of_segment.y;
		if constexpr (belt_utility::belt_direction::bottom_top == direction) return end_of_segment.y;

		return 0ll;
	};
#endif

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
	inline constexpr std::vector<long long> get_current_goal_distance_pointers_item_groups_removed(const std::vector<remove_iterators_>& future_removes) const noexcept
	{
		std::vector<long long> old_distances;
		auto begin_future = future_removes.begin();
		const auto e_iter = item_groups_goal_distance.last();
		for (auto b_iter = item_groups_goal_distance.begin(); b_iter != e_iter; ++b_iter)
		{
			if (begin_future != future_removes.end())
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
			for (auto b_iter = old_distance_values.begin(), e_iter = old_distance_values.end(); b_iter != e_iter; ++b_iter)
			{
				if (*b_iter == 0ll) //same distance value
				{
					++offset_value;
				}
				else
				{
					(*old_dist_iter).offset_ptr(offset_value);
					item_groups_goal_distance[index] = *old_dist_iter;
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

	inline constexpr void item_group_has_zero_count(item_groups_type* ptr, item_groups_data_type* data_ptr) noexcept
	{
		const auto index_ptr = ptr - item_groups.begin().operator->();
		const auto goal_dist_iter = belt_utility::find_which_goal_object_index_belongs_too_binary(index_ptr, item_groups_goal_distance, item_groups_distance_between);
		const auto dist_iter = item_groups_distance_between.begin() + index_ptr;

#ifdef ENABLE_CPP_EXCEPTION_THROW
		if (goal_dist_iter == item_groups_goal_distance.last()) throw std::runtime_error("");
#endif
		remove_iterators.push_back({ _vector::iterator{ ptr }, _data_vector::iterator{ data_ptr }, dist_iter, goal_dist_iter });
	};
	inline constexpr void item_group_has_zero_count(_vector::iterator ptr, _data_vector::iterator data_ptr) noexcept
	{
		const auto index_ptr = ptr - item_groups.begin();
		auto goal_dist_iter = belt_utility::find_which_goal_object_index_belongs_too_binary(index_ptr, item_groups_goal_distance, item_groups_distance_between);
		auto dist_iter = item_groups_distance_between.begin() + index_ptr;

#ifdef ENABLE_CPP_EXCEPTION_THROW
		if (goal_dist_iter == item_groups_goal_distance.last()) throw std::runtime_error("");
#endif
		remove_iterators.emplace_back(ptr, data_ptr, dist_iter, goal_dist_iter);
	};

	template<belt_utility::belt_direction direction>
	inline constexpr void item_group_has_reached_goal(_vector::iterator ptr, _data_vector::iterator item_data, _vector_goal_distance::iterator goal_distance) noexcept
	{
		if (segment_end_points.size() > 0ull)
		{
			const auto end_distance_direction = get_end_distance_direction<direction>();
			const auto direction_y_value = get_direction_y_value<direction>();
			for (long long i2 = 0; i2 < tc::sign(segment_end_points.size()); ++i2)
			{
#ifdef _BOUNDS_CHECKING_
				ASSERT_NOT_CONSTEXPR(i2 < segment_end_points.size());
#endif
				auto segment_ptr = segment_end_points[i2];
				const auto ptr_position = ptr->get_position(end_distance_direction, direction_y_value, goal_distance->get_distance());
				if (segment_ptr->start_of_segment != ptr_position) continue;
				if (segment_ptr->add_item<direction>(ptr->get_first_item(end_distance_direction, direction_y_value, goal_distance->get_distance(), *item_data)))
				{
					if (ptr->count() == 1ll)
					{
						ptr->remove_last_item(*item_data);
						item_group_has_zero_count(ptr, item_data);
					}
					else
					{
						if (item_groups_type::item_removal_result::item_removed_zero_remains == ptr->remove_first_item((*goal_distance).get_unsafe_index_ptr(), *item_data))
							item_group_has_zero_count(ptr, item_data);
					}
					item_was_removed = true;
					return;
				}
			}
		}
	};

	__declspec(noinline) constexpr void item_groups_removal() noexcept
	{
		if (remove_iterators.size() > 0ll)
		{
#ifdef _DEBUG
			auto old_goal_values = get_current_goal_distance_pointers_item_groups_removed(remove_iterators);
			const auto old_size = item_groups.size();
#endif
			auto begin_remove_iter = remove_iterators.begin();
			for (; begin_remove_iter != remove_iterators.end(); ++begin_remove_iter)
			{
				auto goal_dist_writer_iter = begin_remove_iter->item_groups_goal_dist_iter;
				if ((*goal_dist_writer_iter).get_index_ptr() == item_groups_distance_between.begin().operator->())
					(*goal_dist_writer_iter).set_index_ptr(nullptr);
				else if ((goal_dist_writer_iter - item_groups_goal_distance.begin()) > 0ll)
				{
					if ((*(goal_dist_writer_iter - 1ll)).get_index_ptr() == (*goal_dist_writer_iter).get_offset_ptr(-1ll))
					{
						auto tmp_begin_iter = item_groups_goal_distance.begin();
						if ((*goal_dist_writer_iter).get_index_ptr() == *tmp_begin_iter) (*goal_dist_writer_iter).set_index_ptr(nullptr);
						else (*goal_dist_writer_iter).set_checked_index_ptr(nullptr);
					}
				}
				if ((*goal_dist_writer_iter) != nullptr)
					(*goal_dist_writer_iter).update_pointer_and_values(item_groups_distance_between.begin().operator->(), 1ll, goal_distance_dead_object_v);
			}

			remove_iterators.clear();
			return;
		}
	};

public:
	template<belt_utility::belt_direction direction>
	__declspec(noinline) constexpr void update_item_old() noexcept
	{
		auto begin_iter = item_groups_goal_distance.begin();
		auto begin_goal_count_iter = item_groups_goal_item_count.begin();
		auto last_iter = item_groups_goal_distance.last();
		const auto last_goal_destination = inserters.empty() ? last_iter : last_iter - 1ll;

		auto begin_inserter_goal_iter = inserter_slots.begin();
		auto next_inserter_goal_iter = inserter_slots.last();

		const auto end_distance = get_end_distance_direction<direction>();

		if (inserters.size() > 1) next_inserter_goal_iter = begin_inserter_goal_iter + 1;

		while (begin_iter < last_iter)
		{
			if (*begin_iter != nullptr)
			{
				const auto cur_dist = begin_iter->get_distance();
				if (cur_dist > 0ll)
				{
					if (std::is_constant_evaluated() == false) item_groups_type::items_moved_per_frame += (*begin_goal_count_iter);

					if (begin_inserter_goal_iter != inserter_slots.last())
					{
						const auto index_ptr = begin_iter->get_index_from_ptr(item_groups_distance_between.begin().operator->());
						auto& item_data_ref = item_groups_data[index_ptr];
						auto& item_groups_ref = item_groups[index_ptr];

						auto& nested_inserter_vector = inserters[begin_inserter_goal_iter->get_index()];
						const auto temp_last_item_position = cur_dist + item_groups_ref.get_distance_to_last_item(item_data_ref);
						for (auto nested_ins_iter = nested_inserter_vector.begin(), nested_ins_last = nested_inserter_vector.last(); nested_ins_iter != nested_ins_last; ++nested_ins_iter)
						{
							auto inserter_item_type = nested_ins_iter->get_item_type(0);
							auto inserter_position = belt_utility::get_direction_position<direction>(nested_ins_iter->get_position());
							const auto inserter_distance_position = end_distance - inserter_position;
							if (cur_dist <= inserter_distance_position + item_groups_type::belt_item_size)
							{
								if (temp_last_item_position >= inserter_distance_position - item_groups_type::belt_item_size)
								{
									const auto found_data = item_groups_ref.get_first_item_of_type_before_position<direction>(end_distance, cur_dist, item_data_ref, inserter_item_type, inserter_position);
									if (found_data.found_index >= 0)
									{
										if (found_data.item_distance_position >= inserter_position && found_data.item_distance_position <= inserter_position + item_groups_type::belt_item_size)
										{
											nested_ins_iter->grab_item(remove_item<direction>(&item_groups_ref, found_data.found_index));
#ifdef _DEBUG
											++(*(*begin_inserter_goal_iter)).local_grabbed_items;
											(*(*begin_inserter_goal_iter)).loop_count = 0;
#endif
										}
#ifdef _DEBUG
										else
											++(*(*begin_inserter_goal_iter)).missed_grabs;
#endif
									}
								}
#ifdef _DEBUG
								else
									++(*(*begin_inserter_goal_iter)).missed_grabs;
#endif
							}
#ifdef _DEBUG
							else
								++(*(*begin_inserter_goal_iter)).missed_grabs;

							++(*(*begin_inserter_goal_iter)).loop_count;
#endif
						}
					}

					begin_iter->subtract_goal_distance(1ll);
				}
				else if (cur_dist != -1ll)
				{
					const auto index_ptr = begin_iter->get_index_from_ptr(item_groups_distance_between.begin().operator->());
					item_groups[index_ptr].items_stuck_update(item_groups_data[index_ptr]);
				}
			}

			++begin_inserter_goal_iter;
			++begin_iter;
			++begin_goal_count_iter;
		}
	};

	template<belt_utility::belt_direction direction>
	__declspec(noinline) constexpr void update_item() noexcept
	{
		auto begin_iter = item_groups_goal_distance.begin();
		auto begin_goal_count_iter = item_groups_goal_item_count.begin();
		auto last_iter = item_groups_goal_distance.last();
		const auto last_goal_destination = inserters.empty() ? last_iter : last_iter - 1ll;

		auto begin_inserter_goal_iter = inserter_slots.begin();
		auto next_inserter_goal_iter = inserter_slots.last();
		const auto end_inserter_goal_iter = inserter_slots.last();

		if (inserters.size() > 1) next_inserter_goal_iter = begin_inserter_goal_iter + 1;

		while (begin_iter < last_iter)
		{
			if (*begin_iter != nullptr)
			{
				const auto cur_dist = begin_iter->get_distance();
				if (cur_dist > 0ll)
				{
					if (std::is_constant_evaluated() == false) item_groups_type::items_moved_per_frame += (*begin_goal_count_iter);

					if (begin_inserter_goal_iter != end_inserter_goal_iter)
					{
						const auto index_ptr = begin_iter->get_index_from_ptr(item_groups_distance_between.begin().operator->());
						auto& item_data_ref = item_groups_data[index_ptr];
						auto& item_groups_ref = item_groups[index_ptr];

						auto& nested_inserter_vector = inserters[begin_inserter_goal_iter->get_index()];
						for (auto nested_ins_iter = nested_inserter_vector.begin(), nested_ins_last = nested_inserter_vector.last(); nested_ins_iter != nested_ins_last; ++nested_ins_iter)
						{
							//const auto inserter_distance_position = end_distance - inserter_position;
							const auto temp_last_item_position = cur_dist + item_groups_ref.get_distance_to_last_item(item_data_ref);
							if (cur_dist <= nested_ins_iter->get_distance_position_plus() && temp_last_item_position >= nested_ins_iter->get_distance_position_minus())
							{
								const auto end_distance = get_end_distance_direction<direction>();
								const auto inserter_position = belt_utility::get_direction_position<direction>(nested_ins_iter->get_position());
								const auto found_data = item_groups_ref.get_first_item_of_type_before_position<direction>(end_distance, cur_dist, item_data_ref, nested_ins_iter->get_item_type(0), inserter_position);

								if (found_data.found_index >= 0 && found_data.item_distance_position >= inserter_position && found_data.item_distance_position <= inserter_position + item_groups_type::belt_item_size)
								{
									nested_ins_iter->grab_item(remove_item<direction>(&item_groups_ref, found_data.found_index));
#ifdef _DEBUG
									++nested_ins_iter->local_grabbed_items;
									nested_ins_iter->loop_count = 0;
#endif
								}
#ifdef _DEBUG
								else
									++nested_ins_iter->missed_grabs;
#endif
							}
#ifdef _DEBUG
							else
								++nested_ins_iter->missed_grabs;
#endif
#ifdef _DEBUG
							++nested_ins_iter->loop_count;
#endif
						}
					}

					begin_iter->subtract_goal_distance(1ll);
				}
				else if (cur_dist != -1ll)
				{
					const auto index_ptr = begin_iter->get_index_from_ptr(item_groups_distance_between.begin().operator->());
					item_groups[index_ptr].items_stuck_update(item_groups_data[index_ptr]);
				}
			}

			++begin_inserter_goal_iter;
			++begin_iter;
			++begin_goal_count_iter;
		}
	};


	constexpr void update() noexcept
	{
		switch (segment_direction)
		{
			case belt_utility::belt_direction::null:
			case belt_utility::belt_direction::left_right: update<belt_utility::belt_direction::left_right>(); break;
			case belt_utility::belt_direction::right_left: update<belt_utility::belt_direction::right_left>(); break;
			case belt_utility::belt_direction::top_bottom: update<belt_utility::belt_direction::top_bottom>(); break;
			case belt_utility::belt_direction::bottom_top: update<belt_utility::belt_direction::bottom_top>(); break;
		}
	};
private:
	template<belt_utility::belt_direction direction>
	constexpr void update() noexcept
	{
		update_item<direction>();
		if (!item_groups_goal_distance.empty())
		{
			if ((item_groups_goal_distance.begin()->get_distance()) == 0ll)
				item_group_has_reached_goal<direction>(item_groups.begin(), item_groups_data.begin(), item_groups_goal_distance.begin());
		}

		if (!remove_iterators.empty())
			item_groups_removal();
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
				item_groups_goal_distance.emplace(begin_goal, goal_distance.operator->());
				item_groups_goal_item_count.emplace(item_groups_goal_item_count.begin() + (begin_goal - item_groups_goal_distance.begin()), 0ull);
				recalculate_distances_between_from_to((*begin_goal).get_unsafe_index_ptr(), goal_distance, first_distance_between);
				return true;
			}
			else
			{
				--goal_distance;
			}
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
							item_groups_goal_distance.emplace(next_goal, goal_distance.operator->());
							item_groups_goal_item_count.emplace(item_groups_goal_item_count.begin() + (next_goal - item_groups_goal_distance.begin()), 0ull);
							recalculate_distances_between_from_to((*begin_goal).get_unsafe_index_ptr(), goal_distance, first_distance);
							return true;
						}
						else
						{
							--goal_distance;
						}
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
#ifdef ENABLE_CPP_EXCEPTION_THROW
		if (distance_between_inserted_index - 1ll < 0) throw std::runtime_error("negative index");
		if (distance_between_inserted_index - 1ll >= item_groups_distance_between.size()) throw std::runtime_error("index is larger than the vector size");
#endif

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
			if constexpr (belt_utility::distance_slot_inserted_position::new_item_after_last_goal != position_setting)
				begin_inserter_iter = inserters.begin();
			if constexpr (belt_utility::distance_slot_inserted_position::new_item_after_last_goal == position_setting)
			{
				const auto inserter_size = (item_groups_goal_distance.size() - 1ll);
				if (inserter_size < inserters.size())
					begin_inserter_iter = inserters.begin() + inserter_size;
				else
					begin_inserter_iter = inserters.begin();
			}

			auto distance_comparison = belt_utility::get_distance_comparison<direction>(get_end_distance_direction<direction>(), *distance, begin_inserter_iter, begin_inserter_iter, decltype(item_groups)::value_type::belt_item_size);
			if (belt_utility::distance_comparison::distance_is_after == distance_comparison)
			{
				do
				{
					++begin_inserter_iter;
					if (begin_inserter_iter != inserters.last())
						distance_comparison = belt_utility::get_distance_comparison<direction>(get_end_distance_direction<direction>(), *distance, begin_inserter_iter, begin_inserter_iter, decltype(item_groups)::value_type::belt_item_size);
				} while (begin_inserter_iter != inserters.last() && belt_utility::distance_comparison::distance_is_after == distance_comparison);
			}

			decltype(distance_comparison) previous_distance_comp = decltype(distance_comparison)::null;
			decltype(distance_comparison) prev_previous_distance_comp = decltype(distance_comparison)::null;
			if (begin_inserter_iter != inserters.begin() && begin_inserter_iter != inserters.last())
			{
				auto previous_inserter_iter = begin_inserter_iter;

				const auto inserter_distance = end_distance - belt_utility::get_direction_position<direction>(begin_inserter_iter->operator[](0).get_position());
				if (inserter_distance < *distance)
					previous_inserter_iter = begin_inserter_iter - 1ll;

				previous_distance_comp = belt_utility::get_distance_comparison<direction>(get_end_distance_direction<direction>(), *distance, previous_inserter_iter, previous_inserter_iter, decltype(item_groups)::value_type::belt_item_size);
				prev_previous_distance_comp = belt_utility::get_distance_comparison<direction>(get_end_distance_direction<direction>(), *(item_groups_distance_between.begin() + distance_between_inserted_index - 1ll).operator->(), previous_inserter_iter, previous_inserter_iter, decltype(item_groups)::value_type::belt_item_size);

				if (!(belt_utility::distance_comparison::distance_is_inside == previous_distance_comp && belt_utility::distance_comparison::distance_is_inside == prev_previous_distance_comp) &&
					!(belt_utility::distance_comparison::distance_is_inside == previous_distance_comp && belt_utility::distance_comparison::distance_is_before == prev_previous_distance_comp) &&
					belt_utility::distance_comparison::distance_is_before != previous_distance_comp && belt_utility::distance_comparison::distance_is_after != prev_previous_distance_comp)
					skip_loop = true;
			}
			else if (begin_inserter_iter == inserters.last() && item_groups_goal_distance.size() == inserters.size())
			{
				skip_loop = true;
			}

			if (item_groups_goal_distance.size() == inserters.size() && belt_utility::distance_comparison::distance_is_after == distance_comparison)
				skip_loop = true;

			if (skip_loop == false)
			{
				auto begin_iter = item_groups_goal_distance.begin();
				auto next_iter = begin_iter + 1ll;
				const auto end_iter = item_groups_goal_distance.last();
				for (; begin_iter != end_iter; ++begin_iter, ++next_iter)
				{
					if ((*begin_iter) == (item_groups_distance_between.begin() + (distance_between_inserted_index - 1ll)).operator->())
					{
						begin_iter->set_index_ptr(distance);
						auto previous_iter = (item_groups_distance_between.begin() + ((*begin_iter).get_index_from_ptr(item_groups_distance_between.begin().operator->()))) - 1ll;
						*previous_iter = 32ll;

						return belt_utility::need_new_slot_result::update_pointer_to_new_index;
					}
				}
			}

#ifdef __belt_segment_dead_code__
			__debugbreak();

			begin_inserter_iter = inserters.begin();
			auto inserter_distance = end_distance - belt_utility::get_direction_position(segment_direction, begin_inserter_iter->operator[](0).get_position());
			auto last_inserter_distance = end_distance - belt_utility::get_direction_position(segment_direction, ((*begin_inserter_iter).last() - 1ll)->get_position());
			if (item_groups_goal_distance.size() == 1ll && (*distance > inserter_distance || *distance > last_inserter_distance))
			{
				auto begin_iter = item_groups_goal_distance.begin();
				auto old_goal_distance = (*begin_iter).get_index_ptr();
				item_groups_goal_distance[0].set_index_ptr(distance);
				const auto recalc_start_iter = item_groups_distance_between.begin() + ((*begin_iter).get_index_from_ptr(item_groups_distance_between.begin().operator->()));
				const auto recalc_end_iter = item_groups_distance_between.begin();
				recalculate_distances_between_from_to(old_goal_distance, recalc_start_iter, recalc_end_iter);
				return belt_utility::need_new_slot_result::update_pointer_to_new_index;
			}

			if (inserters.size() >= item_groups_goal_distance.size())
			{
				auto tmp_begin_inserter_iter = inserters.begin() + (item_groups_goal_distance.size() - 1ll);
				if (tmp_begin_inserter_iter != inserters.end())
				{
					inserter_distance = end_distance - belt_utility::get_direction_position(segment_direction, tmp_begin_inserter_iter->operator[](0).get_position());
					last_inserter_distance = end_distance - belt_utility::get_direction_position(segment_direction, ((*tmp_begin_inserter_iter).last() - 1ll)->get_position());
					//we should update the pointer to the new index
					auto before_last = item_groups_goal_distance.last() - 1ll;
					if (*distance < before_last->get_distance() && (*distance > inserter_distance || *distance > last_inserter_distance))
					{
						before_last->set_index_ptr(distance);
						auto previous_iter = (item_groups_distance_between.begin() + ((*before_last).get_index_from_ptr(item_groups_distance_between.begin().operator->()))) - 1ll;
						*previous_iter = 32ll;
						return belt_utility::need_new_slot_result::update_pointer_to_new_index;
					}
					//else
						//return belt_utility::need_new_slot_result::need_new_slot;
				}
			}
			else if (item_groups_goal_distance.size() > 2ll)
			{
				inserter_distance = end_distance - belt_utility::get_direction_position(segment_direction, (inserters.last() - 1ull)->operator[](0).get_position());
				last_inserter_distance = end_distance - belt_utility::get_direction_position(segment_direction, ((*(inserters.last() - 1ull)).last() - 1ll)->get_position());
				auto before_last = item_groups_goal_distance.last() - 1ll;
				const auto before_before_last = before_last - 1ll;
				if (item_groups_goal_distance.size() < inserters.size())
				{
					if (before_before_last->get_distance() > *distance && *distance < before_last->get_distance() && (inserter_distance > *distance || last_inserter_distance > *distance))
					{
						before_last->set_index_ptr(distance);
						auto tmp_iter_ptr = item_groups_distance_between.begin().operator->();
						long long index_from_ptr = 0ll;
						decltype(item_groups_distance_between)::iterator previous_iter;
						if (item_groups_goal_distance.size() >= inserters.size())
						{
							index_from_ptr = ((*before_before_last).get_index_from_ptr(tmp_iter_ptr));
							previous_iter = (item_groups_distance_between.begin() + index_from_ptr);
						}
						else
						{
							index_from_ptr = ((*before_last).get_index_from_ptr(tmp_iter_ptr));
							previous_iter = (item_groups_distance_between.begin() + index_from_ptr) - 1ll;
						}
						*previous_iter = 32ll;
						return belt_utility::need_new_slot_result::update_pointer_to_new_index;
					}
				}
				else return belt_utility::need_new_slot_result::update_pointer_to_new_index;
			}

			auto begin_iter = item_groups_goal_distance.begin();
			auto next_iter = begin_iter + 1ll;
			const auto end_iter = item_groups_goal_distance.last();
			for (; next_iter != end_iter; ++begin_iter, ++next_iter)
			{
				if (begin_inserter_iter != inserters.end())
				{
					inserter_distance = end_distance - belt_utility::get_direction_position(segment_direction, begin_inserter_iter->operator[](0).get_position());
					last_inserter_distance = end_distance - belt_utility::get_direction_position(segment_direction, ((*begin_inserter_iter).last() - 1ll)->get_position());
				}
				if (begin_iter->get_distance() > *distance && *distance < next_iter->get_distance() && (*distance > inserter_distance || *distance > last_inserter_distance))
				{
					//if inserter distance is less than begin_iter update **begin_iter 
					if (inserter_distance < begin_iter->get_distance() || last_inserter_distance < begin_iter->get_distance())
					{
						auto old_goal_distance = (*begin_iter).get_index_ptr();
						begin_iter->set_index_ptr(distance);
						const auto recalc_start_iter = item_groups_distance_between.begin() + ((*begin_iter).get_index_from_ptr(item_groups_distance_between.begin().operator->()));
						const auto recalc_end_iter = item_groups_distance_between.begin() + ((*(begin_iter - 1ll)).get_offset_ptr(1ll).get_index_from_ptr(item_groups_distance_between.begin().operator->()));
						recalculate_distances_between_from_to(old_goal_distance, recalc_start_iter, recalc_end_iter);
						return belt_utility::need_new_slot_result::update_pointer_to_new_index;
					}
					else
						return belt_utility::need_new_slot_result::object_is_between_slots;
				}
				/*we should update the pointer to the new index
				if (next_iter == end_iter - 1ll)
				{
					*next_iter = distance;
					return belt_utility::need_new_slot_result::update_pointer_to_new_index;
				}*/

				if (begin_inserter_iter != inserters.last() && begin_inserter_iter + 1 != inserters.last() && (next_iter->get_distance() < inserter_distance || next_iter->get_distance() < last_inserter_distance))
					++begin_inserter_iter;
			};

			if (begin_inserter_iter != inserters.end())
			{
				inserter_distance = end_distance - belt_utility::get_direction_position(segment_direction, begin_inserter_iter->operator[](0).get_position());
				last_inserter_distance = end_distance - belt_utility::get_direction_position(segment_direction, ((*begin_inserter_iter).last() - 1ll)->get_position());
				//we should update the pointer to the new index
				auto before_last = item_groups_goal_distance.last() - 1ll;

				//checks that the inserter is before the new destination and the new destination is after the last one
				if (*distance < before_last->get_distance() && (*distance > inserter_distance || *distance > last_inserter_distance))
				{
					before_last->set_index_ptr(distance);
					auto previous_iter = (item_groups_distance_between.begin() + ((*before_last).get_index_from_ptr(item_groups_distance_between.begin().operator->()))) - 1ll;
					*previous_iter = 32ll;
					return belt_utility::need_new_slot_result::update_pointer_to_new_index;
				}
				/*else if (item_groups_goal_distance.size() >= 2ll && *distance < before_last->get_distance() && (inserter_distance > *distance || last_inserter_distance > *distance))
				{
					auto index_from_ptr = (*before_last).get_index_from_ptr(item_groups_distance_between.begin().operator->());
					if (item_groups.size() == index_from_ptr + 2ll &&
						item_groups.begin() + (index_from_ptr + 1ll) == item_groups.last() - 1ll &&
						item_groups[index_from_ptr + 1].count() == 0ll)
					{
						before_last->set_index_ptr(distance);
						auto previous_iter = item_groups_distance_between.begin() + ((*before_last).get_index_from_ptr(item_groups_distance_between.begin().operator->())) - 1ll;
						*previous_iter = 32ll;
						return belt_utility::need_new_slot_result::update_pointer_to_new_index;
					}
				}*/
			}
			else
			{
				inserter_distance = end_distance - belt_utility::get_direction_position(segment_direction, (inserters.last() - 1ull)->operator[](0).get_position());
				last_inserter_distance = end_distance - belt_utility::get_direction_position(segment_direction, ((*(inserters.last() - 1ull)).last() - 1ll)->get_position());
				auto before_last = item_groups_goal_distance.last() - 1ll;
				const auto before_before_last = before_last - 1ll;
				if (before_before_last->get_distance() > *distance && *distance < before_last->get_distance() && (inserter_distance > *distance || last_inserter_distance > *distance))
				{
					before_last->set_index_ptr(distance);
					auto previous_iter = (item_groups_distance_between.begin() + ((*before_last).get_index_from_ptr(item_groups_distance_between.begin().operator->()))) - 1ll;
					*previous_iter = 32ll;
					return belt_utility::need_new_slot_result::update_pointer_to_new_index;
				}
			}
#endif
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
		const auto goal_object = belt_utility::find_which_goal_object_index_belongs_too(index_ptr_temp, item_groups_goal_distance, item_groups_distance_between);
		if (goal_object != item_groups_goal_distance.last())
		{
			new_distance_value = belt_utility::get_direction_position<direction>(new_item.position) - calculate_distance_(item_groups_distance_between.begin() + index_ptr_temp, goal_object);
		}
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
		auto item_distance_position = belt_utility::get_direction_position<direction>(new_item.position);
		auto new_distance_value = get_end_distance_direction<direction>() - item_distance_position;
		auto& new_distance = item_groups_distance_between.emplace_back(new_distance_value);
		if (!old_distances.empty()) update_old_goal_distance_pointers(old_distances);

		const auto new_slot_result = need_new_goal_distance_slot<direction, belt_utility::distance_slot_inserted_position::new_item_after_last_goal>(&new_distance, item_groups_distance_between.size() - 1ll);
		if (belt_utility::need_new_slot_result::need_new_slot == new_slot_result)
		{
			item_groups_goal_distance.emplace_back(&new_distance);
			item_groups_goal_item_count.emplace_back(1ull);
		}
#ifdef ENABLE_CPP_EXCEPTION_THROW
		if (!inserters.empty() && item_groups_goal_distance.size() > inserters.size() + 1) throw std::runtime_error("");
#endif
		new_iter_group.add_item(get_end_distance_direction<direction>(), &new_distance, new_data_group, new_item, new_item.position);
		const auto goal_object = belt_utility::find_which_goal_object_index_belongs_too_binary(index_ptr_temp, item_groups_goal_distance, item_groups_distance_between);
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
		const auto goal_object = belt_utility::find_which_goal_object_index_belongs_too_binary(index_ptr_temp, item_groups_goal_distance, item_groups_distance_between);
		if (goal_object != item_groups_goal_distance.last() && item_groups_goal_distance.size() > 1ll)
		{
			new_distance_value = belt_utility::get_direction_position<direction>(new_item.position) - calculate_distance_(item_groups_distance_between.begin() + (index_ptr_temp + 1), goal_object);
		}
		const auto new_distance_iter = item_groups_distance_between.emplace(item_groups_distance_between.begin() + (index_ptr_temp + 1), new_distance_value);
		if (!old_distances.empty()) update_old_goal_distance_pointers(old_distances);

		const auto new_slot_result = need_new_goal_distance_slot<direction, belt_utility::distance_slot_inserted_position::new_item_after_iter>(new_distance_iter.operator->(), new_distance_iter - item_groups_distance_between.begin());
		if (belt_utility::need_new_slot_result::need_new_slot == new_slot_result)
		{
			item_groups_goal_distance.emplace_back(new_distance_iter.operator->());
			item_groups_goal_item_count.emplace_back(1ull);
		}

#ifdef ENABLE_CPP_EXCEPTION_THROW
		if (!inserters.empty() && item_groups_goal_distance.size() > inserters.size() + 1) throw std::runtime_error("");
#endif
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
		if (new_iter_group != item_groups.last()) new_iter_group->add_item(get_end_distance_direction<direction>(), new_distance_iter.operator->(), *new_data_group, new_item, new_item.position);
#else
		if (new_iter_group != item_groups.end()) new_iter_group->add_item(new_item, new_item.position);
#endif
		return true;
	};

#ifndef __TEMPLATED_DIRECTION__
	constexpr bool split_item_group(const item_uint& new_item, auto iter, long long index_ptr_temp) noexcept
	{
		const auto index = iter.result->get_first_item_before_position(segment_direction,
			get_end_distance_direction(),
			item_groups_distance_between[index_ptr_temp],
			item_groups_data[index_ptr_temp],
			belt_utility::get_direction_position(segment_direction, new_item.position));

		auto goal_object = belt_utility::find_which_goal_object_index_belongs_too(index_ptr_temp, item_groups_goal_distance, item_groups_distance_between);
		bool is_not_goal_object = true;
		if (goal_object->get_index_ptr() == &item_groups_distance_between[index_ptr_temp])
		{
			goal_object->update_pointers_without_checks(1ll);
			is_not_goal_object = false;
		}

		auto split_group = item_groups[index_ptr_temp].split_from_index(index);
		auto split_data_result = item_data_utility::split_from_index(item_groups_data[index_ptr_temp], index);
		const auto new_distance = item_groups_data[index_ptr_temp].item_distance[item_groups[index_ptr_temp].count() - 1ll] + split_data_result.missing_distance;

		const auto inserted_group = item_groups.insert(item_groups.begin() + index_ptr_temp, split_group);
		const auto inserted_data = item_groups_data.insert(item_groups_data.begin() + index_ptr_temp, split_data_result.data);
		const auto inserted_distance = item_groups_distance_between.insert(item_groups_distance_between.begin() + index_ptr_temp, new_distance);

		short added_index = -1;
		if (is_not_goal_object == false)
		{
			const auto calc_dist = belt_utility::get_distances_from_to2(item_groups_distance_between.begin() + ((*goal_object).get_index_from_ptr(item_groups_distance_between.begin().operator->())), inserted_distance);
			added_index = inserted_group->add_item(get_end_distance_direction(), calc_dist, inserted_distance.operator->(), *inserted_data, new_item, new_item.position);
		}
		else added_index = inserted_group->add_item(get_end_distance_direction(), inserted_distance.operator->(), *inserted_data, new_item, new_item.position);

		return added_index != -1;
	};
#else
	template<belt_utility::belt_direction direction>
	constexpr bool split_item_group(const item_uint& new_item, auto iter, long long index_ptr_temp) noexcept
	{
		const auto index = iter.result->get_first_item_before_position<direction>(get_end_distance_direction<direction>(),
			item_groups_distance_between[index_ptr_temp],
			item_groups_data[index_ptr_temp],
			belt_utility::get_direction_position<direction>(new_item.position));

		auto goal_object = belt_utility::find_which_goal_object_index_belongs_too_binary(index_ptr_temp, item_groups_goal_distance, item_groups_distance_between);
		bool is_not_goal_object = true;
		if (goal_object->get_index_ptr() == &item_groups_distance_between[index_ptr_temp])
		{
			goal_object->update_pointers_without_checks(1ll);
			is_not_goal_object = false;
		}

		auto split_group = item_groups[index_ptr_temp].split_from_index(index);
		auto split_data_result = item_data_utility::split_from_index(item_groups_data[index_ptr_temp], index);
		const auto new_distance = item_groups_data[index_ptr_temp].item_distance[item_groups[index_ptr_temp].count() - 1ll] + split_data_result.missing_distance;

		const auto inserted_group = item_groups.insert(item_groups.begin() + index_ptr_temp, split_group);
		const auto inserted_data = item_groups_data.insert(item_groups_data.begin() + index_ptr_temp, split_data_result.data);
		const auto inserted_distance = item_groups_distance_between.insert(item_groups_distance_between.begin() + index_ptr_temp, new_distance);

		short added_index = -1;
		if (is_not_goal_object == false)
		{
			const auto calc_dist = belt_utility::get_distances_from_to2(item_groups_distance_between.begin() + ((*goal_object).get_index_from_ptr(item_groups_distance_between.begin().operator->())), inserted_distance);
			added_index = inserted_group->add_item(get_end_distance_direction<direction>(), calc_dist, inserted_distance.operator->(), *inserted_data, new_item, new_item.position);
		}
		else added_index = inserted_group->add_item(get_end_distance_direction<direction>(), inserted_distance.operator->(), *inserted_data, new_item, new_item.position);

		return added_index != -1;
	};
#endif

public:
	constexpr bool add_item(const item_uint& new_item) noexcept
	{
		switch (segment_direction)
		{
			case belt_utility::belt_direction::null:
			case belt_utility::belt_direction::left_right: return add_item<belt_utility::belt_direction::left_right>(new_item); break;
			case belt_utility::belt_direction::right_left: return add_item<belt_utility::belt_direction::right_left>(new_item); break;
			case belt_utility::belt_direction::top_bottom: return add_item<belt_utility::belt_direction::top_bottom>(new_item); break;
			case belt_utility::belt_direction::bottom_top: return add_item<belt_utility::belt_direction::bottom_top>(new_item); break;
		}
	};
private:
	template<belt_utility::belt_direction direction>
	constexpr bool add_item(const item_uint& new_item) noexcept
	{
		if (item_groups.size() == 0)
		{
			auto& new_data_group = item_groups_data.emplace_back();
			auto& new_item_group = item_groups.emplace_back();
			auto& new_goal_distance = item_groups_distance_between.emplace_back(get_end_distance_direction<direction>() - belt_utility::get_direction_position<direction>(new_item.position)); //get_end_distance_direction() - belt_utility::get_direction_position(segment_direction, new_item.position)
			item_groups_goal_distance.emplace_back(&new_goal_distance);
			item_groups_goal_item_count.emplace_back(1ull);
#ifdef _BOUNDS_CHECKING_
			ASSERT_NOT_CONSTEXPR(0ll < item_groups.size());
#endif
			const short added_index = new_item_group.add_item(get_end_distance_direction<direction>(), &item_groups_distance_between[0], new_data_group, new_item, new_item.position);

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
			auto item_distance = belt_utility::get_direction_position<direction>(new_item.position);
			auto iter = belt_utility::find_closest_item_group<direction>(get_end_distance_direction<direction>(), item_groups_data, item_groups, item_groups_distance_between, item_groups_goal_distance, belt_utility::get_direction_position<direction>(new_item.position), inserters);

			const auto index_ptr_temp = iter.result - item_groups.begin();

#ifdef _BOUNDS_CHECKING_
			ASSERT_NOT_CONSTEXPR(iter.result != item_groups.end());
#endif
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
						auto goal_object = belt_utility::find_which_goal_object_index_belongs_too_binary(index_ptr_temp, item_groups_goal_distance, item_groups_distance_between);
						++item_groups_goal_item_count[(goal_object - item_groups_goal_distance.begin())];
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
			else if (belt_utility::find_closest_item_group_return_result::new_group_before_iter == iter.scan) //TODO does this add towards the start of end of the belt segment?
			{
				return add_item_before<direction>(new_item, iter, index_ptr_temp);
			}
			else if (belt_utility::find_closest_item_group_return_result::new_group_after_iter == iter.scan) //TODO does this add towards the start of end of the belt segment?
			{
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
				if (iter.result + 1 == item_groups.last())
#else
				if (iter.result + 1 == item_groups.end())
#endif
				{
					return add_item_after_last<direction>(new_item, index_ptr_temp);
				}
				else
				{
					return add_item_after<direction>(new_item, iter, index_ptr_temp);
				}
			}
		}

		return false;
	};

public:
	template<belt_utility::belt_direction direction>
	constexpr std::size_t get_inserter_group_index(auto& group_inserters, index_inserter object) noexcept
	{
		const long long l = group_inserters.size();

		if (l == 1ll)
		{
			if (belt_utility::inserter_fits_results::before == belt_utility::check_if_inserter_is_before_or_after<direction>(group_inserters[0], object)) return 0;
			else return 1;
		}

		for (long long i = 0, i2 = 1; i2 < l; ++i, ++i2)
		{
			const auto test_result = belt_utility::check_if_inserter_is_before_or_after<direction>(group_inserters[i], object);
			if (belt_utility::inserter_fits_results::before == test_result) return i;
			else if (belt_utility::inserter_fits_results::after == test_result)
			{
				const auto second_test_result = belt_utility::check_if_inserter_is_before_or_after<direction>(group_inserters[i2], object);
				if (belt_utility::inserter_fits_results::before == second_test_result) return i;
			}
		}

		if (belt_utility::inserter_fits_results::before == belt_utility::check_if_inserter_is_before_or_after<direction>(group_inserters.back(), object)) return group_inserters.size() - 1ll;
		else return group_inserters.size();
	};

	constexpr belt_utility::add_inserter_return_indexes add_inserter(index_inserter object) noexcept
	{
		switch (segment_direction)
		{
			case belt_utility::belt_direction::null:
			case belt_utility::belt_direction::left_right: return add_inserter<belt_utility::belt_direction::left_right>(object); break;
			case belt_utility::belt_direction::right_left: return add_inserter<belt_utility::belt_direction::right_left>(object); break;
			case belt_utility::belt_direction::top_bottom: return add_inserter<belt_utility::belt_direction::top_bottom>(object); break;
			case belt_utility::belt_direction::bottom_top: return add_inserter<belt_utility::belt_direction::bottom_top>(object); break;
		}
	};
private:
	template<belt_utility::belt_direction direction>
	constexpr belt_utility::add_inserter_return_indexes add_inserter(index_inserter object) noexcept
	{
		constexpr auto max_inserter_distance = item_groups_type::belt_item_size * item_groups_type::max_item_count * 4;

		if (inserters.size() == 0)
		{
			inserters.emplace_back();
			object.set_distance_position_plus(get_end_distance_direction<direction>() - belt_utility::get_direction_position<direction>(object.get_position()) + item_groups_type::belt_item_size);
			object.set_distance_position_minus(get_end_distance_direction<direction>() - belt_utility::get_direction_position<direction>(object.get_position()) - item_groups_type::belt_item_size);
			inserters[0].push_back(object);

			inserter_slots.emplace_back(0ull, 0ull, &inserters);
			return { 0ll, 0ll };
		}

		long long can_fit_index = -1ll;
		long long can_fit_nested_index = -1ll;
		belt_utility::inserter_fits_results insert_results = belt_utility::inserter_fits_results::no_fit;
		if (inserters.size() == 1ll && inserters[0].size() == 1ll)
		{
			if (expr::abs(belt_utility::get_direction_position<direction>(inserters[0ll][0ll].get_position()) - belt_utility::get_direction_position<direction>(object.get_position())) >= max_inserter_distance &&
				expr::abs(belt_utility::get_direction_position<direction>(inserters[0ll].back().get_position()) - belt_utility::get_direction_position<direction>(object.get_position())) >= max_inserter_distance)
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
			auto inserter_first_distance = belt_utility::get_direction_position<direction>(inserters[0][0].get_position()) - belt_utility::get_direction_position<direction>(object.get_position());
			auto inserter_first_last_distance = belt_utility::get_direction_position<direction>(inserters[0].back().get_position()) - belt_utility::get_direction_position<direction>(object.get_position());
			auto inserter_last_distance = belt_utility::get_direction_position<direction>(inserters.back()[0].get_position()) - belt_utility::get_direction_position<direction>(object.get_position());
			auto inserter_last_last_distance = belt_utility::get_direction_position<direction>(inserters.back().back().get_position()) - belt_utility::get_direction_position<direction>(object.get_position());
			if (inserter_last_distance < 0ll &&
				expr::abs(inserter_last_distance) <= max_inserter_distance &&
				expr::abs(inserter_last_last_distance) <= max_inserter_distance)
			{
				insert_results = belt_utility::inserter_fits_results::inbetween;
				can_fit_index = inserters.size() - 1ll;
				can_fit_nested_index = get_inserter_group_index<direction>(inserters.back(), object);
			}
			else if (inserter_first_distance < 0ll &&
				expr::abs(inserter_first_distance) <= max_inserter_distance &&
				expr::abs(inserter_first_last_distance) <= max_inserter_distance)
			{
				insert_results = belt_utility::inserter_fits_results::inbetween;
				can_fit_index = 0ll;
				can_fit_nested_index = get_inserter_group_index<direction>(inserters[0], object);
			}
			else if (expr::abs(inserter_first_distance) >= max_inserter_distance &&
				expr::abs(inserter_first_last_distance) >= max_inserter_distance)
			{
				insert_results = belt_utility::inserter_fits_results::no_fit;
				can_fit_index = inserters.size();
				can_fit_nested_index = 0ll;
			}
			else if (expr::abs(inserter_last_distance) >= max_inserter_distance &&
				expr::abs(inserter_last_last_distance) >= max_inserter_distance)
			{
				insert_results = belt_utility::inserter_fits_results::no_fit;
				can_fit_index = inserters.size();
				can_fit_nested_index = 0ll;
			}
			//first check if it's before the first inserter
			else if (belt_utility::inserter_fits_results::before == belt_utility::check_if_inserter_is_before_or_after<direction>(inserters[0][0], object))
			{
				insert_results = belt_utility::inserter_fits_results::before;
				can_fit_index = 0ll;
				can_fit_nested_index = get_inserter_group_index<direction>(inserters[0ll], object);
			}
			//second check if it's after the last inserter
			else if (belt_utility::inserter_fits_results::after == belt_utility::check_if_inserter_is_before_or_after<direction>(inserters.back()[0], object))
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
					if (expr::abs(belt_utility::get_direction_position<direction>(inserters[i][0].get_position()) - belt_utility::get_direction_position<direction>(object.get_position())) >= max_inserter_distance &&
						expr::abs(belt_utility::get_direction_position<direction>(inserters[i].back().get_position()) - belt_utility::get_direction_position<direction>(object.get_position())) >= max_inserter_distance)
					{
						insert_results = belt_utility::inserter_fits_results::no_fit;
						can_fit_index = l;
						can_fit_nested_index = get_inserter_group_index<direction>(inserters[i2], object);
						break;
					}
					else
					{
						if (belt_utility::inserter_fits_results::inbetween == belt_utility::check_if_inserter_is_between<direction>(inserters[i][0], inserters[i2][0], object))
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
				object.set_distance_position_plus(get_end_distance_direction<direction>() - belt_utility::get_direction_position<direction>(object.get_position()) + item_groups_type::belt_item_size);
				object.set_distance_position_minus(get_end_distance_direction<direction>() - belt_utility::get_direction_position<direction>(object.get_position()) - item_groups_type::belt_item_size);
				group.push_back(object);

				inserter_slots.emplace_back(static_cast<std::size_t>(can_fit_index), 0ull, &inserters);
			}
			else
			{
				inserters[can_fit_index].insert(inserters[can_fit_index].begin() + can_fit_nested_index, object);

				(inserters[can_fit_index].begin() + can_fit_nested_index)->set_distance_position_plus(get_end_distance_direction<direction>() - belt_utility::get_direction_position<direction>(object.get_position()) + item_groups_type::belt_item_size);
				(inserters[can_fit_index].begin() + can_fit_nested_index)->set_distance_position_minus(get_end_distance_direction<direction>() - belt_utility::get_direction_position<direction>(object.get_position()) - item_groups_type::belt_item_size);
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