// BeltOptimizations.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include "second_test.h"

#define NOMINMAX
#include <Windows.h>

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <limits>

#include "const_data.h"

#include "vectors.h"
#include "index_inserter.h"
#include "item.h"

#include "item_32.h"
#include "belt_segment.h"

#ifdef AMDUPROF_
#include <AMDProfileController.h>
#endif

#include <retard_wrapper.h>

#ifdef _DEBUG
constexpr const std::size_t second_test_max_belts_8 = 10'000'000ll;
#else
constexpr const std::size_t second_test_max_belts_8 = 200'000'000ll;
#endif
constexpr const std::size_t throw_value = static_cast<size_t>(static_cast<double>(second_test_max_belts_8) * 0.6);
constexpr const std::size_t item_max_distance = second_test_max_belts_8 * 32ll;
constexpr const std::size_t item_goal_distance_max = (second_test_max_belts_8 / 32ll) * 32ll * 32ll * 2ll;
static_assert(item_goal_distance_max > item_max_distance, "item max distance is greater than the goal");
static_assert(item_goal_distance_max < (std::numeric_limits<long long>::max)(), "max distance is greater then max value of int");
constexpr std::size_t belts_being_simulated = second_test_max_belts_8 / 4ll;
belt_segment second_test_all_belts;
static volatile belt_segment const* second_test_all_belts_ptr = nullptr;

std::size_t second_test_loop_counter = 0ull;
#if __BELT_SWITCH__ == 3
constexpr const std::size_t second_test_max_belts = second_test_max_belts_8 / 32ll;
#elif __BELT_SWITCH__ == 4
constexpr const std::size_t second_test_max_belts = second_test_max_belts_8 / 256;
#endif

void second_test_belt_setup() noexcept
{
#if __BELT_SWITCH__ == 3
	second_test_all_belts = belt_segment{ vec2_int64{0, 0}, vec2_int64{ second_test_max_belts * 32ll * 32ll * 2ll, 0ll} };
	second_test_all_belts_ptr = &second_test_all_belts;
#ifdef _DEBUG
	constexpr long long inserter_pos = 350000;// (32ll * 1024ll) + 16;
#else
	constexpr long long inserter_pos = 350000;// *((second_test_max_belts * 32ll * 32ll) / 350000 - 1ll);
#endif
	constexpr long long max_inserters = (second_test_max_belts * 32ll * 32ll) / inserter_pos - 1ll;
	constexpr long long l = max_inserters;

	std::cout << "Starting to add inserters" << std::endl;
	for (long long i = 0; i < l; ++i)
	{
		constexpr long long lx = 1;
		for (long long x = 0; x < lx; ++x)
		{
			const auto inserterd_index = second_test_all_belts.add_inserter(index_inserter{ vec2_int64{(inserter_pos * i + inserter_pos) + (x * 32ll), 32ll} });
			auto& found_inserter = second_test_all_belts.get_inserter(inserterd_index);
			found_inserter.set_item_type(item_type::wood);
		}
	}
	std::cout << "Finished adding inserters" << std::endl;
#elif __BELT_SWITCH__ == 4
	second_test_all_belts = belt_segment{ vec2_int64{0, 0}, vec2_int64{ second_test_max_belts * 32 * 32 * 8, 0} };
#endif

	long long belt_x_position = 0ll;
	constexpr size_t l2 = second_test_max_belts;
	std::cout << "Starting to add items" << std::endl;
	const auto t1 = std::chrono::high_resolution_clock::now();
	for (std::size_t i = 0; i < l2; ++i)
	{
#if __BELT_SWITCH__ == 3
		for (long long x = 0; x < 32; ++x)
		{
			second_test_all_belts.add_item(item_uint{ item_type::wood, vec2_int64(belt_x_position, 0ll) }, false);
			belt_x_position += 32ll;
		}
#elif __BELT_SWITCH__ == 4
		for (int y = 0; y < 8; ++y)
		{
			for (int x = 0; x < 32; ++x)
			{
				all_belts.add_item(item_uint{ item_type::wood, vec2_int64(((i * (32 * 32 * 8)) + x * 32) + (32 * 32 * y), 0) });
			}
		}
#endif
	}
	//second_test_all_belts.update_all_event_ticks<belt_utility::belt_direction::left_right>();

	std::cout << "Finished adding items" << std::endl;
	const auto t2 = std::chrono::high_resolution_clock::now();

	auto ms_int = duration_cast<std::chrono::milliseconds>(t2 - t1);
	std::cout << "Adding items took: " << ms_int.count() << "ms" << std::endl;
}

void second_test_belt_loop() noexcept
{
	second_test_all_belts.update();
}

std::size_t second_test_get_total_items_on_belts() noexcept
{
	return second_test_all_belts.count_all_items();
}

void second_belt_test()
{
#ifdef AMDUPROF_
	if (!amdProfileStrictResumeImpl()) throw std::runtime_error("");
	amdProfileStrictResumeImpl();
	amdProfileStrictResumeImpl();
	amdProfileStrictResumeImpl();
	amdProfileStrictResumeImpl();
	amdProfileStrictResumeImpl();
	amdProfileStrictResumeImpl();
#endif
	//auto test_goal_distance_is_all_valid_val = test_goal_distance_is_all_valid(0);
	//auto test_new_item_distance_val = test_real_game_scenario_smelters(1);
	std::cout << "Setup starting" << std::endl;
	second_test_belt_setup();
	std::cout << "Setup finished" << std::endl;

	size_t moved_items_per_second = 0;
	size_t while_counter{ 0 };
	size_t second_counter{ 0 };
	size_t loop_counter{ 0 };
	size_t zero_items_moved_counter{ 0 };

#if __BELT_SWITCH__ == 3
	while (while_counter < second_test_max_belts * 10000)
#elif __BELT_SWITCH__ == 4
	while (while_counter < second_test_max_belts * 1000 * 8)
#endif
	{
		const auto t1 = std::chrono::high_resolution_clock::now();
		second_test_belt_loop();
		const auto t2 = std::chrono::high_resolution_clock::now();

		auto ms_int = duration_cast<std::chrono::nanoseconds>(t2 - t1);

		second_counter += ms_int.count();
		++loop_counter;
#if __BELT_SWITCH__ == 3
		moved_items_per_second += item_32::items_moved_per_frame;
#elif __BELT_SWITCH__ == 4
		moved_items_per_second += item_256::items_moved_per_frame;
#endif
		if (item_32::items_moved_per_frame == 0) ++zero_items_moved_counter;
		else zero_items_moved_counter = 0;

		if (zero_items_moved_counter >= 1024)
		{
			const auto total_items_on_belt = second_test_get_total_items_on_belts();
			std::cout << "items moved/s: " << moved_items_per_second << " - tick time: " << ms_int.count() << "nanoseconds - avg time: " << second_counter / loop_counter << " - second counter: " << second_counter << " - loops done : " << loop_counter << " - total on belts : " << total_items_on_belt << " \n";
			return;

#ifdef AMDUPROF_
			if (!amdProfilePauseImpl()) throw std::runtime_error("");
#endif
		}

		if (second_counter >= 1000000000)
		{
			//if (second_test_all_belts_ptr == nullptr) __debugbreak();
			const auto total_items_on_belt = second_test_get_total_items_on_belts();
			std::cout << "items moved/s: " << moved_items_per_second << " - tick time: " << ms_int.count() << "nanoseconds - avg time: " << second_counter / loop_counter << " - loops done : " << loop_counter << " - total on belts : " << total_items_on_belt << " \n";
			if (total_items_on_belt < throw_value)
			{
#ifdef AMDUPROF_
				if (!amdProfilePauseImpl()) throw std::runtime_error("");
#endif
				return;
			}
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