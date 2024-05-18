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

//#define CONSTEXPR_ASSERTS
#ifdef CONSTEXPR_ASSERTS
#define CONSTEXPR_VAR constexpr
#else
#define CONSTEXPR_VAR static
#endif

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

	std::vector<long long> increment_count;
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
	constexpr inline bool validate_vector_resize(const resize_check& old_values) const
	{
		const auto current_state = get_vector_resize();
		if (old_values.s_item_groups_data != current_state.s_item_groups_data) throw std::runtime_error("");
		if (old_values.s_item_groups != current_state.s_item_groups) throw std::runtime_error("");
		if (old_values.s_item_groups_goal_distance != current_state.s_item_groups_goal_distance) throw std::runtime_error("");
		if (old_values.s_item_groups_distance_between != current_state.s_item_groups_distance_between) throw std::runtime_error("");

		return true;
	};

	constexpr inline bool validate_vectors() const
	{
#ifdef _DEBUG
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
	inline constexpr item_uint get_item(std::size_t item_group, std::size_t i) noexcept
	{
#ifdef _BOUNDS_CHECKING_
		ASSERT_NOT_CONSTEXPR(i < item_groups.size())
#endif
			return item_groups[item_group].get(get_end_distance_direction(), get_direction_y_value(), item_groups_distance_between[item_group], item_groups_data[item_group], i);
	};
	inline constexpr item_uint remove_item(item_groups_type* item_group, const long long item_index) noexcept
	{
		const auto index_ptr = item_group - item_groups.begin();
		const auto data_group = item_groups_data.begin() + index_ptr;
		const auto distance_group = item_groups_distance_between.begin() + index_ptr;
		const auto goal_object = belt_utility::find_which_goal_object_index_belongs_too(index_ptr, item_groups_goal_distance, item_groups_distance_between);

		auto return_item = (*item_group).get(get_end_distance_direction(), get_direction_y_value(), (*goal_object).get_distance(), *data_group, item_index);
		//const auto item_distance = (*data_group).item_distance[item_index];
		if (item_groups_type::item_removal_result::item_removed_zero_remains == (*item_group).remove_item(distance_group.operator->(), *data_group, item_index))
			item_group_has_zero_count(item_groups.begin() + index_ptr, data_group);

		--item_groups_goal_item_count[(goal_object - item_groups_goal_distance.begin())];

		//update_distance_between_for_item_changes<belt_utility::item_update_state::removing>(item_index, item_distance, item_group, goal_object);
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
	inline constexpr item_uint remove_item(const long long item_group_index, const long long item_index) noexcept
	{
		return remove_item(&item_groups[item_group_index], item_index);
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

	inline constexpr index_inserter& get_inserter(std::size_t i) noexcept
	{
		std::size_t count_index = 0ull;
		for (auto begin_iter = inserters.begin(); begin_iter != inserters.last(); ++begin_iter)
		{
			if (count_index == i) return begin_iter->operator[](0);
			const auto inserter_size = begin_iter->size();
			if (count_index <= i && count_index + inserter_size >= i) return begin_iter->operator[](i - count_index);
			count_index += inserter_size + 1;
		}

		if (count_index == i) throw std::runtime_error("");

#ifdef _DEBUG
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

#ifdef _DEBUG
		throw std::runtime_error(""); //this shouldn't occur as long as the index is valid and not human entered
#endif
	};

	inline constexpr long long get_new_item_distance(std::size_t i) const noexcept
	{
		return item_groups_distance_between[i];
	};
	inline constexpr long long get_new_item_position(std::size_t i) const noexcept
	{
		const auto goal_object = belt_utility::find_which_goal_object_index_belongs_too(i, item_groups_goal_distance, item_groups_distance_between);
#ifdef _BOUNDS_CHECKING_
		if (item_groups_goal_distance.last() == goal_object) return -1ll;
#endif
		return get_end_distance_direction() - belt_utility::get_distances_from_to2(item_groups_distance_between.begin() + ((*goal_object).get_index_from_ptr(item_groups_distance_between.begin().operator->())), item_groups_distance_between.begin() + i) + 32;
		//return get_end_distance_direction() - item_groups_distance_between[i] + 32;
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
		//return item_groups[tc::unsign(i)].get_goal();
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
		//return item_groups[tc::unsign(i)].get_goal();
	};
	inline constexpr long long goal_distance_in_destinations(long long i) noexcept
	{
#ifdef _BOUNDS_CHECKING_
		ASSERT_NOT_CONSTEXPR(i < item_groups_goal_distance.size());
#endif
		return *item_groups_goal_distance[i].get_index_ptr();
		//return item_groups[tc::unsign(i)].get_goal();
	};
	/*inline constexpr bool has_goal_distance_slot(long long i) const noexcept
	{
#ifdef _BOUNDS_CHECKING_
		ASSERT_NOT_CONSTEXPR(i < inserters.size());

		if (i >= inserters.size()) return false;
#endif
		return inserters[i].get_linked_list_distance().get_index() == i;
		//return item_groups[tc::unsign(i)].get_goal();
	};*/

	inline constexpr long long count_inserters() const noexcept
	{
		std::size_t count_index = 0ull;
		for (auto begin_iter = inserters.begin(); begin_iter != inserters.last(); ++begin_iter)
		{
			count_index += begin_iter->size();
		}
		return count_index;
	};

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
#ifdef _DEBUG
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

#ifdef _DEBUG
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
		const auto goal_dist_iter = belt_utility::find_which_goal_object_index_belongs_too(index_ptr, item_groups_goal_distance, item_groups_distance_between);
		const auto dist_iter = item_groups_distance_between.begin() + index_ptr;
		//auto goal_dist_iter = get_goal_distance_iterator(dist_iter);
#ifdef _DEBUG
		if (goal_dist_iter == item_groups_goal_distance.last()) throw std::runtime_error("");
#endif
		remove_iterators.push_back({ _vector::iterator{ ptr }, _data_vector::iterator{ data_ptr }, dist_iter, goal_dist_iter });
	};
	inline constexpr void item_group_has_zero_count(_vector::iterator ptr, _data_vector::iterator data_ptr) noexcept
	{
		const auto index_ptr = ptr - item_groups.begin();
		auto goal_dist_iter = belt_utility::find_which_goal_object_index_belongs_too(index_ptr, item_groups_goal_distance, item_groups_distance_between);
		auto dist_iter = item_groups_distance_between.begin() + index_ptr;
		//auto goal_dist_iter = get_goal_distance_iterator(dist_iter);
#ifdef _DEBUG
		if (goal_dist_iter == item_groups_goal_distance.last()) throw std::runtime_error("");
#endif
		remove_iterators.emplace_back(ptr, data_ptr, dist_iter, goal_dist_iter);
	};

	inline constexpr void item_group_has_reached_goal(_vector::iterator ptr, _data_vector::iterator item_data, _vector_goal_distance::iterator goal_distance) noexcept
	{
		if (segment_end_points.size() > 0ull)
		{
			const auto end_distance_direction = get_end_distance_direction();
			const auto direction_y_value = get_direction_y_value();
			for (long long i2 = 0; i2 < tc::sign(segment_end_points.size()); ++i2)
			{
#ifdef _BOUNDS_CHECKING_
				ASSERT_NOT_CONSTEXPR(i2 < segment_end_points.size());
#endif
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
			/*
			const auto erase_values = mem::erase_indices<item_groups_type, _vector>(item_groups, remove_iterators, item_groups_goal_distance, item_groups_distance_between);
			//update_old_goal_distance_pointers_item_groups_removed(old_goal_values);
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
			item_groups.erase(erase_values.item_groups_iter, item_groups.last());
			item_groups_data.erase(erase_values.item_groups_data_iter, item_groups_data.last());
			item_groups_distance_between.erase(erase_values.item_groups_dist_iter, item_groups_distance_between.last());
#ifdef _DEBUG
			if (old_size != item_groups.size() + remove_iterators.size()) throw std::runtime_error("");
			validate_goal_pointers(old_goal_values, get_current_goal_distance_pointers_item_groups_removed());
#endif
#else
			const auto before_size = item_groups.size();
			item_groups.erase(erase_values, item_groups.end());
			if (std::is_constant_evaluated() == false)
			{
				if (item_groups.size() != before_size - remove_indexes.size()) throw std::runtime_error("");
			}
#endif
			remove_iterators.clear();

			for (auto biter = inserters.begin(), enditer = inserters.end(); biter != enditer; ++biter)
			{
				biter->update_linked_list_group_data(get_end_distance_direction());
			}
			*/
		}
	};

	/*constexpr void inserters_offset_calculation() noexcept
	{
		std::size_t current_index = 0;
		vec2_uint position{ 0ll, 0ll };
		if (!inserters.empty()) position = inserters[0][0].get_position();
		for (const auto& ref : remove_iterators)
		{
#ifdef _BOUNDS_CHECKING_
			//ASSERT_NOT_CONSTEXPR(ref < item_groups.size());
#endif
			if (ref.item_groups_iter->get_direction_position(get_end_distance_direction(), belt_utility::get_item_group_distance_from_destination(item_groups_distance_between, item_groups_goal_distance, ref.item_groups_iter - item_groups.begin())) <= belt_utility::get_direction_position(segment_direction, position))
			{
				++increment_count[current_index];
				break;
			}
			else
			{
				if (current_index + 1 < inserters.size())
				{
					++current_index;
#ifdef _BOUNDS_CHECKING_
					ASSERT_NOT_CONSTEXPR(current_index < inserters.size());
#endif
					position = inserters[current_index].get_position();
				}
				else break;
			}
		}
	};*/

	/*__declspec(noinline) constexpr void update_inserters() noexcept
	{
		//const long long item_group_offset = 0;
		const long long li = inserters.size();
		const auto end_distance_direction = get_end_distance_direction();
		for (long long i = 0; i < li; ++i)
		{
#ifdef _BOUNDS_CHECKING_
			ASSERT_NOT_CONSTEXPR(i < inserters.size());
#endif
			const auto found_index = inserters[i].update(segment_direction, end_distance_direction);
			if (found_index != -1)
			{
				inserters[i].grab_item(remove_item(&item_groups[inserters[i].get_item_group().get_index()], found_index));
			}
		}
	};*/

	__declspec(noinline) constexpr void update_item() noexcept
	{
		auto begin_iter = item_groups_goal_distance.begin();
		auto begin_goal_count_iter = item_groups_goal_item_count.begin();
		auto last_iter = item_groups_goal_distance.last();
		const auto last_goal_destination = inserters.empty() ? last_iter : last_iter - 1ll;

		//auto begin_inserter_iter = inserters.begin();
		//auto next_inserter_iter = inserters.end();
		auto begin_inserter_goal_iter = inserter_slots.begin();
		auto next_inserter_goal_iter = inserter_slots.last();

		const auto end_distance = get_end_distance_direction();

		if (inserters.size() > 1) next_inserter_goal_iter = begin_inserter_goal_iter + 1;

		while (begin_iter < last_iter)
		{
			if (*begin_iter != nullptr)
			{
				const auto cur_dist = begin_iter->get_distance();

				if (cur_dist > 0ll)
				{
					begin_iter->subtract_goal_distance(1ll);
					if (std::is_constant_evaluated() == false) item_groups_type::items_moved_per_frame += (*begin_goal_count_iter);

					if (begin_inserter_goal_iter != inserter_slots.last())
					{
						const auto inserter_distance_position = end_distance - belt_utility::get_direction_position(segment_direction, (*(*begin_inserter_goal_iter)).get_position());

						if ((*(*begin_inserter_goal_iter)).loop_count >= 256) __debugbreak();
						if ((*begin_iter).get_distance() <= inserter_distance_position)
						{
							const auto index_ptr = begin_iter->get_index_from_ptr(item_groups_distance_between.begin().operator->());
							const auto last_item_position = (*begin_iter).get_distance() + item_groups[index_ptr].get_distance_to_last_item(item_groups_data[index_ptr]);
							if (last_item_position >= inserter_distance_position - item_groups_type::belt_item_size)
							{
								auto inserter_position = belt_utility::get_direction_position(segment_direction, (*(*begin_inserter_goal_iter)).get_position());
								const auto found_index = item_groups[index_ptr].get_first_item_of_type_before_position(segment_direction, end_distance, (*begin_iter).get_distance(), item_groups_data[index_ptr], (*(*begin_inserter_goal_iter)).get_item_type(0), inserter_position);
								auto item_distance_position = item_groups[index_ptr].get_item_direction_position(segment_direction, end_distance, (*begin_iter).get_distance(), item_groups_data[index_ptr], found_index);
								if (item_distance_position >= inserter_position && item_distance_position <= inserter_position + index_inserter::inserter_grid_size)
								{
									(*(*begin_inserter_goal_iter)).grab_item(remove_item(&item_groups[index_ptr], found_index));
									++(*(*begin_inserter_goal_iter)).local_grabbed_items;
									(*(*begin_inserter_goal_iter)).loop_count = 0;
								}
								else
									++(*(*begin_inserter_goal_iter)).missed_grabs;
							}
							else
								++(*(*begin_inserter_goal_iter)).missed_grabs;
						}
						else
							++(*(*begin_inserter_goal_iter)).missed_grabs;

						++(*(*begin_inserter_goal_iter)).loop_count;
					}

					/*if (begin_iter != last_goal_destination && next_inserter_goal_iter != inserter_slots.end())
					{
						if ((*(*next_inserter_goal_iter)).get_linked_list_distance() != (*(*begin_inserter_goal_iter)).get_linked_list_distance())
						{
							const auto inserter_distance_position = end_distance - belt_utility::get_direction_position(segment_direction, (*(*begin_inserter_goal_iter)).get_position());
							if ((*begin_iter).get_distance() < inserter_distance_position)
							{
								const auto index_ptr = begin_iter->get_index_from_ptr(item_groups_distance_between.begin().operator->());
								if (item_groups[index_ptr].count() > 0)
								{
									const auto last_item_position = (*begin_iter).get_distance() + item_groups[index_ptr].get_distance_to_last_item(item_groups_data[index_ptr]);
									if (last_item_position < inserter_distance_position - item_groups_type::belt_item_size)
									{
										auto old_goal_distance = item_groups_distance_between.begin() + ((*begin_iter).get_index_from_ptr(item_groups_distance_between.begin().operator->()));
										if (item_groups_distance_between.begin().operator->() != (*begin_iter).get_index_ptr()) (*begin_iter).update_pointer_and_values(item_groups_distance_between.begin().operator->());
										if (begin_iter + 1ll < last_iter)
										{
											auto next_goal = (begin_iter + 1ll);
											const auto next_goal_index_ptr = (*next_goal).get_index_from_ptr(item_groups_distance_between.begin().operator->());
											auto old_last_distance = belt_utility::get_distances_from_to2(item_groups_distance_between.begin() + next_goal_index_ptr, old_goal_distance + 1ll) + item_groups[next_goal_index_ptr].get_distance_to_last_item(item_groups_data[next_goal_index_ptr]);
#ifdef _DEBUG
											if (*old_goal_distance - old_last_distance < 0ll)
												throw std::runtime_error("");
#endif
											* old_goal_distance = (*old_goal_distance + item_groups[index_ptr].get_distance_to_last_item(item_groups_data[index_ptr])) - old_last_distance;
										}
										if (item_groups_distance_between.begin().operator->() == (*begin_iter).get_index_ptr())
											begin_iter->set_index_ptr(nullptr);

										(*(*begin_inserter_goal_iter)).update_linked_list_group_data(segment_direction, end_distance);
									}
								}
							}
						}
						++begin_inserter_goal_iter;
						next_inserter_goal_iter = begin_inserter_goal_iter + 1;
					}*/
				}
				else if (cur_dist != -1ll)
				{
					const auto index_ptr = begin_iter->get_index_from_ptr(item_groups_distance_between.begin().operator->());
					item_groups[index_ptr].items_stuck_update(item_groups_data[index_ptr]);
					/*auto index_ptr = *begin_iter - item_groups_distance_between.begin();
					auto last_index_ptr = begin_iter - item_groups_goal_distance.begin();
					auto end_index = item_groups_distance_between.size();
					if (last_index_ptr > 0ll)
					{
						end_index = *(begin_iter - 1ll) - item_groups_distance_between.begin();
					}

					for (; index_ptr >= end_index; --index_ptr)
					{

					}*/
				}
			}

			++begin_inserter_goal_iter;
			++begin_iter;
			++begin_goal_count_iter;
		}
	};

	constexpr void update() noexcept
	{
		//increment_count.clear();
		//same size as numbers of inserters
		//if (increment_count.capacity() <= inserters.size()) increment_count.resize(inserters.size());

		update_item();
		if (!item_groups_goal_distance.empty())
		{
			if ((item_groups_goal_distance.begin()->get_distance()) == 0ll)
				item_group_has_reached_goal(item_groups.begin(), item_groups_data.begin(), item_groups_goal_distance.begin());
		}

		//if (!inserters.empty() && !remove_iterators.empty()) inserters_offset_calculation();
		if (!remove_iterators.empty())
			item_groups_removal();

		//update_inserters();
	};
private:
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
		//const long long old_goal_distance = *old_goal_distance_ptr;
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
			//*start = ((old_goal_distance + *start) - new_distance);
			//old_goal_distance += *start;
			--start;
		}

		//if (start == end) *start = old_goal_distance - new_distance;
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
		} while (first_distance_between.operator->() != goal_distance);

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
					} while (first_distance.operator->() != goal_distance);
				}

				begin_goal = next_goal;
			}
		}

		return false;
	};

public:
	constexpr belt_utility::need_new_slot_result need_new_goal_distance_slot(long long* distance) noexcept
	{
		if (inserters.empty())
		{
			if (item_groups_goal_distance.size() == 1ll)
			{
				/*if (is_end_goal_destination(item_groups_goal_distance.begin()) && item_groups_distance_between.size() > 1ll)
				{
					if (insert_new_goal_before_distance(*distance)) return belt_utility::need_new_slot_result::object_is_between_slots;

					//shouldn't occur
					throw std::runtime_error("");
				}
				else*/
				{
					auto begin_iter = item_groups_goal_distance.begin();
					auto old_goal_distance = (*begin_iter).get_index_ptr();
					item_groups_goal_distance[0].set_index_ptr(distance);
					const auto recalc_start_iter = item_groups_distance_between.begin() + ((*begin_iter).get_index_from_ptr(item_groups_distance_between.begin().operator->()));
					const auto recalc_end_iter = item_groups_distance_between.begin();
					recalculate_distances_between_from_to(old_goal_distance, recalc_start_iter, recalc_end_iter);
					return belt_utility::need_new_slot_result::update_pointer_to_new_index;
				}
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
			const auto end_distance = get_end_distance_direction();
			auto begin_inserter_iter = inserters.begin();
			auto inserter_distance = end_distance - belt_utility::get_direction_position(segment_direction, begin_inserter_iter->operator[](0).get_position());
			if (item_groups_goal_distance.size() == 1ll && *distance > inserter_distance)
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
					//we should update the pointer to the new index
					auto before_last = item_groups_goal_distance.last() - 1ll;
					if (*distance < before_last->get_distance() && inserter_distance < *distance)
					{
						//auto old_goal_distance = (*before_last).get_index_ptr();
						before_last->set_index_ptr(distance);
						auto previous_iter = (item_groups_distance_between.begin() + ((*before_last).get_index_from_ptr(item_groups_distance_between.begin().operator->()))) - 1ll;
						*previous_iter = 32ll;
						/*auto recalc_start_iter = item_groups_distance_between.begin() + ((*begin_iter).get_index_ptr() - item_groups_distance_between.begin());
						auto recalc_end_iter = item_groups_distance_between.begin() + (((*(begin_iter - 1ll)).get_index_ptr() + 1ll) - item_groups_distance_between.begin());
						recalculate_distances_between_from_to(old_goal_distance, recalc_start_iter, recalc_end_iter);*/
						return belt_utility::need_new_slot_result::update_pointer_to_new_index;
					}
					//else
						//return belt_utility::need_new_slot_result::need_new_slot;
				}
			}
			else if (item_groups_goal_distance.size() > 2ll)
			{
				inserter_distance = end_distance - belt_utility::get_direction_position(segment_direction, (inserters.last() - 1ull)->operator[](0).get_position());
				auto before_last = item_groups_goal_distance.last() - 1ll;
				const auto before_before_last = before_last - 1ll;
				if (before_before_last->get_distance() > *distance && *distance < before_last->get_distance() && inserter_distance > *distance)
				{
					//auto old_goal_distance = (*before_last).get_index_ptr();
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
					/*auto recalc_start_iter = item_groups_distance_between.begin() + ((*before_last).get_index_ptr() - item_groups_distance_between.begin());
					auto recalc_end_iter = item_groups_distance_between.begin() + (((*(before_last - 1ll)).get_index_ptr() + 1ll) - item_groups_distance_between.begin());
					recalculate_distances_between_from_to(old_goal_distance, recalc_start_iter, recalc_end_iter);*/
					return belt_utility::need_new_slot_result::update_pointer_to_new_index;
				}
			}

			auto begin_iter = item_groups_goal_distance.begin();
			auto next_iter = begin_iter + 1ll;
			const auto end_iter = item_groups_goal_distance.last();
			for (; next_iter != end_iter; ++begin_iter, ++next_iter)
			{
				if (begin_inserter_iter != inserters.end())
					inserter_distance = end_distance - belt_utility::get_direction_position(segment_direction, begin_inserter_iter->operator[](0).get_position());
				if (begin_iter->get_distance() > *distance && *distance < next_iter->get_distance() && *distance > inserter_distance)
				{
					//if inserter distance is less than begin_iter update **begin_iter 
					if (inserter_distance < begin_iter->get_distance())
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
				//we should update the pointer to the new index
				/*if (next_iter == end_iter - 1ll)
				{
					*next_iter = distance;
					return belt_utility::need_new_slot_result::update_pointer_to_new_index;
				}*/

				if (begin_inserter_iter != inserters.last() && begin_inserter_iter + 1 != inserters.last() && next_iter->get_distance() < inserter_distance)
					++begin_inserter_iter;
			};

			if (begin_inserter_iter != inserters.end())
			{
				inserter_distance = end_distance - belt_utility::get_direction_position(segment_direction, begin_inserter_iter->operator[](0).get_position());
				//we should update the pointer to the new index
				auto before_last = item_groups_goal_distance.last() - 1ll;
				if (*distance < before_last->get_distance() && inserter_distance < *distance)
				{
					//auto old_goal_distance = (*before_last).get_index_ptr();
					before_last->set_index_ptr(distance);
					auto previous_iter = (item_groups_distance_between.begin() + ((*before_last).get_index_from_ptr(item_groups_distance_between.begin().operator->()))) - 1ll;
					*previous_iter = 32ll;
					/*auto recalc_start_iter = item_groups_distance_between.begin() + ((*begin_iter).get_index_ptr() - item_groups_distance_between.begin());
					auto recalc_end_iter = item_groups_distance_between.begin() + (((*(begin_iter - 1ll)).get_index_ptr() + 1ll) - item_groups_distance_between.begin());
					recalculate_distances_between_from_to(old_goal_distance, recalc_start_iter, recalc_end_iter);*/
					return belt_utility::need_new_slot_result::update_pointer_to_new_index;
				}
			}
			else
			{
				inserter_distance = end_distance - belt_utility::get_direction_position(segment_direction, (inserters.last() - 1ull)->operator[](0).get_position());
				auto before_last = item_groups_goal_distance.last() - 1ll;
				const auto before_before_last = before_last - 1ll;
				if (before_before_last->get_distance() > *distance && *distance < before_last->get_distance() && inserter_distance > *distance)
				{
					//auto old_goal_distance = (*before_last).get_index_ptr();
					before_last->set_index_ptr(distance);
					auto previous_iter = (item_groups_distance_between.begin() + ((*before_last).get_index_from_ptr(item_groups_distance_between.begin().operator->()))) - 1ll;
					*previous_iter = 32ll;
					//auto recalc_start_iter = item_groups_distance_between.begin() + ((*before_last).get_index_ptr() - item_groups_distance_between.begin());
					//auto recalc_end_iter = item_groups_distance_between.begin() + (((*(before_last - 1ll)).get_index_ptr() + 1ll) - item_groups_distance_between.begin());
					//recalculate_distances_between_from_to(old_goal_distance, recalc_start_iter, recalc_end_iter);
					return belt_utility::need_new_slot_result::update_pointer_to_new_index;
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
	constexpr bool add_item_before(const item_uint& new_item, auto iter, long long index_ptr_temp) noexcept
	{
		auto& new_data_group = item_groups_data.emplace_back();
		const auto new_iter_group = item_groups.emplace(iter.result);

		std::vector<long long> old_distances;
		if (item_groups_distance_between.needs_resize()) old_distances = get_current_goal_distance_values();
		auto new_distance_value = get_end_distance_direction() - belt_utility::get_direction_position(segment_direction, new_item.position);
		const auto goal_object = belt_utility::find_which_goal_object_index_belongs_too(index_ptr_temp, item_groups_goal_distance, item_groups_distance_between);
		if (goal_object != item_groups_goal_distance.last())
		{
			new_distance_value = belt_utility::get_direction_position(segment_direction, new_item.position) - calculate_distance_(item_groups_distance_between.begin() + index_ptr_temp, goal_object);
		}
		const auto new_distance_iter = item_groups_distance_between.emplace(item_groups_distance_between.begin() + index_ptr_temp, new_distance_value);
		if (!old_distances.empty()) update_old_goal_distance_pointers(old_distances);

#ifdef __BELT_SEGMENT_VECTOR_TYPE__
		if (new_iter_group != item_groups.last()) new_iter_group->add_item(get_end_distance_direction(), new_distance_iter.operator->(), new_data_group, new_item, new_item.position);
#else
		if (new_iter_group != item_groups.end()) new_iter_group->add_item(new_item, new_item.position);
#endif
		//const auto goal_object_index = goal_object - item_groups_goal_distance.begin();
		//if (inserters.size() > goal_object_index && goal_object != item_groups_goal_distance.last()) inserters[goal_object_index].update_linked_list_group_data(segment_direction, get_end_distance_direction());
		return true;
	};

	constexpr bool add_item_after_last(const item_uint& new_item, long long index_ptr_temp) noexcept
	{
		auto& new_data_group = item_groups_data.emplace_back();
		auto& new_iter_group = item_groups.emplace_back();

		std::vector<long long> old_distances;
		if (item_groups_distance_between.needs_resize()) old_distances = get_current_goal_distance_values();
		auto item_distance_position = belt_utility::get_direction_position(segment_direction, new_item.position);
		auto new_distance_value = get_end_distance_direction() - item_distance_position;
		//if (goal_object != item_groups_goal_distance.last()) new_distance_value = expr::abs(goal_object->get_distance() - new_distance_value);
		auto& new_distance = item_groups_distance_between.emplace_back(new_distance_value);
		if (!old_distances.empty()) update_old_goal_distance_pointers(old_distances);

		const auto new_slot_result = need_new_goal_distance_slot(&new_distance);
		if (belt_utility::need_new_slot_result::need_new_slot == new_slot_result)
		{
			item_groups_goal_distance.emplace_back(&new_distance);
			item_groups_goal_item_count.emplace_back(1ull);
		}
#ifdef _DEBUG
		if (!inserters.empty() && item_groups_goal_distance.size() > inserters.size() + 1) throw std::runtime_error("");
#endif
		new_iter_group.add_item(get_end_distance_direction(), &new_distance, new_data_group, new_item, new_item.position);
		const auto goal_object = belt_utility::find_which_goal_object_index_belongs_too(index_ptr_temp, item_groups_goal_distance, item_groups_distance_between);
		const auto goal_object_index = goal_object - item_groups_goal_distance.begin();
		if (belt_utility::need_new_slot_result::need_new_slot != new_slot_result) ++item_groups_goal_item_count[goal_object_index];
		//if (inserters.size() > goal_object_index && goal_object != item_groups_goal_distance.last()) inserters[goal_object_index].update_linked_list_group_data(segment_direction, get_end_distance_direction());
		return true;
	};

	constexpr bool add_item_after(const item_uint& new_item, auto iter, long long index_ptr_temp) noexcept
	{
		auto new_data_group = item_groups_data.emplace(item_groups_data.begin() + (iter.result - item_groups.begin()));
		const auto new_iter_group = item_groups.emplace(iter.result);

		std::vector<long long> old_distances;
		if (item_groups_distance_between.needs_resize()) old_distances = get_current_goal_distance_values();
		auto new_distance_value = get_end_distance_direction() - belt_utility::get_direction_position(segment_direction, new_item.position);
		const auto goal_object = belt_utility::find_which_goal_object_index_belongs_too(index_ptr_temp, item_groups_goal_distance, item_groups_distance_between);
		if (goal_object != item_groups_goal_distance.last() && item_groups_goal_distance.size() > 1ll)
		{
			new_distance_value = belt_utility::get_direction_position(segment_direction, new_item.position) - calculate_distance_(item_groups_distance_between.begin() + (index_ptr_temp + 1), goal_object);
		}
		const auto new_distance_iter = item_groups_distance_between.emplace(item_groups_distance_between.begin() + (index_ptr_temp + 1), new_distance_value);
		if (!old_distances.empty()) update_old_goal_distance_pointers(old_distances);

		const auto new_slot_result = need_new_goal_distance_slot(new_distance_iter.operator->());
		if (belt_utility::need_new_slot_result::need_new_slot == new_slot_result)
		{
			item_groups_goal_distance.emplace_back(new_distance_iter.operator->());
			item_groups_goal_item_count.emplace_back(1ull);
		}

#ifdef _DEBUG
		if (!inserters.empty() && item_groups_goal_distance.size() > inserters.size() + 1) throw std::runtime_error("");
#endif
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
		if (new_iter_group != item_groups.last()) new_iter_group->add_item(get_end_distance_direction(), new_distance_iter.operator->(), *new_data_group, new_item, new_item.position);
#else
		if (new_iter_group != item_groups.end()) new_iter_group->add_item(new_item, new_item.position);
#endif
		//const auto goal_object_index = goal_object - item_groups_goal_distance.begin();
		//if (inserters.size() > goal_object_index && goal_object != item_groups_goal_distance.last()) inserters[goal_object_index].update_linked_list_group_data(segment_direction, get_end_distance_direction());
		return true;
	};

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

public:
	constexpr bool add_item(const item_uint& new_item) noexcept
	{
		if (item_groups.size() == 0)
		{
			auto& new_data_group = item_groups_data.emplace_back();
			auto& new_item_group = item_groups.emplace_back();
			auto& new_goal_distance = item_groups_distance_between.emplace_back(get_end_distance_direction() - belt_utility::get_direction_position(segment_direction, new_item.position)); //get_end_distance_direction() - belt_utility::get_direction_position(segment_direction, new_item.position)
			item_groups_goal_distance.emplace_back(&new_goal_distance);
			item_groups_goal_item_count.emplace_back(1ull);
#ifdef _BOUNDS_CHECKING_
			ASSERT_NOT_CONSTEXPR(0ll < item_groups.size());
#endif
			const short added_index = new_item_group.add_item(get_end_distance_direction(), &item_groups_distance_between[0], new_data_group, new_item, new_item.position);
			//if (!inserters.empty()) inserters[0].update_linked_list_group_data(segment_direction, get_end_distance_direction());

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
			auto item_distance = belt_utility::get_direction_position(segment_direction, new_item.position);
			const auto new_slot_result = need_new_goal_distance_slot(&item_distance);

			auto iter = belt_utility::find_closest_item_group(segment_direction, get_end_distance_direction(), item_groups_data, item_groups, item_groups_distance_between, belt_utility::get_direction_position(segment_direction, new_item.position));
			const auto index_ptr_temp = iter.result - item_groups.begin();
#ifdef _BOUNDS_CHECKING_
			ASSERT_NOT_CONSTEXPR(iter.result != item_groups.end());
#endif
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
			if (belt_utility::find_closest_item_group_return_result::invalid_value == iter.scan) return false;
			//if (iter.result == item_groups.last()) return false; //don't think last can be returned unless iter.scan == invalid_value
			if ((iter.result != item_groups.last() && !iter.result->can_add_item(get_end_distance_direction(), &item_groups_distance_between[index_ptr_temp], item_groups_data[index_ptr_temp], new_item.position))) return false;
#else
			if (belt_utility::find_closest_item_group_return_result::invalid_value == iter.scan || iter.result == item_groups.end() || (iter.result != item_groups.end() && !iter.result->can_add_item(new_item.position))) return false;
#endif
			if (belt_utility::find_closest_item_group_return_result::insert_into_group == iter.scan)
			{
				if (iter.result->count() >= 32) return split_item_group(new_item, iter, index_ptr_temp);
				else
				{
					const short added_index = iter.result->add_item(get_end_distance_direction(), &item_groups_distance_between[index_ptr_temp], item_groups_data[index_ptr_temp], new_item, new_item.position);
					if (added_index != -1)
					{
						auto goal_object = belt_utility::find_which_goal_object_index_belongs_too(index_ptr_temp, item_groups_goal_distance, item_groups_distance_between);
						++item_groups_goal_item_count[(goal_object - item_groups_goal_distance.begin())];
						if (goal_object->get_index_ptr() == &item_groups_distance_between[index_ptr_temp])
						{
							long long* previous_goal_ptr_index = nullptr;
							if (goal_object != item_groups_goal_distance.begin()) previous_goal_ptr_index = (*(goal_object - 1ll)).get_unsafe_index_ptr();
							update_distance_between_for_item_changes<belt_utility::item_update_state::adding>(added_index, previous_goal_ptr_index, item_groups_data[index_ptr_temp].item_distance[added_index], &item_groups[index_ptr_temp], goal_object);
						}

						return true;
					}
				}
				return false;
			}
			else if (belt_utility::find_closest_item_group_return_result::new_group_before_iter == iter.scan) //TODO does this add towards the start of end of the belt segment?
			{
				return add_item_before(new_item, iter, index_ptr_temp);
			}
			else if (belt_utility::find_closest_item_group_return_result::new_group_after_iter == iter.scan) //TODO does this add towards the start of end of the belt segment?
			{
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
				if (iter.result + 1 == item_groups.last())
#else
				if (iter.result + 1 == item_groups.end())
#endif
				{
					return add_item_after_last(new_item, index_ptr_temp);
				}
				else
				{
					return add_item_after(new_item, iter, index_ptr_temp);
				}
		}
	}

		return false;
};

	/*constexpr index_iterator<goal_distance, _vector_goal_distance> get_inserter_destination_slot(const index_inserter& inserter, belt_utility::inserter_fits_results insert_results, long long insert_index = -1ll) noexcept
	{
		//TODO make sure that we don't add any item_groups_distance_between values when we don't need any and just return a index based on inserters instead
		//aka don't do this since we add dummy units we later need to take care of, it's better if they index_iterator just has an index and performs a size
		//check on every tick to see if they actually can read anything and just update the values as we go for the item_groups
		//since inserters don't actually need valid values to exists in the vectors, they just need a correct index that they can read from
		//and said index is something they will never modify themselves.
		//
		//	auto& new_dist = item_groups_distance_between.emplace_back(0ll);
		//	auto new_index = item_groups_goal_distance.usize();
		//	item_groups_goal_distance.emplace_back(&new_dist);

		//if we have zero item_groups we emplace a 0ll in item_groups_distance_between
		if (item_groups.empty() && inserters.empty()) return { 0ull, &item_groups_goal_distance };

		auto old_insert_index = insert_index;
		//if we don't have any inserters, but have goal distances we need to check if the current goal distance is before or after the inserter
		if (inserters.empty() && !item_groups_goal_distance.empty())
		{
			auto best_goal_match = item_groups_goal_distance.begin();
			auto begin_iter = item_groups_goal_distance.begin();
			const auto last_iter = item_groups_goal_distance.last();
			for (; begin_iter != last_iter; ++begin_iter)
			{
				if (begin_iter->get_distance() > belt_utility::get_direction_position(segment_direction, inserter.get_position())) break;
				best_goal_match = begin_iter;
			}
			if (best_goal_match != last_iter)
			{
				//(**best_goal_match) = (**best_goal_match) - (get_end_distance_direction() - belt_utility::get_direction_position(segment_direction, inserter.get_position()));
				const auto best_match_index = best_goal_match - item_groups_goal_distance.begin();
				return { static_cast<std::size_t>(best_match_index), &item_groups_goal_distance };
			}
		}

		if (item_groups_distance_between.size() < 2)
			insert_index = 0ll;

		long long ptr_index = 0ll;
		if (!inserters.empty() && !item_groups_goal_distance.empty())
		{
			auto begin_iter = item_groups_goal_distance.begin();
			const auto end_iter = item_groups_goal_distance.last();
			const auto inserter_direction_position = belt_utility::get_direction_position(segment_direction, inserter.get_position());
			for (; begin_iter != end_iter; ++begin_iter)
			{
				if ((begin_iter->get_distance()) > inserter_direction_position) break;
				ptr_index = begin_iter - item_groups_goal_distance.begin();
			}

			const auto best_goal_match = item_groups_goal_distance.begin() + ptr_index;
			if (best_goal_match != end_iter)
			{
				auto best_match_index = best_goal_match - item_groups_goal_distance.begin();

				auto begin_inserter_iter = inserters.begin() + old_insert_index;
				const auto end_inserter_iter = inserters.end();
				for (; begin_inserter_iter < end_inserter_iter; ++begin_inserter_iter)
				{
					if (belt_utility::inserter_fits_results::after != insert_results) ++best_match_index;
					begin_inserter_iter->set_linked_list_distance({ static_cast<std::size_t>(best_match_index), &item_groups_goal_distance });
					if (belt_utility::inserter_fits_results::after == insert_results) ++best_match_index;
				}
				return { static_cast<std::size_t>(insert_index), &item_groups_goal_distance };
			}
		}

		//if we have zero item_groups we need to insert a new item_groups_distance_between value using insert_index and switch the links according to insert_results
		if (belt_utility::inserter_fits_results::inbetween == insert_results)
		{
			auto begin_iter = inserters.begin() + old_insert_index;
			const auto end_iter = inserters.end();
			for (; begin_iter < end_iter; ++begin_iter)
			{
				++insert_index;
				begin_iter->set_linked_list_distance({ static_cast<std::size_t>(insert_index), &item_groups_goal_distance });
			}
			return { static_cast<std::size_t>(old_insert_index), &item_groups_goal_distance };
		}
		if (belt_utility::inserter_fits_results::before == insert_results)
		{
			old_insert_index = old_insert_index - 1 < 0 ? old_insert_index : old_insert_index - 1;
			auto begin_iter = inserters.begin() + old_insert_index;
			const auto end_iter = inserters.end();
			for (; begin_iter < end_iter; ++begin_iter)
			{
				++insert_index;
				begin_iter->set_linked_list_distance({ static_cast<std::size_t>(insert_index), &item_groups_goal_distance });
			}
			return { static_cast<std::size_t>(old_insert_index), &item_groups_goal_distance };
		}
		if (belt_utility::inserter_fits_results::after == insert_results)
		{
			//TODO place begin_iter and end_iter back into the loop body, compiler regression I reported https://developercommunity.visualstudio.com/t/std::vector::iterator-comparison-operato/10605140
			auto begin_iter = inserters.begin() + old_insert_index;
			const auto end_iter = inserters.end();
			for (; begin_iter < end_iter; ++begin_iter)
			{
				begin_iter->set_linked_list_distance({ static_cast<std::size_t>(insert_index), &item_groups_goal_distance });
				++insert_index;
			}
			return { static_cast<std::size_t>(insert_index), &item_groups_goal_distance };
		}

		//should never happen so we throw
#ifdef _DEBUG
		throw std::runtime_error("");
#endif
		return { 0ull, &item_groups_goal_distance };
	};*/

	constexpr std::size_t get_inserter_group_index(auto& group_inserters, index_inserter object) noexcept
	{
		const long long l = group_inserters.size();

		if (l == 1ll)
		{
			if (belt_utility::inserter_fits_results::before == check_if_inserter_is_before_or_after(segment_direction, group_inserters[0], object)) return 0;
			else return 1;
		}

		for (long long i = 0, i2 = 1; i2 < l; ++i, ++i2)
		{
			const auto test_result = check_if_inserter_is_before_or_after(segment_direction, group_inserters[i], object);
			if (belt_utility::inserter_fits_results::before == test_result) return i;
			else if (belt_utility::inserter_fits_results::after == test_result)
			{
				const auto second_test_result = check_if_inserter_is_before_or_after(segment_direction, group_inserters[i2], object);
				if (belt_utility::inserter_fits_results::before == second_test_result) return i;
			}
		}

		if (belt_utility::inserter_fits_results::before == check_if_inserter_is_before_or_after(segment_direction, group_inserters.back(), object)) return group_inserters.size() - 1ll;
		else return group_inserters.size();
	};

	__declspec(noinline) constexpr std::size_t add_inserter(index_inserter object) noexcept
	{
		constexpr auto max_inserter_distance = item_groups_type::belt_item_size * item_groups_type::max_item_count * 4;

		if (inserters.size() == 0)
		{
			//auto new_distance_value = get_end_distance_direction() - belt_utility::get_direction_position(segment_direction, object.get_position());
			//auto new_slot_result = need_new_goal_distance_slot(&new_distance_value);

			//object.set_linked_list_data(index_iterator<item_groups_type, _vector>{ 0ull, & item_groups }, index_iterator<item_groups_data_type, _data_vector>{ 0ull, & item_groups_data }, get_inserter_destination_slot(object, belt_utility::inserter_fits_results::no_fit), & item_groups_distance_between);
			inserters.emplace_back();
			inserters[0].push_back(object);
			inserter_slots.emplace_back(0ull, 0ull, &inserters);
			return 0;
		}

		long long can_fit_index = -1;
		long long can_fit_nested_index = -1;
		belt_utility::inserter_fits_results insert_results = belt_utility::inserter_fits_results::no_fit;
		if (inserters.size() == 1ll && inserters[0].size() == 1ll)
		{
			if (expr::abs(get_direction_position(segment_direction, inserters[0ll][0ll].get_position()) - get_direction_position(segment_direction, object.get_position())) >= max_inserter_distance &&
				expr::abs(get_direction_position(segment_direction, inserters[0ll].back().get_position()) - get_direction_position(segment_direction, object.get_position())) >= max_inserter_distance)
			{
				insert_results = belt_utility::inserter_fits_results::no_fit;
				can_fit_index = inserters.size();
			}
			else
			{
				can_fit_index = 0;
				can_fit_nested_index = get_inserter_group_index(inserters[0ll], object);
			}
		}
		else
		{
			if (expr::abs(get_direction_position(segment_direction, inserters[0][0].get_position()) - get_direction_position(segment_direction, object.get_position())) >= max_inserter_distance &&
				expr::abs(get_direction_position(segment_direction, inserters[0].back().get_position()) - get_direction_position(segment_direction, object.get_position())) >= max_inserter_distance)
			{
				insert_results = belt_utility::inserter_fits_results::no_fit;
				can_fit_index = inserters.size();
			}
			else if (expr::abs(get_direction_position(segment_direction, inserters.back()[0].get_position()) - get_direction_position(segment_direction, object.get_position())) >= max_inserter_distance &&
				expr::abs(get_direction_position(segment_direction, inserters.back().back().get_position()) - get_direction_position(segment_direction, object.get_position())) >= max_inserter_distance)
			{
				insert_results = belt_utility::inserter_fits_results::no_fit;
				can_fit_index = inserters.size();
			}
			//first check if it's before the first inserter
			else if (belt_utility::inserter_fits_results::before == check_if_inserter_is_before_or_after(segment_direction, inserters[0][0], object))
			{
				insert_results = belt_utility::inserter_fits_results::before;
				can_fit_index = 0;
				can_fit_nested_index = get_inserter_group_index(inserters[0ll], object);
			}
			//second check if it's after the last inserter
			else if (belt_utility::inserter_fits_results::after == check_if_inserter_is_before_or_after(segment_direction, inserters.back()[0], object))
				//else if (belt_utility::inserter_fits_results::after == check_if_inserter_is_before_or_after(segment_direction, *inserters.last(), object))
			{
				insert_results = belt_utility::inserter_fits_results::after;
				can_fit_index = inserters.size() - 1;
				can_fit_nested_index = get_inserter_group_index(inserters.back(), object);
			}
			else
			{
				//check if we can place it in between inserters
				const long long l = inserters.size();
				for (long long i = 0, i2 = 1; i2 < l; ++i, ++i2)
				{
					if (expr::abs(get_direction_position(segment_direction, inserters[i][0].get_position()) - get_direction_position(segment_direction, object.get_position())) >= max_inserter_distance &&
						expr::abs(get_direction_position(segment_direction, inserters[i].back().get_position()) - get_direction_position(segment_direction, object.get_position())) >= max_inserter_distance)
					{
						insert_results = belt_utility::inserter_fits_results::no_fit;
						can_fit_index = l;
						break;
					}
					else
					{
						if (belt_utility::inserter_fits_results::inbetween == check_if_inserter_is_between(segment_direction, inserters[i][0], inserters[i2][0], object))
						{
							insert_results = belt_utility::inserter_fits_results::inbetween;
							can_fit_index = i2;
							can_fit_nested_index = get_inserter_group_index(inserters[i2], object);
						}
					}
				}
			}
		}

		if (can_fit_index != -1)
		{
			//object.set_linked_list_data(index_iterator<item_groups_type, _vector>{ 0ull, & item_groups }, index_iterator<item_groups_data_type, _data_vector>{ 0ull, & item_groups_data }, get_inserter_destination_slot(object, insert_results, can_fit_index), & item_groups_distance_between);
			if (belt_utility::inserter_fits_results::no_fit == insert_results && can_fit_index == inserters.size())
			{
				auto& group = inserters.emplace_back();
				group.push_back(object);
				inserter_slots.emplace_back(static_cast<std::size_t>(can_fit_index), 0ull, &inserters);
			}
			else
			{
				inserters[can_fit_index].insert(inserters[can_fit_index].begin() + can_fit_nested_index, object);
			}
			return can_fit_index;
		}

		return -1;
	};

	inline constexpr void add_end_segment_section(belt_segment* ptr) noexcept
	{
		segment_end_points.push_back(ptr);
	};
};