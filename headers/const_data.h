#pragma once

#include "macros.h"

//#define _SIMPLE_MEMORY_LEAK_DETECTION
#ifdef _DEBUG
//#define _BOUNDS_CHECKING_
#endif

#include "item_32.h"
#include "item_256.h"

#define __BELT_SWITCH__ 3

#if __BELT_SWITCH__ == 3
using item_groups_type = item_32;
#elif __BELT_SWITCH__ == 4
using item_groups_type = item_256;
#endif

#include <vector>
#include "mem_vector.h"

#define __BELT_SEGMENT_VECTOR_ITERATORS__
#define __BELT_SEGMENT_VECTOR_TYPE__
#ifdef __BELT_SEGMENT_VECTOR_TYPE__
using _vector = mem::vector<item_groups_type>;
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