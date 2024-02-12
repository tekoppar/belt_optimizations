// BeltOptimizations.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include "first_test.h"

#include <chrono>
#include <vector>

#include "const_data.h"

#include "belt.h"
#include "belt_8.h"
#include "belt_32.h"
#include "item_32.h"
#include "item_256.h"
#include "belt_segment.h"

constexpr const std::size_t max_belts_8 = 20000000;

std::size_t loop_counter = 0;
#if __BELT_SWITCH__ == 0
std::vector<belt> all_belts;
constexpr const std::size_t max_belts = max_belts_8;
#elif __BELT_SWITCH__ == 1
std::vector<belt_8> all_belts;
constexpr const std::size_t max_belts = max_belts_8 / 8;
#elif __BELT_SWITCH__ == 2
std::vector<belt_32> all_belts;
constexpr const std::size_t max_belts = max_belts_8 / 8;
#elif __BELT_SWITCH__ == 3
std::vector<item_32> all_belts;
constexpr const std::size_t max_belts = max_belts_8 / 32;
#elif __BELT_SWITCH__ == 4
std::vector<item_256> all_belts;
constexpr const std::size_t max_belts = max_belts_8 / 256;
#endif

void first_test_belt_setup()
{
#if __BELT_SWITCH__ == 0
	for (std::size_t i = 0, l = max_belts; i < l; ++i)
	{
		all_belts.emplace_back(vec2{ 128.0f * 8 * static_cast<float>(i), 0.0f }, belt_direction::left_right);
	}
#elif __BELT_SWITCH__ == 1
	for (std::size_t i = 0, l = max_belts; i < l; ++i)
	{
		all_belts.emplace_back(vec2{ 128.0f * 8 * static_cast<float>(i), 0.0f }, belt_direction::left_right);
	}
#elif __BELT_SWITCH__ == 2
	for (std::size_t i = 0, l = max_belts; i < l; ++i)
	{
		all_belts.emplace_back(vec2{ 128.0f * 8 * static_cast<float>(i), 0.0f }, belt_direction::left_right);
	}
#elif __BELT_SWITCH__ == 3
	for (std::size_t i = 0, l = max_belts; i < l; ++i)
	{
		all_belts.emplace_back(belt_utility::belt_direction::left_right, max_belts * 32 * 32);
	}
#elif __BELT_SWITCH__ == 4
	for (std::size_t i = 0, l = max_belts; i < l; ++i)
	{
		all_belts.emplace_back(belt_direction::left_right, max_belts * 32 * 32 * 8);
	}
#endif

	for (std::size_t i = 0, l = max_belts; i < l; ++i)
	{
#if __BELT_SWITCH__ == 0
		if (i + 1 != max_belts) all_belts[i].add_neighbour<belt_neighbour::right>(&all_belts[i + 1]);
		all_belts[i].add_item<belt_side::top>(item{});
#elif __BELT_SWITCH__ == 1
		if (i + 1 != max_belts) all_belts[i].add_neighbour<belt_neighbour::right>(&all_belts[i + 1]);
		for (int x = 0; x < 8; ++x)
		{
			all_belts[i].add_item<belt_side::top>(x, item{});
		}
#elif __BELT_SWITCH__ == 2
		if (i + 1 != max_belts) all_belts[i].add_neighbour<belt_neighbour::right>(&all_belts[i + 1]);
		for (int x = 0; x < 8; ++x)
		{
			all_belts[i].add_item<belt_side::top>(item{ item_type::wood }, x * 4);
		}
#elif __BELT_SWITCH__ == 3
		for (int x = 0; x < 32; ++x)
		{
			all_belts[i].add_item(belt_item{ item_type::wood }, vec2_uint((i * (32 * 32)) + x * 32, 0));
		}
#elif __BELT_SWITCH__ == 4
		for (int y = 0; y < 8; ++y)
		{
			for (int x = 0; x < 32; ++x)
			{
				all_belts[i].add_item(belt_item{ item_type::wood }, vec2_uint(((i * (32 * 32 * 8)) + x * 32) + (32 * 32 * y), 0), y);
			}
		}
#endif
	}
}

__declspec(noinline) void first_test_belt_loop()
{
	++loop_counter;
	long long i = all_belts.size() - 1;

#if __BELT_SWITCH__ == 0
	if (loop_counter != 0 && loop_counter % 32 == 0)
	{
		for (; i >= 0; --i)
		{
			all_belts[i].update_belt();
			all_belts[i].shift_items_belt();
		}
	}
	else
	{
		for (; i >= 0; --i)
		{
			all_belts[i].update_belt();
		}
	}
#elif __BELT_SWITCH__ == 1
	if (loop_counter != 0 && loop_counter % 32 == 0)
	{
		for (; i >= 0; --i)
		{
			all_belts[i].update_belt();
			all_belts[i].shift_items_belt();
		}
	}
	else
	{
		for (; i >= 0; --i)
		{
			all_belts[i].update_belt();
		}
	}
#elif __BELT_SWITCH__ == 2
	if (loop_counter != 0 && loop_counter % 32 == 0)
	{
		for (; i >= 0; --i)
		{
			all_belts[i].update_belt();
			all_belts[i].shift_items_belt();
		}
	}
	else
	{
		for (; i >= 0; --i)
		{
			all_belts[i].update_belt();
		}
	}
#elif __BELT_SWITCH__ == 3
	for (; i >= 0; --i)
	{
		all_belts[i].update_belt();
	}
#elif __BELT_SWITCH__ == 4
	for (; i >= 0; --i)
	{
		all_belts[i].update_belt();
	}
#endif

	/*if (loop_counter % 32 == 0)
	{
		all_belts[0].add_item<belt_side::top>(item{});
	}*/
}

std::size_t first_test_get_total_items_on_belts()
{
	std::size_t total_count = 0;
	for (auto& ptr : all_belts)
	{
#if __BELT_SWITCH__ == 0
		total_count += ptr.count();
#elif __BELT_SWITCH__ == 1
		total_count += ptr.count();
#elif __BELT_SWITCH__ == 2
		total_count += ptr.count();
#elif __BELT_SWITCH__ == 3
		total_count += ptr.count();
#elif __BELT_SWITCH__ == 4
		for (int i = 0, l = 8; i < l; ++i)
		{
			total_count += ptr.count(i);
		}
#endif
	}

	return total_count;
};

void first_belt_test()
{
	first_test_belt_setup();

	std::size_t moved_items_per_second = 0;
	std::size_t while_counter{ 0 };
	std::size_t second_counter{ 0 };
	std::size_t loop_counter{ 0 };
#if __BELT_SWITCH__ == 0
	while (while_counter < max_belts * 1000)
#elif __BELT_SWITCH__ == 1
	while (while_counter < max_belts * 1000)
#elif __BELT_SWITCH__ == 2
	while (while_counter < max_belts * 1000)
#elif __BELT_SWITCH__ == 3
	while (while_counter < max_belts * 1000)
#elif __BELT_SWITCH__ == 4
	while (while_counter < max_belts * 1000 * 8)
#endif
	{
		auto t1 = std::chrono::high_resolution_clock::now();
		first_test_belt_loop();
		auto t2 = std::chrono::high_resolution_clock::now();

		auto ms_int = duration_cast<std::chrono::microseconds>(t2 - t1);

		second_counter += ms_int.count();
		++loop_counter;
#if __BELT_SWITCH__ == 0
		moved_items_per_second += belt::items_moved_per_frame;
#elif __BELT_SWITCH__ == 1
		moved_items_per_second += belt_8::items_moved_per_frame;
#elif __BELT_SWITCH__ == 2
		moved_items_per_second += belt_32::items_moved_per_frame;
#elif __BELT_SWITCH__ == 3
		moved_items_per_second += item_32::items_moved_per_frame;
#elif __BELT_SWITCH__ == 4
		moved_items_per_second += item_256::items_moved_per_frame;
#endif
		if (second_counter >= 1000000)
		{
			std::cout << "items moved/s: " << moved_items_per_second << " - tick time: " << ms_int.count() << "microseconds - loops done: " << loop_counter << " - total on belts: " << first_test_get_total_items_on_belts() << "\n";
			moved_items_per_second = 0;
			second_counter = 0;
			loop_counter = 0;
		}
#if __BELT_SWITCH__ == 0
		belt::items_moved_per_frame = 0;
#elif __BELT_SWITCH__ == 1
		belt_8::items_moved_per_frame = 0;
#elif __BELT_SWITCH__ == 2
		belt_32::items_moved_per_frame = 0;
#elif __BELT_SWITCH__ == 3
		item_32::items_moved_per_frame = 0;
#elif __BELT_SWITCH__ == 4
		item_256::items_moved_per_frame = 0;
#endif
		++while_counter;
	}
}