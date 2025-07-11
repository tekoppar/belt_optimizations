#pragma once

#include "macros.h"
#include "mem_vector.h"

#include "vectors.h"

#include "belt_utility_concepts.h"
#include "belt_utility_data.h"
#include <stdexcept>
#include "math_utility.h"
#include <concepts>
#include "index_iterator.h"
#include <vector>

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

	template<belt_direction direction>
	inline static constexpr long long get_direction_position(vec2_int64 pos) noexcept
	{
		if constexpr (belt_direction::left_right == direction) return pos.x;
		if constexpr (belt_direction::right_left == direction) return pos.x;
		if constexpr (belt_direction::top_bottom == direction) return pos.y;
		if constexpr (belt_direction::bottom_top == direction) return pos.y;
	};

	template<typename vector>
	concept is_container = requires(vector v)
	{
		{
			v.begin()
		} -> std::same_as<typename vector::iterator>;
	};
	template<typename object, typename return_value>
	concept is_callable_member = requires(object o, return_value object:: * m_ptr)
	{
		{
			(o.*m_ptr)()
		};
	};

	template<typename value_to_find, typename vector_haystack, typename vector_object = vector_haystack::value_type, typename return_value>
	static constexpr vector_haystack::iterator binary_find(value_to_find needle, vector_haystack& goal_dist_vec, return_value vector_object::* member_ptr) noexcept
		requires(is_container<vector_haystack> == true && is_callable_member<typename vector_haystack::value_type, return_value> == true)
	{
		auto left_goal_iter = goal_dist_vec.begin();
		auto right_goal_iter = goal_dist_vec.last() - 1ll;
		auto half_size = goal_dist_vec.size();

		while (half_size > 1ll)
		{
			half_size = expr::abs((right_goal_iter - left_goal_iter) / 2ll);
			auto temp_half_goal_iter = left_goal_iter + half_size;

			if (((*temp_half_goal_iter).*member_ptr)() < needle) right_goal_iter = temp_half_goal_iter;
			else left_goal_iter = temp_half_goal_iter;
		}

		if (((*left_goal_iter).*member_ptr)() < needle) return left_goal_iter;
		else return right_goal_iter;
	};


	template<belt_utility::belt_direction segment_direction, typename inserter_iterator>
	constexpr belt_utility::distance_comparison get_distance_comparison(long long end_distance, long long distance, inserter_iterator first, inserter_iterator last, long long offset_value) noexcept
	{
		const auto inserter_distance = end_distance - get_direction_position<segment_direction>(first->operator[](0).get_position());
		const auto last_inserter_distance = end_distance - get_direction_position<segment_direction>(((*last).last() - 1ll)->get_position());

		if (inserter_distance >= distance && distance >= last_inserter_distance - offset_value) return belt_utility::distance_comparison::distance_is_inside;
		if (distance > inserter_distance) return belt_utility::distance_comparison::distance_is_before;
		if (distance < last_inserter_distance - offset_value) return belt_utility::distance_comparison::distance_is_after;

#ifdef ENABLE_CPP_EXCEPTION_THROW
		throw std::runtime_error("invalid scenario, fix your code");
#endif

		return belt_utility::distance_comparison::null;
	};

	template<belt_utility::belt_direction segment_direction, typename inserter_vector, typename inserter_iterator = inserter_vector::iterator>
	constexpr inserter_iterator find_which_inserter_group_distance_belongs_in(long long end_distance, long long distance, inserter_vector& inserter_vec, long long offset_value) noexcept
	{
		auto begin_inserter_iter = inserter_vec.begin();
		while (begin_inserter_iter != inserter_vec.last())
		{
			auto distance_comparison = belt_utility::get_distance_comparison<segment_direction>(end_distance, distance, begin_inserter_iter, begin_inserter_iter, offset_value);
			if (belt_utility::distance_comparison::distance_is_after == distance_comparison)
			{
				++begin_inserter_iter;
			}
			else
				return begin_inserter_iter;
		}

		return begin_inserter_iter;
	};

	template<belt_utility::belt_direction direction, typename data_vector, typename vector, typename dist_vector, typename vector_iterator = vector::iterator>
	static constexpr find_closest_item_group_result<vector_iterator> find_closest_item_group(long long segment_end_direction, data_vector& data_vec, vector& vec, dist_vector& dist_vec, long long position) noexcept
		requires(class_has_iterator<vector>)
	{
		constexpr auto max_distance = vector::value_type::max_distance_between_items;
		if (vec.empty()) return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::invalid_value, vector_iterator{} };

		auto last_iter = vec.last() - 1;
		auto last_data_iter = data_vec.last() - 1;
		auto last_dist_iter = dist_vec.last() - 1;
		const auto dir_pos_last_iter = last_iter->get_direction_position(segment_end_direction, *last_dist_iter);
		if (position > dir_pos_last_iter + max_distance) 
			return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::new_group_after_iter, last_iter };
		else if (position > dir_pos_last_iter)
		{
			if (last_iter->count() < 32) 
				return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::insert_into_group, last_iter };
			else 
				return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::new_group_after_iter, last_iter };
		}

		auto begin_iter = vec.begin();
		auto begin_data_iter = data_vec.begin();
		auto begin_dist_iter = dist_vec.begin();
		{
			const auto last_dir_pos_begin_iter = begin_iter->get_last_item_direction_position<direction>(segment_end_direction, *begin_dist_iter, *begin_data_iter);
			if (position < last_dir_pos_begin_iter - max_distance) 
				return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::new_group_before_iter, begin_iter };
			else if (position < last_dir_pos_begin_iter)
			{
				if (begin_iter->count() < 32) 
					return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::insert_into_group, begin_iter };
				else 
					return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::new_group_before_iter, begin_iter };
			}
		}

		long long loop_index{ 0 };
		auto end_iter = vec.last();
		for (; begin_iter != end_iter; ++begin_iter, ++begin_data_iter, ++begin_dist_iter)
		{
			if (begin_iter->get_last_item_direction_position<direction>(segment_end_direction, *begin_dist_iter, *begin_data_iter) - max_distance <= position && begin_iter->get_direction_position(segment_end_direction, *begin_dist_iter) + max_distance >= position) //found matching group
				return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::insert_into_group, begin_iter };
			else if (loop_index + 1ll < vec.size())
			{
				auto tmp = begin_iter + 1ll;
				if (tmp != end_iter)
				{
					auto tmp_data = begin_data_iter + 1ll;
					auto tmp_dist = begin_dist_iter + 1ll;
					if (tmp->get_last_item_direction_position<direction>(segment_end_direction, *tmp_dist, *tmp_data) - max_distance > position && begin_iter->get_direction_position(segment_end_direction, *begin_dist_iter) + max_distance < position) //if vector is sorted from low to high
						return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::new_group_after_iter, begin_iter };
					if (begin_iter->get_last_item_direction_position<direction>(segment_end_direction, *begin_dist_iter, *begin_data_iter) - max_distance > position && tmp->get_direction_position(segment_end_direction, *tmp_dist) + max_distance < position) //if vector is sorted from high to low
						return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::new_group_before_iter, tmp };
				}
				//begin_iter = tmp;
				++loop_index;
			}
			else return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::invalid_value, end_iter };
		}

		return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::invalid_value, end_iter };
	};

	template<typename goal_dist_vector>
	static constexpr goal_dist_vector::iterator find_closest_goal_binary(long long segment_end_direction, goal_dist_vector& goal_dist_vec, long long position) noexcept
	{
		auto distance_position = segment_end_direction - position;

		auto left_goal_iter = goal_dist_vec.begin();
		auto right_goal_iter = goal_dist_vec.last() - 1ll;
		auto half_size = goal_dist_vec.size();

		while (half_size > 1ll)
		{
			half_size = expr::abs((right_goal_iter - left_goal_iter) / 2ll);
			auto temp_half_goal_iter = left_goal_iter + half_size;

			if ((*temp_half_goal_iter).get_distance() < distance_position) right_goal_iter = temp_half_goal_iter;
			else left_goal_iter = temp_half_goal_iter;
		}

		if ((*left_goal_iter).get_distance() > distance_position) return right_goal_iter;
		else return left_goal_iter;
	};

	template<belt_utility::belt_direction direction, typename data_vector, typename vector, typename dist_vector, typename goal_dist_vector, typename vector_iterator = vector::iterator, typename inserter_vector>
	static constexpr find_closest_item_group_result<vector_iterator> find_closest_item_group(long long segment_end_direction, data_vector& data_vec, vector& vec, dist_vector& dist_vec, goal_dist_vector& goal_dist_vec, long long position, inserter_vector& inserter_vec) noexcept
		requires(class_has_iterator<vector>)
	{
		const auto distance_position = segment_end_direction - position;
		const auto found_goal = belt_utility::binary_find(segment_end_direction - position, goal_dist_vec, &goal_dist_vector::value_type::get_distance);
		const auto found_goal_index = found_goal - goal_dist_vec.begin();
		auto found_inserter = inserter_vec.begin() + found_goal_index;

		bool force_new_group_after = false;
		if (found_inserter != inserter_vec.last())
		{
			//const auto found_inserter_position = belt_utility::get_direction_position<direction>(((*found_inserter).last() - 1ll)->get_position());
			force_new_group_after = distance_position <= ((*found_inserter).last() - 1ll)->get_distance_position_minus();
			//force_new_group_after = position >= found_inserter_position + vector::value_type::belt_item_size;
			//if (force_new_group_after != force_new_group_after2) __debugbreak();
			
			//if (position >= found_inserter_position + vector::value_type::belt_item_size)
				//force_new_group_after = true;

			belt_utility::distance_comparison previous_distance_comp = belt_utility::distance_comparison::null;
			belt_utility::distance_comparison prev_previous_distance_comp = belt_utility::distance_comparison::null;
			if (found_inserter != inserter_vec.begin() && found_inserter != inserter_vec.last())
			{
				auto previous_inserter_iter = found_inserter;

				//const auto inserter_distance = segment_end_direction - belt_utility::get_direction_position<direction>(found_inserter->operator[](0).get_position());
				const auto inserter_distance2 = found_inserter->operator[](0).get_distance_position_minus();
				//if ((inserter_distance < distance_position) != (inserter_distance2 < distance_position)) __debugbreak();
				if (inserter_distance2 < distance_position)
					previous_inserter_iter = found_inserter - 1ll;

				previous_distance_comp = belt_utility::get_distance_comparison<direction>(segment_end_direction, distance_position, previous_inserter_iter, previous_inserter_iter, vector::value_type::belt_item_size);
				prev_previous_distance_comp = belt_utility::get_distance_comparison<direction>(segment_end_direction, *(dist_vec.begin() + dist_vec.size() - 1ll).operator->(), previous_inserter_iter, previous_inserter_iter, vector::value_type::belt_item_size);

				if (!(belt_utility::distance_comparison::distance_is_inside == previous_distance_comp && belt_utility::distance_comparison::distance_is_inside == prev_previous_distance_comp) &&
					!(belt_utility::distance_comparison::distance_is_inside == previous_distance_comp && belt_utility::distance_comparison::distance_is_before == prev_previous_distance_comp) &&
					belt_utility::distance_comparison::distance_is_before != previous_distance_comp && belt_utility::distance_comparison::distance_is_after != prev_previous_distance_comp)
					force_new_group_after = true;
				else
					force_new_group_after = false;
			}
		}
		else if (found_inserter == inserter_vec.last() && goal_dist_vec.size() == inserter_vec.size())
		{
			force_new_group_after = true;
		}

		constexpr auto max_distance = vector::value_type::max_distance_between_items;
		if (vec.empty()) 
			return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::invalid_value, vector_iterator{} };

		const auto last_iter = vec.last() - 1;
		const auto last_data_iter = data_vec.last() - 1;
		auto last_dist_iter = dist_vec.last() - 1;
		const auto dir_pos_last_iter = last_iter->get_direction_position(segment_end_direction, *last_dist_iter);
		if (position > dir_pos_last_iter + max_distance) 
			return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::new_group_after_iter, last_iter };
		else if (position > dir_pos_last_iter)
		{
			if (last_iter->count() < 32 && force_new_group_after == false)
				return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::insert_into_group, last_iter };
			else 
				return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::new_group_after_iter, last_iter };
		}

		auto begin_iter = vec.begin();
		auto begin_data_iter = data_vec.begin();
		auto begin_dist_iter = dist_vec.begin();
		{
			const auto last_dir_pos_begin_iter = begin_iter->get_last_item_direction_position<direction>(segment_end_direction, *begin_dist_iter, *begin_data_iter);
			if (position < last_dir_pos_begin_iter - max_distance) 
				return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::new_group_before_iter, begin_iter };
			else if (position < last_dir_pos_begin_iter)
			{
				if (begin_iter->count() < 32 && force_new_group_after == false)
					return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::insert_into_group, begin_iter };
				else
					return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::new_group_before_iter, begin_iter };
			}
		}

		long long loop_index{ 0 };
		const auto end_iter = vec.last();
		for (; begin_iter != end_iter; ++begin_iter, ++begin_data_iter, ++begin_dist_iter)
		{
			if (begin_iter->get_last_item_direction_position<direction>(segment_end_direction, *begin_dist_iter, *begin_data_iter) - max_distance <= position && begin_iter->get_direction_position(segment_end_direction, *begin_dist_iter) + max_distance >= position) //found matching group
				return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::insert_into_group, begin_iter };
			else if (loop_index + 1ll < vec.size())
			{
				const auto tmp = begin_iter + 1ll;
				if (tmp != end_iter)
				{
					auto tmp_data = begin_data_iter + 1ll;
					auto tmp_dist = begin_dist_iter + 1ll;
					if (tmp->get_last_item_direction_position<direction>(segment_end_direction, *tmp_dist, *tmp_data) - max_distance > position && begin_iter->get_direction_position(segment_end_direction, *begin_dist_iter) + max_distance < position) //if vector is sorted from low to high
						return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::new_group_after_iter, begin_iter };
					if (begin_iter->get_last_item_direction_position<direction>(segment_end_direction, *begin_dist_iter, *begin_data_iter) - max_distance > position && tmp->get_direction_position(segment_end_direction, *tmp_dist) + max_distance < position) //if vector is sorted from high to low
						return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::new_group_before_iter, tmp };
				}
				++loop_index;
			}
			else 
				return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::invalid_value, end_iter };
		}

		return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::invalid_value, end_iter };
	};

	template<belt_utility::belt_direction segment_direction, typename type>
	static inline constexpr belt_utility::inserter_fits_results is_inserter_before_or_after(const type& lhs, const type& rhs)
		requires(type_has_grid_size<type>&& type_has_get_position<type>)
	{
		const auto first_inserter_position = get_direction_position<segment_direction>(lhs.get_position());
		const auto test_inserter_position = get_direction_position<segment_direction>(rhs.get_position());
		if (first_inserter_position > test_inserter_position) return belt_utility::inserter_fits_results::before;
		else if (first_inserter_position + type::inserter_grid_size <= test_inserter_position) return belt_utility::inserter_fits_results::after;
		else return belt_utility::inserter_fits_results::no_fit;
	};
	template<belt_utility::belt_direction segment_direction, typename type>
	static inline constexpr belt_utility::inserter_fits_results is_inserter_between(const type& lhs, const type& rhs, const type& subject)
		requires(type_has_grid_size<type>&& type_has_get_position<type>)
	{
		const auto first_inserter_position = get_direction_position<segment_direction>(lhs.get_position());
		const auto second_inserter_position = get_direction_position<segment_direction>(rhs.get_position());
		const auto subject_inserter_position = get_direction_position<segment_direction>(subject.get_position());
		if (first_inserter_position + type::inserter_grid_size <= subject_inserter_position && subject_inserter_position + type::inserter_grid_size <= second_inserter_position) return belt_utility::inserter_fits_results::inbetween;
		return belt_utility::inserter_fits_results::no_fit;
	};

	template<typename vector, typename items_group, typename items_group_data, typename vector_iterator = vector::iterator>
	static constexpr find_closest_active_mode_result<vector_iterator> find_closest_active_mode(long long segment_end_direction, vector& vec, items_group* ptr, items_group_data& item_data) noexcept
		requires(class_has_iterator<vector>)
	{
		if (vec.empty()) return find_closest_active_mode_result<vector_iterator>{ find_closest_active_mode_return_result::invalid_value, vector_iterator{} };

		auto ptr_direction_position = ptr->get_direction_position(segment_end_direction);
		auto last_iter = vec.last() - 1;
		const auto dir_pos_last_iter = last_iter->first_free->get_direction_position(segment_end_direction);

		if (ptr_direction_position > dir_pos_last_iter) return find_closest_active_mode_result<vector_iterator>{ find_closest_active_mode_return_result::new_active_mode_after_iter, last_iter };
		else if (ptr_direction_position < dir_pos_last_iter &&
			ptr_direction_position > last_iter->some_stuck->get_direction_position(segment_end_direction))
			return find_closest_active_mode_result<vector_iterator>{ find_closest_active_mode_return_result::inbetween, last_iter };

		auto begin_iter = vec.begin();
		if (ptr_direction_position < begin_iter->some_stuck->get_direction_position(segment_end_direction)) return find_closest_active_mode_result<vector_iterator>{ find_closest_active_mode_return_result::new_active_mode_before_iter, begin_iter };
		else if (ptr_direction_position > begin_iter->some_stuck->get_direction_position(segment_end_direction) &&
			ptr_direction_position < begin_iter->first_free->get_direction_position(segment_end_direction))
			return find_closest_active_mode_result<vector_iterator>{ find_closest_active_mode_return_result::inbetween, begin_iter };

		long long loop_index{ 0 };
		auto end_iter = vec.last();
		for (; begin_iter != end_iter; ++begin_iter)
		{
			if (begin_iter->some_stuck->get_direction_position(segment_end_direction) > ptr_direction_position &&
				begin_iter->first_free->get_direction_position(segment_end_direction) < ptr_direction_position) //found matching group
				return find_closest_active_mode_result<vector_iterator>{ find_closest_active_mode_return_result::inbetween, begin_iter };
			if (begin_iter->some_stuck->get_direction_position(segment_end_direction) < ptr_direction_position) //found matching group
				return find_closest_active_mode_result<vector_iterator>{ find_closest_active_mode_return_result::new_active_mode_before_iter, begin_iter };
			if (begin_iter->first_free->get_direction_position(segment_end_direction) > ptr_direction_position) //found matching group
				return find_closest_active_mode_result<vector_iterator>{ find_closest_active_mode_return_result::new_active_mode_after_iter, begin_iter };
		}

		return find_closest_active_mode_result<vector_iterator>{ find_closest_active_mode_return_result::invalid_value, end_iter };
	};

	template<typename distance_iter>
	constexpr inline long long get_distances_from_to2(distance_iter from, const distance_iter& to) noexcept
	{
		if (from == to) return *to;
		long long real_distance = *from;
		--from;
		if (from == to) return real_distance + *to;
		while (from != to)
		{
			real_distance += *from;
			--from;
		}

		if (from == to) return real_distance + *to;
		return real_distance;
	};
	template<typename distance_iter, typename goal_iter>
	constexpr inline long long get_distances_from_to(distance_iter dist_iter, const distance_iter& end_dist_iter, const goal_iter& iter) noexcept
	{
		long long real_distance = *dist_iter;
		while (dist_iter != end_dist_iter && (*iter).get_index_ptr() != nullptr && *dist_iter != *(*iter).get_index_ptr())
		{
			real_distance += *dist_iter;
			++dist_iter;
		}

		return (*iter).get_distance() - real_distance;
	};
	template<typename vector_distance_, typename vector_goal_distance_>
	constexpr inline long long get_item_group_distance_from_destination(vector_distance_& distances, vector_goal_distance_& goal_distances, long long index) noexcept
	{
		auto begin_iter = distances.begin();
		auto last_iter = distances.last();
		auto begin_goal_iter = goal_distances.begin();
		auto last_goal_iter = goal_distances.last();

		long long count_index = 0ll;
		while (begin_iter != last_iter && begin_goal_iter != last_goal_iter)
		{
			while (begin_iter != last_iter && (*begin_iter) != 0ll)
			{
				if (count_index == index)
				{
					return get_distances_from_to(begin_iter, last_iter, begin_goal_iter);
				}

				++count_index;
				++begin_iter;
			}

			if (count_index == index) return (*begin_goal_iter).get_distance();

			++begin_goal_iter;
			++begin_iter;
		}

		return -1ll;
	};
	constexpr auto get_goal_object_index(long long index, auto& item_groups_goal_distance, auto& item_groups_distance_between) noexcept
	{
		if (index == item_groups_distance_between.size() - 1ll) return item_groups_goal_distance.last() - 1ll;

		const auto item_groups_distance_between_begin = item_groups_distance_between.begin();
		const auto last = item_groups_goal_distance.last();
		for (auto begin = item_groups_goal_distance.begin(); begin != last; ++begin)
		{
			if ((*begin).get_index_from_ptr(item_groups_distance_between_begin.operator->()) >= index) return begin;
		}

		return item_groups_goal_distance.last();
	};
	constexpr auto get_goal_object_index_binary(long long index, auto& item_groups_goal_distance, auto& item_groups_distance_between) noexcept
	{
		if (index == item_groups_distance_between.size() - 1ll) return item_groups_goal_distance.last() - 1ll;

		/*auto first = item_groups_goal_distance.begin();
		auto last = item_groups_goal_distance.last();
		auto it = std::upper_bound(first, last, index, [&](long long idx, const auto& head)
			{
				return idx < head.get_index_from_ptr(item_groups_distance_between.begin().operator->());
			});

		auto found_std = (it == first) ? first : it - 1;*/

		auto left_goal_iter = item_groups_goal_distance.begin();
		auto right_goal_iter = item_groups_goal_distance.last() - 1ll;
		auto half_size = item_groups_goal_distance.size();
		const auto between_begin_ptr = item_groups_distance_between.begin().operator->();

		while (half_size > 1ll)
		{
			half_size = expr::ceil_div_power2(right_goal_iter - left_goal_iter); //expr::ceil_div(right_goal_iter - left_goal_iter, 2ll);
			auto temp_half_goal_iter = left_goal_iter + half_size;

			if ((*temp_half_goal_iter).get_index_from_ptr(between_begin_ptr) > index) right_goal_iter = temp_half_goal_iter;
			else left_goal_iter = temp_half_goal_iter;
		}

		/*if (found_std != left_goal_iter && found_std != right_goal_iter)
			throw 0;
		*/

		if ((*left_goal_iter).get_index_from_ptr(between_begin_ptr) >= index) return left_goal_iter;
		else return right_goal_iter;
	};
	constexpr auto find_which_goal_object_position_belongs_too(long long position, auto& item_groups_goal_distance) noexcept
	{
		if (item_groups_goal_distance.size() > 1ll)
		{
			for (auto biter = item_groups_goal_distance.begin(), next = biter + 1ll, last = item_groups_goal_distance.last(); biter != last; ++biter)
			{
				if (biter->get_distance() < position && next->get_distance() > position) return biter;
				if (biter->get_distance() > position && next->get_distance() < position) return next;
			}
		}
		else if (!item_groups_goal_distance.empty())
		{
			if (item_groups_goal_distance[0].get_distance() >= position) return item_groups_goal_distance.begin();
		}

		return item_groups_goal_distance.last();
	};
};

namespace mem
{
	template<typename vector_type>
	constexpr inline auto erase_indices(std::vector<vector_type>& data, std::vector<std::size_t>& indicesToDelete) noexcept
	{
		auto indice_iter{ indicesToDelete.begin() };
		auto writer_iter{ data.begin() + *indice_iter };
		auto reader_iter{ writer_iter };
		auto last_iter{ data.end() };

		std::size_t reader_index{ (*indice_iter) };
		if (writer_iter != last_iter)
		{
			for (; reader_iter != last_iter; ++reader_index, ++reader_iter)
			{
				if ((*indice_iter) != reader_index)
				{
					(*writer_iter) = static_cast<vector_type&&>(*reader_iter);
					++writer_iter;
				}
				else
				{
					if (indice_iter + 1 != indicesToDelete.end()) ++indice_iter;
				}
			}
		}

		return writer_iter;
	};
}