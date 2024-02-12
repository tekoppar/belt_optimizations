#pragma once

#include <array>

#include "item.h"
#include "vectors.h"
#include "belt_utility.h"

#define optimization_comp

class belt
{
	constexpr static short belt_length = 128;
public:
	inline static std::size_t items_moved_per_frame = 0;

private:
	const vec2 belt_position{ 0.0f, 0.0f };
	const belt_utility::belt_direction direction{ belt_utility::belt_direction::left_right };
	short item_count_top{ 0 };
	short item_count_bot{ 0 };
	bool contains_item_top[4]{ false, false, false, false };
	bool contains_item_bot[4]{ false, false, false, false };
	unsigned char items_pos_top_x[4]{ 0, 0, 0, 0 };
	unsigned char items_pos_bot_x[4]{ 0, 0, 0, 0 };
	item items_top[5]; //10 instead of 8 to support a % of an item being on a belt
	item items_bot[5]; //10 instead of 8 to support a % of an item being on a belt
	belt* neighbouring_belts[8]{
		nullptr, nullptr, nullptr,
		nullptr,		  nullptr,
		nullptr, nullptr, nullptr
	};
	bool cant_move_item{ false };

	constexpr static belt_utility::belt_color color{ belt_utility::belt_color::gray };
public:
	constexpr belt() = default;
	constexpr belt(const vec2& pos, const belt_utility::belt_direction& dir) noexcept :
		belt_position{ pos },
		direction{ dir }
	{};

	template<belt_utility::belt_side side>
	constexpr item operator[](std::size_t i) noexcept
	{
		if constexpr (belt_utility::belt_side::top == side) return items_top[i];
		if constexpr (belt_utility::belt_side::bottom == side) return items_bot[i];
	};
	template<belt_utility::belt_side side>
	constexpr const item& operator[](std::size_t i) const noexcept
	{
		if constexpr (belt_utility::belt_side::top == side) return items_top[i];
		if constexpr (belt_utility::belt_side::bottom == side) return items_bot[i];
	};
	template<belt_utility::belt_side side>
	constexpr item get(std::size_t i) noexcept
	{
		if constexpr (belt_utility::belt_side::top == side) return items_top[i];
		if constexpr (belt_utility::belt_side::bottom == side) return items_bot[i];
	};
	template<belt_utility::belt_side side>
	constexpr const item& get(std::size_t i) const noexcept
	{
		if constexpr (belt_utility::belt_side::top == side) return items_top[i];
		if constexpr (belt_utility::belt_side::bottom == side) return items_bot[i];
	};

	short count() const noexcept
	{
		return item_count_top + item_count_bot;
	};

	template<belt_utility::belt_neighbour quad_pos>
	constexpr void add_neighbour(belt* ptr) noexcept
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
	__declspec(noinline) constexpr bool add_item(const item& new_item) noexcept
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
	constexpr bool remove_item(const short index) noexcept
	{
		belt* neighbour_belt = neighbouring_belts[static_cast<int>(get_neighbour_to_direction(direction))];
		if constexpr (belt_utility::belt_side::top == side) if (neighbour_belt)
		{
			if (neighbour_belt->add_item<belt_utility::belt_side::top>(items_top[index]))
			{
				--item_count_top;
				contains_item_top[3] = false;
				items_pos_top_x[3] = 0;
				items_top[item_count_top] = item{};
				return true;
			}
			else return false;
		}
		if constexpr (belt_utility::belt_side::bottom == side) if (neighbour_belt)
		{
			if (neighbour_belt->add_item<belt_utility::belt_side::bottom>(items_bot[index]))
			{
				--item_count_bot;
				contains_item_bot[3] = false;
				items_pos_bot_x[3] = 0;
				items_bot[item_count_bot] = item{};
				return true;
			}
			else return false;
		}

		if constexpr (belt_utility::belt_side::top == side)
		{
			const int l = item_count_top < 4 ? item_count_top : 3;
			for (int i = index; i < l; ++i)
			{
				items_top[i] = items_top[i + 1];
				items_pos_top_x[i] = items_pos_top_x[i + 1];
			}
			--item_count_top;
			contains_item_top[3] = false;
			items_pos_top_x[3] = 0;
			items_top[item_count_top] = item{};
		}
		if constexpr (belt_utility::belt_side::bottom == side)
		{
			const int l = item_count_bot < 4 ? item_count_bot : 3;
			for (int i = index; i < l; ++i)
			{
				items_bot[i] = items_bot[i + 1];
				items_pos_bot_x[i] = items_pos_bot_x[i + 1];
			}
			--item_count_bot;
			contains_item_bot[3] = false;
			items_pos_bot_x[3] = 0;
			items_bot[item_count_bot] = item{};
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
			if (items_pos_top_x[3] >= 128)
			{
				if (!remove_item<belt_utility::belt_side::top>(3)) cant_move_item = true;
			}
			if (items_pos_bot_x[3] >= 128)
			{
				if (!remove_item<belt_utility::belt_side::bottom>(3)) cant_move_item = true;
			}
		}
		else
		{
			for (int i = 0; i < item_count_top; ++i)
			{
				if (outside_belt(items_top[i].position))
				{
					if (remove_item<belt_utility::belt_side::top>(i)) cant_move_item = false;
				}
			}
			for (int i = 0; i < item_count_bot; ++i)
			{
				if (outside_belt(items_bot[i].position))
				{
					if (remove_item<belt_utility::belt_side::bottom>(i)) cant_move_item = false;
				}
			}
		}
	};
	constexpr void update_belt(int loop_counter) noexcept
	{
		if (item_count_top == 0 && item_count_bot == 0) return;

		if (cant_move_item == false)
		{
			if (std::is_constant_evaluated() == false)
			{
				items_moved_per_frame += static_cast<std::size_t>(item_count_top) + item_count_bot;

				if (item_count_top > 0)
				{
#ifdef optimization_comp
					unsigned long long* ptr_addition_char = (unsigned long long*)contains_item_top;
					unsigned long* ptr_values = (unsigned long*)&items_pos_top_x;
					*ptr_values = *ptr_values + (*ptr_addition_char & 0x01010101);
#else
					for (int i = 0, l = 4; i < l; ++i)
					{
						if (contains_item_top[i]) ++items_pos_top_x[i];
					}
#endif
					if (items_pos_top_x[3] >= 128)
					{
						if (remove_item<belt_utility::belt_side::top>(3))
						{
						}
						else if (contains_item_top[3] != false)
						{
							cant_move_item = true;
						}
					}
				}
				if (item_count_bot > 0)
				{
#ifdef optimization_comp
					unsigned long* ptr_addition_char = (unsigned long*)contains_item_bot;//get_ptr_addition_value(color);
					unsigned long* ptr_values = (unsigned long*)&items_pos_bot_x;
					*ptr_values = *ptr_values + (*ptr_addition_char & 0x01010101);
#else
					for (int i = 0, l = 4; i < l; ++i)
					{
						if (contains_item_bot[i]) ++items_pos_bot_x[i];
					}
#endif
					if (items_pos_bot_x[3] >= 128)
					{
						if (remove_item<belt_utility::belt_side::bottom>(3))
						{
						}
						else if (contains_item_bot[3] != false)
						{
							cant_move_item = true;
						}
					}
				}

				if (loop_counter != 0 && loop_counter % 32 == 0)
				{
					if (item_count_top > 0 && cant_move_item == false)
					{
#ifdef optimization_comp
						//if (items_pos_top_x[3] == 97) __debugbreak();
						//we shift the contains and position arrays by 1 to the right
						unsigned long* ptr_addition_char = (unsigned long*)contains_item_top;
						*ptr_addition_char = ((unsigned long)*ptr_addition_char << 8);// ((unsigned long)*ptr_addition_char << 8) | ((unsigned long)*ptr_addition_char >> (32 - 8));
						ptr_addition_char = (unsigned long*)items_pos_top_x;
						*ptr_addition_char = ((unsigned long)*ptr_addition_char << 8); //((unsigned long)*ptr_addition_char << 8) | ((unsigned long)*ptr_addition_char >> (32 - 8))
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
					if (item_count_bot > 0 && cant_move_item == false)
					{
#ifdef optimization_comp
						//we shift the contains and position arrays by 1 to the right
						unsigned long* ptr_addition_char = (unsigned long*)contains_item_bot;
						*ptr_addition_char = ((unsigned long)*ptr_addition_char << 8) | ((unsigned long)*ptr_addition_char >> (32 - 8));
						ptr_addition_char = (unsigned long*)items_pos_bot_x;
						*ptr_addition_char = ((unsigned long)*ptr_addition_char << 8) | ((unsigned long)*ptr_addition_char >> (32 - 8));
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
				for (int i = 0; i < item_count_top; ++i)
				{
					items_top[i].position += (belt_utility::belt_direction_multi[static_cast<int>(direction)] * vec2{ static_cast<float>((belt_utility::belt_speed)(color)), static_cast<float>((belt_utility::belt_speed)(color)) });
					if (std::is_constant_evaluated() == false) ++items_moved_per_frame;

					if (outside_belt(items_top[i].position))
					{
						if (remove_item<belt_utility::belt_side::top>(i)) --i;
						else
						{
							cant_move_item = true;
						}
					}
				}
				for (int i = 0; i < item_count_bot; ++i)
				{
					items_bot[i].position += (belt_utility::belt_direction_multi[static_cast<int>(direction)] * vec2{ static_cast<float>((belt_utility::belt_speed)(color)), static_cast<float>((belt_utility::belt_speed)(color)) });
					if (std::is_constant_evaluated() == false) ++items_moved_per_frame;

					if (outside_belt(items_bot[i].position))
					{
						if (remove_item<belt_utility::belt_side::bottom>(i)) --i;
						else
						{
							cant_move_item = true;
						}
					}
				}
			}
		}
		else try_to_remove_item();
	};
};

//template<std::size_t N, template <std::size_t> typename cont_type>
consteval float test_belt(/*const cont_type<N>& arr*/) noexcept
{
	belt test_belt;
	item_array test_arr{ item{}, item{}, item{}, item{} };

	for (std::size_t i = 0; i < test_arr.size(); ++i)
	{
		if (test_belt.add_item<belt_utility::belt_side::top>(test_arr[i]) != false)
		{
		}
	}

	for (int i = 0, l = 128; i < l; ++i)
	{
		test_belt.update_belt(i);
	}

	return test_belt.get<belt_utility::belt_side::top>(0).position.x;
};

consteval float test()
{
	float* ptr = new float[10];
	for (int i = 0; i < 10; ++i)
	{
		ptr[i] = static_cast<float>(i);
	}

	ptr[5] = ptr[2];

	float return_value = ptr[5];
	delete[] ptr;
	return return_value;
};
static_assert(test() == 2.0f, "no");

constexpr float test_val = test_belt();
static_assert(test_belt() == 0.0f, "wrong count");