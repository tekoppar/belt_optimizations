#pragma once

#include <array>

#include <immintrin.h>

#include "item.h"
#include "vectors.h"
#include "belt_utility.h"

#define optimization_comp

class belt_8
{
	constexpr static short belt_length = 128;
public:
	inline static std::size_t items_moved_per_frame = 0;

private:
	const vec2 belt_position{ 0.0f, 0.0f };
	const belt_utility::belt_direction direction{ belt_utility::belt_direction::left_right };
	unsigned char item_count_top[8]{
		0, 0, 0, 0,
		0, 0, 0, 0
	};
	unsigned char item_count_bot[8]{
		0, 0, 0, 0,
		0, 0, 0, 0
	};
	bool contains_item_top[8][4]{
		{ false, false, false, false }, { false, false, false, false }, { false, false, false, false }, { false, false, false, false },
		{ false, false, false, false }, { false, false, false, false }, { false, false, false, false }, { false, false, false, false }
	};
	bool contains_item_bot[8][4]{
		{ false, false, false, false }, { false, false, false, false }, { false, false, false, false }, { false, false, false, false },
		{ false, false, false, false }, { false, false, false, false }, { false, false, false, false }, { false, false, false, false }
	};
	unsigned char items_pos_top_x[8][4]{
		{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
		{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }
	};
	unsigned char items_pos_bot_x[8][4]{
		{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 },
		{ 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }
	};
	item items_top[8][4]; //10 instead of 8 to support a % of an item being on a belt
	item items_bot[8][4]; //10 instead of 8 to support a % of an item being on a belt
	belt_8* neighbouring_belts[8]{
		nullptr, nullptr, nullptr,
		nullptr,		  nullptr,
		nullptr, nullptr, nullptr
	};
	bool cant_move_item{ false };

	constexpr static belt_utility::belt_color color{ belt_utility::belt_color::gray };
public:
	constexpr belt_8() = default;
	constexpr belt_8(const vec2& pos, const belt_utility::belt_direction& dir) noexcept :
		belt_position{ pos },
		direction{ dir }
	{};

	/*template<belt_utility::belt_side side, std::size_t x>
	constexpr item operator[](std::size_t y) noexcept
	{
		if constexpr (belt_utility::belt_side::top == side) return items_top[x][y];
		if constexpr (belt_utility::belt_side::bottom == side) return items_bot[x][y];
	};
	template<belt_utility::belt_side side, std::size_t x>
	constexpr const item& operator[](std::size_t y) const noexcept
	{
		if constexpr (belt_utility::belt_side::top == side) return items_top[x][y];
		if constexpr (belt_utility::belt_side::bottom == side) return items_bot[x][y];
	};*/
	template<belt_utility::belt_side side>
	constexpr item get(std::size_t x, std::size_t y) noexcept
	{
		if constexpr (belt_utility::belt_side::top == side) return items_top[x][y];
		if constexpr (belt_utility::belt_side::bottom == side) return items_bot[x][y];
	};
	template<belt_utility::belt_side side>
	constexpr const item& get(std::size_t x, std::size_t y) const noexcept
	{
		if constexpr (belt_utility::belt_side::top == side) return items_top[x][y];
		if constexpr (belt_utility::belt_side::bottom == side) return items_bot[x][y];
	};

	constexpr short count() const noexcept
	{
		short l_count = 0;
		for (int i = 0, l = 8; i < l; ++i)
		{
			l_count += item_count_top[i];
			l_count += item_count_bot[i];
		}
		return l_count;
	};

	template<belt_utility::belt_neighbour quad_pos>
	constexpr void add_neighbour(belt_8* ptr) noexcept
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
	__declspec(noinline) constexpr bool add_item(const unsigned char x_index, const item& new_item) noexcept
	{
		if constexpr (belt_utility::belt_side::top == side)
		{
			if (contains_item_top[x_index][0] == false)
			{
				contains_item_top[x_index][0] = true;
				items_top[x_index][item_count_top[x_index]] = new_item;
				++item_count_top[x_index];
				return true;
			}
		}
		if constexpr (belt_utility::belt_side::bottom == side)
		{
			if (contains_item_bot[x_index][0] == false)
			{
				contains_item_bot[x_index][0] = true;
				items_bot[x_index][item_count_bot[x_index]] = new_item;
				++item_count_bot[x_index];
				return true;
			}
		}
		return false;
	};
	template<belt_utility::belt_side side>
	constexpr bool remove_item(const unsigned char x_index, const short index) noexcept
	{
		belt_8* neighbour_belt = neighbouring_belts[static_cast<int>(get_neighbour_to_direction(direction))];
		if constexpr (belt_utility::belt_side::top == side) if (neighbour_belt)
		{
			if (neighbour_belt->add_item<belt_utility::belt_side::top>(x_index, items_top[x_index][index]))
			{
				--item_count_top[x_index];
				contains_item_top[x_index][3] = false;
				items_pos_top_x[x_index][3] = 0;
				items_top[x_index][item_count_top[x_index]] = item{};
				return true;
			}
			else return false;
		}
		if constexpr (belt_utility::belt_side::bottom == side) if (neighbour_belt)
		{
			if (neighbour_belt->add_item<belt_utility::belt_side::bottom>(x_index, items_bot[x_index][index]))
			{
				--item_count_bot[x_index];
				contains_item_bot[x_index][3] = false;
				items_pos_bot_x[x_index][3] = 0;
				items_bot[x_index][item_count_bot[x_index]] = item{};
				return true;
			}
			else return false;
		}

		if constexpr (belt_utility::belt_side::top == side)
		{
			const int l = item_count_top[x_index] < 4 ? item_count_top[x_index] : 3;
			for (int i = index; i < l; ++i)
			{
				items_top[x_index][i] = items_top[x_index][i + 1];
				items_pos_top_x[x_index][i] = items_pos_top_x[x_index][i + 1];
			}
			--item_count_top[x_index];
			contains_item_top[x_index][3] = false;
			items_pos_top_x[x_index][3] = 0;
			items_top[x_index][item_count_top[x_index]] = item{};
		}
		if constexpr (belt_utility::belt_side::bottom == side)
		{
			const int l = item_count_bot[x_index] < 4 ? item_count_bot[x_index] : 3;
			for (int i = index; i < l; ++i)
			{
				items_bot[x_index][i] = items_bot[x_index][i + 1];
				items_pos_bot_x[x_index][i] = items_pos_bot_x[x_index][i + 1];
			}
			--item_count_bot[x_index];
			contains_item_bot[x_index][3] = false;
			items_pos_bot_x[x_index][3] = 0;
			items_bot[x_index][item_count_bot[x_index]] = item{};
		}

		return true;
	};
	template<belt_utility::belt_side side>
	constexpr bool remove_item_local(const unsigned char x_index, const short index) noexcept
	{
		if (x_index >= 7) return false;

		if constexpr (belt_utility::belt_side::top == side)
		{
			if (contains_item_top[x_index + 1][0] == false)
			{
				contains_item_top[x_index + 1][0] = true;
				items_top[x_index + 1][item_count_top[x_index + 1]] = items_top[x_index][index];
				++item_count_top[x_index + 1];

				--item_count_top[x_index];
				contains_item_top[x_index][3] = false;
				items_pos_top_x[x_index][3] = 0;
				items_top[x_index][item_count_top[x_index]] = item{};
				return true;
			}
			else return false;
		}
		if constexpr (belt_utility::belt_side::bottom == side)
		{
			if (contains_item_bot[x_index + 1][0] == false)
			{
				contains_item_bot[x_index + 1][0] = true;
				items_bot[x_index + 1][item_count_bot[x_index + 1]] = items_bot[x_index][index];
				++item_count_bot[x_index + 1];

				--item_count_bot[x_index];
				contains_item_bot[x_index][3] = false;
				items_pos_bot_x[x_index][3] = 0;
				items_bot[x_index][item_count_bot[x_index]] = item{};
				return true;
			}
			else return false;
		}

		if constexpr (belt_utility::belt_side::top == side)
		{
			const int l = item_count_top[x_index] < 4 ? item_count_top[x_index] : 3;
			for (int i = index; i < l; ++i)
			{
				items_top[x_index][i] = items_top[x_index][i + 1];
				items_pos_top_x[x_index][i] = items_pos_top_x[x_index][i + 1];
			}
			--item_count_top[x_index];
			contains_item_top[x_index][3] = false;
			items_pos_top_x[x_index][3] = 0;
			items_top[x_index][item_count_top[x_index]] = item{};
		}
		if constexpr (belt_utility::belt_side::bottom == side)
		{
			const int l = item_count_bot[x_index] < 4 ? item_count_bot[x_index] : 3;
			for (int i = index; i < l; ++i)
			{
				items_bot[x_index][i] = items_bot[x_index][i + 1];
				items_pos_bot_x[x_index][i] = items_pos_bot_x[x_index][i + 1];
			}
			--item_count_bot[x_index];
			contains_item_bot[x_index][3] = false;
			items_pos_bot_x[x_index][3] = 0;
			items_bot[x_index][item_count_bot[x_index]] = item{};
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
			if (items_pos_top_x[7][3] >= 128)
			{
				if (!remove_item<belt_utility::belt_side::top>(7, 3))
				{
					cant_move_item = true;
				}
			}
			for (int x = 6; x >= 0; --x)
			{
				if (items_pos_top_x[x][3] >= 128)
				{
					if (!remove_item_local<belt_utility::belt_side::top>(x, 3))
					{
						cant_move_item = true;
					}
				}
			}
			if (items_pos_bot_x[7][3] >= 128)
			{
				if (!remove_item<belt_utility::belt_side::bottom>(7, 3))
				{
					cant_move_item = true;
				}
			}
			for (int x = 6; x >= 0; --x)
			{
				if (items_pos_bot_x[x][3] >= 128)
				{
					if (!remove_item_local<belt_utility::belt_side::bottom>(x, 3))
					{
						cant_move_item = true;
					}
				}
			}
		}
		else
		{
			for (int y = 0; y < item_count_top[7]; ++y)
			{
				if (outside_belt(items_top[7][y].position))
				{
					if (remove_item<belt_utility::belt_side::top>(7, y)) cant_move_item = false;
				}
			}
			for (int x = 6; x >= 0; --x)
			{
				for (int y = 0; y < item_count_top[x]; ++y)
				{
					if (outside_belt(items_top[x][y].position))
					{
						if (remove_item_local<belt_utility::belt_side::top>(x, y)) cant_move_item = false;
					}
				}
			}
			for (int y = 0; y < item_count_bot[7]; ++y)
			{
				if (outside_belt(items_bot[7][y].position))
				{
					if (remove_item<belt_utility::belt_side::bottom>(7, y)) cant_move_item = false;
				}
			}
			for (int x = 6; x >= 0; --x)
			{
				for (int y = 0; y < item_count_bot[x]; ++y)
				{
					if (outside_belt(items_bot[x][y].position))
					{
						if (remove_item_local<belt_utility::belt_side::bottom>(x, y)) cant_move_item = false;
					}
				}
			}
		}
	};
	constexpr void update_belt(int loop_counter) noexcept
	{
		if (count() == 0) return;

		if (cant_move_item == false)
		{
			if (std::is_constant_evaluated() == false)
			{
				items_moved_per_frame += count();

#ifdef optimization_comp
				//unsigned long long* ptr_addition_char = (unsigned long long*)contains_item_top;//get_ptr_addition_value(color);
				//unsigned long* ptr_values = (unsigned long*)&items_pos_top_x;
				__m256i const_imm{ 0x01010101, 0x01010101, 0x01010101, 0x01010101,
				0x01010101, 0x01010101, 0x01010101, 0x01010101,
				0x01010101, 0x01010101, 0x01010101, 0x01010101,
				0x01010101, 0x01010101, 0x01010101, 0x01010101,
				0x01010101, 0x01010101, 0x01010101, 0x01010101,
				0x01010101, 0x01010101, 0x01010101, 0x01010101,
				0x01010101, 0x01010101, 0x01010101, 0x01010101,
				0x01010101, 0x01010101, 0x01010101, 0x01010101 };
				__m256i anded_top = _mm256_and_si256(*(__m256i*)contains_item_top, const_imm);
				__m256i added_top = _mm256_add_epi8(*(__m256i*)items_pos_top_x, anded_top);
				std::memcpy(items_pos_top_x, &added_top.m256i_i8[0], 32);

				//*ptr_values = *ptr_values + (*ptr_addition_char & 0x01010101);
#else
				for (int i = 0, l = 4; i < l; ++i)
				{
					if (contains_item_top[i]) ++items_pos_top_x[i];
				}
#endif
				if (items_pos_top_x[7][3] >= 128)
				{
					if (remove_item<belt_utility::belt_side::top>(7, 3))
					{
					}
					else if (contains_item_top[7][3] != false)
					{
						cant_move_item = true;
					}
				}
				for (int x = 6; x >= 0; --x)
				{
					if (items_pos_top_x[x][3] >= 128)
					{
						if (remove_item_local<belt_utility::belt_side::top>(x, 3))
						{
						}
						else if (contains_item_top[x][3] != false)
						{
							cant_move_item = true;
						}
					}
				}
#ifdef optimization_comp
				//unsigned long* ptr_addition_char = (unsigned long*)contains_item_bot;//get_ptr_addition_value(color);
				//unsigned long* ptr_values = (unsigned long*)&items_pos_bot_x;
				//*ptr_values = *ptr_values + (*ptr_addition_char & 0x01010101);
				__m256i anded_bot = _mm256_and_si256(*(__m256i*)contains_item_bot, const_imm);
				__m256i added_bot = _mm256_add_epi8(*(__m256i*)items_pos_bot_x, anded_bot);
				std::memcpy(items_pos_bot_x, &added_bot.m256i_i8[0], sizeof(__m256i));
#else
				for (int i = 0, l = 4; i < l; ++i)
				{
					if (contains_item_bot[i]) ++items_pos_bot_x[i];
				}
#endif
				if (items_pos_bot_x[7][3] >= 128)
				{
					if (remove_item<belt_utility::belt_side::bottom>(7, 3))
					{
					}
					else if (contains_item_bot[7][3] != false)
					{
						cant_move_item = true;
					}
				}
				for (int x = 6; x >= 0; --x)
				{
					if (items_pos_bot_x[x][3] >= 128)
					{
						if (remove_item_local<belt_utility::belt_side::bottom>(x, 3))
						{
						}
						else if (contains_item_bot[x][3] != false)
						{
							cant_move_item = true;
						}
					}
				}

				if (loop_counter != 0 && loop_counter % 32 == 0)
				{
					if (cant_move_item == false)
					{
#ifdef optimization_comp
						//we shift the contains and position arrays by 1 to the right
						/*
						unsigned long* ptr_addition_char = (unsigned long*)contains_item_top;
						*ptr_addition_char = ((unsigned long)*ptr_addition_char << 8) | ((unsigned long)*ptr_addition_char >> (32 - 8));
						ptr_addition_char = (unsigned long*)items_pos_top_x;
						*ptr_addition_char = ((unsigned long)*ptr_addition_char << 8) | ((unsigned long)*ptr_addition_char >> (32 - 8));
						//*/

						auto first_shuffle = _mm256_slli_epi32(*(__m256i*)contains_item_top, 8);
						//second_shuffle = _mm256_srli_epi32(*(__m256i*)contains_item_top, 32 - 8);
						//ored = _mm256_or_epi32(first_shuffle, second_shuffle);
						std::memcpy(contains_item_top, &first_shuffle.m256i_i8[0], 32);

						first_shuffle = _mm256_slli_epi32(*(__m256i*)items_pos_top_x, 8);
						//second_shuffle = _mm256_srli_epi32(*(__m256i*)items_pos_top_x, 32 - 8);
						//ored = _mm256_or_epi32(first_shuffle, second_shuffle);
						std::memcpy(items_pos_top_x, &first_shuffle.m256i_i8[0], 32);
#else
						bool previous_b = contains_item_top[3], current_b = false;
						unsigned char previous_c = items_pos_top_x[3], current_c = 0;
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
					if (cant_move_item == false)
					{
#ifdef optimization_comp
						//we shift the contains and position arrays by 1 to the right
						/*
						unsigned long* ptr_addition_char = (unsigned long*)contains_item_bot;
						*ptr_addition_char = ((unsigned long)*ptr_addition_char << 8) | ((unsigned long)*ptr_addition_char >> (32 - 8));
						ptr_addition_char = (unsigned long*)items_pos_bot_x;
						*ptr_addition_char = ((unsigned long)*ptr_addition_char << 8) | ((unsigned long)*ptr_addition_char >> (32 - 8));
						//*/

						auto first_shuffle = _mm256_slli_epi32(*(__m256i*)contains_item_bot, 8);
						//auto second_shuffle = _mm256_srli_epi32(*(__m256i*)contains_item_bot, 32 - 8);
						//auto ored = _mm256_or_epi32(first_shuffle, second_shuffle);
						std::memcpy(contains_item_bot, &first_shuffle.m256i_i8[0], 32);

						first_shuffle = _mm256_slli_epi32(*(__m256i*)items_pos_bot_x, 8);
						//second_shuffle = _mm256_srli_epi32(*(__m256i*)items_pos_bot_x, 32 - 8);
						//ored = _mm256_or_epi32(first_shuffle, second_shuffle);
						std::memcpy(items_pos_bot_x, &first_shuffle.m256i_i8[0], 32);
#else
						bool previous_b = contains_item_bot[3], current_b = false;
						unsigned char previous_c = items_pos_bot_x[3], current_c = 0;
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
	}
			else
			{
				for (int y = 0; y < item_count_top[7]; ++y)
				{
					items_top[7][y].position += (belt_utility::belt_direction_multi[static_cast<int>(direction)] * vec2{ static_cast<float>((belt_utility::belt_speed)(color)), static_cast<float>((belt_utility::belt_speed)(color)) });

					if (outside_belt(items_top[7][y].position))
					{
						if (remove_item<belt_utility::belt_side::top>(7, y)) --y;
						else
						{
							cant_move_item = true;
						}
					}
				}
				for (int y = 0; y < item_count_bot[7]; ++y)
				{
					items_bot[7][y].position += (belt_utility::belt_direction_multi[static_cast<int>(direction)] * vec2{ static_cast<float>((belt_utility::belt_speed)(color)), static_cast<float>((belt_utility::belt_speed)(color)) });

					if (outside_belt(items_bot[7][y].position))
					{
						if (remove_item<belt_utility::belt_side::bottom>(7, y)) --y;
						else
						{
							cant_move_item = true;
						}
					}
				}
				for (int x = 6; x >= 0; --x)
				{
					for (int y = 0; y < item_count_top[x]; ++y)
					{
						items_top[x][y].position += (belt_utility::belt_direction_multi[static_cast<int>(direction)] * vec2{ static_cast<float>((belt_utility::belt_speed)(color)), static_cast<float>((belt_utility::belt_speed)(color)) });

						if (outside_belt(items_top[x][y].position))
						{
							if (remove_item_local<belt_utility::belt_side::top>(x, y)) --y;
							else
							{
								cant_move_item = true;
							}
						}
					}
					for (int y = 0; y < item_count_bot[x]; ++y)
					{
						items_bot[x][y].position += (belt_utility::belt_direction_multi[static_cast<int>(direction)] * vec2{ static_cast<float>((belt_utility::belt_speed)(color)), static_cast<float>((belt_utility::belt_speed)(color)) });

						if (outside_belt(items_bot[x][y].position))
						{
							if (remove_item_local<belt_utility::belt_side::bottom>(x, y)) --y;
							else
							{
								cant_move_item = true;
							}
						}
					}
				}
			}
}
		else try_to_remove_item();
	};
};

//template<std::size_t N, template <std::size_t> typename cont_type>
consteval float test_belt8(/*const cont_type<N>& arr*/) noexcept
{
	belt_8 test_belt;
	item_array test_arr{ item{}, item{}, item{}, item{} };

	for (std::size_t i = 0; i < test_arr.size(); ++i)
	{
		if (test_belt.add_item<belt_utility::belt_side::top>(0, test_arr[i]) != false)
		{
		}
	}

	for (int i = 0, l = 128; i < l; ++i)
	{
		test_belt.update_belt(i);
	}

	return test_belt.get<belt_utility::belt_side::top>(0, 0).position.x;
};

constexpr float test_val_8 = test_belt8();
static_assert(test_belt8() == 0.0f, "wrong count");