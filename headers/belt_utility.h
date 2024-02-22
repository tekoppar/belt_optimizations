#pragma once

#include <vector>
#include "mem_vector.h"

#include "vectors.h"
#include "item_32.h"

#include "type_conversion.h"
#include "belt_utility_concepts.h"
#include "belt_utility_data.h"

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

	//template<long long start_x, long long start_y, long long end_x, long long end_y>
	static consteval belt_direction direction_from_positions(vec2_uint start, vec2_uint end) noexcept
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
	static constexpr long long get_direction_position(vec2_uint pos) noexcept
	{
		if constexpr (belt_direction::left_right == direction) return pos.x;
		if constexpr (belt_direction::right_left == direction) return pos.x;
		if constexpr (belt_direction::top_bottom == direction) return pos.y;
		if constexpr (belt_direction::bottom_top == direction) return pos.y;

		return 0ll;
	};
	inline static constexpr long long get_direction_position(belt_direction direction, vec2_uint pos) noexcept
	{
		switch (direction)
		{
		default:
		case belt_direction::null:
		case belt_direction::left_right: return pos.x;
		case belt_direction::right_left: return pos.x;
		case belt_direction::top_bottom: return pos.y;
		case belt_direction::bottom_top: return pos.y;
		}
	};

	template<typename data_vector, typename vector, typename compare_position, typename vector_iterator = vector::iterator>
	static constexpr find_closest_item_group_result<vector_iterator> find_closest_item_group(belt_utility::belt_direction direction, long long segment_end_direction, data_vector& data_vec, vector& vec, const compare_position& position) noexcept
		requires(element_has_member<compare_position>&& class_has_iterator<vector>)
	{
		if (vec.empty()) return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::invalid_value, vector_iterator{} };

		auto last_iter = vec.last() - 1;
		auto last_data_iter = data_vec.last() - 1;
		const auto dir_pos_last_iter = last_iter->get_direction_position(segment_end_direction);
		if (position.x > dir_pos_last_iter + 255ll) return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::new_group_after_iter, last_iter };
		else if (position.x > dir_pos_last_iter)
		{
			if (last_iter->count(&last_data_iter->contains_item[0]) < 32) return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::insert_into_group, last_iter };
			else return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::new_group_after_iter, last_iter };
		}

		auto begin_iter = vec.begin();
		auto begin_data_iter = data_vec.begin();
		{
			const auto last_dir_pos_begin_iter = begin_iter->get_last_item_direction_position(direction, segment_end_direction, *begin_data_iter);
			if (position.x < last_dir_pos_begin_iter - 255ll) return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::new_group_before_iter, begin_iter };
			else if (position.x < last_dir_pos_begin_iter)
			{
				if (begin_iter->count(&begin_data_iter->contains_item[0]) < 32) return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::insert_into_group, begin_iter };
				else return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::new_group_before_iter, begin_iter };
			}
		}

		long long loop_index{ 0 };
		auto end_iter = vec.last();
		for (; begin_iter != end_iter; ++begin_iter, ++begin_data_iter)
		{
			if (begin_iter->get_last_item_direction_position(direction, segment_end_direction, *begin_data_iter) - 255ll <= position.x && begin_iter->get_direction_position(segment_end_direction) + 255ll >= position.x) //found matching group
				return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::insert_into_group, begin_iter };
			else if (loop_index + 1ll < vec.size())
			{
				auto tmp = begin_iter + 1ll;
				if (tmp != end_iter)
				{
					auto tmp_data = begin_data_iter + 1ll;
					if (tmp->get_last_item_direction_position(direction, segment_end_direction, *tmp_data) - 255ll > position.x && begin_iter->get_direction_position(segment_end_direction) + 255ll < position.x) //if vector is sorted from low to high
						return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::new_group_after_iter, begin_iter };
					if (begin_iter->get_last_item_direction_position(direction, segment_end_direction, *begin_data_iter) - 255ll > position.x && tmp->get_direction_position(segment_end_direction) + 255ll < position.x) //if vector is sorted from high to low
						return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::new_group_before_iter, tmp };
				}
				//begin_iter = tmp;
				++loop_index;
			}
			else return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::invalid_value, end_iter };
		}

		return find_closest_item_group_result<vector_iterator>{ find_closest_item_group_return_result::invalid_value, end_iter };
	};
	template<typename data_vector>
	static constexpr auto find_closest_item_group(belt_utility::belt_direction direction, long long segment_end_direction, data_vector& data_vec, std::vector<item_32>& vec, const vec2_uint& position) noexcept
	{
		if (vec.empty()) return belt_utility::find_closest_item_group_result<std::vector<item_32>::iterator>{ find_closest_item_group_return_result::invalid_value, vec.end() };

		auto last_iter = vec.end() - 1;
		auto last_data_iter = data_vec.last() - 1;
		const auto dir_pos_last_iter = last_iter->get_direction_position(segment_end_direction);
		if (position.x > dir_pos_last_iter + 255ll) return belt_utility::find_closest_item_group_result<std::vector<item_32>::iterator>{ find_closest_item_group_return_result::new_group_after_iter, last_iter };
		else if (position.x > dir_pos_last_iter)
		{
			if (last_iter->count(last_data_iter->contains_item) < 32) return belt_utility::find_closest_item_group_result<std::vector<item_32>::iterator>{ find_closest_item_group_return_result::insert_into_group, last_iter };
			else return belt_utility::find_closest_item_group_result<std::vector<item_32>::iterator>{ find_closest_item_group_return_result::new_group_after_iter, last_iter };
		}

		auto begin_iter = vec.begin();
		auto begin_data_iter = data_vec.begin();
		//{
		const auto last_dir_pos_begin_iter = begin_iter->get_last_item_direction_position(direction, segment_end_direction, *begin_data_iter);
		if (position.x < last_dir_pos_begin_iter - 255ll) return belt_utility::find_closest_item_group_result<std::vector<item_32>::iterator>{ find_closest_item_group_return_result::new_group_before_iter, begin_iter };
		else if (position.x < last_dir_pos_begin_iter)
		{
			if (begin_iter->count(begin_data_iter->contains_item) < 32) return belt_utility::find_closest_item_group_result<std::vector<item_32>::iterator>{ find_closest_item_group_return_result::insert_into_group, begin_iter };
			else return belt_utility::find_closest_item_group_result<std::vector<item_32>::iterator>{ find_closest_item_group_return_result::new_group_before_iter, begin_iter };
		}
		//}

		std::size_t loop_index{ 0 };
		auto end_iter = vec.end();
		for (; begin_iter != end_iter; ++begin_iter, ++begin_data_iter)
		{
			if (begin_iter->get_last_item_direction_position(direction, segment_end_direction, *begin_data_iter) - 255ll <= position.x && begin_iter->get_direction_position(segment_end_direction) + 255ll >= position.x) //found matching group
				return belt_utility::find_closest_item_group_result<std::vector<item_32>::iterator>{ find_closest_item_group_return_result::insert_into_group, begin_iter };
			else if (loop_index + 1ull < vec.size())
			{
				auto tmp = begin_iter + 1ull;
				if (tmp != end_iter)
				{
					auto tmp_data = begin_data_iter + 1ll;
					if (tmp->get_last_item_direction_position(direction, segment_end_direction, *tmp_data) - 255ll > position.x && begin_iter->get_direction_position(segment_end_direction) + 255ll < position.x) //if vector is sorted from low to high
						return belt_utility::find_closest_item_group_result<std::vector<item_32>::iterator>{ find_closest_item_group_return_result::new_group_after_iter, begin_iter };
					if (begin_iter->get_last_item_direction_position(direction, segment_end_direction, *begin_data_iter) - 255ll > position.x && tmp->get_direction_position(segment_end_direction) + 255ll < position.x) //if vector is sorted from high to low
						return belt_utility::find_closest_item_group_result<std::vector<item_32>::iterator>{ find_closest_item_group_return_result::new_group_before_iter, tmp };
				}
				//begin_iter = tmp;
				++loop_index;
			}
			else
			{
				return belt_utility::find_closest_item_group_result<std::vector<item_32>::iterator>{ find_closest_item_group_return_result::invalid_value, end_iter };
			}
		}

		return belt_utility::find_closest_item_group_result<std::vector<item_32>::iterator>{ find_closest_item_group_return_result::invalid_value, end_iter };
	};

	template<typename type>
	static inline constexpr belt_utility::inserter_fits_results check_if_inserter_is_before_or_after(belt_utility::belt_direction segment_direction, const type& lhs, const type& rhs)
		requires(type_has_grid_size<type>&& type_has_get_position<type>)
	{
		const auto first_inserter_position = get_direction_position(segment_direction, lhs.get_position());
		const auto test_inserter_position = get_direction_position(segment_direction, rhs.get_position());
		if (first_inserter_position > test_inserter_position) return belt_utility::inserter_fits_results::before;
		else if (first_inserter_position + type::inserter_grid_size < test_inserter_position) return belt_utility::inserter_fits_results::after;
		else return belt_utility::inserter_fits_results::no_fit;
	};
	template<typename type>
	static inline constexpr belt_utility::inserter_fits_results check_if_inserter_is_between(belt_utility::belt_direction segment_direction, const type& lhs, const type& rhs, const type& subject)
		requires(type_has_grid_size<type>&& type_has_get_position<type>)
	{
		const auto first_inserter_position = get_direction_position(segment_direction, lhs.get_position());
		const auto second_inserter_position = get_direction_position(segment_direction, rhs.get_position());
		const auto subject_inserter_position = get_direction_position(segment_direction, subject.get_position());
		if (first_inserter_position + type::inserter_grid_size > subject_inserter_position && subject_inserter_position + type::inserter_grid_size < second_inserter_position) return belt_utility::inserter_fits_results::inbetween;
		return belt_utility::inserter_fits_results::no_fit;
	};

	template<typename vector, typename items_group, typename items_group_data, typename vector_iterator = vector::iterator>
	static constexpr find_closest_active_mode_result<vector_iterator> find_closest_active_mode(belt_utility::belt_direction direction, long long segment_end_direction, vector& vec, items_group* ptr, items_group_data& item_data) noexcept
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