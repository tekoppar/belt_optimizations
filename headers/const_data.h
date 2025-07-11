#pragma once

#include "item_32.h"
#include "index_inserter.h"
#include "mem_vector.h"
#include "shared_classes.h"

//#define _SIMPLE_MEMORY_LEAK_DETECTION
/*#ifdef _DEBUG
#define _BOUNDS_CHECKING_
#endif*/

#define __BELT_SWITCH__ 3

#if __BELT_SWITCH__ == 3
using item_groups_type = item_32;
using item_groups_data_type = item_32_data;
#elif __BELT_SWITCH__ == 4
using item_groups_type = item_256;
#endif

struct item_groups_head
{
	long long distance{ -1ll };
	long long next_item_group_index{ -1ll };
	item_groups_type item_group;
	item_groups_data_type item_group_data;
};

#define __BELT_SEGMENT_VECTOR_ITERATORS__
#define __BELT_SEGMENT_VECTOR_TYPE__
#ifdef __BELT_SEGMENT_VECTOR_TYPE__

using _data_vector = mem::vector<item_groups_data_type, mem::Allocating_Type::ALIGNED_MALLOC, mem::allocator<item_groups_data_type, mem::Allocating_Type::ALIGNED_MALLOC>, mem::use_memcpy::force_checks_off>;
using _vector = mem::vector<item_groups_type, mem::Allocating_Type::ALIGNED_MALLOC, mem::allocator<item_groups_type, mem::Allocating_Type::ALIGNED_MALLOC>, mem::use_memcpy::force_checks_off>;
using _vector_distance = mem::vector<long long, mem::Allocating_Type::ALIGNED_MALLOC, mem::allocator<long long, mem::Allocating_Type::ALIGNED_MALLOC>, mem::use_memcpy::force_checks_off>;
using _vector_goal_distance = mem::vector<goal_distance, mem::Allocating_Type::ALIGNED_MALLOC, mem::allocator<goal_distance, mem::Allocating_Type::ALIGNED_MALLOC>, mem::use_memcpy::force_checks_off>;
using _vector_item_groups_head = mem::vector<item_groups_head, mem::Allocating_Type::ALIGNED_MALLOC, mem::allocator<item_groups_head, mem::Allocating_Type::ALIGNED_MALLOC>, mem::use_memcpy::force_checks_off>;

using _simple_inserter_vector = mem::vector<index_inserter, mem::Allocating_Type::ALIGNED_NEW, mem::allocator<index_inserter, mem::Allocating_Type::ALIGNED_NEW>, mem::use_memcpy::force_checks_off>;
using _vector_inserters = mem::vector<_simple_inserter_vector, mem::Allocating_Type::ALIGNED_NEW, mem::allocator<_simple_inserter_vector, mem::Allocating_Type::ALIGNED_NEW>, mem::use_memcpy::force_checks_off>;

using _vector_item_groups_head_type = _vector_item_groups_head;

struct remove_iterators_
{
	typename _vector::iterator item_groups_iter{ nullptr };
	typename _data_vector::iterator item_groups_data_iter{ nullptr };
	typename _vector_distance::iterator item_groups_dist_iter{ nullptr };
	typename _vector_goal_distance::iterator item_groups_goal_dist_iter{ nullptr };
};
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