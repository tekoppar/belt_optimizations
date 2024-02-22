#pragma once

#include <iostream>

extern void second_test_belt_setup() noexcept;
extern __declspec(noinline) void second_test_belt_loop() noexcept;
extern std::size_t second_test_get_total_items_on_belts() noexcept;
extern void second_belt_test();