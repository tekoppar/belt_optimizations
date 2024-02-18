#pragma once

#include <type_traits>
#include <utility>

#include "const_data.h"

#include "vectors.h"
#include "index_iterator.h"
#include "item.h"
#include "item_32.h"
#include "item_256.h"

class index_inserter
{
	static inline std::size_t bad_calls{ 0 };
	static inline std::size_t good_calls{ 0 };
	static inline std::size_t good_bad_calls{ 0 };
	static inline std::size_t missed_clearing_item_groups{ 0 };
	static inline std::size_t wrong_item_group{ 0 };
	static inline std::size_t wrong_grab_calls{ 0 };
	static inline std::size_t good_grab_calls{ 0 };
public:
	static inline constexpr long long inserter_grid_size{ 32ll };

private:
	vec2_uint position{ 0, 0 };
	index_iterator<item_groups_type, _vector> item_group{ 0ull, nullptr };
	long long distance_shift_item_group{ 0 };
	item_group_scan active_mode = item_group_scan::no_close;
	short sleep_timer{ 32 * 32 };
	bool is_sleeping{ false };
	item_type item_need_types[8]{ item_type::pink_square, item_type::pink_square, item_type::pink_square, item_type::pink_square,
	item_type::pink_square, item_type::pink_square, item_type::pink_square, item_type::pink_square };
	__declspec(align(32)) item_uint item;

public:
	constexpr index_inserter() noexcept
	{};
	constexpr index_inserter(vec2_uint pos) noexcept :
		position{ pos }
	{};
	constexpr ~index_inserter() noexcept
	{};

	constexpr index_inserter(const index_inserter& o) noexcept :
		position{ o.position },
		item_group{ o.item_group },
		sleep_timer{ o.sleep_timer },
		is_sleeping{ o.is_sleeping },
		item{ o.item }
	{
		if (std::is_constant_evaluated())
		{
			constexpr long long l = 8;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = o.item_need_types[i];
			}
		}
		else
		{
			std::memcpy(&item_need_types[0], &o.item_need_types[0], 16);
		}
	};
	constexpr index_inserter(index_inserter&& o) noexcept :
		position{ std::exchange(o.position, vec2_uint{}) },
		item_group{ std::exchange(o.item_group, {0ull, nullptr}) },
		sleep_timer{ std::exchange(o.sleep_timer, 1024) },
		is_sleeping{ std::exchange(o.is_sleeping, false) },
		item{ std::exchange(o.item, item_uint{}) }
	{
		if (std::is_constant_evaluated())
		{
			constexpr long long l = 8;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = o.item_need_types[i];
			}
		}
		else
		{
			std::memcpy(&item_need_types[0], &o.item_need_types[0], 16);
		}
	};
	constexpr index_inserter& operator=(const index_inserter& o) noexcept
	{
		position = o.position;
		item_group = o.item_group;
		sleep_timer = o.sleep_timer;
		is_sleeping = o.is_sleeping;
		item = o.item;

		if (std::is_constant_evaluated())
		{
			constexpr long long l = 8;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = o.item_need_types[i];
			}
		}
		else
		{
			std::memcpy(&item_need_types[0], &o.item_need_types[0], 16);
		}

		return *this;
	};
	constexpr index_inserter& operator=(index_inserter&& o) noexcept
	{
		position = std::exchange(o.position, vec2_uint{});
		item_group = std::exchange(o.item_group, { 0ull, nullptr });
		sleep_timer = std::exchange(o.sleep_timer, 1024);
		is_sleeping = std::exchange(o.is_sleeping, false);
		item = std::exchange(o.item, item_uint{});

		if (std::is_constant_evaluated())
		{
			constexpr long long l = 8;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = std::move(o.item_need_types[i]);
			}
		}
		else
		{
			std::memcpy(&item_need_types[0], &o.item_need_types[0], 16);
		}

		return *this;
	};

	inline constexpr bool get_is_sleeping() const noexcept
	{
		return is_sleeping;
	};
	inline constexpr void set_is_sleeping(bool val) noexcept
	{
		is_sleeping = val;
	};
	inline constexpr void reset_sleep_timer() noexcept
	{
		sleep_timer = 1024;
	};
	inline constexpr vec2_uint get_position() const noexcept
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
		else
		{
			std::memcpy(&item_need_types[0], &types[0], 16);
		}
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
	inline constexpr bool has_linked_list_data() const noexcept
	{
		return item_group != nullptr;
	};
	inline constexpr index_iterator<item_groups_type, _vector>& get_linked_list_data() noexcept
	{
		return item_group;
	};
	constexpr void set_linked_list_data(const index_iterator<item_groups_type, _vector>& iter) noexcept
	{
		if (std::is_constant_evaluated() == false && !iter.vector_empty() && (*iter).count() > 0) ++missed_clearing_item_groups;
		item_group = iter;
		set_distance_shift_item_group();
	};
	inline constexpr void grab_item(const item_uint& _item) noexcept
	{
		item = _item;
	};
	inline constexpr item_uint& get_item() noexcept
	{
		return item;
	};
	constexpr void set_distance_shift_item_group() noexcept
	{
		if (item_group)
		{
			const long long tmp = position.x - item_group.operator*().get_last_item_direction_position();
			if (tmp > -0) distance_shift_item_group = tmp;
			else if (std::is_constant_evaluated() == false)
			{
				++wrong_item_group;
				distance_shift_item_group = 0;
			}
		}
	};

	constexpr item_group_scan scan_for_closest_group(long long item_group_offset) noexcept
	{
		if (item_group_offset > 0)
		{
			//if (distance_shift_item_group == 0) ++item_group_offset;
			if (item_group.is_prev_valid()) //TODO replace with is offset_valid
			{
				if ((*(item_group - item_group_offset)).get_last_item_direction_position() <= position.x && (*(item_group - item_group_offset)).get_direction_position() >= position.x)
				{
					set_linked_list_data(item_group - item_group_offset);
					return item_group_scan::same; //means were in between the item group
				}
				if (0 < (position.x - (*(item_group - item_group_offset)).get_direction_position()) && (position.x - (*(item_group - item_group_offset)).get_direction_position()) < 255ll && (*(item_group - item_group_offset)).count() > 0)
				{
					set_linked_list_data(item_group - item_group_offset);
					return item_group_scan::same;
				}
				if ((item_group - item_group_offset).is_next_valid() && (*(item_group - item_group_offset + 1)).get_direction_position() > position.x && (*(item_group - item_group_offset + 1)).get_last_item_direction_position() > position.x &&
					(*(item_group - item_group_offset)).get_direction_position() < position.x && (*(item_group - item_group_offset)).get_last_item_direction_position() < position.x)
				{
					set_linked_list_data(item_group - item_group_offset);
					return item_group_scan::same;
				}
			}
			if (std::is_constant_evaluated() == false) ++good_bad_calls;
		}

		if ((*item_group).get_last_item_direction_position() <= position.x && (*item_group).get_direction_position() >= position.x) return item_group_scan::same; //means were in between the current item group
		if (0 < (position.x - (*item_group).get_direction_position()) && (position.x - (*item_group).get_direction_position()) < 255ll && (*item_group).count() > 0) return item_group_scan::same;
		if (item_group.is_next_valid() && (*(item_group + 1)).get_direction_position() > position.x && (*(item_group + 1)).get_last_item_direction_position() > position.x &&
			(*item_group).get_direction_position() < position.x && (*item_group).get_last_item_direction_position() < position.x)
		{
			return item_group_scan::same;
		}

		decltype(item_group) next_iter{ item_group };
		decltype(item_group) prev_iter{ item_group };

		if (next_iter.is_next_valid() && (*(next_iter + 1ull)).get_direction_position() >= position.x && (*(next_iter + 1ull)).get_last_item_direction_position() <= position.x)
		{
			set_linked_list_data(next_iter + 1ull);
			return item_group_scan::found_next;
		}
		if (prev_iter.is_prev_valid() && (*(prev_iter - 1)).get_direction_position() <= position.x && (*(prev_iter - 1)).get_last_item_direction_position() <= position.x &&
			(*prev_iter).get_direction_position() > position.x && (*prev_iter).get_last_item_direction_position() > position.x)
		{
			set_linked_list_data(prev_iter - 1);
			return item_group_scan::found_prev;
		}
		/*if (prev_iter.is_prev_valid() && (prev_iter - 2).is_prev_valid() && (*(prev_iter - 2)).get_direction_position() <= position.x && (*(prev_iter - 2)).get_last_item_direction_position() <= position.x &&
			(*prev_iter).get_direction_position() > position.x && (*prev_iter).get_last_item_direction_position() > position.x)
		{
			item_group = prev_iter - 2;
			return item_group_scan::found_prev;
		}*/

		if ((*prev_iter).get_last_item_direction_position() > position.x) //scan backwards
			//if (prev_iter.is_prev_valid() && (*(prev_iter - 1)).get_last_item_direction_position() > position.x) //i = inserter, g = item groups, _ = belt g__g__g__i__ 
		{
			//std::size_t iter_offset_count{ 0ull };
			auto current_index = prev_iter.get_index();
			for (; current_index > 0ull; --current_index)
			{
				--prev_iter;
				//++iter_offset_count;
				//if (std::is_constant_evaluated() == false && iter_offset_count > 10000) __debugbreak();
				if ((*prev_iter).get_last_item_direction_position() <= position.x && (*prev_iter).get_direction_position() >= position.x) //found a new group backwards that the inserter is between
				{
					set_linked_list_data(prev_iter);
					return item_group_scan::found_prev;
				}
				else if ((*prev_iter).get_direction_position() <= position.x) //all item groups from this index is in front of the inserter
				{
					set_linked_list_data(prev_iter);
					return item_group_scan::no_close_backwards;
				}
			}

			set_linked_list_data(index_iterator{ 0ull, item_group });
			return item_group_scan::no_close;
		}
		if ((*next_iter).get_direction_position() < position.x) //scan forward
			//if (next_iter.is_next_valid() && (*(next_iter + 1)).get_direction_position() < position.x) //i = inserter, g = item groups, _ = belt __i__g__g__g
		{
			//std::size_t iter_offset_count{ 0ull };
			auto vector_size = next_iter.get_vector_size();
			auto current_index = next_iter.get_index();
			for (; current_index < vector_size; ++current_index)
			{
				prev_iter = next_iter;
				++next_iter;
				//++iter_offset_count;
				//if (std::is_constant_evaluated() == false && iter_offset_count > 20000) __debugbreak();
				if ((*next_iter).get_last_item_direction_position() <= position.x && (*next_iter).get_direction_position() >= position.x) //found a new group forwards that the inserter is between
				{
					set_linked_list_data(next_iter);
					return item_group_scan::found_next;
				}
				else if ((*prev_iter).get_last_item_direction_position() <= position.x && (*next_iter).get_direction_position() >= position.x)
				{
					set_linked_list_data(prev_iter);
					return item_group_scan::found_next;
				}
			}

			set_linked_list_data(index_iterator{ item_group, last_index_iterator });
			return item_group_scan::no_close;
		}

		return item_group_scan::no_close;
	};

	constexpr bool sleep_update() noexcept
	{
		if (item_group_scan::no_close == active_mode)
		{
			--sleep_timer;
			is_sleeping = sleep_timer == 0;
		}

		return is_sleeping;
	};
	__declspec(noinline) constexpr void update(long long item_group_offset) noexcept
	{
		if (item_group)
		{
			if (item_group_scan::same == active_mode)
			{
				if (distance_shift_item_group <= 0)
				{
					active_mode = scan_for_closest_group(item_group_offset);
					if (std::is_constant_evaluated() == false) ++good_calls;
				}
			}
			else
			{
				if (std::is_constant_evaluated() == false) ++bad_calls;
				active_mode = scan_for_closest_group(item_group_offset);
			}

			//if (std::is_constant_evaluated() == false) ++bad_calls;
			//active_mode = scan_for_closest_group(item_group_offset);

			--distance_shift_item_group;
			if (item_group_scan::no_close != active_mode)
			{
				const auto found_index = (*item_group).get_first_item_of_type_before_position(get_item_type(0), position);
				if (found_index != -1 && position.x == (*item_group).get_item_direction_position(found_index))
				{
					grab_item((*item_group).get(found_index));
					(*item_group).remove_item(found_index);
					if (std::is_constant_evaluated() == false) ++good_grab_calls;
				}
				else
				{
					if (std::is_constant_evaluated() == false) ++wrong_grab_calls;
				}
			}
			else
			{
				sleep_timer = 1024;
			}
		}
	};
};