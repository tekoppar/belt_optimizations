#pragma once

#include "belt_segment.h"


CONSTEXPR_VAR auto test_belt_segment() noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{255, 0} };
	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ 0ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 64ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::log, vec2_uint{ 96ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 128ll, 0ll } });

	//return first_segment.get_item(0).get_distance_to_item(2);
	//return first_segment.get_item(0).get_direction_position() - first_segment.get_item(0).get_distance_to_item(3);
	//first_segment.add_item(item_uint{ test_arr[5].type, vec2_uint{ 42ll, 0ll } });
	//return first_segment.get_item(0).count();
	//return first_segment.get_item(0).get(4).type;
	//used to check support that adding items in between is possible
	//test_belt.add_item(test_arr[4], vec2_uint{ 0ll, 0ll });
	//test_belt.add_item(test_arr[5], vec2_uint{ 32ll, 0ll });
	//return test_belt.get(4).type; 

	for (int i = 0, l = 128 + 32; i < l; ++i)
	{
		first_segment.update();
	}

	return first_segment.get_item(0, 0).position.x;
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto test_val_belt_segment = test_belt_segment();
static_assert(test_belt_segment() == 255, "wrong count");
#endif


CONSTEXPR_VAR auto test_moving_items_between_belt_segments(std::size_t return_index) noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{255, 0} };
	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ 0ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 64ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::log, vec2_uint{ 96ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 128ll, 0ll } });
	belt_segment second_segment{ vec2_uint{255, 0}, vec2_uint{1024, 0} };
	first_segment.add_end_segment_section(&second_segment);
	for (int i = 0, l = 127 + 32 + 32; i < l; ++i)
	{
		second_segment.update();
		first_segment.update();
	}

	//return first_segment.get_item_group(0).count();
	//return second_segment.get_item_group(0).count();
	//return second_segment.count_item_groups();
	//return first_segment.get_item_group(0).get(return_index).position.x;
	//return second_segment.get_item_group(0).get_goal();
	return second_segment.get_item(0, tc::sign(return_index)).position.x;
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto moving_items_between_belts_val = test_moving_items_between_belt_segments(0);
static_assert(test_moving_items_between_belt_segments(0) == 319ll, "item did not jump to the second segment");
#endif


CONSTEXPR_VAR auto test_removing_item_group(std::size_t return_index) noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{255, 0} };
	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ 0ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 64ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::log, vec2_uint{ 96ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 128ll, 0ll } });
	belt_segment second_segment{ vec2_uint{255, 0}, vec2_uint{1024, 0} };
	first_segment.add_end_segment_section(&second_segment);

	for (int i = 0, l = 128 + 32; i < l; ++i)
	{
		first_segment.update();
		second_segment.update();
	}

	//return first_segment.goal_distance_in_destinations(0);
	if (return_index == 0) return first_segment.get_item_group(0).count();
	else return second_segment.get_item_group(0).count();
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto test_removing_item_group_val = test_removing_item_group(0);
static_assert(test_removing_item_group(0) == 2, "item did not jump to the second segment");
static_assert(test_removing_item_group(1) == 2, "item did not jump to the second segment");
#endif


CONSTEXPR_VAR auto test_incrementing_if_some_are_stuck(std::size_t return_index) noexcept
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

	return first_segment.get_item(0, return_index).position.x;
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto some_are_stuck3 = test_incrementing_if_some_are_stuck(3);
constexpr auto some_are_stuck2 = test_incrementing_if_some_are_stuck(2);
constexpr auto some_are_stuck1 = test_incrementing_if_some_are_stuck(1);
constexpr auto some_are_stuck0 = test_incrementing_if_some_are_stuck(0);
static_assert(test_incrementing_if_some_are_stuck(3) == 159, "position is wrong, incrementing position while items are stuck doesn't work for 3");
static_assert(test_incrementing_if_some_are_stuck(2) == 191, "position is wrong, incrementing position while items are stuck doesn't work for 2");
static_assert(test_incrementing_if_some_are_stuck(1) == 223, "position is wrong, incrementing position while items are stuck doesn't work for 1");
static_assert(test_incrementing_if_some_are_stuck(0) == 255, "position is wrong, incrementing position while items are stuck doesn't work for 0");
#endif


CONSTEXPR_VAR auto test_adding_in_middle(std::size_t return_index) noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{400, 0} };
	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ 10ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 160ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::log, vec2_uint{ 224ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 288ll, 0ll } });

	auto val = first_segment.add_item(item_uint{ item_type::brick, vec2_uint{ 42ll, 0ll } });
	//return first_segment.get_item_group(0).count();
	if (!val) return item_type::pink_square;
	return first_segment.get_item(0, return_index).type;
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto in_middle2_val = test_adding_in_middle(4);
static_assert(test_adding_in_middle(2) == item_type::stone, "wrong type so adding in the middle got something wrong");
static_assert(test_adding_in_middle(3) == item_type::brick, "wrong type so adding in the middle got something wrong");
//static_assert(test_adding_in_middle(4) == item_type::wood, "wrong type so adding in the middle got something wrong");
#endif


CONSTEXPR_VAR auto test_last_item_distance() noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{400, 0} };
	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ 10ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 160ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::log, vec2_uint{ 224ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 288ll, 0ll } });

	for (int i = 0, l = 170; i < l; ++i)
	{
		first_segment.update();
	}
	//return first_segment.get_item_group(0).get_item_direction_position(first_segment.segment_direction, first_segment.get_end_distance_direction(), first_segment.get_new_item_distance(0), first_segment.get_item_data_group(0), 3ll);
	return first_segment.get_item_group(0).get_last_item_direction_position(first_segment.segment_direction, first_segment.get_end_distance_direction(), first_segment.item_groups_distance_between[0], first_segment.get_item_data_group(0));
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto test_last_item_distance_val = test_last_item_distance();
static_assert(test_last_item_distance() == 180, "item distance is incorrect");
#endif


CONSTEXPR_VAR auto test_item_distance(std::size_t return_index) noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{400, 0} };
	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ 10ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 160ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::log, vec2_uint{ 224ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 288ll, 0ll } });

	first_segment.get_item_group(0).remove_item(&first_segment.item_groups_distance_between[return_index], first_segment.get_item_data_group(0), return_index);
	//return first_segment.get_item_group(0).get_item_direction_position(first_segment.segment_direction, first_segment.get_end_distance_direction(), first_segment.get_new_item_distance(0), first_segment.get_item_data_group(0), return_index);
	return first_segment.get_item_group(0).get_distance_to_item(first_segment.get_item_data_group(0), return_index);
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto test_item_distance_val = test_item_distance(1);
static_assert(test_item_distance(1) == 128, "item distance is incorrect");
#endif


CONSTEXPR_VAR auto test_inserter_item() noexcept
{
	belt_segment first_segment{ vec2_uint{0ll, 0ll}, vec2_uint{400ll, 0ll} };

	first_segment.add_inserter(index_inserter{ vec2_uint{320ll, 32ll} });
	first_segment.get_inserter(0).set_item_type(item_type::stone);

	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ 10ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 160ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::log, vec2_uint{ 224ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 288ll, 0ll } });

	for (int i = 0, l = 128 + 32; i < l; ++i)
	{
		first_segment.update();
	}
	//auto test_type_pos = first_segment.get_item_group(0).get_first_item_of_type_before_position(first_segment.segment_direction, first_segment.get_end_distance_direction(), first_segment.get_new_item_distance(0), first_segment.get_item_data_group(0), item_type::stone, vec2_uint{ 320ll , 0ll });
	//return test_type_pos;
	return first_segment.get_inserter(0).get_item().type;
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto test_inserter_item_val = test_inserter_item();
static_assert(test_inserter_item() == item_type::stone, "inserter hasn't grabbed item");
#endif


CONSTEXPR_VAR auto test_inserter_can_grab_all_items() noexcept
{
	belt_segment first_segment{ vec2_uint{0ll, 0ll}, vec2_uint{1024ll, 0ll} };

	first_segment.add_inserter(index_inserter{ vec2_uint{448ll, 32ll} });
	first_segment.get_inserter(0).set_item_type(item_type::stone);

	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 0ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 32ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 64ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 96ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 128ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 160ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 192ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 224ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 256ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 288ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 320ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 352ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 384ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 416ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 448ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 480ll, 0ll } });

	auto real_pos = first_segment.get_item_group(0).get_item_direction_position(first_segment.segment_direction, first_segment.get_end_distance_direction(), first_segment.get_new_item_distance(0), first_segment.get_item_data_group(0), 13);

	first_segment.get_item_group(0).remove_item(&first_segment.item_groups_distance_between[0], first_segment.get_item_data_group(0), 12);
	first_segment.get_item_group(0).remove_item(&first_segment.item_groups_distance_between[0], first_segment.get_item_data_group(0), 11);
	first_segment.get_item_group(0).remove_item(&first_segment.item_groups_distance_between[0], first_segment.get_item_data_group(0), 10);
	first_segment.get_item_group(0).remove_item(&first_segment.item_groups_distance_between[0], first_segment.get_item_data_group(0), 9);
	first_segment.get_item_group(0).remove_item(&first_segment.item_groups_distance_between[0], first_segment.get_item_data_group(0), 8);
	first_segment.get_item_group(0).remove_item(&first_segment.item_groups_distance_between[0], first_segment.get_item_data_group(0), 7);
	first_segment.get_item_group(0).remove_item(&first_segment.item_groups_distance_between[0], first_segment.get_item_data_group(0), 6);
	first_segment.get_item_group(0).remove_item(&first_segment.item_groups_distance_between[0], first_segment.get_item_data_group(0), 5);
	first_segment.get_item_group(0).remove_item(&first_segment.item_groups_distance_between[0], first_segment.get_item_data_group(0), 4);
	first_segment.get_item_group(0).remove_item(&first_segment.item_groups_distance_between[0], first_segment.get_item_data_group(0), 3);

	auto new_pos = first_segment.get_item_group(0).get_item_direction_position(first_segment.segment_direction, first_segment.get_end_distance_direction(), first_segment.get_new_item_distance(0), first_segment.get_item_data_group(0), first_segment.get_item_group(0).count() - 3);

	//auto test_type_pos = first_segment.get_item_group(0).get_first_item_of_type_before_position(first_segment.segment_direction, first_segment.get_end_distance_direction(), first_segment.get_new_item_distance(0), first_segment.get_item_data_group(0), item_type::stone, vec2_uint{ 448ll , 0ll });
	//return test_type_pos;
	//return real_pos;
	//return new_pos;
	return real_pos == new_pos;
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto test_inserter_can_grab_all_items_val = test_inserter_can_grab_all_items();
static_assert(test_inserter_can_grab_all_items() == true, "item position moved when it shouldn't have");
#endif


/*CONSTEXPR_VAR auto test_inserter_moved_to_no_group() noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{1024, 0} };

	first_segment.add_inserter(index_inserter{ vec2_uint{256 + 64, 32} });
	first_segment.get_inserter(0).set_item_type(item_type::stone);

	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ 288ll - 278ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 288ll - 128ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::log, vec2_uint{ 288ll - 64ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 288ll, 0ll } });

	for (int i = 0, l = 256 + 128; i < l; ++i)
	{
		first_segment.update();
	}

	return first_segment.get_inserter(0).has_linked_list_data();
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto test_moved_group_val = test_inserter_moved_to_no_group();
static_assert(test_inserter_moved_to_no_group() == true, "inserter hasn't been moved");
#endif*/

CONSTEXPR_VAR auto test_item_groups_distance_between_value(int index) noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{2048, 0} };

	for (int i = 0, l = 32; i < l; ++i) first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ i * 32ll, 0ll } });
	//for (int i = 0, l = 32; i < l; ++i) first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ (i * 32ll) + 4096ll, 0ll } });
	//for (int i = 0, l = 32; i < l; ++i) first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ (i * 32ll) + (4096ll + 4096ll), 0ll } });
	//for (int i = 0, l = 32; i < l; ++i) first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ (i * 32ll) + (4096ll + 4096ll + 12288ll), 0ll } });


	//return first_segment.get_item_group(1).get_direction_position(first_segment.get_end_distance_direction(), first_segment.goal_distance_in_group(1));
	//return first_segment.get_item_group(1).get_last_item_direction_position(first_segment.segment_direction, first_segment.get_end_distance_direction(), first_segment.get_item_data_group(1));
	//return first_segment.get_new_item_distance(index);
	return first_segment.get_item(0, index).position.x;
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto test_item_groups_distance_between_val = test_item_groups_distance_between_value(0);
static_assert(test_item_groups_distance_between_value(31) == 0ll, "inserter hasn't been moved");
static_assert(test_item_groups_distance_between_value(30) == 32ll, "inserter hasn't been moved");
static_assert(test_item_groups_distance_between_value(29) == 64ll, "inserter hasn't been moved");
static_assert(test_item_groups_distance_between_value(28) == 96ll, "inserter hasn't been moved");
#endif


CONSTEXPR_VAR auto test_new_item_distance_vectors(int index) noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{1024 * 32, 0} };

	for (int i = 0, l = 32; i < l; ++i) first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ i * 32ll, 0ll } });
	for (int i = 0, l = 32; i < l; ++i) first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ (i * 32ll) + 4096ll, 0ll } });
	for (int i = 0, l = 32; i < l; ++i) first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ (i * 32ll) + (4096ll + 4096ll), 0ll } });
	for (int i = 0, l = 32; i < l; ++i) first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ (i * 32ll) + (4096ll + 4096ll + 12288ll), 0ll } });

	//return first_segment.get_item_group(0).count();
	//return first_segment.get_item_group(1).get_direction_position(first_segment.get_end_distance_direction(), first_segment.goal_distance_in_group(1));
	//return first_segment.get_item_group(1).get_last_item_direction_position(first_segment.segment_direction, first_segment.get_end_distance_direction(), first_segment.get_item_data_group(1));
	return first_segment.get_new_item_position(index);
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto test_new_item_distance_val = test_new_item_distance_vectors(0);
static_assert(test_new_item_distance_vectors(0) == 1024ll, "inserter hasn't been moved");
static_assert(test_new_item_distance_vectors(1) == 1024 + 4096, "inserter hasn't been moved");
static_assert(test_new_item_distance_vectors(2) == 1024 + 4096 + 4096, "inserter hasn't been moved");
static_assert(test_new_item_distance_vectors(3) == 1024 + 4096 + 4096 + 12288, "inserter hasn't been moved");
#endif


/*CONSTEXPR_VAR auto test_multiple_inserters_getting_destination_slots(auto i) noexcept
{
	belt_segment first_segment{ vec2_uint{0ll, 0ll}, vec2_uint{400ll, 0ll} };

	first_segment.add_inserter(index_inserter{ vec2_uint{0ll + ((256 * 32) * 5), 32ll} });
	first_segment.add_inserter(index_inserter{ vec2_uint{0ll, 32ll} });
	first_segment.add_inserter(index_inserter{ vec2_uint{0ll + ((256 * 32) * 2), 32ll} });
	first_segment.add_inserter(index_inserter{ vec2_uint{0ll + ((256 * 32) * 1), 32ll} });
	first_segment.add_inserter(index_inserter{ vec2_uint{0ll + ((256 * 32) * 3), 32ll} });
	first_segment.add_inserter(index_inserter{ vec2_uint{0ll + ((256 * 32) * 4), 32ll} });

	//return first_segment.get_item_groups_goal_distance_size();
	//return first_segment.count_inserters();
	return first_segment.has_goal_distance_slot(i);
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto test_multiple_inserters_getting_destination_slots_val = test_multiple_inserters_getting_destination_slots(0);
static_assert(test_multiple_inserters_getting_destination_slots(0) == true, "inserter 0 doesn't have a slot");
static_assert(test_multiple_inserters_getting_destination_slots(1) == true, "inserter 1 doesn't have a slot");
static_assert(test_multiple_inserters_getting_destination_slots(2) == true, "inserter 2 doesn't have a slot");
static_assert(test_multiple_inserters_getting_destination_slots(3) == true, "inserter 3 doesn't have a slot");
static_assert(test_multiple_inserters_getting_destination_slots(4) == true, "inserter 4 doesn't have a slot");
static_assert(test_multiple_inserters_getting_destination_slots(5) == true, "inserter 5 doesn't have a slot");
#endif*/

struct booleans
{
	bool a{ false };
	bool b{ false };
	bool c{ false };
	bool d{ false };
	bool e{ false };
	bool f{ false };

	constexpr bool operator==(const booleans& rhs) const noexcept
	{
		return a == rhs.a && b == rhs.b && c == rhs.c && d == rhs.d && e == rhs.e && f == rhs.f;
	};
};
CONSTEXPR_VAR auto test_mixing_inserters_and_item_groups(std::size_t return_index) noexcept
{
	booleans count_conditions{};
	belt_segment first_segment{ vec2_uint{0ll, 0ll}, vec2_uint{14096ll, 0ll} };
	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ 0ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 64ll, 0ll } });

	if (first_segment.goal_distance_in_destinations(0) == 14096ll - 64ll) count_conditions.a = true;

	first_segment.add_inserter(index_inserter{ vec2_uint{256ll, 32ll} });

	first_segment.add_item(item_uint{ item_type::log, vec2_uint{ 96ll, 0ll } });

	first_segment.add_inserter(index_inserter{ vec2_uint{1024ll, 32ll} });

	first_segment.add_item(item_uint{ item_type::copper, vec2_uint{ 128ll + 256ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 2048ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 4048ll, 0ll } });

	const int l = 128 + 32;
	for (int i = 0; i < l; ++i)
	{
		first_segment.update();
	}

	//return first_segment.goal_distance_in_destinations(0);
	if (first_segment.get_item_group(0).count() == 3) count_conditions.c = true;
	if (first_segment.get_item_groups_goal_distance_size() == 3ll) count_conditions.f = true;

	return count_conditions;
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto test_mixing_inserters_and_item_groups_val = test_mixing_inserters_and_item_groups(0);
static_assert(test_mixing_inserters_and_item_groups(0) == booleans{ true, false, true, false, false, true }, "item did not jump to the second segment");
static_assert(test_mixing_inserters_and_item_groups(0) == booleans{ true, false, true, false, false, true }, "item did not jump to the second segment");
#endif


CONSTEXPR_VAR auto test_splitting_item_groups(int index) noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{1024 * 32, 0} };

	const int l = 32;
	for (int i = 0; i < l; ++i) first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ i * 128ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ (10 * 128ll) + 32ll, 0ll } });

	//return first_segment.get_item_group(0).count();
	//return first_segment.get_item_group(1).get_direction_position(first_segment.get_end_distance_direction(), first_segment.goal_distance_in_group(1));
	//return first_segment.get_item_group(1).get_last_item_direction_position(first_segment.segment_direction, first_segment.get_end_distance_direction(), first_segment.get_item_data_group(1));
	//return first_segment.get_new_item_position(index);
	return first_segment.count_items_in_group(index);
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto test_splitting_item_groups_val = test_splitting_item_groups(0);
static_assert(test_splitting_item_groups(0) == 12ll, "inserter hasn't been moved");
static_assert(test_splitting_item_groups(1) == 21ll, "inserter hasn't been moved");
#endif


CONSTEXPR_VAR auto test_goal_distance_is_all_valid(int index) noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{1024 * 32, 0} };
	for (int i = 0, l = 32; i < l; ++i)
	{
		if (i == 0)
		{
			first_segment.add_inserter(index_inserter{ vec2_uint{ i * 128ll, 0ll } });
			first_segment.get_inserter(i).set_item_type(item_type::wood);
		}
		else
		{
			auto inserter_old_count = first_segment.count_inserters();
			first_segment.add_inserter(index_inserter{ vec2_uint{ (i * 128ll) + 256ll, 0ll } });
			if (inserter_old_count + 1 != first_segment.count_inserters()) throw std::runtime_error("");
			first_segment.get_inserter(i).set_item_type(item_type::wood);
		}
	}
	for (int i = 0, l = 64; i < l; ++i)
	{
		if (i == 0) first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ i * 128ll, 0ll } });
		else first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ (i * 128ll) + 128ll, 0ll } });
	}

	for (int i = 0, l = 128 + 32; i < l; ++i)
	{
		first_segment.update();
	}

	//return first_segment.get_new_item_position(index);
	//return first_segment.get_inserter(index).has_linked_list_distance();
	//return first_segment.get_item_group(0).count();
	//return first_segment.get_item_group(1).get_direction_position(first_segment.get_end_distance_direction(), first_segment.goal_distance_in_group(1));
	//return first_segment.get_item_group(1).get_last_item_direction_position(first_segment.segment_direction, first_segment.get_end_distance_direction(), first_segment.get_item_data_group(1));
	const bool return_val = first_segment.get_goal_distance(index).get_index_ptr() != nullptr;
	return return_val;
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto test_goal_distance_is_all_valid_val = test_goal_distance_is_all_valid(1);
#endif


CONSTEXPR_VAR auto test_real_game_scenario_smelters(int index) noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{1024 * 32, 0} };
	for (int i = 0, l = 32; i < l; ++i)
	{
		if (i == 0)
		{
			first_segment.add_inserter(index_inserter{ vec2_uint{ i * 128ll, 0ll } });
			first_segment.get_inserter(i).set_item_type(item_type::wood);
		}
		else
		{
			first_segment.add_inserter(index_inserter{ vec2_uint{ (i * 128ll) + 256ll, 0ll } });
			first_segment.get_inserter(i).set_item_type(item_type::wood);
		}
	}
	for (int i = 0, l = 64; i < l; ++i)
	{
		if (i == 0) first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ i * 128ll, 0ll } });
		else first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ (i * 128ll) + 128ll, 0ll } });
	}

	for (int i = 0, l = 128 + 32; i < l; ++i)
	{
		first_segment.update();
	}

	//return first_segment.get_item_group(0).count();
	//return first_segment.get_item_group(1).get_direction_position(first_segment.get_end_distance_direction(), first_segment.goal_distance_in_group(1));
	//return first_segment.get_item_group(1).get_last_item_direction_position(first_segment.segment_direction, first_segment.get_end_distance_direction(), first_segment.get_item_data_group(1));
	return first_segment.get_new_item_position(index);
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto test_real_game_scenario_smelters_val = test_real_game_scenario_smelters(0);
static_assert(test_real_game_scenario_smelters(0) == 1024ll, "inserter hasn't been moved");
static_assert(test_real_game_scenario_smelters(1) == 1024 + 4096, "inserter hasn't been moved");
static_assert(test_real_game_scenario_smelters(2) == 1024 + 4096 + 4096, "inserter hasn't been moved");
static_assert(test_real_game_scenario_smelters(3) == 1024 + 4096 + 4096 + 12288, "inserter hasn't been moved");
#endif