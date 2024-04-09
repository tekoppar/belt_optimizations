#pragma once

#include <immintrin.h>
#include <type_traits>

#include "item.h"
#include "vectors.h"
#include "belt_utility_data.h"
#include "belt_intrinsics.h"
#include "shared_classes.h"
#include "type_conversion.h"

class belt_segment;
//#include "single_list_block.h"

#define optimization_comp

/*
* The concept for this class is as items move on a belt there's only two things we actually care about
* The current position of the first item and the goal destination. With that in mind there's only two
* variables we have to iterate for in this class's 32 items. So for each update called were moving 32
* items instead of 4 (belt.h) or 32 (belt_32.h).
*
* There's also another advantage of this setup, if you use any of the belt classes to move items, at
* some point items will hit a point where they will need to move from one belt to another. This comes
* in fixed intervals of every x instance of said belt, so the performance hit of moving an item to
* another belt and then shifting the arrays to the left is gonna add up quite quickly.
*
* But since we don't have "belts" in this instance, we just have a line of items. That problem will
* only occur once an item reaches it's destination. So we can move our items until the goal distance
* is equal to zero, then we would either flag is_stuck or in my case I decided to just remove the item.
* If you wanted to the items to stop you would just set is_stuck to true.
*
* So the setup for all this requires that item_32's are created when items enter new segments of "belts".
* Each segment stores all the item_32 that currently in said segment. A belt moving copper that splits into
* two around the middle would have 3 segments, from the start up to the split, then at the start of the split
* to the end for each split. So it would look something like this where s = start of a segment,
* e = end of a segment, c = copper plate, _ = is belt part and | = the splitter.
* s___c___c___c___c_e|s__c___c___c___e
*  					 |s__c___c___c___e
*
* So every time an item moves to a new segment it either needs to be added to an already existing item_32
* or create a new item_32 to add itself too. An item can only be added to a item_32 if the distance between
* the last items position and the newly added is less than or equal too 255, item_32 only does less than.
* There's no support for adding items in the middle, but that can be achieved by comparing the x position
* of the new item with the distance to x item until it's less than and then check if the next item in the
* series has a distance greater than 63 if it does the item can fit and you just shift the arrays from that index.
*
* Direction is currently not used, so at the moment the class only supports movement from left to right.
* There's not much that need to be changed to support that, I would recommend switching the x and y values
* based on the direction, since then there's no need to have code for all 4 directions and only need left to
* right or right to left.
*
* One big issue to solve is how to deal with items having multiple destinations that overlaps back and fourth.
* One solution would be to add an array of 32 bools used to indicate if an item is reserved, this would be
* used so that when an inserter wants an item. It checks from it's position backwards to the start of the belt
* segment if an item it wants to pickup exist, it would do this by looping over all the stored item_32 in said
* segment and checking if an item that matches the item_type it wants is there. If it finds a matching item the
* inserter stores the distance from it to the item, then it decrements the distance the same item_32 does ever
* update and once it reaches zero the item is where the inserter picks it up.
*
* This solution has some major advantages, items don't need to be split up into different item_32 based on what
* inserters wants to pick up, there's no need to keep track of any data besides a bool being toggled. Nothing
* in the code in item_32 needs to change except to add in support for the reserved bool code to indicate that an
* item is reserved and shift said array when an item is removed. Besides that the item_32 can keep moving until
* it either reaches zero items or the final end segment.
*
* The way is_stuck would be used would be outside the item_32 instance and be in whatever loop is used to call
* the update method. Since each start of a segments holds all the instances it would probably be good to put the
* loop code there, then you can check the first segment in the vector if it's stuck. If it is stuck all item_32
* in the segment is also stuck, if it's not stuck then all instances can move. So there's only a need to check
* if one instance of item_32 can move or not to move all instances in said segment. So in the segments code loop
* you would just check the first segment, if it can move you do the loop, if it can't move you return. Then each
* segment would have a link to connected segments and signal each other if something can move or not.
*
* For inserters inserting items onto a belt you can use the reserved array to mark that an item is gonna be inserted
* and use the same logic as taking items but instead adding onto it. Would need to add in checks for if a spot is
* reserved in the add_item code, but that's just one more check.
*
*
* TODO
*
* It might be a good idea to test if adding 8bit and 16bit mode variable to the class would be a benefit since
* than you could have a greater distance between items if there's not enough items moving to avoid creating too
* many instances of item_32 to hold few items. You would only need to switch on the add and shift parts to use
* 16bit intrinsics instead of 8bit. This way you could still store the instances as objects instead of ptrs and
* avoid using virtual functions to have a item_32_8bit and item_32_16bit version in the same vector.
*/

class item_32_data
{
public:
	__declspec(align(64)) bool contains_item[32]{
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false
	};
	__declspec(align(64)) short item_distance[32]{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	__declspec(align(64)) belt_item items[32];

	constexpr item_32_data() noexcept
	{};
	constexpr ~item_32_data() noexcept
	{};

	constexpr item_32_data(const item_32_data& o) noexcept
	{
		if (std::is_constant_evaluated() == false)
		{
			std::memcpy(&contains_item[0], &o.contains_item[0], 32);
			std::memcpy(&item_distance[0], &o.item_distance[0], 64);
			std::memcpy(&items[0], &o.items[0], 64);
		}
		else
		{
			for (long long i = 0; i < 32; ++i)
			{
				contains_item[i] = o.contains_item[i];
				item_distance[i] = o.item_distance[i];
				items[i] = o.items[i];
			}
		}
	};
	constexpr item_32_data(item_32_data&& o) noexcept
	{
		if (std::is_constant_evaluated() == false)
		{
			std::memcpy(&contains_item[0], &o.contains_item[0], 32);
			std::memcpy(&item_distance[0], &o.item_distance[0], 64);
			std::memcpy(&items[0], &o.items[0], 64);
		}
		else
		{
			for (long long i = 0; i < 32; ++i)
			{
				contains_item[i] = o.contains_item[i];
				item_distance[i] = o.item_distance[i];
				items[i] = o.items[i];
			}
		}
	};
	constexpr item_32_data& operator=(const item_32_data& o) noexcept
	{
		if (std::is_constant_evaluated() == false)
		{
			std::memcpy(&contains_item[0], &o.contains_item[0], 32);
			std::memcpy(&item_distance[0], &o.item_distance[0], 64);
			std::memcpy(&items[0], &o.items[0], 64);
		}
		else
		{
			for (long long i = 0; i < 32; ++i)
			{
				contains_item[i] = o.contains_item[i];
				item_distance[i] = o.item_distance[i];
				items[i] = o.items[i];
			}
		}

		return *this;
	};
	constexpr item_32_data& operator=(item_32_data&& o) noexcept
	{
		if (std::is_constant_evaluated() == false)
		{
			std::memcpy(&contains_item[0], &o.contains_item[0], 32);
			std::memcpy(&item_distance[0], &o.item_distance[0], 64);
			std::memcpy(&items[0], &o.items[0], 64);
		}
		else
		{
			for (long long i = 0; i < 32; ++i)
			{
				contains_item[i] = o.contains_item[i];
				item_distance[i] = o.item_distance[i];
				items[i] = o.items[i];
			}
		}

		return *this;
	};
};

namespace item_data_utility
{
	struct item_32_data_split
	{
		item_32_data data;
		int missing_distance;
	};

	constexpr item_32_data_split split_from_index(item_32_data* _this, short index) noexcept
	{
		item_32_data split_left = *_this;
		const long long l = static_cast<long long>(index) + 1;
		const auto new_count = 32 - (index + 1);

		const auto remove_distance = split_left.item_distance[l];
		const auto missing_distance = remove_distance - split_left.item_distance[index];
		for (long long i = 0; i < new_count; ++i)
		{
			split_left.contains_item[i] = split_left.contains_item[l + i];
			split_left.item_distance[i] = split_left.item_distance[l + i] - remove_distance;
			split_left.items[i] = split_left.items[l + i];
		}

		for (long long i = new_count; i < 32; ++i)
		{
			split_left.contains_item[i] = false;
			split_left.item_distance[i] = 0;
			split_left.items[i] = belt_item{};
		}
		for (long long i = l; i < 32; ++i)
		{
			_this->contains_item[i] = false;
			_this->item_distance[i] = 0;
			_this->items[i] = belt_item{};
		}

		return { split_left, missing_distance };
	};
	constexpr item_32_data_split split_from_index(item_32_data& _this, short index) noexcept
	{
		return split_from_index(&_this, index);
	};

	constexpr static inline void shift_arrays_left(int item_count, item_32_data& item_data) noexcept
	{
		if (std::is_constant_evaluated())
		{
			for (long long i = item_count - 1ll; i >= 0ll; --i)
			{
				item_data.contains_item[i + 1] = item_data.contains_item[i];
				item_data.item_distance[i + 1] = item_data.item_distance[i];
				item_data.items[i + 1] = item_data.items[i];
			}
		}
		else
		{
			belt_utility::_mm256_slli_si256__p<1>((__m256i*) & item_data.contains_item[0]);
			belt_utility::_mm512_slli2x256_si512__<2>((__m256i*) & item_data.item_distance[0]);
			belt_utility::_mm512_slli2x256_si512__<2>((__m256i*) & item_data.items[0]);
#ifdef _SIMPLE_MEMORY_LEAK_DETECTION
			detect_memory_leak(this);
#endif
		}
	};
	constexpr static inline void forced_shift_arrays_left(int item_count, item_32_data& item_data) noexcept
	{
		for (long long i = item_count - 1ll; i >= 0ll; --i)
		{
			item_data.contains_item[i + 1] = item_data.contains_item[i];
			item_data.item_distance[i + 1] = item_data.item_distance[i];
			item_data.items[i + 1] = item_data.items[i];
		}
	};
	constexpr static inline void shift_arrays_left_from_index(int item_count, item_32_data& item_data, long long index) noexcept
	{
		for (long long i = item_count - 1ll; i >= index; --i)
		{
			item_data.contains_item[i + 1] = item_data.contains_item[i];
			item_data.item_distance[i + 1] = item_data.item_distance[i];
			item_data.items[i + 1] = item_data.items[i];
		}
	};
	constexpr static inline void forced_shift_arrays_right(int item_count, item_32_data& item_data) noexcept
	{
		bool previous_b_x = item_data.contains_item[item_count - 1], current_b_x = false;
		short previous_c_x = item_data.item_distance[item_count - 1], current_c_x = 0;
		belt_item previous_i_x = item_data.items[item_count - 1], current_i_x = {};
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
		for (long long i = expr::max(tc::widen<long long>(item_count), 31ll, 32ll); i >= 0; --i)
#else
		for (long long i = item_count; i >= 0; --i)
#endif
		{
			current_b_x = item_data.contains_item[i];
			item_data.contains_item[i] = previous_b_x;
			previous_b_x = current_b_x;

			current_c_x = item_data.item_distance[i];
			item_data.item_distance[i] = previous_c_x;
			previous_c_x = current_c_x;

			current_i_x = item_data.items[i];
			item_data.items[i] = previous_i_x;
			previous_i_x = current_i_x;
		}
		if (item_count > 0)
		{
			item_data.contains_item[item_count - 1] = false;
			item_data.item_distance[item_count - 1] = 0;
		}
	};
	constexpr static inline void shift_arrays_right(int item_count, item_32_data& item_data) noexcept
	{
		if (std::is_constant_evaluated())
		{
			bool previous_b_x = item_data.contains_item[item_count - 1], current_b_x = false;
			short previous_c_x = item_data.item_distance[item_count - 1], current_c_x = 0;
			belt_item previous_i_x = item_data.items[item_count - 1], current_i_x = {};
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
			for (long long i = expr::max(tc::widen<long long>(item_count), 31ll, 32ll); i >= 0; --i)
#else
			for (long long i = item_count; i >= 0; --i)
#endif
			{
				current_b_x = item_data.contains_item[i];
				item_data.contains_item[i] = previous_b_x;
				previous_b_x = current_b_x;

				current_c_x = item_data.item_distance[i];
				item_data.item_distance[i] = previous_c_x;
				previous_c_x = current_c_x;

				current_i_x = item_data.items[i];
				item_data.items[i] = previous_i_x;
				previous_i_x = current_i_x;
			}
			if (item_count > 0)
			{
				item_data.contains_item[item_count - 1] = false;
				item_data.item_distance[item_count - 1] = 0;
			}
		}
		else
		{
			belt_utility::_mm256_srli_si256__p<1>((__m256i*) & item_data.contains_item[0]);
			belt_utility::_mm512_srli2x256_si512__<2>((__m256i*) & item_data.item_distance[0]);
			belt_utility::_mm512_srli2x256_si512__<2>((__m256i*) & item_data.items[0]);
			item_data.contains_item[item_count - 1] = false;
			item_data.item_distance[item_count - 1] = 0;
#ifdef _SIMPLE_MEMORY_LEAK_DETECTION
			detect_memory_leak(this);
#endif
		}
	};
	constexpr static inline void shift_arrays_right_from_index(int item_count, item_32_data& item_data, long long index) noexcept
	{
		bool previous_b_x = false, current_b_x = item_data.contains_item[item_count - 1];
		short previous_c_x = 0, current_c_x = item_data.item_distance[item_count - 1];
		belt_item previous_i_x = {}, current_i_x = item_data.items[item_count - 1];
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
		for (long long i = expr::max(tc::widen<long long>(item_count), 31ll, 32ll); i >= index; --i)
#else
		for (long long i = static_cast<long long>(item_count) - 1ll; i >= index; --i)
#endif
		{
			current_b_x = item_data.contains_item[i];
			item_data.contains_item[i] = previous_b_x;
			previous_b_x = current_b_x;

			current_c_x = item_data.item_distance[i];
			item_data.item_distance[i] = previous_c_x;
			previous_c_x = current_c_x;

			current_i_x = item_data.items[i];
			item_data.items[i] = previous_i_x;
			previous_i_x = current_i_x;
		}
	};
};

class item_32
{
	constexpr static short single_belt_length = 128;
	constexpr static short belt_length = 128 * 8;
public:
	inline static long long items_moved_per_frame = 0;
	constexpr static inline long long max_item_count = 32;
	constexpr static long long belt_item_size = 32;
	constexpr static int max_distance_between_items = std::numeric_limits<short>::max() - belt_item_size;

	enum class item_removal_result
	{
		item_not_removed,
		item_removed,
		item_removed_zero_remains
	};

private:
	short item_count{ 0 }; //32-39

	constexpr static belt_utility::belt_color color{ belt_utility::belt_color::gray };
public:
	constexpr item_32() noexcept
	{};
	constexpr item_32(short _item_count) noexcept :
		item_count{ _item_count }
	{};
	constexpr ~item_32() noexcept
	{};

	constexpr item_32(const item_32& o) noexcept :
		item_count{ o.item_count }
	{};
	constexpr item_32(item_32&& o) noexcept :
		item_count{ o.item_count }
	{};
	constexpr item_32& operator=(const item_32& o) noexcept
	{
		item_count = o.item_count;

		return *this;
	};
	constexpr item_32& operator=(item_32&& o) noexcept
	{
		item_count = o.item_count;

		return *this;
	};

	constexpr item_32 split_from_index(short index) noexcept
	{
		const short old = item_count - (index + 1);
		item_count = index + 1;
		return old;
	};

	constexpr long long get_direction_position(long long segment_end_direction, long long item_goal_distance) const noexcept
	{
		return (segment_end_direction - item_goal_distance);
	};

	inline constexpr const item_uint get(long long segment_end_direction, long long segment_y_direction, long long item_goal_distance, item_32_data& item_data, long long i) const noexcept
	{
#ifdef _BOUNDS_CHECKING_
		if (i < item_count)
#endif
			return { item_data.items[i].type, vec2_uint{get_direction_position(segment_end_direction, item_goal_distance) - get_distance_to_item(item_data, i), segment_y_direction} };
#ifdef _BOUNDS_CHECKING_
		else
			return {};
#endif
	};
	inline constexpr item_uint get_first_item(long long segment_end_direction, long long segment_y_direction, long long item_goal_distance, item_32_data& item_data) const noexcept
	{
#ifdef _BOUNDS_CHECKING_
		if (i < item_count)
#endif
			return { item_data.items[0ll].type, vec2_uint{get_direction_position(segment_end_direction, item_goal_distance), segment_y_direction} };
#ifdef _BOUNDS_CHECKING_
		else
			return {};
#endif
	};

	inline constexpr vec2_uint get_position(long long segment_end_direction, long long segment_y_direction, long long item_goal_distance) const noexcept
	{
		return { get_direction_position(segment_end_direction, item_goal_distance), segment_y_direction };
	};
	inline constexpr long long get_item_direction_position(belt_utility::belt_direction direction, const long long segment_end_direction, long long item_goal_distance, item_32_data& item_data, short index) const noexcept
	{
		//index = fix_index(item_data, index);
		switch (direction)
		{
			default:
			case belt_utility::belt_direction::null:
			case belt_utility::belt_direction::left_right: return get_direction_position(segment_end_direction, item_goal_distance) - get_distance_to_item(item_data, index);
			case belt_utility::belt_direction::right_left: return get_direction_position(segment_end_direction, item_goal_distance) + get_distance_to_item(item_data, index);
			case belt_utility::belt_direction::top_bottom: return get_direction_position(segment_end_direction, item_goal_distance) - get_distance_to_item(item_data, index);
			case belt_utility::belt_direction::bottom_top: return get_direction_position(segment_end_direction, item_goal_distance) + get_distance_to_item(item_data, index);
		}
	};
	inline constexpr long long get_last_item_direction_position(belt_utility::belt_direction direction, const long long segment_end_direction, long long item_goal_distance, item_32_data& item_data) const noexcept
	{
		switch (direction)
		{
			default:
			case belt_utility::belt_direction::null:
			case belt_utility::belt_direction::left_right: return get_direction_position(segment_end_direction, item_goal_distance) - get_distance_to_last_item(item_data);
			case belt_utility::belt_direction::right_left: return get_direction_position(segment_end_direction, item_goal_distance) + get_distance_to_last_item(item_data);
			case belt_utility::belt_direction::top_bottom: return get_direction_position(segment_end_direction, item_goal_distance) - get_distance_to_last_item(item_data);
			case belt_utility::belt_direction::bottom_top: return get_direction_position(segment_end_direction, item_goal_distance) + get_distance_to_last_item(item_data);
		}
	};
	inline constexpr long long get_goal(long long* item_goal_distance) const noexcept
	{
		return *item_goal_distance;
	};
	inline constexpr long long count() const noexcept
	{
		return item_count;
	};
	inline constexpr bool can_add_item() const noexcept
	{
		return item_count < belt_item_size;
	};

	constexpr long long get_distance_to_last_item(item_32_data& item_data) const noexcept
	{
		return item_data.item_distance[item_count - 1];
	};
	constexpr long long get_item_distance(item_32_data item_data, long long item_index) const noexcept
	{
		if (item_index >= item_count) return 0;
		return item_data.item_distance[item_index];
	};
	constexpr long long get_distance_to_item(item_32_data& item_data, long long item_index) const noexcept
	{
		if (item_index >= item_count) return 0;
		return tc::widen<long long>(item_data.item_distance[item_index]);
	};

	constexpr bool can_add_item(const long long segment_end_direction, long long* item_goal_distance, item_32_data& item_data, vec2_uint new_item_position) const noexcept
	{
		if (*item_goal_distance >= belt_item_size) return true;
		if (new_item_position.x >= get_direction_position(segment_end_direction, *item_goal_distance) + belt_item_size) return true;

		const long long distance_to_last_item = (new_item_position.x - get_distance_to_last_item(item_data));
		if (get_direction_position(segment_end_direction, *item_goal_distance) - get_distance_to_last_item(item_data) > new_item_position.x && distance_to_last_item <= 255u) return true;

		for (long long i = 0; i < item_count; ++i)
		{
			if (new_item_position.x > get_direction_position(segment_end_direction, *item_goal_distance) - get_distance_to_item(item_data, i + 1) && item_data.item_distance[i + 1] >= belt_item_size) return true;
		}
		return false;
	};

	// in the case that we add to an item_group that's not the goal group we need the calculated distance to get the real direction_position
	// else the new_item_position will be relative to the world while the direction_position will be relative to itself
	constexpr short add_item(const long long segment_end_direction, long long item_distance_direction, long long* item_goal_distance, item_32_data& item_data, const belt_item& new_item, vec2_uint new_item_position) noexcept
	{
		if (item_count == 0ll)
		{
			//*item_goal_distance = *item_goal_distance - new_item_position.x;
			++item_count;
			item_data.contains_item[item_count - 1] = true;
			item_data.item_distance[item_count - 1] = static_cast<short>(0);
			item_data.items[item_count - 1] = new_item;
			return 0;
		}
		//if we should add before the first item
		//checks if the new item position is greater then the current item position + 32
		//and that there's still space to fit the item before the goal
		auto direction_position = get_direction_position(segment_end_direction, item_distance_direction);
		if (new_item_position.x >= direction_position + belt_item_size && *item_goal_distance >= belt_item_size)
		{
			const long long new_distance = new_item_position.x - direction_position;
			*item_goal_distance = *item_goal_distance - new_distance;
			for (int i = 0; i < item_count; ++i) item_data.item_distance[i] += static_cast<short>(new_distance);
			item_data_utility::shift_arrays_left(item_count, item_data);
			++item_count;
			item_data.contains_item[0] = true;
			item_data.item_distance[0] = static_cast<short>(0);
			item_data.item_distance[1] = static_cast<short>(new_distance);
			item_data.items[0] = new_item;
			//last_distance_to_item += item_data->item_distance[1];
			return 0;
		}

		//if we should add after the last item
		//it does this by comparing that the last items position is greater then the new item
		//and then checks if the distance between the last item and the new item is less than 256
		const long long distance_to_last_item = (new_item_position.x - get_distance_to_last_item(item_data));
		if (direction_position - get_distance_to_last_item(item_data) > new_item_position.x && distance_to_last_item <= 255u)
		{
			++item_count;
			item_data.contains_item[item_count - 1] = true;
			item_data.item_distance[item_count - 1] = static_cast<short>(new_item_position.x - direction_position);
			item_data.items[item_count - 1] = new_item;
			//last_distance_to_item += item_data->item_distance[item_count - 1];
			return item_count - 1ll;
		}

		//if we should add in between items
		for (long long i = 0; i < item_count; ++i)
		{
			if (new_item_position.x > direction_position - get_distance_to_item(item_data, i + 1) && item_data.item_distance[i + 1] >= 64)
			{
				item_data_utility::shift_arrays_right_from_index(item_count, item_data, i + 1);
				++item_count;
				item_data.contains_item[i + 1] = true;
				item_data.item_distance[i + 1] = static_cast<short>(new_item_position.x - direction_position);
				item_data.items[i + 1] = new_item;
				//last_distance_to_item += item_data->item_distance[i + 1];
				return tc::narrow<short>(i) + 1;
			}
		}

		return -1;
	};
	__forceinline constexpr short add_item(const long long segment_end_direction, long long* item_goal_distance, item_32_data& item_data, const belt_item& new_item, vec2_uint new_item_position) noexcept
	{
		return add_item(segment_end_direction, *item_goal_distance, item_goal_distance, item_data, new_item, new_item_position);
	};
	constexpr short old_add_item(const long long segment_end_direction, long long* item_goal_distance, item_32_data& item_data, const belt_item& new_item, vec2_uint new_item_position) noexcept
	{
		if (item_count == 0ll)
		{
			//*item_goal_distance = *item_goal_distance - new_item_position.x;
			++item_count;
			item_data.contains_item[item_count - 1] = true;
			item_data.item_distance[item_count - 1] = static_cast<short>(0);
			item_data.items[item_count - 1] = new_item;
			return 0;
		}
		//if we should add before the first item
		//checks if the new item position is greater then the current item position + 32
		//and that there's still space to fit the item before the goal
		auto direction_position = get_direction_position(segment_end_direction, *item_goal_distance);
		if (new_item_position.x >= direction_position + belt_item_size && *item_goal_distance >= belt_item_size)
		{
			const long long new_distance = new_item_position.x - direction_position;
			*item_goal_distance = *item_goal_distance - new_distance;
			for (int i = 0; i < item_count; ++i) item_data.item_distance[i] += static_cast<short>(new_distance);
			item_data_utility::shift_arrays_left(item_count, item_data);
			++item_count;
			item_data.contains_item[0] = true;
			item_data.item_distance[0] = static_cast<short>(0);
			item_data.item_distance[1] = static_cast<short>(new_distance);
			item_data.items[0] = new_item;
			//last_distance_to_item += item_data->item_distance[1];
			return 0;
		}

		//if we should add after the last item
		//it does this by comparing that the last items position is greater then the new item
		//and then checks if the distance between the last item and the new item is less than 256
		const long long distance_to_last_item = (new_item_position.x - get_distance_to_last_item(item_data));
		if (direction_position - get_distance_to_last_item(item_data) > new_item_position.x && distance_to_last_item <= 255u)
		{
			++item_count;
			item_data.contains_item[item_count - 1] = true;
			item_data.item_distance[item_count - 1] = static_cast<short>(new_item_position.x - direction_position);
			item_data.items[item_count - 1] = new_item;
			//last_distance_to_item += item_data->item_distance[item_count - 1];
			return item_count - 1;
		}

		//if we should add in between items
		for (long long i = 0; i < item_count; ++i)
		{
			if (new_item_position.x > direction_position - get_distance_to_item(item_data, i + 1) && item_data.item_distance[i + 1] >= 64)
			{
				item_data_utility::shift_arrays_right_from_index(item_count, item_data, i + 1);
				++item_count;
				item_data.contains_item[i + 1] = true;
				item_data.item_distance[i + 1] = static_cast<short>(new_item_position.x - direction_position);
				item_data.items[i + 1] = new_item;
				//last_distance_to_item += item_data->item_distance[i + 1];
				return tc::narrow<short>(i) + 1;
			}
		}

		return -1;
	};

	constexpr item_removal_result remove_item(long long* const item_goal_distance, item_32_data& item_data, short index) noexcept
	{
#ifdef _DEBUG
		if (index >= 32) return item_removal_result::item_not_removed;
#endif
		long long new_goal_distance = 0;
		if (item_count > 1ll)
		{
			if (index == 0)
			{
				new_goal_distance = item_count >= 2ll ? get_distance_to_item(item_data, 1ll) : 0ll;
				item_data.contains_item[index] = false;
				item_data.item_distance[index] = 0;
				item_data_utility::shift_arrays_right(item_count, item_data);
				//for (int i = 0; i < item_count; ++i) item_data.item_distance[i] += new_goal_distance;
				//*item_goal_distance += new_goal_distance;
				//last_distance_to_item -= new_goal_distance;
				--item_count;
			}
			else
			{
				//const unsigned char removed_distance = item_data.item_distance[index];
				item_data.contains_item[index] = false;
				item_data.item_distance[index] = 0;
				item_data.items[index] = belt_item{};
				item_data_utility::shift_arrays_right_from_index(item_count, item_data, index);
				//for (int i = index; i < item_count; ++i) item_data.item_distance[i] += removed_distance;
				//item_data.item_distance[index] += removed_distance;
				//last_distance_to_item -= removed_distance;
				--item_count;
			}

			return item_removal_result::item_removed;
		}
		else
		{
			//last_distance_to_item = 0;
			*item_goal_distance += item_data.item_distance[index];
			--item_count;
			item_data.contains_item[index] = false;
			item_data.item_distance[index] = 0;
			item_data.items[index] = belt_item{};
			return item_removal_result::item_removed_zero_remains;
		}

		return item_removal_result::item_not_removed;
	};
	constexpr item_removal_result remove_first_item(long long* const item_goal_distance, item_32_data& item_data) noexcept
	{
#ifdef _DEBUG
		if (item_count == 0) return item_removal_result::item_not_removed;
#endif
		if (item_count > 1ll)
		{
			const long long new_goal_distance = item_count >= 2ll ? item_data.item_distance[1ll] : 0ll;
			item_data.contains_item[0ll] = false;
			item_data.item_distance[0ll] = 0;
			item_data_utility::shift_arrays_right(item_count, item_data);
			*item_goal_distance += new_goal_distance;
			//last_distance_to_item -= new_goal_distance;
			--item_count;
		}
		else
		{
			//last_distance_to_item = 0;
			//*item_goal_distance->update_goal_distance(0ll);
			--item_count;
			*item_goal_distance -= item_data.item_distance[0ll];
			item_data.contains_item[0ll] = false;
			item_data.item_distance[0ll] = 0;
			item_data.items[0ll] = belt_item{};
			return item_removal_result::item_removed_zero_remains;
			//belt_segment_helpers::item_group_has_zero_count(segment_ptr, this, &item_data);
		}

		return item_removal_result::item_removed;
	};
	constexpr void remove_last_item(item_32_data& item_data) noexcept
	{
		--item_count;
		item_data.contains_item[0ll] = false;
		item_data.item_distance[0ll] = 0;
		item_data.items[0ll] = belt_item{};
	};
	constexpr short get_first_item_of_type(item_type type, item_32_data& item_data) const noexcept
	{
		for (long long i = 0ll; i < item_count; ++i)
		{
			if (item_data.contains_item[i] && item_data.items[i].type == type) return tc::narrow<short>(i);
		}

		return -1;
	};
	constexpr short get_first_item_before_position(belt_utility::belt_direction direction, const long long segment_end_direction, long long item_goal_distance, item_32_data& item_data, long long direction_position) const noexcept
	{
		for (long long i = 0ll; i < item_count; ++i)
		{
			auto item_position = get_item_direction_position(direction, segment_end_direction, item_goal_distance, item_data, tc::narrow<short>(i));
			if (item_position < direction_position) return tc::narrow<short>(i - 1);
		}

		return -1;
	};
	constexpr short get_first_item_of_type_before_position(belt_utility::belt_direction direction, const long long segment_end_direction, long long item_goal_distance, item_32_data& item_data, item_type type, long long direction_position) const noexcept
	{
		for (long long i = 0ll; i < item_count; ++i)
		{
			if (item_data.items[i].type != type) continue;
			auto item_position = get_item_direction_position(direction, segment_end_direction, item_goal_distance, item_data, tc::narrow<short>(i));
			if (item_position <= direction_position + belt_item_size) return tc::narrow<short>(i);
		}

		return -1;
	};
	constexpr item_removal_result try_to_remove_item(goal_distance* item_goal_distance, item_32_data& item_data) noexcept
	{
		if (std::is_constant_evaluated() == false)
		{
			if (item_data.item_distance[belt_item_size - 1] == 0 && item_data.contains_item[belt_item_size - 1])
			{
				return remove_item(item_goal_distance->get_unsafe_index_ptr(), item_data, belt_item_size - 1);
			}
		}
		else
		{
			if (item_goal_distance->get_distance() == 0)
			{
				return remove_item(item_goal_distance->get_unsafe_index_ptr(), item_data, belt_item_size - 1);
			}
		}
	};

	constexpr void update_belt(goal_distance* item_goal_distance) const noexcept
	{
		if (std::is_constant_evaluated() == false) items_moved_per_frame += count();
		item_goal_distance->subtract_goal_distance(1);
	};
	/*
	* continue incrementing the item_distance until they are all at 32
	* but only the first item that can be moved should be moved
	* the principle is the same here as on the normal update
	* moving one item moves all item behind it too.
	*/
	constexpr void items_stuck_update(item_32_data& item_data) const noexcept
	{
		long long moved_items = 0;
		long long previous_item_dist = belt_item_size;
		for (long long i = 1; i < item_count; ++i)
		{
			if (previous_item_dist < item_data.item_distance[i])
			{
				--item_data.item_distance[i];
				++moved_items;
			}
			previous_item_dist = item_data.item_distance[i] + belt_item_size;
			/*else if (item_data.contains_item[i] == false && item_data.item_distance[i] > 0)
			{
				--item_data.item_distance[i];
				if (item_data.item_distance[i])
				{
					if (remove_empty_item(item_data, i)) --i;
				}
			}*/
		}

		if (std::is_constant_evaluated() == false) items_moved_per_frame += moved_items;
	};
	inline constexpr bool is_goal_distance_zero(long long* item_goal_distance) const noexcept
	{
		return *item_goal_distance == 0;
	};
};


constexpr auto test_item_32_data_split(int split_index) noexcept
{
	item_32_data data{};
	for (long long i = 0; i < 32; ++i)
	{
		data.contains_item[i] = true;
		data.item_distance[i] = i * 32;
		data.items[i] = belt_item{};
	}

	auto split_result = item_data_utility::split_from_index(data, split_index);
	auto split_data = split_result.data;
	auto split_left = 32 - (split_index + 1);
	auto split_right = split_index + 1;

	if (split_left <= 0 && split_left >= 32) return false;
	if (split_right <= 0 && split_right >= 32) return false;

	for (long long i = 0; i < split_right; ++i)
	{
		if (data.contains_item[i] != true) return false;
	}
	for (long long i = 0; i < split_left; ++i)
	{
		if (split_data.contains_item[i] != true) return false;
	}
	for (long long i = split_right; i < 32; ++i)
	{
		if (data.contains_item[i] != false) return false;
	}
	for (long long i = split_left; i < 32; ++i)
	{
		if (split_data.contains_item[i] != false) return false;
	}

	if (split_right >= 1 && data.contains_item[split_right - 1] != true) return false;
	if (split_left >= 1 && split_data.contains_item[split_left - 1] != true) return false;
	if (data.contains_item[split_right] != false) return false;
	if (split_data.contains_item[split_left] != false) return false;
	if (data.item_distance[0] != 0) return false;
	if (split_data.item_distance[0] + split_result.missing_distance != 32) return false;
	if (data.item_distance[1] != 32) return false;
	if (split_data.item_distance[1] + split_result.missing_distance != 64) return false;

	return true;
};
static_assert(test_item_32_data_split(15) == true, "did not split item data");
static_assert(test_item_32_data_split(10) == true, "did not split item data");
static_assert(test_item_32_data_split(24) == true, "did not split item data");
