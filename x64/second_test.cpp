// BeltOptimizations.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include "second_test.h"

#include <chrono>
#include <iostream>
#include <limits>

#include "vectors.h"
#include "index_inserter.h"
#include "item.h"

#include "belt_segment.h"

#ifdef AMDUPROF_
#include <AMDProfileController.h>
#endif

#ifdef _DEBUG
constexpr const std::size_t second_test_max_belts_8 = 10'000'000ll;
#else
constexpr const size_t second_test_max_belts_8 = 2'00'000'000ll;
#endif
constexpr const size_t throw_value = static_cast<size_t>(static_cast<double>(second_test_max_belts_8) * 0.6);
constexpr const size_t item_max_distance = second_test_max_belts_8 * 32ll;
constexpr const size_t item_goal_distance_max = (second_test_max_belts_8 / 32ll) * 32ll * 32ll * 2ll;
static_assert(item_goal_distance_max > item_max_distance, "item max distance is greater than the goal");
static_assert(item_goal_distance_max < (std::numeric_limits<long long>::max)(), "max distance is greater then max value of int");
constexpr size_t belts_being_simulated = second_test_max_belts_8 / 4ll;
static volatile belt_segment const* second_test_all_belts_ptr = nullptr;

size_t second_test_loop_counter = 0ull;
constexpr const size_t second_test_max_belts = second_test_max_belts_8 / 32ll;

void second_test_belt_setup(belt_segment& bs)
{
	bs = belt_segment{ vec2_int64{0, 0}, vec2_int64{ second_test_max_belts * 32ll * 32ll * 2ll, 0ll} };
	second_test_all_belts_ptr = &bs;
#ifdef _DEBUG
	constexpr long long inserter_pos = 4096 - 128;// (32ll * 1024ll) + 16;
#else
	constexpr long long inserter_pos = 3500000;// *((second_test_max_belts * 32ll * 32ll) / 350000 - 1ll);
#endif
	constexpr long long max_inserters = (second_test_max_belts * 32ll * 32ll) / inserter_pos - 1ll;
	constexpr long long l = 8;// max_inserters;

	std::cout << "Starting to add " << max_inserters << " inserters" << std::endl;
	for (long long i = 0; i < l; ++i)
	{
		constexpr long long lx = 1;
		for (long long x = 0; x < lx; ++x)
		{
			const auto inserterd_index = bs.add_inserter(index_inserter{ vec2_int64{(inserter_pos * i + inserter_pos) + (x * 32ll), 32ll} });
			auto& found_inserter = bs.get_belt_inserter(inserterd_index);
			found_inserter.set_item_type(item_type::wood);
		}
	}
	std::cout << "Finished adding inserters" << std::endl;

	long long belt_x_position = 0ll;
	constexpr size_t l2 = second_test_max_belts;
	std::cout << "Starting to add " << second_test_max_belts_8 << " items" << std::endl;
	const auto t1 = std::chrono::high_resolution_clock::now();
	for (size_t i = 0; i < l2; ++i)
	{
		for (long long x = 0; x < 32; ++x)
		{
			bs.add_item(item_uint{ item_type::wood, vec2_int64(belt_x_position, 0ll) }, false);
			belt_x_position += 32ll;
		}
	}

	std::cout << "Finished adding items" << std::endl;
	const auto t2 = std::chrono::high_resolution_clock::now();

	auto ms_int = duration_cast<std::chrono::milliseconds>(t2 - t1);
	std::cout << "Adding items took: " << ms_int.count() << "ms" << std::endl;
}

void second_test_belt_loop(belt_segment* bs) noexcept
{
	bs->update();
}

size_t second_test_get_total_items_on_belts(belt_segment& bs) noexcept
{
	return bs.count_all_items();
}

void second_belt_test()
{
	belt_segment second_test_all_belts;
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
	second_test_belt_setup(second_test_all_belts);
	std::cout << "Setup finished" << std::endl;

	size_t moved_items_per_second = 0;
	size_t while_counter{ 0 };
	size_t second_counter{ 0 };
	size_t loop_counter{ 0 };
	//size_t zero_items_moved_counter{ 0 };

	while (while_counter < second_test_max_belts * 10000)
	{
		const auto t1 = std::chrono::high_resolution_clock::now();
		second_test_belt_loop(&second_test_all_belts);
		const auto t2 = std::chrono::high_resolution_clock::now();

		auto ms_int = duration_cast<std::chrono::nanoseconds>(t2 - t1);

		second_counter += ms_int.count();
		++loop_counter;
		moved_items_per_second += second_test_all_belts.items_moved_per_frame;
		second_test_all_belts.items_moved_per_frame = 0ull;
		/*if (item_32::items_moved_per_frame == 0) ++zero_items_moved_counter;
		else zero_items_moved_counter = 0;

		if (zero_items_moved_counter >= 1024)
		{
			const auto total_items_on_belt = second_test_get_total_items_on_belts();
			std::cout << "items moved/s: " << moved_items_per_second << " - tick time: " << ms_int.count() << "nanoseconds - avg time: " << second_counter / loop_counter << " - second counter: " << second_counter << " - loops done : " << loop_counter << " - total on belts : " << total_items_on_belt << " \n";
			return;

#ifdef AMDUPROF_
			if (!amdProfilePauseImpl()) throw std::runtime_error("");
#endif
		}*/

		if (second_counter >= 1000000000)
		{
			//if (second_test_all_belts_ptr == nullptr) __debugbreak();
			const auto total_items_on_belt = second_test_get_total_items_on_belts(second_test_all_belts);
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

		++while_counter;
	}
}