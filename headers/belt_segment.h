#pragma once

#include <type_traits>
#include <stdexcept>
#include <vector>

#include "type_conversion.h"
#include "math_utility.h"
#include "mem_utilities.h"

#include "mem_vector.h"

#include "const_data.h"
#include "belt_utility_data.h"
#include "belt_utility.h"
#include "vectors.h"
#include "index_iterator.h"
#include "item.h"
#include "index_inserter.h"

#define CONSTEXPR_ASSERTS
#ifdef CONSTEXPR_ASSERTS
#define CONSTEXPR_VAR constexpr
#else
#define CONSTEXPR_VAR static
#endif

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
	belt_utility::belt_direction segment_direction{ belt_utility::belt_direction::left_right };
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
	_vector item_groups{ 4 };
#else
	_vector item_groups;
#endif
	std::vector<index_inserter> inserters;
	std::vector<index_inserter> sleeping_inserters;
	// belt_segments that this segment moves items onto
	std::vector<belt_segment*> segment_end_points;
	// belt_segments that will move items onto this segment
	std::vector<belt_segment*> connected_segments;

	std::vector<long long> increment_count;
#ifdef __BELT_SEGMENT_VECTOR_ITERATORS__
	std::vector<_vector::iterator> remove_iterators;
	bool item_was_removed = false;
#endif

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

	inline constexpr item_groups_type& get_item(std::size_t i) noexcept
	{
#ifdef _BOUNDS_CHECKING_
		ASSERT_NOT_CONSTEXPR(i < item_groups.size())
#endif
			return item_groups[i];
	};
	inline constexpr const item_groups_type& get_item(std::size_t i) const noexcept
	{
#ifdef _BOUNDS_CHECKING_
		ASSERT_NOT_CONSTEXPR(i < item_groups.size());
#endif
		return item_groups[i];
	};
	inline constexpr index_inserter& get_inserter(std::size_t i) noexcept
	{
#ifdef _BOUNDS_CHECKING_
		ASSERT_NOT_CONSTEXPR(i < inserters.size());
#endif
		return inserters[i];
	};
	inline constexpr const index_inserter& get_inserter(std::size_t i) const noexcept
	{
#ifdef _BOUNDS_CHECKING_
		ASSERT_NOT_CONSTEXPR(i < inserters.size());
#endif
		return inserters[i];
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
		if (std::is_constant_evaluated() == false && item_groups.size() > 256ll)
		{
			const long long initial_count = item_groups.size();
			const auto loops = expr::divide_with_remainder(initial_count, 4ll);
			long long* begin_iter = (long long*)(item_groups.begin()).operator->();
			begin_iter += 4ll;
			constexpr long long iter_offset = mem::ptr_type_offset<item_groups_type, long long, 4ll>();
			long long tmp1{ 0ll }, tmp2{ 0ll }, tmp3{ 0ll }, tmp4{ 0ll };
			for (long long i = loops.div; i > 0ll; --i)
			{
				mem::pre_fetch_cacheline_ptrs<item_groups_type, 4ll, _MM_HINT_NTA, long long>(begin_iter + iter_offset);
				//mem::prefetch_offset<4ll - 1ll, 4ll - 1ll, 192ll, item_groups_type, _MM_HINT_NTA>{}.___mm_prefetch___(((item_groups_type*)(begin_iter + iter_offset)));
				//_mm_prefetch((const char*)(begin_iter + iter_offset), _MM_HINT_NTA);
				//_mm_prefetch((const char*)(begin_iter + iter_offset + 24), _MM_HINT_NTA);
				//_mm_prefetch((const char*)(begin_iter + iter_offset + 48), _MM_HINT_NTA);
				//_mm_prefetch((const char*)(begin_iter + iter_offset + 72), _MM_HINT_NTA);
				tmp1 = tmp1 + *begin_iter;
				tmp2 = tmp2 + *(begin_iter + 24);
				tmp3 = tmp3 + *(begin_iter + 48);
				tmp4 = tmp4 + *(begin_iter + 72);
				begin_iter += 96;
			}
			total = tmp1 + tmp2 + tmp3 + tmp4;
			for (long long i = loops.div * 4ll, l = i + loops.rem; i < l; ++i)
			{
				total += item_groups[i].count();
			}
			return total;
		}
		else
		{
			const long long l = item_groups.size();
			for (long long i = 0; i < l; ++i)
			{
#ifdef _BOUNDS_CHECKING_
				ASSERT_NOT_CONSTEXPR(i < item_groups.size());
#endif
				total += item_groups[i].count();
			}
		}
		return total;
	};
	inline constexpr long long goal_distance_in_group(long long i) const noexcept
	{
#ifdef _BOUNDS_CHECKING_
		ASSERT_NOT_CONSTEXPR(i < item_groups.size());
#endif
		return item_groups[tc::unsign(i)].get_goal();
	};
	inline constexpr long long count_inserters() const noexcept
	{
		return tc::narrow<long long>(inserters.size());
	};

	inline constexpr long long get_start_direction_value() const noexcept
	{
		switch (segment_direction)
		{
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
	inline constexpr void move_inserter_to_sleeping(long long index) noexcept
	{
#ifdef _BOUNDS_CHECKING_
		ASSERT_NOT_CONSTEXPR(index < inserters.size());
#endif
		inserters[tc::unsign(index)].reset_sleep_timer();
		sleeping_inserters.push_back(std::move(inserters[tc::unsign(index)]));
		inserters.erase(inserters.begin() + index);
	};

	inline constexpr void item_group_has_zero_count(item_groups_type* ptr) noexcept
	{
		remove_iterators.emplace_back(_vector::iterator{ ptr });
	};
	inline constexpr void item_group_has_reached_goal(item_groups_type* ptr) noexcept
	{
		if (segment_end_points.size() > 0ull)
		{
			for (long long i2 = 0; i2 < tc::sign(segment_end_points.size()); ++i2)
			{
#ifdef _BOUNDS_CHECKING_
				ASSERT_NOT_CONSTEXPR(i2 < segment_end_points.size());
#endif
				auto segment_ptr = segment_end_points[tc::unsign(i2)];
				if (segment_ptr->start_of_segment != ptr->get_position()) continue;

				const auto tmp = ptr->get(0);
				if (segment_ptr->add_item(tmp))
				{
					ptr->remove_item(0);
					item_was_removed = true;
					break;
				}
			}
		}
		else ptr->set_active_mode(belt_utility::belt_update_mode::first_stuck);
	};

	constexpr void update() noexcept
	{
		increment_count.clear();
		//same size as numbers of inserters
		if (increment_count.capacity() <= inserters.size()) increment_count.resize(inserters.size());

#ifdef __BELT_SEGMENT_VECTOR_ITERATORS__
		auto begin_iter = item_groups.begin();
		/*if (std::is_constant_evaluated() == false)
		{
			if (!mem::is_aligned<item_groups_type, mem::get_closest_alignmnet<item_groups_type>()>(begin_iter.operator->())) throw std::runtime_error("");
		}*/
		auto last_iter = item_groups.last();
		bool local_item_was_removed = item_was_removed;
		item_was_removed = false;
		if (local_item_was_removed)
		{
			while (begin_iter != last_iter)
			{
				begin_iter->set_active_mode(belt_utility::belt_update_mode::free);
				begin_iter->update_belt();
				++begin_iter;
			}
		}
		else
		{
			if (item_groups.size() > 256ll)
			{
				const long long initial_count = item_groups.size();
				const auto loops = expr::divide_with_remainder(initial_count, 4ll);
				//if (std::is_constant_evaluated() == false) mem::pre_fetch_cacheline_ptrs<item_groups_type, 4ll>(&item_groups[0] + 4ll);
				for (long long i = loops.div, i_loop = 0ll; i > 0ll; --i)
				{
					mem::pre_fetch_cacheline_ptrs<item_groups_type, 4ll>(&item_groups[i_loop] + 4ll);
					//mem::pre_fetch_cacheline_ptrs<item_groups_type, 1ll>(&item_groups[i_loop] + 1ll);
					item_groups[i_loop].update_belt();
					//mem::pre_fetch_cacheline_ptrs<item_groups_type, 1ll>(&item_groups[i_loop] + 2ll);
					item_groups[i_loop + 1ll].update_belt();
					//mem::pre_fetch_cacheline_ptrs<item_groups_type, 1ll>(&item_groups[i_loop] + 3ll);
					item_groups[i_loop + 2ll].update_belt();
					//mem::pre_fetch_cacheline_ptrs<item_groups_type, 1ll>(&item_groups[i_loop] + 4ll);
					item_groups[i_loop + 3ll].update_belt();
					/*if (std::is_constant_evaluated() == false)
					{
						if (std::is_constant_evaluated() == false) mem::pre_fetch_cacheline_ptrs<item_groups_type, 4ll>(&item_groups[i_loop] + 8ll);
						mem::pre_fetch_cacheline_ptrs<item_groups_type, 4ll>(&item_groups[i_loop] + 4ll);
					}*/
					i_loop += 4ll;
				}
				for (long long i = loops.div * 4ll, l = i + loops.rem; i < l; ++i)
				{
					item_groups[i].update_belt();
				}
			}
			else
			{
				while (begin_iter != last_iter)
				{
					begin_iter->update_belt();
					++begin_iter;
				}
				/*auto size = item_groups.size();
				auto item_ptr = item_groups.values.first;
				for (; item_ptr != item_groups.values.last; ++item_ptr)
				{
					(*item_ptr).update_belt();
					//++begin_iter;
				}*/
			}
		}

		if (!inserters.empty() && !remove_iterators.empty())
		{
			std::size_t current_index = 0;
			vec2_uint position{ 0ll, 0ll };
			if (!inserters.empty()) position = inserters[0].get_position();
			for (const auto& ref : remove_iterators)
			{
#ifdef _BOUNDS_CHECKING_
				ASSERT_NOT_CONSTEXPR(ref < item_groups.size());
#endif
				if (ref->get_direction_position() <= position.x)
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
		}
		if (!remove_iterators.empty())
		{
			const auto erase_values = mem::erase_indices<item_groups_type>(item_groups, remove_iterators);
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
			item_groups.erase(erase_values, item_groups.last());
#else
			const auto before_size = item_groups.size();
			item_groups.erase(erase_values, item_groups.end());
			if (std::is_constant_evaluated() == false) {
				if (item_groups.size() != before_size - remove_indexes.size()) throw std::runtime_error("");
			}
#endif
			remove_iterators.clear();
		}
#else
		std::vector<std::size_t> remove_indexes;
		long long l = item_groups.size();
		for (long long i = 0; i < l; ++i)
		{
#ifdef _BOUNDS_CHECKING_
			ASSERT_NOT_CONSTEXPR(i < item_groups.size());
#endif
			item_groups_type& current_item_group = item_groups[i];
			if (current_item_group.count() > 0)
			{
				if (current_item_group.is_goal_distance_zero()) //index 0 of the group should be moved
				{
					if (segment_end_points.size() > 0ull)
					{
						for (long long i2 = 0; i2 < tc::sign(segment_end_points.size()); ++i2)
						{
#ifdef _BOUNDS_CHECKING_
							ASSERT_NOT_CONSTEXPR(i2 < segment_end_points.size());
#endif
							auto segment_ptr = segment_end_points[tc::unsign(i2)];
							if (segment_ptr->start_of_segment != current_item_group.get_position()) continue;

							const auto tmp = current_item_group.get(0);
							if (segment_ptr->add_item(tmp))
							{
								current_item_group.remove_item(0);
								item_was_removed = true;
								break;
							}
						}
					}
					else current_item_group.set_active_mode(belt_utility::belt_update_mode::first_stuck);
				}
				if (item_was_removed) current_item_group.set_active_mode(belt_utility::belt_update_mode::free);

				switch (current_item_group.get_active_mode())
				{
					default:
					case belt_utility::belt_update_mode::free: current_item_group.update_belt(); break;
					case belt_utility::belt_update_mode::first_stuck: current_item_group.items_stuck_update(); break;
					case belt_utility::belt_update_mode::all_stuck: break;
				}
			}
			else remove_indexes.emplace_back(tc::widen<unsigned long long>(i));
		}

		if (!inserters.empty() && !remove_indexes.empty())
		{
			std::size_t current_index = 0;
			vec2_uint position{ 0ll, 0ll };
			if (!inserters.empty()) position = inserters[0].get_position();
			for (const auto& ref : remove_indexes)
			{
#ifdef _BOUNDS_CHECKING_
				ASSERT_NOT_CONSTEXPR(ref < item_groups.size());
#endif
				if (item_groups[ref].get_direction_position() <= position.x)
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
			const auto erase_values = mem::erase_indices(item_groups, remove_indexes);
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
			item_groups.erase(erase_values, item_groups.last());
#else
			const auto before_size = item_groups.size();
			item_groups.erase(erase_values, item_groups.end());
			if (std::is_constant_evaluated() == false) {
				if (item_groups.size() != before_size - remove_indexes.size()) throw std::runtime_error("");
			}
		}
#endif
#endif

		long long item_group_offset = 0;
		for (long long i = 0, li = tc::sign(inserters.size()); i < li; ++i)
		{
#ifdef _BOUNDS_CHECKING_
			ASSERT_NOT_CONSTEXPR(i < inserters.size());
#endif
			if (inserters[tc::unsign(i)].sleep_update())
			{
				move_inserter_to_sleeping(i);
				--i;
				--li;
			}
			else
			{
				item_group_offset += increment_count[tc::unsign(i)];
				inserters[tc::unsign(i)].update(item_group_offset);
			}
		}
	};

	constexpr short wake_up_inserter(item_type type) noexcept
	{
		const long long l = tc::sign(sleeping_inserters.size());
		for (long long i = 0; i < l; ++i)
		{
			if (sleeping_inserters[tc::unsign(i)].check_if_need_type(type))
			{
				sleeping_inserters[tc::unsign(i)].set_is_sleeping(false);
				inserters.push_back(sleeping_inserters[tc::unsign(i)]);
				sleeping_inserters.erase(sleeping_inserters.begin() + i);
				return tc::narrow<short>(inserters.size()) - 1ll;
			}
		}

		return -1;
	};

	constexpr bool add_item(const item_uint& new_item) noexcept
	{
		if (item_groups.size() == 0)
		{
			auto& new_item_group = item_groups.emplace_back(segment_direction, get_end_distance_direction());
			new_item_group.set_owner_ptr(this);
#ifdef _BOUNDS_CHECKING_
			ASSERT_NOT_CONSTEXPR(0ll < item_groups.size());
#endif
			const short added_index = new_item_group.add_item(new_item, new_item.position);
			if (added_index != -1)
			{
				wake_up_inserter(new_item.type);
				return true;
			}
			return false;
		}
		else
		{
			auto iter = belt_utility::find_closest_item_group(item_groups, new_item.position);
#ifdef _BOUNDS_CHECKING_
			ASSERT_NOT_CONSTEXPR(iter.result != item_groups.end());
#endif
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
			if (belt_utility::find_closest_item_group_return_result::invalid_value == iter.scan || iter.result == item_groups.last() || (iter.result != item_groups.last() && !iter.result->can_add_item(new_item.position))) return false;
#else
			if (belt_utility::find_closest_item_group_return_result::invalid_value == iter.scan || iter.result == item_groups.end() || (iter.result != item_groups.end() && !iter.result->can_add_item(new_item.position))) return false;
#endif
			if (belt_utility::find_closest_item_group_return_result::insert_into_group == iter.scan)
			{
				if (iter.result->count() >= 32) return false; //need to split item_group into 2
				else
				{
					const short added_index = iter.result->add_item(new_item, new_item.position);
					if (added_index != -1)
					{
						wake_up_inserter(new_item.type);
						return true;
					}
				}
				return false;
			}
			else if (belt_utility::find_closest_item_group_return_result::new_group_before_iter == iter.scan)
			{
				auto new_iter_group = item_groups.emplace(iter.result, segment_direction, get_end_distance_direction());
				(new_iter_group + 1)->set_owner_ptr(this);
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
				if (new_iter_group != item_groups.last()) new_iter_group->add_item(new_item, new_item.position);
#else
				if (new_iter_group != item_groups.end()) new_iter_group->add_item(new_item, new_item.position);
#endif
				wake_up_inserter(new_item.type);
				return true;
			}
			else if (belt_utility::find_closest_item_group_return_result::new_group_after_iter == iter.scan)
			{
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
				if (iter.result + 1 == item_groups.last())
#else
				if (iter.result + 1 == item_groups.end())
#endif
				{
					auto& new_iter_group = item_groups.emplace_back(segment_direction, get_end_distance_direction());
					new_iter_group.set_owner_ptr(this);
					new_iter_group.add_item(new_item, new_item.position);
					wake_up_inserter(new_item.type);
					return true;
				}
				else
				{
					auto new_iter_group = item_groups.emplace(iter.result + 1, segment_direction, get_end_distance_direction());
					new_iter_group->set_owner_ptr(this);
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
					if (new_iter_group != item_groups.last()) new_iter_group->add_item(new_item, new_item.position);
#else
					if (new_iter_group != item_groups.end()) new_iter_group->add_item(new_item, new_item.position);
#endif
					wake_up_inserter(new_item.type);
					return true;
				}
			}
		}

		return false;
	};

	constexpr std::size_t add_inserter(index_inserter object) noexcept
	{
		if (inserters.size() == 0)
		{
			object.set_linked_list_data(index_iterator<item_groups_type, _vector>{ 0ull, & item_groups });
			inserters.push_back(object);
			return 0;
		}

		long long can_fit_index = -1;
		if (inserters.size() == 1)
		{
			const auto test_result = check_if_inserter_is_before_or_after(segment_direction, inserters[0], object);
			if (belt_utility::inserter_fits_results::before == test_result) can_fit_index = 0;
			else if (belt_utility::inserter_fits_results::after == test_result) can_fit_index = 1;
		}
		else
		{
			//first check if it's before the first inserter
			if (belt_utility::inserter_fits_results::before == check_if_inserter_is_before_or_after(segment_direction, inserters[0], object)) can_fit_index = 0;
			//second check if it's after the last inserter
			else if (belt_utility::inserter_fits_results::after == check_if_inserter_is_before_or_after(segment_direction, inserters.back(), object)) can_fit_index = 0;
			else
			{
				//check if we can place it inbetween inserters
				const long long l = tc::narrow<long long>(inserters.size());
				for (long long i = 0, i2 = 1; i2 < l; ++i, ++i2)
				{
					if (belt_utility::inserter_fits_results::inbetween == check_if_inserter_is_between(segment_direction, inserters[tc::unsign(i)], inserters[tc::unsign(i2)], object)) can_fit_index = i2;
				}
			}
		}

		if (can_fit_index != -1)
		{
			object.set_linked_list_data(index_iterator<item_groups_type, _vector>{ 0ull, & item_groups });
			inserters.insert(inserters.begin() + can_fit_index, object);
			//inserters.push_back(object);
			return can_fit_index;
		}

		return -1;
	};

	inline constexpr void add_end_segment_section(belt_segment* ptr) noexcept
	{
		segment_end_points.push_back(ptr);
	};
};


CONSTEXPR_VAR auto test_belt_segment() noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{255, 0} };
	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ 0ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 64ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::log, vec2_uint{ 96ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 128ll, 0ll } });

	//return first_segment.get_item(0).get_distance_to_item(2);
	//return first_segment.get_item(0).get_direction_position() - first_segment.get_item(0).get_distance_to_item(3);
	//first_segment.add_item(item_uint{ test_arr[5].type, vec2_uint{ 42ll, 0ll } });
	//return first_segment.get_item(0).count();
	//return first_segment.get_item(0).get(4).type;
	//used to check support that adding items inbetween is possible
	//test_belt.add_item(test_arr[4], vec2_uint{ 0ll, 0ll });
	//test_belt.add_item(test_arr[5], vec2_uint{ 32ll, 0ll });
	//return test_belt.get(4).type; 

	for (int i = 0, l = 128 + 32; i < l; ++i)
	{
		first_segment.update();
	}

	return first_segment.get_item(0).get(0).position.x;
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto test_val_belt_segment = test_belt_segment();
static_assert(test_belt_segment() == 255, "wrong count");
#endif


CONSTEXPR_VAR auto test_moving_items_between_belt_segments(std::size_t return_index) noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{255, 0} };
	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ 0ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 64ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::log, vec2_uint{ 96ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 128ll, 0ll } });
	belt_segment second_segment{ vec2_uint{255, 0}, vec2_uint{1024, 0} };
	first_segment.add_end_segment_section(&second_segment);
	for (int i = 0, l = 127 + 32 + 32; i < l; ++i)
	{
		second_segment.update();
		first_segment.update();
	}

	//return first_segment.get_item(0).count();
	//return second_segment.get_item(0).count();
	//return second_segment.count_item_groups();
	//return first_segment.get_item(0).get(return_index).position.x;
	//return second_segment.get_item(0).get_goal();
	return second_segment.get_item(0).get(tc::sign(return_index)).position.x;
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto moving_items_between_belts_val = test_moving_items_between_belt_segments(0);
static_assert(test_moving_items_between_belt_segments(0) == 319ll, "item did not jump to the second segment");
#endif


CONSTEXPR_VAR auto test_removing_item_group(std::size_t return_index) noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{255, 0} };
	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ 0ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 64ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::log, vec2_uint{ 96ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 128ll, 0ll } });
	belt_segment second_segment{ vec2_uint{255, 0}, vec2_uint{1024, 0} };
	first_segment.add_end_segment_section(&second_segment);

	for (int i = 0, l = 128 + 32; i < l; ++i)
	{
		first_segment.update();
		second_segment.update();
	}

	if (return_index == 0) return first_segment.get_item(0).count();
	else return second_segment.get_item(0).count();
};
#ifdef CONSTEXPR_ASSERTS
static_assert(test_removing_item_group(0) == 2, "item did not jump to the second segment");
static_assert(test_removing_item_group(1) == 2, "item did not jump to the second segment");
#endif


CONSTEXPR_VAR auto test_incrementing_if_some_are_stuck(std::size_t return_index) noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{255, 0} };
	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ 0ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 64ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::log, vec2_uint{ 96ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 128ll, 0ll } });

	for (int i = 0, l = 127 + 32 + 32; i < l; ++i)
	{
		first_segment.update();
	}

	return first_segment.get_item(0).get(return_index).position.x;
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto some_are_stuck3 = test_incrementing_if_some_are_stuck(3);
constexpr auto some_are_stuck2 = test_incrementing_if_some_are_stuck(2);
constexpr auto some_are_stuck1 = test_incrementing_if_some_are_stuck(1);
constexpr auto some_are_stuck0 = test_incrementing_if_some_are_stuck(0);
static_assert(test_incrementing_if_some_are_stuck(3) == 159, "position is wrong, incrementing position while items are stuck doesn't work for 3");
static_assert(test_incrementing_if_some_are_stuck(2) == 191, "position is wrong, incrementing position while items are stuck doesn't work for 2");
static_assert(test_incrementing_if_some_are_stuck(1) == 223, "position is wrong, incrementing position while items are stuck doesn't work for 1");
static_assert(test_incrementing_if_some_are_stuck(0) == 255, "position is wrong, incrementing position while items are stuck doesn't work for 0");
#endif


CONSTEXPR_VAR auto test_adding_in_middle(std::size_t return_index) noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{400, 0} };
	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ 10ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 160ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::log, vec2_uint{ 224ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 288ll, 0ll } });

	auto val = first_segment.add_item(item_uint{ item_type::brick, vec2_uint{ 42ll, 0ll } });
	if (!val) return item_type::pink_square;
	return first_segment.get_item(0).get(return_index).type;
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto in_middle2_val = test_adding_in_middle(4);
static_assert(test_adding_in_middle(2) == item_type::stone, "wrong type so adding in the middle got something wrong");
static_assert(test_adding_in_middle(3) == item_type::brick, "wrong type so adding in the middle got something wrong");
static_assert(test_adding_in_middle(4) == item_type::wood, "wrong type so adding in the middle got something wrong");
#endif


CONSTEXPR_VAR auto test_item_distance(std::size_t return_index) noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{400, 0} };
	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ 10ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 160ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::log, vec2_uint{ 224ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 288ll, 0ll } });

	first_segment.get_item(0).remove_item(return_index);
	return first_segment.get_item(0).get_distance_to_item(return_index);
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto test_item_distance_val = test_item_distance(1);
static_assert(test_item_distance(1) == 128, "item distance is incorrect");
#endif


CONSTEXPR_VAR auto test_inserter_item() noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{400, 0} };

	first_segment.add_inserter(index_inserter{ vec2_uint{256 + 64, 32} });
	first_segment.get_inserter(0).set_item_type(item_type::stone);

	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ 288ll - 278ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 288ll - 128ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::log, vec2_uint{ 288ll - 64ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 288ll, 0ll } });

	for (int i = 0, l = 128 + 32; i < l; ++i)
	{
		first_segment.update();
	}

	return first_segment.get_inserter(0).get_item().type;
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto test_inserter_item_val = test_inserter_item();
static_assert(test_inserter_item() == item_type::stone, "inserter hasn't grabbed item");
#endif


CONSTEXPR_VAR auto test_inserter_moved_to_no_group() noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{1024, 0} };

	first_segment.add_inserter(index_inserter{ vec2_uint{256 + 64, 32} });
	first_segment.get_inserter(0).set_item_type(item_type::stone);

	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ 288ll - 278ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 288ll - 128ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::log, vec2_uint{ 288ll - 64ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 288ll, 0ll } });

	for (int i = 0, l = 256 + 128; i < l; ++i)
	{
		first_segment.update();
	}

	return first_segment.get_inserter(0).has_linked_list_data();
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto test_moved_group_val = test_inserter_moved_to_no_group();
static_assert(test_inserter_moved_to_no_group() == true, "inserter hasn't been moved");
#endif