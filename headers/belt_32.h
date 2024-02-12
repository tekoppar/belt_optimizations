#pragma once

#include <array>

#include "item.h"
#include "vectors.h"
#include "belt_utility.h"

#define optimization_comp

class belt_32
{
	constexpr static short single_belt_length = 128;
	constexpr static short belt_length = 128 * 8;
	constexpr static unsigned char belt_item_size = 32;
public:
	inline static std::size_t items_moved_per_frame = 0;

private:
	const vec2 belt_position{ 0.0f, 0.0f };
	const belt_utility::belt_direction direction{ belt_utility::belt_direction::left_right };
	unsigned char item_count_top{ 0 };
	unsigned char item_count_bot{ 0 };
	__declspec(align(32)) bool contains_item_top[32]{
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false
	};
	__declspec(align(32)) bool contains_item_bot[32]{
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false
	};
	__declspec(align(32)) unsigned char items_pos_top_x[32]{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	__declspec(align(32)) unsigned char items_pos_bot_x[32]{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	__declspec(align(64)) belt_item items_top[32];
	__declspec(align(64)) belt_item items_bot[32];
	belt_32* neighbouring_belts[8]{
		nullptr, nullptr, nullptr,
		nullptr,		  nullptr,
		nullptr, nullptr, nullptr
	};
	bool cant_move_item{ false };

	constexpr static belt_utility::belt_color color{ belt_utility::belt_color::gray };
public:
	constexpr belt_32() = default;
	constexpr belt_32(const vec2& pos, const belt_utility::belt_direction& dir) noexcept :
		belt_position{ pos },
		direction{ dir }
	{};

	template<belt_utility::belt_side side>
	constexpr item operator[](std::size_t i) noexcept
	{
		if constexpr (belt_utility::belt_side::top == side) return { items_top[i].type, vec2{belt_position.x + (items_pos_top_x[i] % 32) + i * 32.0f, belt_position.y} };
		if constexpr (belt_utility::belt_side::bottom == side) return { items_bot[i].type, vec2{belt_position.x + (items_pos_bot_x[i] % 32) + i * 32.0f, belt_position.y} };
	};
	template<belt_utility::belt_side side>
	constexpr const item& operator[](std::size_t i) const noexcept
	{
		if constexpr (belt_utility::belt_side::top == side) return { items_top[i].type, vec2{belt_position.x + (items_pos_top_x[i] % 32) + i * 32.0f, belt_position.y} };
		if constexpr (belt_utility::belt_side::bottom == side) return { items_bot[i].type, vec2{belt_position.x + (items_pos_bot_x[i] % 32) + i * 32.0f, belt_position.y} };
	};
	template<belt_utility::belt_side side>
	constexpr item get(std::size_t i) noexcept
	{
		if constexpr (belt_utility::belt_side::top == side) return { items_top[i].type, vec2{belt_position.x + (items_pos_top_x[i] % 32) + i * 32.0f, belt_position.y} };
		if constexpr (belt_utility::belt_side::bottom == side) return { items_bot[i].type, vec2{belt_position.x + (items_pos_bot_x[i] % 32) + i * 32.0f, belt_position.y} };
	};
	template<belt_utility::belt_side side>
	constexpr const item& get(std::size_t i) const noexcept
	{
		if constexpr (belt_utility::belt_side::top == side) return { items_top[i].type, vec2{belt_position.x + (items_pos_top_x[i] % 32) + i * 32.0f, belt_position.y} };
		if constexpr (belt_utility::belt_side::bottom == side) return { items_bot[i].type, vec2{belt_position.x + (items_pos_bot_x[i] % 32) + i * 32.0f, belt_position.y} };
	};

	constexpr short count() const noexcept
	{
		return item_count_top + item_count_bot;
	};

	template<belt_utility::belt_neighbour quad_pos>
	constexpr void add_neighbour(belt_32* ptr) noexcept
	{
		if constexpr (belt_utility::belt_neighbour::top_m == quad_pos)
		{
			neighbouring_belts[1] = ptr;
		}
		if constexpr (belt_utility::belt_neighbour::left == quad_pos)
		{
			neighbouring_belts[3] = ptr;
		}
		if constexpr (belt_utility::belt_neighbour::right == quad_pos)
		{
			neighbouring_belts[4] = ptr;
		}
		if constexpr (belt_utility::belt_neighbour::bot_m == quad_pos)
		{
			neighbouring_belts[6] = ptr;
		}
	};

	template<belt_utility::belt_side side>
	constexpr bool add_item(const belt_item& new_item) noexcept
	{
		if constexpr (belt_utility::belt_side::top == side)
		{
			if (contains_item_top[0] == false)
			{
				contains_item_top[0] = true;
				items_top[item_count_top] = new_item;
				++item_count_top;
				return true;
			}
		}
		if constexpr (belt_utility::belt_side::bottom == side)
		{
			if (contains_item_bot[0] == false)
			{
				contains_item_bot[0] = true;
				items_bot[item_count_bot] = new_item;
				++item_count_bot;
				return true;
			}
		}
		return false;
	};
	template<belt_utility::belt_side side>
	constexpr bool add_item(const belt_item& new_item, std::size_t index) noexcept
	{
		if constexpr (belt_utility::belt_side::top == side)
		{
			if (contains_item_top[index] == false)
			{
				contains_item_top[index] = true;
				items_pos_top_x[index] = (index * 32) % 255;
				items_top[index] = new_item;
				++item_count_top;
				return true;
			}
		}
		if constexpr (belt_utility::belt_side::bottom == side)
		{
			if (contains_item_bot[index] == false)
			{
				contains_item_bot[index] = true;
				items_pos_bot_x[index] = index * 32;
				items_bot[index] = new_item;
				++item_count_bot;
				return true;
			}
		}
		return false;
	};
	template<belt_utility::belt_side side>
	constexpr bool remove_item(const short index) noexcept
	{
		if (std::is_constant_evaluated() == false)
		{
			belt_32* neighbour_belt = neighbouring_belts[static_cast<int>(get_neighbour_to_direction(direction))];
			if constexpr (belt_utility::belt_side::top == side) if (neighbour_belt)
			{
				if (neighbour_belt->add_item<belt_utility::belt_side::top>(items_top[index]))
				{
					--item_count_top;
					contains_item_top[belt_item_size - 1] = false;
					items_pos_top_x[belt_item_size - 1] = 0;
					items_top[item_count_top] = belt_item{};
					return true;
				}
				else return false;
			}
			if constexpr (belt_utility::belt_side::bottom == side) if (neighbour_belt)
			{
				if (neighbour_belt->add_item<belt_utility::belt_side::bottom>(items_bot[index]))
				{
					--item_count_bot;
					contains_item_bot[belt_item_size - 1] = false;
					items_pos_bot_x[belt_item_size - 1] = 0;
					items_bot[item_count_bot] = belt_item{};
					return true;
				}
				else return false;
			}
		}

		if constexpr (belt_utility::belt_side::top == side)
		{
			/*const int l = item_count_top < belt_item_size ? item_count_top : belt_item_size - 1;
			for (int i = index; i < l; ++i)
			{
				items_top[i] = items_top[i + 1];
				items_pos_top_x[i] = items_pos_top_x[i + 1];
			}*/
			--item_count_top;
			contains_item_top[belt_item_size - 1] = false;
			items_pos_top_x[belt_item_size - 1] = 0;
			items_top[item_count_top] = belt_item{};
		}
		if constexpr (belt_utility::belt_side::bottom == side)
		{
			/*const int l = item_count_bot < belt_item_size ? item_count_bot : belt_item_size - 1;
			for (int i = index; i < l; ++i)
			{
				items_bot[i] = items_bot[i + 1];
				items_pos_bot_x[i] = items_pos_bot_x[i + 1];
			}*/
			--item_count_bot;
			contains_item_bot[belt_item_size - 1] = false;
			items_pos_bot_x[belt_item_size - 1] = 0;
			items_bot[item_count_bot] = belt_item{};
		}

		return true;
	};
	constexpr bool outside_belt(vec2 pos) const noexcept
	{
		switch (direction)
		{
			case belt_utility::belt_direction::left_right: return pos.x >= belt_position.x + belt_length;
			case belt_utility::belt_direction::right_left: return pos.x <= belt_position.x;
			case belt_utility::belt_direction::top_bottom: return pos.y >= belt_position.y + belt_length;
			case belt_utility::belt_direction::bottom_top: return pos.y <= belt_position.y;
		}
	};
	constexpr void try_to_remove_item() noexcept
	{
		if (std::is_constant_evaluated() == false)
		{
			if (items_pos_top_x[belt_item_size - 1] == 0 && contains_item_top[belt_item_size - 1])
			{
				if (!remove_item<belt_utility::belt_side::top>(belt_item_size - 1))
				{
					cant_move_item = true;
				}
			}
			if (items_pos_bot_x[belt_item_size - 1] == 0 && contains_item_bot[belt_item_size - 1])
			{
				if (!remove_item<belt_utility::belt_side::bottom>(belt_item_size - 1))
				{
					cant_move_item = true;
				}
			}
		}
		else
		{
			if (items_pos_top_x[belt_item_size - 1] == 0 && contains_item_top[belt_item_size - 1])
			{
				if (remove_item<belt_utility::belt_side::top>(belt_item_size - 1)) cant_move_item = false;
			}
			if (items_pos_bot_x[belt_item_size - 1] == 0 && contains_item_bot[belt_item_size - 1])
			{
				if (remove_item<belt_utility::belt_side::bottom>(belt_item_size - 1)) cant_move_item = false;
			}
		}
	};
	constexpr void update_belt() noexcept
	{
		if (count() == 0) return;

		if (cant_move_item == false)
		{
			if (std::is_constant_evaluated() == false)
			{
				//items_moved_per_frame += count();

#ifdef optimization_comp
				belt_utility::_mm256_add_si256__p((__m256i*)items_pos_top_x, (__m256i*)contains_item_top);
#else
				for (int i = 0, l = 4; i < l; ++i)
				{
					if (contains_item_top[i]) ++items_pos_top_x[i];
				}
#endif
#ifdef optimization_comp
				belt_utility::_mm256_add_si256__p((__m256i*)items_pos_bot_x, (__m256i*)contains_item_bot);
#else
				for (int i = 0, l = 4; i < l; ++i)
				{
					if (contains_item_bot[i]) ++items_pos_bot_x[i];
				}
#endif
			}
			else
			{
				for (int i = 0; i < belt_item_size; ++i)
				{
					if (contains_item_top[i]) items_pos_top_x[i] += 1;
					if (contains_item_bot[i]) items_pos_bot_x[i] += 1;
				}

				if (items_pos_top_x[belt_item_size - 1] == 0 && contains_item_top[belt_item_size - 1])
				{
					if (!remove_item<belt_utility::belt_side::top>(belt_item_size - 1))
					{
						cant_move_item = true;
					}
				}

				if (items_pos_bot_x[belt_item_size - 1] == 0 && contains_item_bot[belt_item_size - 1])
				{
					if (!remove_item<belt_utility::belt_side::bottom>(belt_item_size - 1))
					{
						cant_move_item = true;
					}
				}
			}
		}
		else try_to_remove_item();
	};
	constexpr void shift_items_belt() noexcept
	{
		if (count() == 0) return;

		if (cant_move_item == false)
		{
			if (std::is_constant_evaluated() == false)
			{
				//if (loop_counter == 256) __debugbreak();
				if (items_pos_top_x[belt_item_size - 1] == 0 && contains_item_top[belt_item_size - 1])
				{
					if (remove_item<belt_utility::belt_side::top>(belt_item_size - 1))
					{
					}
					else if (contains_item_top[belt_item_size - 1] != false)
					{
						cant_move_item = true;
					}
				}

				if (items_pos_bot_x[belt_item_size - 1] == 0 && contains_item_bot[belt_item_size - 1])
				{
					if (remove_item<belt_utility::belt_side::bottom>(belt_item_size - 1))
					{
					}
					else if (contains_item_bot[belt_item_size - 1] != false)
					{
						cant_move_item = true;
					}
				}

				if (item_count_top > 0 && cant_move_item == false)
				{
#ifdef optimization_comp
					//we shift the contains and position arrays by 1 to the right
					belt_utility::_mm256_slli_si256__p<1>((__m256i*)contains_item_top);
					belt_utility::_mm256_slli_si256__p<1>((__m256i*)items_pos_top_x);
					belt_utility::_mm512_slli2x256_si512__<2>((__m256i*)items_top);
#else
					bool previous_b = contains_item_top[belt_item_size - 1], current_b = false;
					unsigned char previous_c = items_pos_top_x[belt_item_size - 1], current_c = 0;
					for (int i = 0; i < 4; ++i)
					{
						current_b = contains_item_top[i];
						contains_item_top[i] = previous_b;
						previous_b = current_b;
						//contains_item_top[i - 1] = false;
						current_c = items_pos_top_x[i];
						items_pos_top_x[i] = previous_c;
						previous_c = current_c;
						//items_pos_top_x[i - 1] = 0;
					}
#endif
				}
				if (item_count_bot > 0 && cant_move_item == false)
				{
#ifdef optimization_comp
					//we shift the contains and position arrays by 1 to the right
					belt_utility::_mm256_slli_si256__p<1>((__m256i*)contains_item_bot);
					belt_utility::_mm256_slli_si256__p<1>((__m256i*)items_pos_bot_x);
					belt_utility::_mm512_slli2x256_si512__<2>((__m256i*)items_bot);
#else
					bool previous_b = contains_item_bot[belt_item_size - 1], current_b = false;
					unsigned char previous_c = items_pos_bot_x[belt_item_size - 1], current_c = 0;
					for (int i = 0; i < 4; ++i)
					{
						current_b = contains_item_bot[i];
						contains_item_bot[i] = previous_b;
						previous_b = current_b;
						//contains_item_bot[i - 1] = false;
						current_c = items_pos_bot_x[i];
						items_pos_bot_x[i] = previous_c;
						previous_c = current_c;
						//items_pos_bot_x[i - 1] = 0;
					}
#endif
				}
			}
			else
			{
				belt_item previous_i_x = items_top[belt_item_size - 1], current_i_x = {};
				belt_item previous_i_y = items_bot[belt_item_size - 1], current_i_y = {};

				bool previous_b_x = contains_item_top[belt_item_size - 1], current_b_x = false;
				unsigned char previous_c_x = items_pos_top_x[belt_item_size - 1], current_c_x = 0;
				bool previous_b_y = contains_item_bot[belt_item_size - 1], current_b_y = false;
				unsigned char previous_c_y = items_pos_bot_x[belt_item_size - 1], current_c_y = 0;
				for (int i = 0; i < belt_item_size; ++i)
				{
					current_b_x = contains_item_top[i];
					contains_item_top[i] = previous_b_x;
					previous_b_x = current_b_x;
					current_c_x = items_pos_top_x[i];
					items_pos_top_x[i] = previous_c_x;
					previous_c_x = current_c_x;

					current_b_y = contains_item_bot[i];
					contains_item_bot[i] = previous_b_y;
					previous_b_y = current_b_y;
					current_c_y = items_pos_bot_x[i];
					items_pos_bot_x[i] = previous_c_y;
					previous_c_y = current_c_y;

					current_i_x = items_top[i];
					items_top[i] = previous_i_x;
					previous_i_x = current_i_x;
					current_i_y = items_bot[i];
					items_bot[i] = previous_i_y;
					previous_i_y = current_i_y;
				}

				if (items_pos_top_x[belt_item_size - 1] == 0 && contains_item_top[belt_item_size - 1])
				{
					if (!remove_item<belt_utility::belt_side::top>(belt_item_size - 1))
					{
						cant_move_item = true;
					}
				}

				if (items_pos_bot_x[belt_item_size - 1] == 0 && contains_item_bot[belt_item_size - 1])
				{
					if (!remove_item<belt_utility::belt_side::bottom>(belt_item_size - 1))
					{
						cant_move_item = true;
					}
				}
			}
		}
		else try_to_remove_item();
	};
};

//template<std::size_t N, template <std::size_t> typename cont_type>
consteval float test_belt32(/*const cont_type<N>& arr*/) noexcept
{
	belt_32 test_belt;
	item_array test_arr{ item{item_type::wood}, item{item_type::wood}, item{item_type::wood}, item{item_type::wood} };

	for (std::size_t i = 0; i < test_arr.size(); ++i)
	{
		if (test_belt.add_item<belt_utility::belt_side::top>(test_arr[i], i) != false)
		{
		}
	}

	for (int i = 0, l = 300; i < l; ++i)
	{
		if (i != 0 && i % 32 == 0)
		{
			test_belt.update_belt();
			test_belt.shift_items_belt();
		}
		else
			test_belt.update_belt();
	}

	return test_belt.get<belt_utility::belt_side::top>(9).position.x;
};

constexpr float test_val_32 = test_belt32();
static_assert(test_belt32() == 300.0f, "wrong count");