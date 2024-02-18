#pragma once

#include "belt_segment_shared.h"

#include <immintrin.h>
#include <type_traits>

#include "type_conversion.h"
#include "mem_utilities.h"

#include "item.h"
#include "vectors.h"
#include "belt_utility_data.h"
#include "belt_intrinsics.h"

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
	__declspec(align(32)) bool contains_item[32]{
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false
	};
	__declspec(align(32)) unsigned char item_distance[32]{
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
			std::memcpy(&item_distance[0], &o.item_distance[0], 32);
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
			std::memcpy(&item_distance[0], &o.item_distance[0], 32);
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
			std::memcpy(&item_distance[0], &o.item_distance[0], 32);
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
			std::memcpy(&item_distance[0], &o.item_distance[0], 32);
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

class item_32
{
	constexpr static short single_belt_length = 128;
	constexpr static short belt_length = 128 * 8;
public:
	inline static long long items_moved_per_frame = 0;
	constexpr static long long belt_item_size = 32;
	constexpr static unsigned char max_distance_between_items = 255u;

	static void detect_memory_leak(item_32* ptr)
	{
		if (ptr->item_count > 32 || ptr->item_position_y != 0) throw std::runtime_error("");
	};

private:
	long long item_position_y{ 0 }; //0-7
	long long item_position_x{ 0 }; //8-15
	long long item_goal_distance{ 0 }; //16-23
	const belt_utility::belt_direction direction{ belt_utility::belt_direction::left_right }; //24-27
	int last_distance_to_item{ 0 }; //28-31
	alignas(8) long long item_count{ 0 }; //32-39
	belt_utility::belt_update_mode active_mode{ belt_utility::belt_update_mode::free }; //40-43
	alignas(8) belt_segment* owner_ptr { nullptr }; //43-51
	alignas(8) item_32_data* item_data { nullptr };
	/*__declspec(align(32)) bool contains_item[32]{
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
		false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false
	};
	__declspec(align(32)) unsigned char item_distance[32]{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	};
	__declspec(align(64)) belt_item items[32];*/

	constexpr static belt_utility::belt_color color{ belt_utility::belt_color::gray };
public:
	constexpr item_32() noexcept
	{};
	constexpr item_32(const belt_utility::belt_direction dir, item_32_data* item_ptr) noexcept :
		direction{ dir },
		item_data{ item_ptr }
	{};
	constexpr item_32(const belt_utility::belt_direction dir, long long goal_position, item_32_data* item_ptr) noexcept :
		item_goal_distance{ goal_position },
		direction{ dir },
		item_data{ item_ptr }
	{};
	constexpr ~item_32() noexcept
	{};

	constexpr item_32(const item_32& o) noexcept :
		item_position_y{ o.item_position_y },
		item_position_x{ o.item_position_x },
		item_goal_distance{ o.item_goal_distance },
		direction{ o.direction },
		last_distance_to_item{ o.last_distance_to_item },
		item_count{ o.item_count },
		active_mode{ o.active_mode },
		owner_ptr{ o.owner_ptr },
		item_data{ o.item_data }
	{};
	constexpr item_32(item_32&& o) noexcept :
		item_position_y{ o.item_position_y },
		item_position_x{ o.item_position_x },
		item_goal_distance{ o.item_goal_distance },
		direction{ o.direction },
		last_distance_to_item{ o.last_distance_to_item },
		item_count{ o.item_count },
		active_mode{ o.active_mode },
		owner_ptr{ o.owner_ptr },
		item_data{ std::move(o.item_data) }
	{};
	constexpr item_32& operator=(const item_32& o) noexcept
	{
		item_position_y = o.item_position_y;
		item_position_x = o.item_position_x;
		item_goal_distance = o.item_goal_distance;
		last_distance_to_item = o.last_distance_to_item;
		item_count = o.item_count;
		active_mode = o.active_mode;
		owner_ptr = o.owner_ptr;
		item_data = o.item_data;

		return *this;
	};
	constexpr item_32& operator=(item_32&& o) noexcept
	{
		item_position_y = o.item_position_y;
		item_position_x = o.item_position_x;
		item_goal_distance = o.item_goal_distance;
		last_distance_to_item = o.last_distance_to_item;
		item_count = o.item_count;
		active_mode = o.active_mode;
		owner_ptr = o.owner_ptr;
		item_data = std::move(o.item_data);

		return *this;
	};

	constexpr item_uint operator[](long long i) noexcept
	{
#ifdef _BOUNDS_CHECKING_
		if (i < item_count)
#endif
			return { item_data->items[i].type, vec2_uint{item_position_x + get_distance_to_item(i), item_position_y} };
	};
	constexpr const item_uint operator[](long long i) const noexcept
	{
#ifdef _BOUNDS_CHECKING_
		if (i < item_count)
#endif
			return { item_data->items[i].type, vec2_uint{item_position_x - get_distance_to_item(i), item_position_y} };
	};
	inline constexpr item_uint get(long long i) noexcept
	{
#ifdef _BOUNDS_CHECKING_
		if (i < item_count)
#endif
			return { item_data->items[i].type, vec2_uint{item_position_x - get_distance_to_item(i), item_position_y} };
#ifdef _BOUNDS_CHECKING_
		else
			return {};
#endif
	};
	inline constexpr const item_uint get(long long i) const noexcept
	{
#ifdef _BOUNDS_CHECKING_
		if (i < item_count)
#endif
			return { item_data->items[i].type, vec2_uint{item_position_x - get_distance_to_item(i), item_position_y} };
#ifdef _BOUNDS_CHECKING_
		else
			return {};
#endif
	};

	inline constexpr vec2_uint get_position() const noexcept
	{
		return { item_position_x, item_position_y };
	};
	inline constexpr long long get_direction_position() const noexcept
	{
		switch (direction)
		{
			default:
			case belt_utility::belt_direction::null:
			case belt_utility::belt_direction::left_right: return item_position_x;
			case belt_utility::belt_direction::right_left: return item_position_x;
			case belt_utility::belt_direction::top_bottom: return item_position_y;
			case belt_utility::belt_direction::bottom_top: return item_position_y;
		}
	};
	inline constexpr long long get_item_direction_position(short index) const noexcept
	{
		switch (direction)
		{
			default:
			case belt_utility::belt_direction::null:
			case belt_utility::belt_direction::left_right: return item_position_x - get_distance_to_item(index);
			case belt_utility::belt_direction::right_left: return item_position_x + get_distance_to_item(index);
			case belt_utility::belt_direction::top_bottom: return item_position_y - get_distance_to_item(index);
			case belt_utility::belt_direction::bottom_top: return item_position_y + get_distance_to_item(index);
		}
	};
	inline constexpr long long get_last_item_direction_position() const noexcept
	{
		switch (direction)
		{
			default:
			case belt_utility::belt_direction::null:
			case belt_utility::belt_direction::left_right: return item_position_x - get_distance_to_last_item();
			case belt_utility::belt_direction::right_left: return item_position_x + get_distance_to_last_item();
			case belt_utility::belt_direction::top_bottom: return item_position_y - get_distance_to_last_item();
			case belt_utility::belt_direction::bottom_top: return item_position_y + get_distance_to_last_item();
		}
	};
	inline constexpr long long get_goal() const noexcept
	{
		return item_goal_distance;
	};
	inline constexpr long long count() const noexcept
	{
		return item_count;
	};
	inline constexpr bool can_add_item() const noexcept
	{
		return item_count < belt_item_size;
	};
	inline constexpr belt_utility::belt_update_mode get_active_mode() const noexcept
	{
		return active_mode;
	};
	inline constexpr void set_active_mode(belt_utility::belt_update_mode new_mode) noexcept
	{
		active_mode = new_mode;
	};
	inline constexpr void set_owner_ptr(belt_segment* owner) noexcept
	{
		owner_ptr = owner;
	};
	inline constexpr void set_item_data_ptr(item_32_data* data_ptr) noexcept
	{
		item_data = data_ptr;
	};
private:
	constexpr long long fast_distance_to_last_item(const unsigned char* ch) const noexcept
	{
		auto c1 = ch[0] + ch[1];
		auto c2 = ch[2] + ch[3];
		auto c3 = ch[4] + ch[5];
		auto c4 = ch[6] + ch[7];
		auto c5 = ch[8] + ch[9];
		auto c6 = ch[10] + ch[11];
		auto c7 = ch[12] + ch[13];
		auto c8 = ch[14] + ch[15];

		auto c9 = ch[16] + ch[17];
		auto c10 = ch[18] + ch[19];
		auto c11 = ch[20] + ch[21];
		auto c12 = ch[22] + ch[23];
		auto c13 = ch[24] + ch[25];
		auto c14 = ch[26] + ch[27];
		auto c15 = ch[28] + ch[29];
		auto c16 = ch[30] + ch[31];

		int i1 = c1 + c2;
		int i2 = c3 + c4;
		int i3 = c5 + c6;
		int i4 = c7 + c8;
		int i5 = c9 + c10;
		int i6 = c11 + c12;
		int i7 = c13 + c14;
		int i8 = c15 + c16;

		long l1 = i1 + i2;
		long l2 = i3 + i4;
		long l3 = i5 + i6;
		long l4 = i7 + i8;

		long long ll1 = l1 + l2;
		long long ll2 = l3 + l4;
		return ll1 + ll2;
	};
public:
	constexpr long long get_distance_to_last_item() const noexcept
	{
		return last_distance_to_item;// fast_distance_to_last_item(item_data->item_distance);
		const auto local_item_count = item_count;
		if (local_item_count == 0) return 0;
		long long total_distance = 0;
		for (long long i = 0; i < local_item_count; ++i)
		{
			total_distance += static_cast<long long>(item_data->item_distance[i]);
		}

		return total_distance;
	};
	constexpr long long get_distance_to_item(long long item_index) const noexcept
	{
		if (item_index >= item_count) return 0;
		const auto local_item_count = item_count;
		long long total_distance = 0;
		const long long l = item_index + 1;
		for (long long i = 0; i < l; ++i)
		{
			if (i < local_item_count)
				total_distance += tc::widen<long long>(item_data->item_distance[i]);
		}

		return total_distance;
	};

	constexpr bool can_add_item(vec2_uint new_item_position) const noexcept
	{
		if (new_item_position.x >= item_position_x + 32 && item_goal_distance >= 32u) return true;

		const long long distance_to_last_item = (new_item_position.x - get_distance_to_last_item());
		if (get_direction_position() - get_distance_to_last_item() > new_item_position.x && distance_to_last_item <= 255u) return true;

		const long long l = count() - 1;
		for (long long i = 0; i < l; ++i)
		{
			if (new_item_position.x > get_direction_position() - get_distance_to_item(i + 1) && item_data->item_distance[i + 1] >= 32) return true;
		}
		return false;

		//return new_item_position.x - tc::narrow<long long>(get_distance_to_last_item()) <= 255u;
		//(new_item_position.x - get_last_item_direction_position()) > 32;
	};
	constexpr short add_item(const belt_item& new_item, vec2_uint new_item_position) noexcept
	{
		if (item_count == 0ll)
		{
			item_position_x = new_item_position.x;
			item_position_y = new_item_position.y;
			item_goal_distance -= new_item_position.x;
			++item_count;
			item_data->contains_item[item_count - 1] = true;
			item_data->item_distance[item_count - 1] = static_cast<unsigned char>(0);
			item_data->items[item_count - 1] = new_item;
			return 0;
		}
		//if we should add before the first item
		//checks if the new item position is greater then the current item position + 32
		//and that there's still space to fit the item before the goal
		if (new_item_position.x >= item_position_x + 32 && item_goal_distance >= 32u)
		{
			const long long new_distance = new_item_position.x - item_position_x;
			item_goal_distance -= new_distance;
			shift_arrays_left();
			item_position_x = new_item_position.x;
			item_position_y = new_item_position.y;
			++item_count;
			item_data->contains_item[0] = true;
			item_data->item_distance[0] = static_cast<unsigned char>(0);
			item_data->item_distance[1] = static_cast<unsigned char>(new_distance);
			item_data->items[0] = new_item;
			last_distance_to_item += item_data->item_distance[1];
			return 0;
		}

		//if we should add after the last item
		//it does this by comparing that the last items position is greater then the new item
		//and then checks if the distance between the last item and the new item is less than 256
		const long long distance_to_last_item = (new_item_position.x - get_distance_to_last_item());
		if (get_direction_position() - get_distance_to_last_item() > new_item_position.x && distance_to_last_item <= 255u)
		{
			++item_count;
			item_data->contains_item[item_count - 1] = true;
			item_data->item_distance[item_count - 1] = static_cast<unsigned char>(distance_to_last_item);
			item_data->items[item_count - 1] = new_item;
			last_distance_to_item += item_data->item_distance[item_count - 1];
			return item_count - 1;
		}

		//if we should add in between items
		const long long l = count() - 1ll;
		for (long long i = 0; i < l; ++i)
		{
			if (new_item_position.x > get_direction_position() - get_distance_to_item(i + 1) && item_data->item_distance[i + 1] >= 64)
			{
				shift_arrays_right_from_index(i + 1);
				++item_count;
				item_data->contains_item[i + 1] = true;
				item_data->item_distance[i + 1] = static_cast<unsigned char>(new_item_position.x - get_distance_to_item(i + 1));
				item_data->items[i + 1] = new_item;
				last_distance_to_item += item_data->item_distance[i + 1];
				return tc::narrow<short>(i) + 1;
			}
		}

		return -1;
	};
	constexpr bool remove_item(const short index) noexcept
	{
		long long new_goal_distance = 0;
		if (item_count > 1ll)
		{
			if (index == 0)
			{
				new_goal_distance = item_count >= 2ll ? get_distance_to_item(1) : 0;
				item_data->contains_item[index] = false;
				item_data->item_distance[index] = 0;
				shift_arrays_right();
				item_goal_distance += new_goal_distance;
				item_position_x -= new_goal_distance;
				last_distance_to_item -= new_goal_distance;
				--item_count;
			}
			else
			{
				const unsigned char removed_distance = item_data->item_distance[index];
				item_data->contains_item[index] = false;
				item_data->item_distance[index] = 0;
				item_data->items[index] = belt_item{};
				shift_arrays_right_from_index(index);
				item_data->item_distance[index] += removed_distance;
				last_distance_to_item -= removed_distance;
				--item_count;
			}
		}
		else
		{
			last_distance_to_item = 0;
			item_goal_distance = 0;
			--item_count;
			item_data->contains_item[index] = false;
			item_data->item_distance[index] = 0;
			item_data->items[index] = belt_item{};
			belt_segment_helpers::item_group_has_zero_count(owner_ptr, this, item_data);

			//item_position_x = 0;

			//reinterpret_cast<mem::single_list_block_node<item_32>*>(((&item_position_y) - 8))->erase();
			//TODO at this point the item_32 should be removed since it no longer contains any items nor a goal
		}

		return true;
	};
	constexpr short get_first_item_of_type(item_type type) const noexcept
	{
		for (long long i = 0ll; i < item_count; ++i)
		{
			if (item_data->items[i].type == type) return tc::narrow<short>(i);
		}

		return -1;
	};
	constexpr short get_first_item_of_type_before_position(item_type type, vec2_uint pos) const noexcept
	{
		for (long long i = 0ll; i < item_count; ++i)
		{
			if (item_data->items[i].type == type && get_item_direction_position(tc::narrow<short>(i)) <= pos.x) return tc::narrow<short>(i);
		}

		return -1;
	};
	constexpr void recalculate_distance(short from_index, unsigned char added_distance) noexcept
	{
		for (long long i = from_index; i < item_count; ++i)
		{
			if (tc::widen<unsigned long long>(item_data->item_distance[tc::unsign(i)]) + added_distance > 255) return;

			item_data->item_distance[tc::unsign(i)] += added_distance;
		}
	};
	constexpr void recalculate_distance(const vec2_uint& old_position, const vec2_uint new_position) noexcept
	{
		const unsigned char new_distance = tc::narrow<unsigned char>(new_position.x - old_position.x);
		for (long long i = 0ll; i < item_count; ++i)
		{
			item_data->item_distance[tc::unsign(i)] += new_distance;
		}
	};
	constexpr void shift_arrays_left() noexcept
	{
		if (std::is_constant_evaluated())
		{
			for (long long i = item_count - 1ll; i >= 0ll; --i)
			{
				item_data->contains_item[i + 1] = item_data->contains_item[i];
				item_data->item_distance[i + 1] = item_data->item_distance[i];
				item_data->items[i + 1] = item_data->items[i];
			}
		}
		else
		{
			belt_utility::_mm256_slli_si256__p<1>((__m256i*) & item_data->contains_item[0]);
			belt_utility::_mm256_slli_si256__p<1>((__m256i*) & item_data->item_distance[0]);
			belt_utility::_mm512_slli2x256_si512__<2>((__m256i*) & item_data->items[0]);
#ifdef _SIMPLE_MEMORY_LEAK_DETECTION
			detect_memory_leak(this);
#endif
		}
	};
	constexpr void forced_shift_arrays_left() noexcept
	{
		for (long long i = item_count - 1ll; i >= 0ll; --i)
		{
			item_data->contains_item[i + 1] = item_data->contains_item[i];
			item_data->item_distance[i + 1] = item_data->item_distance[i];
			item_data->items[i + 1] = item_data->items[i];
		}
	};
	constexpr void shift_arrays_left_from_index(long long index) noexcept
	{
		for (long long i = item_count - 1ll; i >= index; --i)
		{
			item_data->contains_item[i + 1] = item_data->contains_item[i];
			item_data->item_distance[i + 1] = item_data->item_distance[i];
			item_data->items[i + 1] = item_data->items[i];
		}
	};
	constexpr void forced_shift_arrays_right() noexcept
	{
		bool previous_b_x = item_data->contains_item[item_count - 1], current_b_x = false;
		unsigned char previous_c_x = item_data->item_distance[item_count - 1], current_c_x = 0;
		belt_item previous_i_x = item_data->items[item_count - 1], current_i_x = {};
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
		for (long long i = expr::max(tc::widen<long long>(item_count), 31ll, 32ll); i >= 0; --i)
#else
		for (long long i = item_count; i >= 0; --i)
#endif
		{
			current_b_x = item_data->contains_item[i];
			item_data->contains_item[i] = previous_b_x;
			previous_b_x = current_b_x;

			current_c_x = item_data->item_distance[i];
			item_data->item_distance[i] = previous_c_x;
			previous_c_x = current_c_x;

			current_i_x = item_data->items[i];
			item_data->items[i] = previous_i_x;
			previous_i_x = current_i_x;
		}
		/*belt_item previous_i_x = item_data->items[item_count - 1], current_i_x = {};
		bool previous_b_x = item_data->contains_item[item_count - 1], current_b_x = false;
		unsigned char previous_c_x = item_data->item_distance[item_count - 1], current_c_x = 0;
		for (long long i = item_count; i >= 0; --i)
		{
			current_b_x = item_data->contains_item[i];
			item_data->contains_item[i] = previous_b_x;
			previous_b_x = current_b_x;

			current_c_x = item_data->item_distance[i];
			item_data->item_distance[i] = previous_c_x;
			previous_c_x = current_c_x;

			current_i_x = item_data->items[i];
			item_data->items[i] = previous_i_x;
			previous_i_x = current_i_x;
		}*/
		if (item_count > 0)
		{
			item_data->contains_item[item_count - 1] = false;
			item_data->item_distance[item_count - 1] = 0;
		}
	};
	constexpr void shift_arrays_right() noexcept
	{
		if (std::is_constant_evaluated())
		{
			bool previous_b_x = item_data->contains_item[item_count - 1], current_b_x = false;
			unsigned char previous_c_x = item_data->item_distance[item_count - 1], current_c_x = 0;
			belt_item previous_i_x = item_data->items[item_count - 1], current_i_x = {};
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
			for (long long i = expr::max(tc::widen<long long>(item_count), 31ll, 32ll); i >= 0; --i)
#else
			for (long long i = item_count; i >= 0; --i)
#endif
			{
				current_b_x = item_data->contains_item[i];
				item_data->contains_item[i] = previous_b_x;
				previous_b_x = current_b_x;

				current_c_x = item_data->item_distance[i];
				item_data->item_distance[i] = previous_c_x;
				previous_c_x = current_c_x;

				current_i_x = item_data->items[i];
				item_data->items[i] = previous_i_x;
				previous_i_x = current_i_x;
			}
			/*belt_item previous_i_x = item_data->items[item_count - 1], current_i_x = {};
			bool previous_b_x = item_data->contains_item[item_count - 1], current_b_x = false;
			unsigned char previous_c_x = item_data->item_distance[item_count - 1], current_c_x = 0;
			for (long long i = item_count; i >= 0; --i)
			{
				current_b_x = item_data->contains_item[i];
				item_data->contains_item[i] = previous_b_x;
				previous_b_x = current_b_x;

				current_c_x = item_data->item_distance[i];
				item_data->item_distance[i] = previous_c_x;
				previous_c_x = current_c_x;

				current_i_x = item_data->items[i];
				item_data->items[i] = previous_i_x;
				previous_i_x = current_i_x;
			}*/
			if (item_count > 0)
			{
				item_data->contains_item[item_count - 1] = false;
				item_data->item_distance[item_count - 1] = 0;
			}
		}
		else
		{
			belt_utility::_mm256_srli_si256__p<1>((__m256i*) & item_data->contains_item[0]);
			belt_utility::_mm256_srli_si256__p<1>((__m256i*) & item_data->item_distance[0]);
			belt_utility::_mm512_srli2x256_si512__<2>((__m256i*) & item_data->items[0]);
			item_data->contains_item[item_count - 1] = false;
			item_data->item_distance[item_count - 1] = 0;
#ifdef _SIMPLE_MEMORY_LEAK_DETECTION
			detect_memory_leak(this);
#endif
		}
	};
	constexpr void shift_arrays_right_from_index(long long index) noexcept
	{
		bool previous_b_x = item_data->contains_item[item_count - 1], current_b_x = false;
		unsigned char previous_c_x = item_data->item_distance[item_count - 1], current_c_x = 0;
		belt_item previous_i_x = item_data->items[item_count - 1], current_i_x = {};
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
		for (long long i = expr::max(tc::widen<long long>(item_count), 31ll, 32ll); i >= index; --i)
#else
		for (long long i = item_count; i >= index; --i)
#endif
		{
			current_b_x = item_data->contains_item[i];
			item_data->contains_item[i] = previous_b_x;
			previous_b_x = current_b_x;

			current_c_x = item_data->item_distance[i];
			item_data->item_distance[i] = previous_c_x;
			previous_c_x = current_c_x;

			current_i_x = item_data->items[i];
			item_data->items[i] = previous_i_x;
			previous_i_x = current_i_x;
		}
	};
	inline constexpr bool outside_belt(vec2_uint pos) const noexcept
	{
		switch (direction)
		{
			default:
			case belt_utility::belt_direction::null:
			case belt_utility::belt_direction::left_right: return pos.x >= item_position_x + belt_length;
			case belt_utility::belt_direction::right_left: return pos.x <= item_position_x;
			case belt_utility::belt_direction::top_bottom: return pos.y >= item_position_y + belt_length;
			case belt_utility::belt_direction::bottom_top: return pos.y <= item_position_y;
		}
	};
	constexpr void try_to_remove_item() noexcept
	{
		if (std::is_constant_evaluated() == false)
		{
			if (item_data->item_distance[belt_item_size - 1] == 0 && item_data->contains_item[belt_item_size - 1])
			{
				if (!remove_item(belt_item_size - 1))
				{
					active_mode = belt_utility::belt_update_mode::first_stuck;//is_stuck = true;
				}
			}
		}
		else
		{
			if (item_goal_distance == 0)
			{
				if (remove_item(belt_item_size - 1)) active_mode = belt_utility::belt_update_mode::free;// is_stuck = false;
			}
		}
	};
	constexpr void update_belt() noexcept
	{
		//if (active_mode == belt_utility::belt_update_mode::free)
		//{
			//if (std::is_constant_evaluated() == false) items_moved_per_frame += count();

		++item_position_x;
		--item_goal_distance;
		if (item_goal_distance == 0ll) belt_segment_helpers::item_group_has_reached_goal(owner_ptr, this);
		/*}
		else if (belt_utility::belt_update_mode::first_stuck == active_mode)
		{
			items_stuck_update();
		}*/
	};
	/*
	* continue incrementing the item_distance until they are all at 32
	* but only the first item that can be moved should be moved
	* the principle is the same here as on the normal update
	* moving one item moves all item behind it too.
	*/
	constexpr void items_stuck_update() noexcept
	{
		long long moved_items = 0;
		for (long long i = 1; i < item_count; ++i)
		{
			if (item_data->item_distance[i] > 32)
			{
				--item_data->item_distance[i];
				moved_items = item_count - i;
			}
		}

		if (moved_items == 0) active_mode = belt_utility::belt_update_mode::all_stuck;//is_all_stuck = true;
		if (std::is_constant_evaluated() == false) items_moved_per_frame += moved_items;
	};
	inline constexpr bool is_goal_distance_zero() const noexcept
	{
		return item_goal_distance == 0;
	};
};
/*constexpr bool test_item_iterator() noexcept
{
	std::vector<item_32> items;
	items.emplace_back(belt_utility::belt_direction::left_right, 500ll);
	items.emplace_back(belt_utility::belt_direction::left_right, 500ll);
	items.emplace_back(belt_utility::belt_direction::left_right, 500ll);
	items.emplace_back(belt_utility::belt_direction::left_right, 500ll);
	items.emplace_back(belt_utility::belt_direction::left_right, 500ll);

	auto begin_iter = items.begin();

	begin_iter->add_item({ item_type::log }, vec2_uint{ 0, 0 });
	begin_iter->add_item({ item_type::log }, vec2_uint{ 64, 0 });
	begin_iter->add_item({ item_type::log }, vec2_uint{ 128, 0 });

	item_uint new_item{ item_type::log, vec2_uint{256, 0} };

	auto iter = belt_utility::find_closest_item_group(items, new_item.position);
	if (belt_utility::find_closest_item_group_return_result::insert_into_group == iter.scan)
	{
		if (iter.result == items.end()) throw std::runtime_error("");

		if (iter.result->count() >= 32) //need to split item_group into 2
		{
			throw std::runtime_error("");
			return false;
		}
		else
		{
			const short added_index = iter.result->add_item(new_item, new_item.position);
			if (added_index != -1)
			{
				return true;
			}
		}
		return false;
	}


	auto last_iter = items.end();
	--last_iter;

	while (begin_iter != last_iter)
	{
		++begin_iter;
	}

	return true;
};
static_assert(test_item_iterator() == true, "no");*/