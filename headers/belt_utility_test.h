#pragma once

#include "belt_utility.h"
#include "item.h"
#include "vectors.h"
#include "belt_segment.h"

static auto ____test_incrementing_if_some_are_stuck(std::size_t return_index) noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{255, 0} };
	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ 0ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 64ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::log, vec2_uint{ 96ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 128ll, 0ll } });

	for (int i = 0, l = 127 + 32 + 32; i < l; ++i)
	{
		first_segment.update();
	}

	return first_segment.get_item_group(0).count(&first_segment.get_item_data_group(0).contains_item[0]);
	//return first_segment.get_item(0ull).get(1ull).position.x;
};

long long return_val = ____test_incrementing_if_some_are_stuck(0ull);
//static_assert(____test_incrementing_if_some_are_stuck(0ull) == 255ll, "no");