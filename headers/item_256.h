#pragma once

#include <array>

#include <immintrin.h>

#include "item.h"
#include "vectors.h"
#include "belt_utility.h"

#define optimization_comp

/*
* See item_32.h for information regarding how the class works. Since item_256 is just 8 arrays of item_32
*/

class item_256
{
	constexpr static short single_belt_length = 128;
	constexpr static short belt_length = 128 * 8;
	constexpr static unsigned char belt_item_size = 32;
	constexpr static unsigned char max_distance_between_items = 255;
public:
	inline static std::size_t items_moved_per_frame = 0;

private:
	long long item_position_y[8]{ 0, 0, 0, 0, 0, 0, 0, 0 };
	long long item_position_x[8]{ 0, 0, 0, 0, 0, 0, 0, 0 };
	std::size_t item_goal_distance[8]{ 0, 0, 0, 0, 0, 0, 0, 0 };
	const belt_utility::belt_direction direction[8]{
		belt_utility::belt_direction::left_right, belt_utility::belt_direction::left_right, belt_utility::belt_direction::left_right, belt_utility::belt_direction::left_right,
		belt_utility::belt_direction::left_right, belt_utility::belt_direction::left_right, belt_utility::belt_direction::left_right, belt_utility::belt_direction::left_right
	};
	unsigned char item_count[8]{ 0, 0, 0, 0, 0, 0, 0, 0 };
	bool is_stuck[8]{ false, false, false, false, false, false, false, false };
	__declspec(align(32)) bool contains_item[8][32]
	{
		{
			false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
			false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false
		},
		{
			false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
			false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false
		},
		{
			false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
			false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false
		},
		{
			false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
			false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false
		},
		{
			false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
			false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false
		},
		{
			false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
			false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false
		},
		{
			false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
			false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false
		},
		{
			false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
			false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false
		}
	};
	__declspec(align(32)) unsigned char item_distance[8][32]{
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		},
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		},
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		},
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		},
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		},
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		},
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		},
		{
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
		}
	};
	__declspec(align(64)) belt_item items[8][32];

	constexpr static belt_utility::belt_color color{ belt_utility::belt_color::gray };
public:
	constexpr item_256() = default;
	constexpr item_256(const belt_utility::belt_direction& dir) noexcept :
		direction{ dir, dir, dir, dir, dir, dir, dir, dir }
	{};
	constexpr item_256(const belt_utility::belt_direction& dir, std::size_t goal_position) noexcept :
		direction{ dir, dir, dir, dir, dir, dir, dir, dir },
		item_goal_distance{ goal_position, goal_position, goal_position, goal_position, goal_position, goal_position, goal_position, goal_position }
	{};

	constexpr item_uint get(std::size_t arr_index, std::size_t i) noexcept
	{
		return { items[arr_index][i].type, vec2_uint{item_position_x[arr_index] - get_distance_to_item(arr_index, i), item_position_y[arr_index]} };
	};
	constexpr const item_uint& get(std::size_t arr_index, std::size_t i) const noexcept
	{
		return { items[arr_index][i].type, vec2_uint{item_position_x[arr_index] - get_distance_to_item(arr_index, i), item_position_y[arr_index]} };
	};

	constexpr std::size_t get_goal(std::size_t arr_index) const noexcept
	{
		return item_goal_distance[arr_index];
	};
	constexpr short count(std::size_t arr_index) const noexcept
	{
		return item_count[arr_index];
	};

	constexpr std::size_t get_distance_to_last_item(std::size_t arr_index) const noexcept
	{
		if (item_count[arr_index] == 0) return 0;
		std::size_t total_distance = 0;
		for (int i = 0, l = item_count[arr_index]; i < l; ++i)
		{
			total_distance += static_cast<std::size_t>(item_distance[arr_index][i]);
		}

		return total_distance;
	};
	constexpr long long get_distance_to_item(std::size_t arr_index, std::size_t item_index) const noexcept
	{
		if (item_index > item_count[arr_index]) return 0;
		long long total_distance = 0;
		for (int i = 0, l = item_index + 1; i < l; ++i)
		{
			total_distance += static_cast<long long>(item_distance[arr_index][i]);
		}

		return total_distance;
	};

	constexpr bool add_item(const belt_item& new_item, vec2_uint new_item_position, std::size_t arr_index) noexcept
	{
		if (item_count[arr_index] == 0)
		{
			item_position_x[arr_index] = new_item_position.x;
			item_position_y[arr_index] = new_item_position.y;
			item_goal_distance[arr_index] -= new_item_position.x;
			contains_item[arr_index][item_count[arr_index]] = true;
			items[arr_index][item_count[arr_index]] = new_item;
			item_distance[arr_index][item_count[arr_index]] = static_cast<unsigned char>(0);
			++item_count[arr_index];
			return true;
		}
		if (new_item_position.x > item_position_x[arr_index] && new_item_position.x - item_position_x[arr_index] < 255)
		{
			std::size_t new_distance = new_item_position.x - item_position_x[arr_index];
			item_distance[arr_index][0] = static_cast<unsigned char>(new_distance);
			item_goal_distance[arr_index] -= new_distance;
			shift_arrays_left(arr_index);
			item_position_x[arr_index] = new_item_position.x;
			item_position_y[arr_index] = new_item_position.y;
			contains_item[arr_index][0] = true;
			items[arr_index][0] = new_item;
			item_distance[arr_index][0] = static_cast<unsigned char>(0);

			++item_count[arr_index];
			return true;
		}

		const long long l = static_cast<long long>(count(arr_index)) - 1ll;
		for (long long i = 0; i < l; ++i)
		{
			if (new_item_position.x < get_distance_to_item(arr_index, i) && item_distance[arr_index][i + 1] > 63)
			{
				shift_arrays_left_from_index(arr_index, i + 1);
				contains_item[arr_index][i + 1] = true;
				items[arr_index][i + 1] = new_item;
				item_distance[arr_index][i + 1] = static_cast<unsigned char>(new_item_position.x - get_distance_to_item(arr_index, i));
				++item_count[arr_index];
				return true;
			}
		}

		long long distance_to_last_item = (new_item_position.x - static_cast<long long>(get_distance_to_last_item(arr_index)));
		if (distance_to_last_item < 255)
		{
			contains_item[arr_index][item_count[arr_index]] = true;
			items[arr_index][item_count[arr_index]] = new_item;
			item_distance[arr_index][item_count[arr_index]] = static_cast<unsigned char>(distance_to_last_item);
			++item_count[arr_index];
			return true;
		}
		return false;
	};
	constexpr bool remove_item(std::size_t arr_index, const short index) noexcept
	{
		std::size_t new_goal_distance = 0;
		if (item_count[arr_index] > 1)
		{
			new_goal_distance = get_distance_to_item(arr_index, 1);
			item_goal_distance[arr_index] = new_goal_distance;
			item_position_x[arr_index] -= new_goal_distance;
			shift_arrays_right(arr_index);
			item_distance[arr_index][0] = 0;
			--item_count[arr_index];
		}
		else
		{
			--item_count[arr_index];
			contains_item[arr_index][index] = false;
			item_distance[arr_index][index] = 0;
			items[arr_index][index] = belt_item{};
			item_goal_distance[arr_index] = 0;
			item_position_x[arr_index] = 0;

			//TODO at this point the item_256 should be removed since it no longer contains any items nor a goal
		}

		return true;
	};
	constexpr void recalculate_distance(const vec2_uint& old_position, const vec2_uint new_position, std::size_t arr_index) noexcept
	{
		std::size_t new_distance = new_position.x - old_position.x;
		for (int i = 0; i < item_count[arr_index]; ++i)
		{
			item_distance[arr_index][i] = item_distance[arr_index][i] + new_distance;
		}
	};
	constexpr void shift_arrays_left(std::size_t arr_index) noexcept
	{
		if (std::is_constant_evaluated())
		{
			for (int i = item_count[arr_index]; i >= 0; --i)
			{
				contains_item[arr_index][i + 1 % belt_item_size] = contains_item[arr_index][i];
				item_distance[arr_index][i + 1 % belt_item_size] = item_distance[arr_index][i];
				items[arr_index][i + 1 % belt_item_size] = items[arr_index][i];
			}
		}
		else
		{
			belt_utility::_mm256_slli_si256__p<1>((__m256i*)&contains_item[arr_index]);
			belt_utility::_mm256_slli_si256__p<1>((__m256i*)&item_distance[arr_index]);
			belt_utility::_mm512_slli2x256_si512__<2>((__m256i*)&items[arr_index]);
		}
	};
	constexpr void shift_arrays_left_from_index(std::size_t arr_index, std::size_t index) noexcept
	{
		for (int i = item_count[arr_index]; i >= index; --i)
		{
			contains_item[arr_index][i + 1 % belt_item_size] = contains_item[arr_index][i];
			item_distance[arr_index][i + 1 % belt_item_size] = item_distance[arr_index][i];
			items[arr_index][i + 1 % belt_item_size] = items[arr_index][i];
		}
	};
	constexpr void shift_arrays_right(std::size_t arr_index) noexcept
	{
		if (std::is_constant_evaluated())
		{
			belt_item previous_i_x = items[arr_index][item_count[arr_index] - 1], current_i_x = {};
			bool previous_b_x = contains_item[arr_index][item_count[arr_index] - 1], current_b_x = false;
			unsigned char previous_c_x = item_distance[arr_index][item_count[arr_index] - 1], current_c_x = 0;
			for (int i = item_count[arr_index]; i >= 0; --i)
			{
				current_b_x = contains_item[arr_index][i];
				contains_item[arr_index][i] = previous_b_x;
				previous_b_x = current_b_x;
				current_c_x = item_distance[arr_index][i];
				item_distance[arr_index][i] = previous_c_x;
				previous_c_x = current_c_x;

				current_i_x = items[arr_index][i];
				items[arr_index][i] = previous_i_x;
				previous_i_x = current_i_x;
			}
		}
		else
		{
			belt_utility::_mm256_srli_si256__p<1>((__m256i*)&contains_item[arr_index]);
			belt_utility::_mm256_srli_si256__p<1>((__m256i*)&item_distance[arr_index]);
			belt_utility::_mm512_srli2x256_si512__<2>((__m256i*)&items[arr_index]);
		}
	};
	constexpr void shift_arrays_right_from_index(std::size_t arr_index, std::size_t index) noexcept
	{
		belt_item previous_i_x = items[arr_index][item_count[arr_index] - 1], current_i_x = {};
		bool previous_b_x = contains_item[arr_index][item_count[arr_index] - 1], current_b_x = false;
		unsigned char previous_c_x = item_distance[arr_index][item_count[arr_index] - 1], current_c_x = 0;
		for (int i = item_count[arr_index]; i >= index; --i)
		{
			current_b_x = contains_item[arr_index][i];
			contains_item[arr_index][i] = previous_b_x;
			previous_b_x = current_b_x;
			current_c_x = item_distance[arr_index][i];
			item_distance[arr_index][i] = previous_c_x;
			previous_c_x = current_c_x;

			current_i_x = items[arr_index][i];
			items[arr_index][i] = previous_i_x;
			previous_i_x = current_i_x;
		}
	};
	constexpr bool outside_belt(vec2_uint pos, std::size_t arr_index) const noexcept
	{
		switch (direction[arr_index])
		{
			case belt_utility::belt_direction::left_right: return pos.x >= item_position_x[arr_index] + belt_length;
			case belt_utility::belt_direction::right_left: return pos.x <= item_position_x[arr_index];
			case belt_utility::belt_direction::top_bottom: return pos.y >= item_position_y[arr_index] + belt_length;
			case belt_utility::belt_direction::bottom_top: return pos.y <= item_position_y[arr_index];
		}
	};
	constexpr void try_to_remove_item(std::size_t arr_index) noexcept
	{
		if (std::is_constant_evaluated() == false)
		{
			if (item_distance[arr_index][belt_item_size - 1] == 0 && contains_item[arr_index][belt_item_size - 1])
			{
				if (!remove_item(arr_index, belt_item_size - 1))
				{
					is_stuck[arr_index] = true;
				}
			}
		}
		else
		{
			if (item_goal_distance[arr_index] == 0)
			{
				if (remove_item(arr_index, belt_item_size - 1)) is_stuck[arr_index] = false;
			}
		}
	};
	constexpr void update_belt() noexcept
	{
		//if (count() == 0) return;

		//if (is_stuck == false)
		//{
		if (std::is_constant_evaluated() == false)
		{
			for (int i = 0, l = 8; i < l; ++i)
				items_moved_per_frame += count(i);

#ifdef optimization_comp
			belt_utility::_mm256_add64_si256__p((__m256i*)&item_position_x[0], _mm256_set1_epi64x(1));
			belt_utility::_mm256_sub64_si256__p((__m256i*)&item_goal_distance[0], _mm256_set1_epi64x(1));

			for (int i = 0, l = 8; i < l; ++i)
			{
				if (item_goal_distance[i] == 0)
				{
					if (remove_item(i, 0)) is_stuck[i] = false;
				}
			}
			//_mm256_add_si256__p((__m256i*)item_distance, (__m256i*)contains_item);
#else
			for (int i = 0, l = 4; i < l; ++i)
			{
				if (contains_item[i]) ++item_distance[i];
			}
#endif
		}
		else
		{
			for (int i = 0, l = 8; i < l; ++i)
			{
				++item_position_x[i];
				--item_goal_distance[i];

				if (item_goal_distance[i] <= 0)
				{
					if (remove_item(i, 0)) is_stuck[i] = false;
				}
			}
		}
		//}
			//else try_to_remove_item();
	};
};

constexpr auto test_item_256() noexcept
{
	item_256 test_belt{ belt_utility::belt_direction::left_right, 255 };
	item_array_uint test_arr{ item_uint{item_type::wood}, item_uint{item_type::wood}, item_uint{item_type::wood}, item_uint{item_type::wood}, item_uint{item_type::wood}, item_uint{item_type::stone}, item_uint{item_type::wood}, item_uint{item_type::wood} };

	for (std::size_t i = 0; i < 4; ++i)
	{
		for (std::size_t y = 0; y < 8; ++y)
		{
			if (test_belt.add_item(test_arr[i], vec2_uint{ static_cast<long long>(i) * 32ll, 0ll }, y) != false)
			{
			}
		}
	}

	//used to check support that adding items inbetween is possible
	//test_belt.add_item(test_arr[4], vec2_uint{ 0ll, 0ll }, 0);
	//test_belt.add_item(test_arr[5], vec2_uint{ 32ll, 0ll }, 0);
	//return test_belt.get(0, 4).type;

	for (int i = 0, l = 127 + 32 + 32; i < l; ++i)
	{
		test_belt.update_belt();
	}

	return test_belt.get(1, 0).position.x;
};

constexpr auto test_val_item_256 = test_item_256();
static_assert(test_item_256() == 223.0f, "wrong count");