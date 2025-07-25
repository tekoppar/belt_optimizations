#pragma once

#include <belt_segment.h>

extern void second_test_belt_setup(belt_segment& bs);
extern __declspec(noinline) void second_test_belt_loop(belt_segment* bs) noexcept;
extern size_t second_test_get_total_items_on_belts(belt_segment& bs) noexcept;
extern void second_belt_test();