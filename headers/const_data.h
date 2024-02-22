#pragma once

#include <vector>

#include "macros.h"
#include "item_32.h"
#include "item_256.h"
#include "mem_vector.h"

//#define _SIMPLE_MEMORY_LEAK_DETECTION
#ifdef _DEBUG
//#define _BOUNDS_CHECKING_
#endif


#define __BELT_SWITCH__ 3

#if __BELT_SWITCH__ == 3
using item_groups_type = item_32;
using item_groups_data_type = item_32_data;
#elif __BELT_SWITCH__ == 4
using item_groups_type = item_256;
#endif

#define __BELT_SEGMENT_VECTOR_ITERATORS__
#define __BELT_SEGMENT_VECTOR_TYPE__
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
using _data_vector = mem::vector<item_groups_data_type, mem::Allocating_Type::ALIGNED_NEW, mem::allocator<item_groups_data_type, mem::Allocating_Type::ALIGNED_NEW>, mem::use_memcpy::force_checks_off>;
using _vector = mem::vector<item_groups_type, mem::Allocating_Type::ALIGNED_NEW, mem::allocator<item_groups_type, mem::Allocating_Type::ALIGNED_NEW>, mem::use_memcpy::force_checks_off>;
struct remove_iterators_
{
	_vector::iterator item_groups_iter{ nullptr };
	_data_vector::iterator item_groups_data_iter{ nullptr };
};
/*
* how to use: have a loop that uses the begin() of the item_group vector as begin_iter, and
* if the active_mode_iters vector is not empty grab begin() as mode_iters, grab last() as
* end_mode_iters and loop until mode_iters == end_mode_iters. Do a second loop and
* loop until the begin_iter == mode_iters.some_stuck and then call mode_iters.some_stuck->items_stuck_update();
* set begin_iter = mode_iters.first_free and continue the outer loop. After those two loops
* have a third loop that loops until begin_iter == (last of the item_group vector)
* this is too catch if the last active_mode_iters doesn't point to the end
*/
struct active_mode_iters
{
	//the item group that has some items stuck that needs to have items_stuck_update called
	_vector::iterator some_stuck{ nullptr };
	//first free is the first free item group after some_stuck, anything in between those are stuck
	_vector::iterator first_free{ nullptr };
};
using active_mode_vector = mem::vector<active_mode_iters, mem::Allocating_Type::ALIGNED_NEW, mem::allocator<active_mode_iters, mem::Allocating_Type::ALIGNED_NEW>, mem::use_memcpy::force_checks_off>;
#else
using _vector = std::vector<item_groups_type>;
#endif

struct item_group_linked_entry
{
	item_groups_type* ptr{ nullptr }; //points to item_32
	item_group_linked_entry* next_group{ nullptr };
};

enum class item_group_scan
{
	same,
	found_next,
	found_prev,
	no_close_forwards,
	no_close_backwards,
	no_close
};