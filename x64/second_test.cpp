// BeltOptimizations.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include "second_test.h"

#include <chrono>
#include <vector>
#include <iostream>
#include <stdexcept>

#include "const_data.h"

#include "vectors.h"
#include "index_inserter.h"
#include "item.h"

#include "item_32.h"
#include "item_256.h"
#include "belt_segment.h"

#include "belt_utility_test.h"

#include "cpu_info.h"

constexpr const std::size_t second_test_max_belts_8 = 20000000ll;
constexpr const std::size_t throw_value = second_test_max_belts_8 * 0.97;
constexpr const std::size_t item_max_distance = second_test_max_belts_8 * 32ll;
constexpr const std::size_t item_goal_distance = (second_test_max_belts_8 / 32ll) * 32ll * 32ll * 2ll;
static_assert(item_goal_distance > item_max_distance, "item max distance is greater than the goal");
constexpr const std::size_t belts_being_simulated = second_test_max_belts_8 / 4ll;
belt_segment second_test_all_belts;

std::size_t second_test_loop_counter = 0ull;
#if __BELT_SWITCH__ == 3
constexpr const std::size_t second_test_max_belts = second_test_max_belts_8 / 32ll;
#elif __BELT_SWITCH__ == 4
constexpr const std::size_t second_test_max_belts = second_test_max_belts_8 / 256;
#endif

void second_test_belt_setup()
{
#if __BELT_SWITCH__ == 3
	second_test_all_belts = belt_segment{ vec2_uint{0, 0}, vec2_uint{ second_test_max_belts * 32ll * 32ll * 2ll, 0ll} };
	constexpr long long inserter_pos = 35000ll;
	constexpr long long max_inserters = (second_test_max_belts * 32ll) / inserter_pos - 1ll;
	for (long long i = 0, l = max_inserters; i < l; ++i)
	{
		auto inserterd_index = second_test_all_belts.add_inserter(index_inserter{ vec2_uint{inserter_pos * i + 35000ll, 32ll} });
		second_test_all_belts.get_inserter(inserterd_index).set_item_type(item_type::wood);
	}
#elif __BELT_SWITCH__ == 4
	second_test_all_belts = belt_segment{ vec2_uint{0, 0}, vec2_uint{ second_test_max_belts * 32 * 32 * 8, 0} };
#endif

	for (std::size_t i = 0, l = second_test_max_belts; i < l; ++i)
	{
#if __BELT_SWITCH__ == 3
		for (long long x = 0; x < 32; ++x)
		{
			second_test_all_belts.add_item(item_uint{ item_type::wood, vec2_uint(x * 32ll + (32ll * 32ll * i), 0ll) });
		}
#elif __BELT_SWITCH__ == 4
		for (int y = 0; y < 8; ++y)
		{
			for (int x = 0; x < 32; ++x)
			{
				all_belts.add_item(item_uint{ item_type::wood, vec2_uint(((i * (32 * 32 * 8)) + x * 32) + (32 * 32 * y), 0) });
			}
		}
#endif
	}
}

void second_test_belt_loop()
{
	second_test_all_belts.update();
}

std::size_t second_test_get_total_items_on_belts()
{
	return second_test_all_belts.count_all_items();
}

void second_belt_test()
{
	__m256i values = _mm256_set1_epi8(96);
	values.m256i_i8[0] = 0;
	values.m256i_i8[1] = 1;
	values.m256i_i8[2] = 14;

	auto temp1 = _mm256_or_si256(values, _mm256_add_epi8(values, _mm256_set1_epi8(255)));
	belt_utility::_mm256_slli_si256__p<1>(&temp1);
	auto comp_replace = _mm256_and_si256(temp1, _mm256_set1_epi8(1));
	_mm256_store_si256(&values, _mm256_sub_epi8(values, comp_replace));

	second_test_belt_setup();

	std::size_t moved_items_per_second = 0;
	std::size_t while_counter{ 0 };
	std::size_t second_counter{ 0 };
	std::size_t loop_counter{ 0 };

#if __BELT_SWITCH__ == 3
	while (while_counter < second_test_max_belts * 1000)
#elif __BELT_SWITCH__ == 4
	while (while_counter < second_test_max_belts * 1000 * 8)
#endif
	{
		auto t1 = std::chrono::high_resolution_clock::now();
		second_test_belt_loop();
		auto t2 = std::chrono::high_resolution_clock::now();

		auto ms_int = duration_cast<std::chrono::microseconds>(t2 - t1);

		second_counter += ms_int.count();
		++loop_counter;
#if __BELT_SWITCH__ == 3
		moved_items_per_second += item_32::items_moved_per_frame;
#elif __BELT_SWITCH__ == 4
		moved_items_per_second += item_256::items_moved_per_frame;
#endif
		if (second_counter >= 1000000)
		{
			std::cout << "items moved/s: " << moved_items_per_second << " - tick time: " << ms_int.count() << "microseconds - avg time: " << second_counter / loop_counter << " - loops done : " << loop_counter << " - total on belts : " << second_test_get_total_items_on_belts() << " \n";
			if (second_test_get_total_items_on_belts() < throw_value) throw std::runtime_error("");
			moved_items_per_second = 0;
			second_counter = 0;
			loop_counter = 0;
		}
#if __BELT_SWITCH__ == 3
		item_32::items_moved_per_frame = 0;
#elif __BELT_SWITCH__ == 4
		item_256::items_moved_per_frame = 0;
#endif
		++while_counter;
	}
}