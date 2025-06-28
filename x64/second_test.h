#pragma once

#include <iostream>
#include <belt_segment.h>

extern void second_test_belt_setup(belt_segment& second_test_all_belts) noexcept;
extern __declspec(noinline) void second_test_belt_loop(belt_segment& second_test_all_belts) noexcept;
extern std::size_t second_test_get_total_items_on_belts(belt_segment& second_test_all_belts) noexcept;
extern void second_belt_test();