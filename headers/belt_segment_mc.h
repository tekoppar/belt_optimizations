#pragma once

#include <vector>
#include <forward_list>

#include "const_data.h"
#include "vectors.h"
//#include "single_list.h"
#include "single_list_block.h"
#include "item.h"
#include "item_32.h"
#include "item_256.h"
#include "inserter.h"

//#define CONSTEXPR_ASSERTS
#ifdef CONSTEXPR_ASSERTS
#define CONSTEXPR_VAR constexpr
#else
#define CONSTEXPR_VAR static
#endif

class belt_segment
{
public:
	//protected:
	vec2_uint start_of_segment{ 0, 0 };
	vec2_uint end_of_segment{ 0, 0 };
	belt_utility::belt_direction segment_direction{ belt_utility::belt_direction::left_right };
	mem::single_list_block<item_groups_type> item_groups_linked_list;
	std::vector<inserter> inserters_no_linked_group;
	std::vector<inserter> inserters;
	std::vector<inserter> sleeping_inserters;
	// belt_segments that this segment moves items onto
	std::vector<belt_segment*> segment_end_points;
	// belt_segments that will move items onto this segment
	std::vector<belt_segment*> connected_segments;

public:
	constexpr belt_segment() noexcept
	{};
	constexpr belt_segment(vec2_uint start, vec2_uint end) noexcept :
		start_of_segment{ start },
		end_of_segment{ end }
	{
		if (start_of_segment.x != end_of_segment.x)
		{
			if (start_of_segment.x < end_of_segment.x) segment_direction = belt_utility::belt_direction::left_right;
			else segment_direction = belt_utility::belt_direction::right_left;
		}
		else
		{
			if (start_of_segment.y < end_of_segment.y) segment_direction = belt_utility::belt_direction::top_bottom;
			else segment_direction = belt_utility::belt_direction::bottom_top;
		}
	};

	__forceinline constexpr mem::b_iterator<mem::single_list_block_node<item_groups_type>> get_item(long long i) noexcept
	{
		return item_groups_linked_list[i];
	};
	__forceinline constexpr const mem::b_iterator<mem::single_list_block_node<item_groups_type>> get_item(long long i) const noexcept
	{
		return item_groups_linked_list[i];
	};
	constexpr inserter& get_inserter(std::size_t i) noexcept
	{
		return inserters[i];
	};
	constexpr const inserter& get_inserter(std::size_t i) const noexcept
	{
		return inserters[i];
	};
	constexpr inserter& get_no_group_inserter(std::size_t i) noexcept
	{
		return inserters_no_linked_group[i];
	};
	constexpr const inserter& get_no_group_inserter(std::size_t i) const noexcept
	{
		return inserters_no_linked_group[i];
	};
	constexpr long long count_item_groups() const noexcept
	{
		return item_groups_linked_list.size();
	};
	constexpr long long count_items_in_group(long long i) noexcept
	{
		return item_groups_linked_list[i]->count();
	};
	constexpr long long count_all_items() const noexcept
	{
		long long total = 0;
		for (auto it = item_groups_linked_list.begin(); it != item_groups_linked_list.end(); ++it) total += it->count();
		return total;
	};
	constexpr long long goal_distance_in_group(long long i) noexcept
	{
		return item_groups_linked_list[i]->get_goal();
	};
	constexpr long long count_inserters() const noexcept
	{
		return inserters.size();
	};

	constexpr long long get_start_direction_value() const noexcept
	{
		switch (segment_direction)
		{
			case belt_utility::belt_direction::left_right: return start_of_segment.x;
			case belt_utility::belt_direction::right_left: return start_of_segment.x;
			case belt_utility::belt_direction::top_bottom: return start_of_segment.y;
			case belt_utility::belt_direction::bottom_top: return start_of_segment.y;
			case belt_utility::belt_direction::null: return 0;
		}
	};
	constexpr long long get_end_distance() const noexcept
	{
		switch (segment_direction)
		{
			case belt_utility::belt_direction::left_right: return end_of_segment.x - start_of_segment.x;
			case belt_utility::belt_direction::right_left: return start_of_segment.x - end_of_segment.x;
			case belt_utility::belt_direction::top_bottom: return end_of_segment.y - start_of_segment.y;
			case belt_utility::belt_direction::bottom_top: return start_of_segment.y - end_of_segment.y;
			case belt_utility::belt_direction::null: return 0;
		}
	};
	constexpr void move_inserter_to_sleeping(long long index) noexcept
	{
		inserters[index].reset_sleep_timer();
		sleeping_inserters.push_back(std::move(inserters[index]));
		inserters.erase(inserters.begin() + index);
	};
	constexpr void move_inserter_from_no_group_to_active(long long index) noexcept
	{
		inserters_no_linked_group[index].reset_sleep_timer();
		inserters.push_back(std::move(inserters_no_linked_group[index]));
		inserters_no_linked_group.erase(inserters_no_linked_group.begin() + index);
	};
	constexpr void move_inserter_from_active_to_no_group(long long index) noexcept
	{
		inserters[index].reset_sleep_timer();
		inserters_no_linked_group.push_back(std::move(inserters[index]));
		inserters.erase(inserters.begin() + index);
	};

	constexpr void update() noexcept
	{
		bool item_was_removed = false;
		for (auto it = item_groups_linked_list.begin(); it != item_groups_linked_list.end(); ++it)
		{
			if (it->is_goal_distance_zero()) //index 0 of the group should be moved
			{
				if (segment_end_points.size() > 0)
				{
					for (auto& end_point : segment_end_points)
					{
						if (end_point->start_of_segment != it->get_position()) continue;

						auto tmp = it->get(0);
						if (end_point->add_item(tmp))
						{
							it->remove_item(0);
							item_was_removed = true;
							break;
						}
					}
				}
				else
				{
					it->set_active_mode(belt_utility::belt_update_mode::first_stuck);
					//it->set_is_stuck(true);
				}
			}

			if (item_was_removed)
			{
				//it->set_is_stuck(false);
				//it->set_is_all_stuck(false);
				it->set_active_mode(belt_utility::belt_update_mode::free);
			}

			if (it->count() == 0) it = item_groups_linked_list.erase(it);
			else
			{
				switch (it->get_active_mode())
				{
					default:
					case belt_utility::belt_update_mode::free: it->update_belt(); break;
					case belt_utility::belt_update_mode::first_stuck: it->items_stuck_update(); break;
					case belt_utility::belt_update_mode::all_stuck: break;
				}
				/*if (it->get_is_stuck())
				{
					if (!it->get_is_all_stuck()) it->items_stuck_update();
				}
				else it->update_belt();*/
			}
		}

		check_inserters_no_linked_group();
		for (int i = 0, l = inserters.size(); i < l; ++i)
		{
			if (inserters[i].sleep_update())
			{
				move_inserter_to_sleeping(i);
				--i;
				--l;
			}
			else
			{
				inserters[i].update();
				if (inserters[i].has_linked_list_data() == false)
				{
					move_inserter_from_active_to_no_group(i);
					--i;
					--l;
				}
			}
		}
	};

	constexpr short wake_up_inserter(item_type type) noexcept
	{
		for (int i = 0, l = sleeping_inserters.size(); i < l; ++i)
		{
			if (sleeping_inserters[i].check_if_need_type(type))
			{
				sleeping_inserters[i].set_is_sleeping(false);
				inserters.push_back(sleeping_inserters[i]);
				sleeping_inserters.erase(sleeping_inserters.begin() + i);
				return inserters.size() - 1;
			}
		}

		return -1;
	};
	constexpr short do_inserters_reserve_check(item_type type) noexcept
	{
		for (int i = 0, l = inserters.size(); i < l; ++i)
		{
			if (inserters[i].check_if_need_type(type)) return i;
		}

		return -1;
	};
	constexpr void perform_inserter_checks(item_type type, mem::single_list_block_node<item_groups_type>* group) noexcept
	{
		check_inserters_single_linked_group(group);

		short reserve_check = do_inserters_reserve_check(type);
		if (reserve_check != -1)
		{
			get_inserter(reserve_check).set_linked_list_data(find_closest_linked_group_from_position(get_inserter(reserve_check).get_position()));
		}
		else
		{
			reserve_check = wake_up_inserter(type);
			if (reserve_check != -1)
			{
				get_inserter(reserve_check).set_linked_list_data(find_closest_linked_group_from_position(get_inserter(reserve_check).get_position()));
			}
		}
	};
	constexpr mem::single_list_block_node<item_groups_type>* check_if_inserter_can_add_linked_group(vec2_uint position) noexcept
	{
		for (auto it = item_groups_linked_list.begin(); it != item_groups_linked_list.end(); ++it)
		{
			if (it->get_direction_position() == get_direction_position(segment_direction, position)) return it;
		}

		return nullptr;
	};
	constexpr bool check_if_inserter_can_add_linked_group(vec2_uint position, mem::single_list_block_node<item_groups_type>* group) noexcept
	{
		return (group->object.get_direction_position() == get_direction_position(segment_direction, position));
	};
	constexpr void check_inserters_no_linked_group() noexcept
	{
		for (int i = 0, l = inserters_no_linked_group.size(); i < l; ++i)
		{
			auto found_index = check_if_inserter_can_add_linked_group(inserters_no_linked_group[i].get_position());
			if (found_index)
			{
				inserters_no_linked_group[i].set_linked_list_data(found_index);
				move_inserter_from_no_group_to_active(i);
				--i;
				--l;
			}
		}
	};
	constexpr void check_inserters_single_linked_group(mem::single_list_block_node<item_groups_type>* group) noexcept
	{
		for (int i = 0, l = inserters_no_linked_group.size(); i < l; ++i)
		{
			if (check_if_inserter_can_add_linked_group(inserters_no_linked_group[i].get_position(), group))
			{
				inserters_no_linked_group[i].set_linked_list_data(group);
				move_inserter_from_no_group_to_active(i);
				--i;
				--l;
			}
		}
	};

	constexpr bool add_item(const item_uint& new_item) noexcept
	{
		if (item_groups_linked_list.empty())
		{
			item_groups_linked_list.emplace_back(segment_direction, get_end_distance());
			auto last_group_it = item_groups_linked_list.last();
			short added_index = last_group_it->add_item(new_item, new_item.position);
			if (added_index != -1)
			{
				perform_inserter_checks(new_item.type, last_group_it);
				return true;
			}
			return false;
		}
		else
		{
			if (item_groups_linked_list.last()->count() < 32)
			{
				auto last_group_it = item_groups_linked_list.last();
				short added_index = last_group_it->add_item(new_item, new_item.position);
				if (added_index != -1)
				{
					perform_inserter_checks(new_item.type, last_group_it);
					return true;
				}
				return false;
			}
			if (item_groups_linked_list.last()->count() == 32)
			{
				item_groups_linked_list.emplace_back(segment_direction, get_end_distance());
				auto last_group_it = item_groups_linked_list.last();

				short added_index = last_group_it->add_item(new_item, new_item.position);
				if (added_index != -1)
				{
					perform_inserter_checks(new_item.type, last_group_it);
					return true;
				}
				return false;
			}

			for (auto it = item_groups_linked_list.begin(); it != item_groups_linked_list.end(); ++it)
			{
				if (!it->can_add_item()) continue;
				if (it->can_add_item(new_item.position))
				{
					short added_index = it->add_item(new_item, new_item.position);
					if (added_index != -1)
					{
						perform_inserter_checks(new_item.type, it);
						return true;
					}
					return false;
				}
			}

			if (item_groups_linked_list.last()->get_last_item_direction_position() != get_start_direction_value())
			{
				item_groups_linked_list.emplace_back(segment_direction, get_end_distance());
				auto last_group_it = item_groups_linked_list.last();

				short added_index = last_group_it->add_item(new_item, new_item.position);
				if (added_index != -1)
				{
					perform_inserter_checks(new_item.type, last_group_it);
					return true;
				}
				return false;
			}
		}

		return false;
	};

	constexpr bool add_inserter(inserter object) noexcept
	{
		auto found_index = check_if_inserter_can_add_linked_group(object.get_position());
		if (found_index)
		{
			object.set_linked_list_data(found_index);

			if (inserters.size() == 0)
			{
				inserters.push_back(object);
				return true;
			}

			bool can_fit = true;
			for (int i = 0, l = inserters.size(); i < l; ++i)
			{
				if (inserters[i].get_position() == object.get_position()) can_fit = false;
			}

			if (can_fit) inserters.push_back(object);

			return can_fit;
		}
		else
		{
			inserters_no_linked_group.push_back(object);
			return true;
		}
	};

	constexpr void add_end_segment_section(belt_segment* ptr) noexcept
	{
		segment_end_points.push_back(ptr);
	};
	constexpr mem::single_list_block_node<item_groups_type>* find_closest_linked_group_from_position(vec2_uint pos) noexcept
	{
		long long closest_distance = 99999999999999999ll;
		long long closest_index = -1;
		auto closest_it = item_groups_linked_list.begin();
		for (auto it = item_groups_linked_list.begin(); it != item_groups_linked_list.end(); ++it)
		{
			long long current_distance = it->get_position().x - pos.x;
			if (current_distance > 0 && current_distance < closest_distance)
			{
				closest_distance = current_distance;
				closest_it = it;
			}
		}
		if (closest_it) return closest_it;
		else return nullptr;
	};
};

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
	//used to check support that adding items inbetween is possible
	//test_belt.add_item(test_arr[4], vec2_uint{ 0ll, 0ll });
	//test_belt.add_item(test_arr[5], vec2_uint{ 32ll, 0ll });
	//return test_belt.get(4).type; 

	for (int i = 0, l = 128 + 32; i < l; ++i)
	{
		first_segment.update();
	}

	//return first_segment.item_groups_linked_list[0]->count();

	return first_segment.get_item(0)->get(0).position.x;
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
		first_segment.update();
		second_segment.update();
	}

	return second_segment.get_item(0)->get(return_index).position.x;
};
#ifdef CONSTEXPR_ASSERTS
static_assert(test_moving_items_between_belt_segments(0) == 319, "item did not jump to the second segment");
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

	if (return_index == 0) return first_segment.get_item(0)->count();
	else return second_segment.get_item(0)->count();
};
#ifdef CONSTEXPR_ASSERTS
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

	return first_segment.get_item(0)->get(return_index).position.x;
};
#ifdef CONSTEXPR_ASSERTS
static_assert(test_incrementing_if_some_are_stuck(3) == 159, "position is wrong, incrementing position while items are stuck doesn't work for 3");
static_assert(test_incrementing_if_some_are_stuck(2) == 191, "position is wrong, incrementing position while items are stuck doesn't work for 2");
static_assert(test_incrementing_if_some_are_stuck(1) == 223, "position is wrong, incrementing position while items are stuck doesn't work for 1");
static_assert(test_incrementing_if_some_are_stuck(0) == 255, "position is wrong, incrementing position while items are stuck doesn't work for 0");
#endif


CONSTEXPR_VAR auto test_adding_in_middle(std::size_t return_index) noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{400, 0} };
	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ 288ll - 278ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 288ll - 128ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::log, vec2_uint{ 288ll - 64ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 288ll, 0ll } });

	first_segment.add_item(item_uint{ item_type::brick, vec2_uint{ 42ll, 0ll } });
	return first_segment.get_item(0)->get(return_index).type;
};
#ifdef CONSTEXPR_ASSERTS
static_assert(test_adding_in_middle(2) == item_type::stone, "wrong type so adding in the middle got something wrong");
static_assert(test_adding_in_middle(3) == item_type::brick, "wrong type so adding in the middle got something wrong");
static_assert(test_adding_in_middle(4) == item_type::wood, "wrong type so adding in the middle got something wrong");
#endif


CONSTEXPR_VAR auto test_item_distance(std::size_t return_index) noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{400, 0} };
	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ 288ll - 278ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 288ll - 128ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::log, vec2_uint{ 288ll - 64ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 288ll, 0ll } });

	first_segment.get_item(0)->remove_item(return_index);
	return first_segment.get_item(0)->get_distance_to_item(return_index);
};
#ifdef CONSTEXPR_ASSERTS
static_assert(test_item_distance(1) == 128, "item distance is incorrect");
#endif


CONSTEXPR_VAR auto test_inserter_item() noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{400, 0} };

	first_segment.add_inserter(inserter{ vec2_uint{256 + 64, 32} });
	first_segment.get_no_group_inserter(0).set_item_type(item_type::stone);

	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ 288ll - 278ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 288ll - 128ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::log, vec2_uint{ 288ll - 64ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 288ll, 0ll } });

	for (int i = 0, l = 128 + 32; i < l; ++i)
	{
		first_segment.update();
	}

	return first_segment.get_inserter(0).get_item().type;
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto test_inserter_item_val = test_inserter_item();
static_assert(test_inserter_item() == item_type::stone, "inserter hasn't grabbed item");
#endif


CONSTEXPR_VAR auto test_inserter_moved_to_no_group() noexcept
{
	belt_segment first_segment{ vec2_uint{0, 0}, vec2_uint{1024, 0} };

	first_segment.add_inserter(inserter{ vec2_uint{256 + 64, 32} });
	first_segment.get_no_group_inserter(0).set_item_type(item_type::stone);

	first_segment.add_item(item_uint{ item_type::wood, vec2_uint{ 288ll - 278ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::stone, vec2_uint{ 288ll - 128ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::log, vec2_uint{ 288ll - 64ll, 0ll } });
	first_segment.add_item(item_uint{ item_type::iron, vec2_uint{ 288ll, 0ll } });

	for (int i = 0, l = 256 + 128; i < l; ++i)
	{
		first_segment.update();
	}

	return first_segment.get_no_group_inserter(0).has_linked_list_data();
};
#ifdef CONSTEXPR_ASSERTS
constexpr auto test_moved_group_val = test_inserter_moved_to_no_group();
static_assert(test_inserter_moved_to_no_group() == false, "inserter hasn't been moved");
#endif