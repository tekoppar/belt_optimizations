#pragma once

#include <type_traits>
#include <utility>

#include "const_data.h"
#include "belt_segment_shared.h"
#include "belt_segment_shared.inl"

#include "vectors.h"
#include "single_list.h"
#include "single_list_block.h"
#include "item.h"
#include "item_32.h"
#include "item_256.h"

class inserter
{
	vec2_uint position{ 0, 0 };
	mem::single_list_block_node<item_groups_type>* item_group{ nullptr };
	item_uint item;
	item_type item_need_types[8]{ item_type::pink_square, item_type::pink_square, item_type::pink_square, item_type::pink_square,
	item_type::pink_square, item_type::pink_square, item_type::pink_square, item_type::pink_square };
	bool is_sleeping{ false };
	short sleep_timer{ 32 * 32 };

public:
	constexpr inserter() noexcept
	{};
	constexpr inserter(vec2_uint pos) noexcept :
		position{ pos }
	{};

	constexpr inserter(const inserter& o) noexcept :
		position{ o.position },
		item{ o.item },
		is_sleeping{ o.is_sleeping },
		sleep_timer{ o.sleep_timer }
	{
		if (std::is_constant_evaluated())
		{
			for (int i = 0, l = 8; i < l; ++i)
			{
				item_need_types[i] = o.item_need_types[i];
			}
		}
		else
		{
			std::memcpy(item_need_types, o.item_need_types, 16);
		}
	};
	constexpr inserter(inserter&& o) noexcept :
		position{ std::exchange(o.position, vec2_uint{}) },
		item_group{ std::exchange(o.item_group, nullptr) },
		item{ std::exchange(o.item, item_uint{}) },
		is_sleeping{ std::exchange(o.is_sleeping, false) },
		sleep_timer{ std::exchange(o.sleep_timer, 1024) }
	{
		if (std::is_constant_evaluated())
		{
			for (int i = 0, l = 8; i < l; ++i)
			{
				item_need_types[i] = o.item_need_types[i];
			}
		}
		else
		{
			std::memcpy(item_need_types, o.item_need_types, 16);
		}
	};
	constexpr inserter& operator=(const inserter& o) noexcept
	{
		position = o.position;
		item = o.item;
		is_sleeping = o.is_sleeping;
		sleep_timer = o.sleep_timer;

		if (std::is_constant_evaluated())
		{
			for (int i = 0, l = 8; i < l; ++i)
			{
				item_need_types[i] = o.item_need_types[i];
			}
		}
		else
		{
			std::memcpy(item_need_types, o.item_need_types, 16);
		}

		return *this;
	};
	constexpr inserter& operator=(inserter&& o) noexcept
	{
		position = std::exchange(o.position, vec2_uint{});
		item_group = std::exchange(o.item_group, nullptr);
		item = std::exchange(o.item, item_uint{});
		is_sleeping = std::exchange(o.is_sleeping, false);
		sleep_timer = std::exchange(o.sleep_timer, 1024);

		if (std::is_constant_evaluated())
		{
			for (int i = 0, l = 8; i < l; ++i)
			{
				item_need_types[i] = o.item_need_types[i];
			}
		}
		else
		{
			std::memcpy(item_need_types, o.item_need_types, 16);
		}

		return *this;
	};

	constexpr bool get_is_sleeping() const noexcept
	{
		return is_sleeping;
	};
	constexpr void set_is_sleeping(bool val) noexcept
	{
		is_sleeping = val;
	};
	constexpr void reset_sleep_timer() noexcept
	{
		sleep_timer = 1024;
	};
	constexpr vec2_uint get_position() const noexcept
	{
		return position;
	};
	constexpr item_type get_item_type(short index) const noexcept
	{
		return item_need_types[index];
	};
	constexpr void set_item_type(item_type type) noexcept
	{
		item_need_types[0] = type;
	};
	constexpr void set_item_types(item_type types[8]) noexcept
	{
		if (std::is_constant_evaluated())
		{
			for (int i = 0, l = 8; i < l; ++i)
			{
				item_need_types[i] = types[i];
			}
		}
		else
		{
			std::memcpy(item_need_types, types, 16);
		}
	};
	constexpr bool check_if_need_type(item_type type) const noexcept
	{
		for (int i = 0, l = 8; i < l; ++i)
		{
			if (item_need_types[i] == type) return true;
		}

		return false;
	};
	constexpr bool has_linked_list_data() const noexcept
	{
		return item_group != nullptr;
	}
	constexpr mem::single_list_block_node<item_groups_type>* get_linked_list_data() noexcept
	{
		return item_group;
	};
	constexpr void set_linked_list_data(mem::single_list_block_node<item_groups_type>* ptr) noexcept
	{
		item_group = ptr;
	};
	constexpr void grab_item(const item_uint& _item) noexcept
	{
		item = _item;
	};
	constexpr item_uint& get_item() noexcept
	{
		return item;
	};

	constexpr bool sleep_update() noexcept
	{
		--sleep_timer;
		is_sleeping = sleep_timer == 0;
		return is_sleeping;
	};
	constexpr void update(belt_utility::belt_direction direction, long long segment_end_direction, long long segment_y_direction, belt_segment* segment_ptr) noexcept
	{
		if (item_group)
		{
			long long last_item_position = 0;
			while (true)
			{
				if (item_group->object.get_position(segment_end_direction, segment_y_direction).x < position.x)
				{
					item_group = nullptr;
					return;
				}

				last_item_position = item_group->object.get_last_item_direction_position(direction, segment_end_direction);
				if (last_item_position > position.x)
				{
					if (item_group->prev)
					{
						item_group = item_group->prev;
						break;
					}
					else
					{
						item_group = nullptr;
						return;
					}
				}
				else
					break;
			}

			if (last_item_position <= position.x)
			{
				auto found_index = item_group->object.get_first_item_of_type(get_item_type(0));
				if (found_index != -1 && item_group->object.get_item_direction_position(direction, found_index) == position.x)
				{
					grab_item(item_group->object.get(found_index));
					item_group->object.remove_item(found_index);
				}
			}
		}
	};
};