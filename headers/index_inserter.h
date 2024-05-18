#pragma once

#include <type_traits>
#include <utility>

#include "const_data.h"

#include "vectors.h"
#include "index_iterator.h"
#include "item.h"
#include "belt_utility_data.h"
#include "shared_classes.h"
#include "belt_utility.h"

//PEEPO random rambling of fixing the destination slot with multiple inserters in a row
/*
* The reason destination slots were so performant is because they only end up calling methods on objects which actually have a need for it.
* If you have a line of items (or item groups) only the first in line to a destination has a direct need to have something done to it, all
* the others can be calculated later once the first in line is done on stored data. And this is what the destination slot did, since only 
* the first item group in line to a inserter is being updated, once an item passes said inserter we start moving the items to a new item group.
*
* TLDR, only calls what's directly necessary
*
* reasons why destination slots being unique per item group is good
* + item groups maps 1 to 1 with inserters
*	+ no need to look up if item group is correct, it always is
*	- if destination slots are shared between inserters we will need to check if an item group is the correct one every update call
*		- if it's not the correct one we will need to scan backwards/forwards until we find the correct one
*		- grouping inserters based on some kind of criteria to a destination slot cause the same issue, need to check every update if it's the correct one
*			- grouping also has the added negative effect of when, and how do we actually group
*				- if we group based on item groups we will incur expensive checks
*					- grouping on item groups means destination slot grouping isn't fixed and changes as the item groups changes
*						- very expensive checks that needs to be done per update, per destination group
* - if we keep unique slots per item group we will need to split item groups based on destination slots
*	- if inserters are in close proximity (more or less how it will always be) we will be splitting every X (size of inserter/belt) to create/move new groups of very few items
*		- these groups have a very short life span so will be expensive to allocate/destroy over and over
*			+ we can reuse existing groups
*			- we hard to do correct and keep track and incurs extra checks
*
*
* current solutions
* - develop two designs, keep the destination slot one for longer distance based belts and design a new one when destination slots would stop being unique per item group
*
* (destination slots is not really the correct term, leader would be more accurate (although not in the case of inserters, it's just the first inserter, but we
* still split them up for memory reasons, so that one group of inserters can live on the stack (if that's possible for the compiler)))
* But, could we do better. Could we repeat the destination slot design, by going deeper. The only reason inserters needs to be updated per update is to check if the can grab an item,
* if so grab it. That's all they do and all they need to do. So, with that in mind. What if, we never update inserters on update calls.
*
* If we invert the idea with the destination slot that inserters would have a item group slot in mind, and instead go with the same design but for inserters.
* We would get a list of inserters in line, with the closest one being the first one. And if we then when updating the item groups in the destination slots
* (these slots will be split based on the inserter destination slots) the destination slots now corresponds 1 to 1 with the inserters destinations slots,
* so each item group in a destination slot has a link to a list of inserters. So each update for the leader item group would be to decrement it's distance,
* and then check if said distance allows the inserter to grab an item and call the inserter grab item method.
*
* Once an item passes the first inserter, we split that item of into it's own group, but it keeps the inserter destination slot pointer
* (this pointer is gonna be an object that stores a pointer to the inserter destination vector, but also two indexes, one for the destination slot vector,
* and one for the vector inside that)  but increments it to the next one in line. If there is no inserter next in line,
* the item group should check if there's another item group a head of it and merge into it's destination slot.
* If not it checks if it can increment the destination inserter slot, if not there are no more inserters and we just set it to nullptr.
*
* Now you might be thinking, wouldn't that end up in a lot of checks. Not really, when the item group checks if the inserter can grab the first item,
* if it can grab it but doesn't need it we perform the item group split there, we now just have an else outcome to something that previously didn't.
* So there's no need for any extra checks besides the ones that are already done. The extra cost will be the item group splitting.
* But in a real scenario the only reason for a split would be if it can't keep up with grabbing items, or if it doesn't need it.
* This would naturally need to be done with any other solution too. There won't even be any exponential growth in update calls needed,
* as the only growth needed is limited to the number of inserters. And since we removed the need for inserters to have updates on their own, that might even out in the end.
*/

class belt_segment;

class index_inserter
{
public:
	static inline constexpr long long inserter_grid_size{ 32ll };
	static inline long long grabbed_items{ 0ll };

private:
	vec2_uint position{ 0, 0 };
	//index_iterator<item_groups_type, _vector> item_group{ 0ull, nullptr };
	//index_iterator<item_groups_data_type, _data_vector> item_group_data{ 0ull, nullptr };
	//index_iterator<goal_distance, _vector_goal_distance> item_group_distance{ 0ull, nullptr };
	//_vector_distance const* index_calculation_vector{ nullptr };
	item_type item_need_types[8]{ item_type::pink_square, item_type::pink_square, item_type::pink_square, item_type::pink_square,
	item_type::pink_square, item_type::pink_square, item_type::pink_square, item_type::pink_square };
	__declspec(align(32)) item_uint item;
#ifdef _DEBUG
public:
	long long loop_count = 0ll;
	long long missed_grabs = 0ll;
	long long local_grabbed_items = 0ll;
	long long no_item_found = 0ll;
	long long wrong_goal_pointer_frame_count = 0ll;
	long long wrong_goal_pointer_updated = 0ll;
#endif

public:
	constexpr index_inserter() noexcept
	{};
	constexpr index_inserter(vec2_uint pos) noexcept :
		position{ pos }
	{};
	constexpr ~index_inserter() noexcept
	{};

	constexpr index_inserter(const index_inserter& o) noexcept :
		position{ o.position },
		/*item_group{o.item_group},
		item_group_data{ o.item_group_data },
		item_group_distance{ o.item_group_distance },
		index_calculation_vector{ o.index_calculation_vector },*/
		item{ o.item }
	{
		if (std::is_constant_evaluated())
		{
			constexpr long long l = 8;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = o.item_need_types[i];
			}
		}
		else std::memcpy(&item_need_types[0], &o.item_need_types[0], 16);
	};
	constexpr index_inserter(index_inserter&& o) noexcept :
		position{ std::exchange(o.position, vec2_uint{}) },
		/*item_group{std::exchange(o.item_group, {0ull, nullptr})},
		item_group_data{ std::exchange(o.item_group_data, {0ull, nullptr}) },
		item_group_distance{ std::exchange(o.item_group_distance, {0ull, nullptr}) },
		index_calculation_vector{ std::exchange(o.index_calculation_vector, nullptr) },*/
		item{ std::exchange(o.item, item_uint{}) }
	{
		if (std::is_constant_evaluated())
		{
			constexpr long long l = 8;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = o.item_need_types[i];
			}
		}
		else std::memcpy(&item_need_types[0], &o.item_need_types[0], 16);
	};
	constexpr index_inserter& operator=(const index_inserter& o) noexcept
	{
		position = o.position;
		/*item_group = o.item_group;
		item_group_data = o.item_group_data;
		item_group_distance = o.item_group_distance;
		index_calculation_vector = o.index_calculation_vector;*/
		item = o.item;

		if (std::is_constant_evaluated())
		{
			constexpr long long l = 8;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = o.item_need_types[i];
			}
		}
		else std::memcpy(&item_need_types[0], &o.item_need_types[0], 16);

		return *this;
	};
	constexpr index_inserter& operator=(index_inserter&& o) noexcept
	{
		position = std::exchange(o.position, vec2_uint{});
		/*item_group = std::exchange(o.item_group, {0ull, nullptr});
		item_group_data = std::exchange(o.item_group_data, { 0ull, nullptr });
		item_group_distance = std::exchange(o.item_group_distance, { 0ull, nullptr });
		index_calculation_vector = std::exchange(o.index_calculation_vector, nullptr);*/
		item = std::exchange(o.item, item_uint{});

		if (std::is_constant_evaluated())
		{
			constexpr long long l = 8;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = std::move(o.item_need_types[i]);
			}
		}
		else std::memcpy(&item_need_types[0], &o.item_need_types[0], 16);

		return *this;
	};

	inline constexpr vec2_uint get_position() const noexcept
	{
		return position;
	};
	inline constexpr item_type get_item_type(short index) const noexcept
	{
		return item_need_types[index];
	};
	inline constexpr void set_item_type(item_type type) noexcept
	{
		item_need_types[0] = type;
	};
	constexpr void set_item_types(item_type* types) noexcept
	{
		if (std::is_constant_evaluated())
		{
			constexpr long long l = 8;
			for (long long i = 0; i < l; ++i)
			{
				item_need_types[i] = types[i];
			}
		}
		else std::memcpy(&item_need_types[0], &types[0], 16);
	};
	constexpr bool check_if_need_type(item_type type) const noexcept
	{
		constexpr long long l = 8;
		for (long long i = 0; i < l; ++i)
		{
			if (item_need_types[i] == type) return true;
		}

		return false;
	};
	/*inline constexpr bool has_linked_list_data() const noexcept
	{
		return item_group != nullptr;
	};
	inline constexpr index_iterator<item_groups_type, _vector>& get_linked_list_data() noexcept
	{
		return item_group;
	};
	inline constexpr void set_linked_list_distance(const index_iterator<goal_distance, _vector_goal_distance>& dist_iter) noexcept
	{
		item_group_distance = dist_iter;
	};
	inline constexpr index_iterator<goal_distance, _vector_goal_distance>& get_linked_list_distance() noexcept
	{
		return item_group_distance;
	};
	inline constexpr const index_iterator<goal_distance, _vector_goal_distance>& get_linked_list_distance() const noexcept
	{
		return item_group_distance;
	};
	inline constexpr bool has_linked_list_distance() const noexcept
	{
		return item_group_distance != nullptr;
	};
private:
	constexpr bool check_if_indexes_matches(belt_utility::belt_direction direction, long long segment_end_direction) noexcept
	{
		if (!item_group || !item_group.vector_empty()) return false;
		if (!item_group_data || !item_group_data.vector_empty()) return false;
		if (!item_group_distance || !item_group_distance.vector_empty()) return false;
		if (!index_calculation_vector || !index_calculation_vector->empty()) return false;

		auto index_ptr = (*item_group_distance).get_index_ptr() - &index_calculation_vector->operator[](0);

		if (std::is_constant_evaluated() == false)
		{
			if (index_calculation_vector && index_calculation_vector->size() > index_ptr && index_ptr > 0ll)
			{
				auto tmp = (item_group_distance - 1);
				auto tmp1 = (*(item_group - 1)).get_distance_to_last_item(*(item_group_data - 1));
				auto dist_to_last_item = (*item_group).get_distance_to_last_item(*item_group_data);
				auto inserter_dist = (segment_end_direction - belt_utility::get_direction_position(direction, position));
#ifdef _DEBUG
				if ((*item_group_distance).get_distance() < inserter_dist && (*item_group_distance).get_distance() + dist_to_last_item < inserter_dist && (*tmp).get_distance() > inserter_dist && (*tmp).get_distance() + tmp1 < inserter_dist)
				{
					++wrong_goal_pointer_frame_count;
					if (wrong_goal_pointer_frame_count > 1024ll)
						throw std::runtime_error("");
				}
#endif
				//current set pointer is past us, go to tmp
				if ((*item_group_distance).get_distance() < inserter_dist && (*item_group_distance).get_distance() + dist_to_last_item < inserter_dist && (*tmp).get_distance() > inserter_dist && (*tmp).get_distance() + tmp1 > inserter_dist)
				{
					(*item_group_distance).update_pointer_and_values();
#ifdef _DEBUG
					++wrong_goal_pointer_updated;
#endif
					index_ptr = (*item_group_distance).get_index_ptr() - &index_calculation_vector->operator[](0);
				}
			}
		}

		if (index_ptr == item_group.get_index()) return true;
		else return false;
	};*/
public:
	/*constexpr bool update_linked_list_group_data(belt_utility::belt_direction direction, long long segment_end_direction) noexcept
	{
		//if (item_group && !item_group.vector_empty()) return false;
		//if (item_group_data && !item_group_data.vector_empty()) return false;
		//if (item_group_distance && !item_group_distance.vector_empty()) return false;
		//if (index_calculation_vector && !index_calculation_vector->empty()) return false;
		if ((*item_group_distance).get_index_ptr() == nullptr) return false;

		long long index_ptr = (*item_group_distance).get_index_ptr() - &index_calculation_vector->operator[](0);

		if (index_ptr != item_group.get_index()) 
		{
			item_group.set_index(index_ptr);
			item_group_data.set_index(index_ptr);
			return true;
		}

		if (std::is_constant_evaluated() == false)
		{
			if (index_calculation_vector && index_calculation_vector->size() > index_ptr && index_ptr > 0ll)
			{
				auto tmp = (item_group_distance - 1);
				auto tmp1 = (*(item_group - 1)).get_distance_to_last_item(*(item_group_data - 1));
				auto dist_to_last_item = (*item_group).get_distance_to_last_item(*item_group_data);
				auto inserter_dist = (segment_end_direction - belt_utility::get_direction_position(direction, position));
#ifdef _DEBUG
				if ((*item_group_distance).get_distance() < inserter_dist && (*item_group_distance).get_distance() + dist_to_last_item < inserter_dist && (*tmp).get_distance() > inserter_dist && (*tmp).get_distance() + tmp1 < inserter_dist)
				{
					++wrong_goal_pointer_frame_count;
					if (wrong_goal_pointer_frame_count > 1024ll)
						throw std::runtime_error("");
				}
#endif
				//current set pointer is past us, go to tmp
				if ((*item_group_distance).get_distance() < inserter_dist && (*item_group_distance).get_distance() + dist_to_last_item < inserter_dist && (*tmp).get_distance() > inserter_dist && (*tmp).get_distance() + tmp1 > inserter_dist)
				{
					(*item_group_distance).update_pointer_and_values();
#ifdef _DEBUG
					++wrong_goal_pointer_updated;
#endif
					index_ptr = (*item_group_distance).get_index_ptr() - &index_calculation_vector->operator[](0);
				}
			}
		}
		if (index_ptr < 0ll) return false;
		if (index_ptr == item_group.get_index()) return true;

#ifdef _DEBUG
		wrong_goal_pointer_frame_count = 0ll;
#endif
		item_group.set_index(index_ptr);
		item_group_data.set_index(index_ptr);
		return true;
	};*/
public:
	/*constexpr void set_linked_list_data(const index_iterator<item_groups_type, _vector>& iter, const index_iterator<item_groups_data_type, _data_vector> data_iter, const index_iterator<goal_distance, _vector_goal_distance>& dist_iter, _vector_distance const* dist_ptr) noexcept
	{
		item_group = iter;
		item_group_data = data_iter;
		item_group_distance = dist_iter;
		index_calculation_vector = dist_ptr;
	};*/
	inline constexpr void grab_item(const item_uint& _item) noexcept
	{
		item = _item;
	};
	inline constexpr item_uint& get_item() noexcept
	{
		return item;
	};
	/*inline constexpr index_iterator<item_groups_type, _vector> get_item_group() const noexcept
	{
		return item_group;
	};*/

	/*constexpr void update_linked_data(belt_utility::belt_direction direction, long long segment_end_direction) noexcept
	{
		//long long index_ptr = (*item_group_distance).get_index_ptr() - &index_calculation_vector->operator[](0);
		if (std::is_constant_evaluated() == false)
		{
			if (item_group_distance.get_index() > 0ll)
			{
				auto tmp = (item_group_distance - 1);
				auto tmp1 = (*(item_group - 1)).get_distance_to_last_item(*(item_group_data - 1));
				auto dist_to_last_item = (*item_group).get_distance_to_last_item(*item_group_data);
				auto inserter_dist = (segment_end_direction - belt_utility::get_direction_position(direction, position));

				//current set pointer is past us, go to tmp
				if ((*item_group_distance).get_distance() < inserter_dist && (*item_group_distance).get_distance() + dist_to_last_item < inserter_dist && (*tmp).get_distance() > inserter_dist && (*tmp).get_distance() + tmp1 > inserter_dist)
				{
					(*item_group_distance).update_pointer_and_values();
				}
			}
		}
	};
	constexpr bool linked_data_validation(belt_utility::belt_direction direction, long long segment_end_direction) noexcept
	{
		if (check_if_indexes_matches(direction, segment_end_direction) == false) return update_linked_list_group_data(direction, segment_end_direction);
		return true;
	};
	inline constexpr short update(belt_utility::belt_direction direction, long long segment_end_direction) noexcept
	{
		if ((*item_group_distance).get_index_ptr() != nullptr)
		{
			auto inserter_position = belt_utility::get_direction_position(direction, position);
			//if (check_if_indexes_matches(segment_end_direction) == false) update_linked_list_group_data(segment_end_direction);

			//auto inserter_dist = (segment_end_direction - belt_utility::get_direction_position(direction, position));
			//auto dist_to_last_item = (*item_group).get_distance_to_last_item(*item_group_data);
			const auto found_index = (*item_group).get_first_item_of_type_before_position(direction, segment_end_direction, (*item_group_distance).get_distance(), *item_group_data, get_item_type(0), inserter_position);
			if (found_index != -1)
			{
				auto item_distance_position = (*item_group).get_item_direction_position(direction, segment_end_direction, (*item_group_distance).get_distance(), *item_group_data, found_index);
				//if (item_group_data.get_index() == 615) __debugbreak();
#ifdef _DEBUG
				//if (inserter_position - item_distance_position > 2048ll) throw std::runtime_error("");
#endif
				if (item_distance_position >= inserter_position && item_distance_position <= inserter_position + inserter_grid_size)
				{
					if (std::is_constant_evaluated() == false) ++grabbed_items;
#ifdef _DEBUG
					loop_count = 0ll;
					++local_grabbed_items;

					if (position.x == 4865000ll && local_grabbed_items == 32) __debugbreak();
#endif
					return found_index;
					//grab_item(segment_ptr->remove_item(item_group, found_index));
					//grab_item((*item_group).get(segment_end_direction, segment_y_direction, (*item_group_distance).get_distance(), *item_group_data, found_index));
					//(*item_group).remove_item(&(*item_group_distance), segment_ptr, *item_group_data, found_index);
				}
			}
#ifdef _DEBUG
			else
			{
				++no_item_found;
				if (std::is_constant_evaluated() == false)
				{
					if (no_item_found >= 1000ll) throw std::runtime_error("");
				}
			}
			++loop_count;
			if (loop_count > 32ll)
			{
				//if (item_group_data.get_index() == 615) __debugbreak();
				++missed_grabs;
				loop_count = 0ll;
			}
#endif
		}

		return -1;
	};*/
};