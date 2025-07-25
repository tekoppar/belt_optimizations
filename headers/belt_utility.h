#pragma once

#include <concepts>
#include <vector>

#include <item_settings.h>
#include "const_data.h"
#include "mem_vector.h"
#include "shared_classes.h"

#include "vectors.h"

#include "belt_utility_concepts.h"
#include "belt_utility_data.h"
#include "math_utility.h"
#include <index_inserter_settings.h>

namespace belt_utility
{
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
	template<typename object, typename return_value>
	concept is_member = requires(object o, return_value object:: * m_ptr)
	{
		{
			(o.*m_ptr)
		};
	};

	template<typename value_to_find, typename vector_haystack, typename vector_object = vector_haystack::value_type, typename return_value>
	static constexpr vector_haystack::iterator binary_find(value_to_find needle, vector_haystack& goal_dist_vec, return_value vector_object::* member_ptr) noexcept
		requires(is_container<vector_haystack> == true && is_member<typename vector_haystack::value_type, return_value> == true)
	{
		auto left_goal_iter = goal_dist_vec.begin();
		auto right_goal_iter = goal_dist_vec.last() - 1ll;
		auto half_size = goal_dist_vec.size();

		while (half_size > 1ll)
		{
			half_size = expr::ceil_div_power2(right_goal_iter - left_goal_iter);
			auto temp_half_goal_iter = left_goal_iter + half_size;
			/*half_size = expr::abs((right_goal_iter - left_goal_iter) / 2ll);
			auto temp_half_goal_iter = left_goal_iter + half_size;*/

			if (((*temp_half_goal_iter).*member_ptr) < needle) right_goal_iter = temp_half_goal_iter;
			else left_goal_iter = temp_half_goal_iter;
		}

		if (((*left_goal_iter).*member_ptr) < needle) return left_goal_iter;
		else return right_goal_iter;
	};

	constexpr belt_utility::distance_comparison get_distance_comparison(long long distance, const inserter_type& first, const inserter_type& last) noexcept
	{
		const long long inserter_distance = first.get_distance_position_minus();
		const long long last_inserter_distance = last.get_distance_position_minus();

		if (inserter_distance < distance && distance < last_inserter_distance) return belt_utility::distance_comparison::distance_is_inside;
		if (distance > inserter_distance) return belt_utility::distance_comparison::distance_is_before;
		if (distance < last_inserter_distance) return belt_utility::distance_comparison::distance_is_after;

#ifdef ENABLE_CPP_EXCEPTION_THROW
		throw std::runtime_error("invalid scenario, fix your code");
#endif

		return belt_utility::distance_comparison::null;
	};
	constexpr belt_utility::distance_comparison get_distance_comparison(long long end_distance, long long distance, const inserter_type& first, const inserter_type& last, long long offset_value) noexcept
	{
		const auto inserter_distance = first.get_distance_position_plus();
		const auto last_inserter_distance = last.get_distance_position_plus();

		if (inserter_distance >= distance && distance >= last_inserter_distance - offset_value) return belt_utility::distance_comparison::distance_is_inside;
		if (distance > inserter_distance) return belt_utility::distance_comparison::distance_is_before;
		if (distance < last_inserter_distance - offset_value) return belt_utility::distance_comparison::distance_is_after;

#ifdef ENABLE_CPP_EXCEPTION_THROW
		throw std::runtime_error("invalid scenario, fix your code");
#endif

		return belt_utility::distance_comparison::null;
	};

	template<belt_utility::belt_direction segment_direction>
	constexpr _vector_inserters::iterator find_which_inserter_group_distance_belongs_in(long long end_distance, long long distance, _vector_inserters& inserter_vec, long long offset_value) noexcept
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

	static constexpr _vector_item_groups_head_type::iterator find_closest_goal_binary(long long segment_end_direction, _vector_item_groups_head_type& goal_dist_vec, long long position) noexcept
	{
		const auto distance_position = segment_end_direction - position;

		auto left_goal_iter = goal_dist_vec.begin();
		auto right_goal_iter = goal_dist_vec.last() - 1ll;
		auto half_size = goal_dist_vec.size();

		while (half_size > 1ll)
		{
			half_size = expr::abs((right_goal_iter - left_goal_iter) / 2ll);
			auto temp_half_goal_iter = left_goal_iter + half_size;

			if ((*temp_half_goal_iter).distance < distance_position) right_goal_iter = temp_half_goal_iter;
			else left_goal_iter = temp_half_goal_iter;
		}

		if ((*left_goal_iter).distance > distance_position) return right_goal_iter;
		else return left_goal_iter;
	};

	constexpr _vector_item_groups_head_type::iterator get_head_item_belongs_too(long long item_distance, _vector_item_groups_head& item_groups_heads) noexcept
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

	template<belt_direction direction>
	constexpr find_closest_item_group_result<_vector::iterator> find_closest_item_group_binary(
		const _vector_item_groups_head::iterator item_group_head,
		const long long head_index,
		long long head_vector_size,
		const long long new_goal_distance,
		_vector& item_groups_vec,
		_data_vector& item_groups_data_vec,
		_vector_distance& item_groups_distance_vec, 
		_simple_inserter_vector& inserter_vec,
		_inserter_group_indexes& inserter_group_indexes) noexcept
	{
		const _simple_inserter_vector::iterator found_inserter = head_index < inserter_vec.size() ? inserter_vec.begin() + inserter_group_indexes[head_index].start : inserter_vec.last();

		bool force_new_group_after = false;
		if (found_inserter == inserter_vec.last() && head_vector_size == inserter_group_indexes.size())
			force_new_group_after = true;
		else if (found_inserter != inserter_vec.last() && distance_comparison::distance_is_after == get_distance_comparison(new_goal_distance, inserter_vec[inserter_group_indexes[head_index].start], inserter_vec[inserter_group_indexes[head_index].end]))
			force_new_group_after = true;

		const _vector::iterator end_iter = item_groups_vec.last();

		if (item_group_head->item_group.count() < item_settings::max_item_count && force_new_group_after == false)
			return { find_closest_item_group_return_result::insert_into_group, end_iter };
		if (new_goal_distance > item_group_head->distance - item_settings::max_distance_between_items)
			return { find_closest_item_group_return_result::new_group_after_iter, end_iter };

		auto right_of_binary_search = item_group_head->next_item_group_index;
		auto left_of_binary_search = head_index == 0 ? 0 : (item_group_head - 1ll)->next_item_group_index;

		long long half_index = 0ll;
		while (left_of_binary_search != right_of_binary_search)
		{
			half_index = left_of_binary_search + expr::ceil_div_power2(right_of_binary_search - left_of_binary_search); // return (lhs + 1) >> 1;

			const auto last_item_position = item_groups_vec[half_index].get_last_item_direction_position<direction>(item_groups_distance_vec[half_index], item_groups_data_vec[half_index]);
			if (new_goal_distance > last_item_position - item_settings::max_distance_between_items)
			{
				if (new_goal_distance < item_groups_distance_vec[half_index] - item_settings::max_distance_between_items) //found matching group
					return { find_closest_item_group_return_result::insert_into_group, item_groups_vec.begin() + half_index };

				right_of_binary_search -= half_index;
			}
			else
				left_of_binary_search += half_index;
		}

		if (item_groups_vec[half_index].count() < item_settings::max_item_count && force_new_group_after == false)
			return { find_closest_item_group_return_result::insert_into_group, item_groups_vec.begin() + half_index };
		if (new_goal_distance > item_groups_distance_vec[half_index] - item_settings::max_distance_between_items)
			return { find_closest_item_group_return_result::new_group_after_iter, item_groups_vec.begin() + half_index };
		if (new_goal_distance < item_settings::max_distance_between_items + item_groups_distance_vec[half_index])
			return { find_closest_item_group_return_result::new_group_before_iter, item_groups_vec.begin() + half_index };

		return { find_closest_item_group_return_result::invalid_value, end_iter };
	};

	template<belt_direction direction>
	static constexpr find_closest_item_group_result<_vector::iterator> find_closest_item_group(
		long long segment_end_direction,
		_data_vector& data_vec,
		_vector& vec,
		_vector_distance& dist_vec,
		_vector_item_groups_head_type::iterator head_iter,
		long long head_index,
		long long head_vector_size,
		long long position,
		_vector_inserters& inserter_vec
	) noexcept
		requires(class_has_iterator<_vector>)
	{
		const long long distance_position = segment_end_direction - position;
		_vector_inserters::iterator found_inserter = inserter_vec.last();
		if (head_index >= 0)
			found_inserter = inserter_vec.begin() + head_index;

		bool force_new_group_after = false;
		if (found_inserter != inserter_vec.last())
		{
			const belt_utility::distance_comparison distance_comparison = belt_utility::get_distance_comparison<direction>(segment_end_direction, distance_position, found_inserter, found_inserter, item_settings::belt_item_size);
			if (belt_utility::distance_comparison::distance_is_after == distance_comparison) 
				force_new_group_after = true;
			else
			{
				force_new_group_after = distance_position <= ((*found_inserter).last() - 1ll)->get_distance_position_minus();

				belt_utility::distance_comparison previous_distance_comp = belt_utility::distance_comparison::null;
				belt_utility::distance_comparison prev_previous_distance_comp = belt_utility::distance_comparison::null;
				if (found_inserter != inserter_vec.begin() && found_inserter != inserter_vec.last())
				{
					_vector_inserters::iterator previous_inserter_iter = found_inserter;

					const long long inserter_distance2 = found_inserter->operator[](0).get_distance_position_minus();
					if (inserter_distance2 < distance_position)
						previous_inserter_iter = found_inserter - 1ll;

					previous_distance_comp = belt_utility::get_distance_comparison<direction>(segment_end_direction, distance_position, previous_inserter_iter, previous_inserter_iter, item_settings::belt_item_size);
					prev_previous_distance_comp = belt_utility::get_distance_comparison<direction>(segment_end_direction, *(dist_vec.begin() + dist_vec.size() - 1ll).operator->(), previous_inserter_iter, previous_inserter_iter, item_settings::belt_item_size);

					if (!(belt_utility::distance_comparison::distance_is_inside == previous_distance_comp && belt_utility::distance_comparison::distance_is_inside == prev_previous_distance_comp) &&
						!(belt_utility::distance_comparison::distance_is_inside == previous_distance_comp && belt_utility::distance_comparison::distance_is_before == prev_previous_distance_comp) &&
						belt_utility::distance_comparison::distance_is_before != previous_distance_comp && belt_utility::distance_comparison::distance_is_after != prev_previous_distance_comp)
						force_new_group_after = true;
					else
						force_new_group_after = false;
				}
			}
		}
		else if (found_inserter == inserter_vec.last() && head_vector_size == inserter_vec.size())
			force_new_group_after = true;

		const _vector::iterator end_iter = vec.last();
		constexpr int max_distance = item_settings::max_distance_between_items;

		{
			const long long dir_pos_last_iter = head_iter->item_group.get_direction_position(segment_end_direction, head_iter->distance);
			if (position > dir_pos_last_iter + max_distance)
				return { find_closest_item_group_return_result::new_group_after_iter, end_iter };
			else if (position > dir_pos_last_iter)
			{
				if (head_iter->item_group.count() < item_settings::max_item_count && force_new_group_after == false)
					return { find_closest_item_group_return_result::insert_into_group, end_iter };
				else
					return { find_closest_item_group_return_result::new_group_after_iter, end_iter };
			}
		}

		if (vec.empty())
			return { find_closest_item_group_return_result::invalid_value, end_iter };

		const _vector::iterator last_iter = vec.last() - 1;
		const _vector_distance::iterator last_dist_iter = dist_vec.last() - 1;
		const long long dir_pos_last_iter = last_iter->get_direction_position(segment_end_direction, *last_dist_iter);
		if (position > dir_pos_last_iter + max_distance)
			return { find_closest_item_group_return_result::new_group_after_iter, last_iter };
		else if (position > dir_pos_last_iter)
		{
			if (last_iter->count() < item_settings::max_item_count && force_new_group_after == false)
				return { find_closest_item_group_return_result::insert_into_group, last_iter };
			else
				return { find_closest_item_group_return_result::new_group_after_iter, last_iter };
		}

		_vector::iterator begin_iter = vec.begin();
		_data_vector::iterator begin_data_iter = data_vec.begin();
		_vector_distance::iterator begin_dist_iter = dist_vec.begin();
		{
			const long long last_dir_pos_begin_iter = begin_iter->get_last_item_direction_position<direction>(segment_end_direction, *begin_dist_iter, *begin_data_iter);
			if (position < last_dir_pos_begin_iter - max_distance)
				return { find_closest_item_group_return_result::new_group_before_iter, begin_iter };
			else if (position < last_dir_pos_begin_iter)
			{
				if (begin_iter->count() < item_settings::max_item_count && force_new_group_after == false)
					return { find_closest_item_group_return_result::insert_into_group, begin_iter };
				else
					return { find_closest_item_group_return_result::new_group_before_iter, begin_iter };
			}
		}

		long long loop_index{ 0 };
		for (; begin_iter != end_iter; ++begin_iter, ++begin_data_iter, ++begin_dist_iter)
		{
			if (begin_iter->get_last_item_direction_position<direction>(segment_end_direction, *begin_dist_iter, *begin_data_iter) - max_distance <= position)
			{
				if (begin_iter->get_direction_position(segment_end_direction, *begin_dist_iter) + max_distance >= position) //found matching group
					return { find_closest_item_group_return_result::insert_into_group, begin_iter };
			}
			else if (loop_index + 1ll < vec.size())
			{
				const _vector::iterator tmp = begin_iter + 1ll;
				if (tmp != end_iter)
				{
					_data_vector::iterator tmp_data = begin_data_iter + 1ll;
					_vector_distance::iterator tmp_dist = begin_dist_iter + 1ll;
					if (tmp->get_last_item_direction_position<direction>(segment_end_direction, *tmp_dist, *tmp_data) - max_distance > position)
					{
						if (begin_iter->get_direction_position(segment_end_direction, *begin_dist_iter) + max_distance < position) //if vector is sorted from low to high
							return { find_closest_item_group_return_result::new_group_after_iter, begin_iter };
					}
					if (begin_iter->get_last_item_direction_position<direction>(segment_end_direction, *begin_dist_iter, *begin_data_iter) - max_distance > position)
					{
						if (tmp->get_direction_position(segment_end_direction, *tmp_dist) + max_distance < position) //if vector is sorted from high to low
							return { find_closest_item_group_return_result::new_group_before_iter, tmp };
					}
				}
				++loop_index;
			}
			else
				return { find_closest_item_group_return_result::invalid_value, end_iter };
		}

		return { find_closest_item_group_return_result::invalid_value, end_iter };
	};

	template<belt_utility::belt_direction segment_direction, typename type>
	static inline constexpr belt_utility::inserter_fits_results is_inserter_before_or_after(const type& lhs, const type& rhs)
		requires(type_has_get_distance_position<type>)
	{
		const auto first_inserter_position = lhs.get_distance_position_plus(); //get_direction_position<segment_direction>(lhs.get_position());
		const auto test_inserter_position = rhs.get_distance_position_plus(); //get_direction_position<segment_direction>(rhs.get_position());
		if (first_inserter_position < test_inserter_position) return belt_utility::inserter_fits_results::before;
		else if (first_inserter_position + index_inserter_settings::inserter_grid_size > test_inserter_position) return belt_utility::inserter_fits_results::after;
		else return belt_utility::inserter_fits_results::no_fit;
	};
	template<belt_utility::belt_direction segment_direction, typename type>
	static inline constexpr belt_utility::inserter_fits_results is_inserter_between(const type& lhs, const type& rhs, const type& subject)
		requires(type_has_get_distance_position<type>)
	{
		const auto first_inserter_position = lhs.get_distance_position_plus();// get_direction_position<segment_direction>(lhs.get_position());
		const auto second_inserter_position = rhs.get_distance_position_plus();// get_direction_position<segment_direction>(rhs.get_position());
		const auto subject_inserter_position = subject.get_distance_position_plus();// get_direction_position<segment_direction>(subject.get_position());
		if (first_inserter_position + index_inserter_settings::inserter_grid_size > subject_inserter_position && subject_inserter_position + index_inserter_settings::inserter_grid_size <= second_inserter_position) return belt_utility::inserter_fits_results::inbetween;
		return belt_utility::inserter_fits_results::no_fit;
	};

	constexpr inline long long get_distances_from_to2(_vector_distance::iterator from, const _vector_distance::iterator& to) noexcept
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
	constexpr inline long long get_distances_from_to(_vector_distance::iterator dist_iter, const _vector_distance::iterator& end_dist_iter, const _vector_item_groups_head_type::iterator& iter) noexcept
	{
		long long real_distance = *dist_iter;
		while (dist_iter != end_dist_iter && (*iter).distance != -1ll && *dist_iter != (*iter).distance)
		{
			real_distance += *dist_iter;
			++dist_iter;
		}

		return (*iter).distance - real_distance;
	};
	constexpr inline long long get_item_group_distance_from_destination(_vector_distance& distances, _vector_item_groups_head_type& goal_distances, long long index) noexcept
	{
		auto begin_iter = distances.begin();
		const auto last_iter = distances.last();
		auto begin_goal_iter = goal_distances.begin();
		const auto last_goal_iter = goal_distances.last();

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

			if (count_index == index) return (*begin_goal_iter).distance;

			++begin_goal_iter;
			++begin_iter;
		}

		return -1ll;
	};
	constexpr _vector_item_groups_head_type::iterator get_goal_object_index(long long index, _vector_item_groups_head_type& item_groups_goal_distance, _vector_distance& item_groups_distance_between) noexcept
	{
		if (index == item_groups_distance_between.size() - 1ll) return item_groups_goal_distance.last() - 1ll;

		const auto item_groups_distance_between_begin = item_groups_distance_between.begin();
		const auto last = item_groups_goal_distance.last();
		for (auto begin = item_groups_goal_distance.begin(); begin != last; ++begin)
		{
			if ((*begin).next_item_group_index + 1 >= index) return begin;
		}

		return item_groups_goal_distance.last();
	};
	constexpr _vector_item_groups_head_type::iterator get_goal_object_index_binary(long long index, _vector_item_groups_head_type& item_groups_goal_distance, _vector_distance& item_groups_distance_between) noexcept
	{
		if (index == item_groups_distance_between.size() - 1ll) return item_groups_goal_distance.last() - 1ll;

		auto left_goal_iter = item_groups_goal_distance.begin();
		auto right_goal_iter = item_groups_goal_distance.last() - 1ll;
		auto half_size = item_groups_goal_distance.size();
		//const auto between_begin_ptr = item_groups_distance_between.begin().operator->();

		while (half_size > 1ll)
		{
			half_size = expr::ceil_div_power2(right_goal_iter - left_goal_iter);
			auto temp_half_goal_iter = left_goal_iter + half_size;

			if ((*temp_half_goal_iter).next_item_group_index + 1 > index) right_goal_iter = temp_half_goal_iter;
			else left_goal_iter = temp_half_goal_iter;
		}

		if ((*left_goal_iter).next_item_group_index + 1 >= index) return left_goal_iter;
		else return right_goal_iter;
	};
	constexpr _vector_item_groups_head_type::iterator find_which_goal_object_position_belongs_too(long long position, _vector_item_groups_head_type& item_groups_goal_distance) noexcept
	{
		if (item_groups_goal_distance.size() > 1ll)
		{
			const auto last = item_groups_goal_distance.last();
			for (auto biter = item_groups_goal_distance.begin(), next = biter + 1ll; biter != last; ++biter)
			{
				if (biter->distance < position && next->distance > position) return biter;
				if (biter->distance > position && next->distance < position) return next;
			}
		}
		else if (!item_groups_goal_distance.empty())
		{
			if (item_groups_goal_distance[0].distance >= position) return item_groups_goal_distance.begin();
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