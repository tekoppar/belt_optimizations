#pragma once

#include "shared_classes.h"
#include "mem_vector.h"
#include "item_32.h"
#include "index_inserter.h"
#include "global_classes.h"

//#define _SIMPLE_MEMORY_LEAK_DETECTION
/*#ifdef _DEBUG
#define _BOUNDS_CHECKING_
#endif*/

using item_groups_type = item_32;
using item_groups_data_type = item_32_data;
using inserter_type = belt_segment_index_inserter;

/*struct item_group_n_data
{
	mem::vector<long long, mem::Allocating_Type::ALIGNED_MALLOC, mem::allocator<long long, mem::Allocating_Type::ALIGNED_MALLOC>, mem::use_memcpy::force_checks_off> distances;
	mem::vector<item_groups_type, mem::Allocating_Type::ALIGNED_MALLOC, mem::allocator<item_groups_type, mem::Allocating_Type::ALIGNED_MALLOC>, mem::use_memcpy::force_checks_off> item_groups;
	mem::vector<item_groups_data_type, mem::Allocating_Type::ALIGNED_MALLOC, mem::allocator<item_groups_data_type, mem::Allocating_Type::ALIGNED_MALLOC>, mem::use_memcpy::force_checks_off> item_group_data;
	long long internal_index{ -1ll };

	constexpr void reserve() noexcept
	{
		constexpr const int size = 8ll;
		distances.reserve(size);
		distances.values.last += size;
		item_groups.reserve(size);
		item_groups.values.last += size;
		item_group_data.reserve(size);
		item_group_data.values.last += size;
	};
};*/

using _distance_type = item_groups_distance;

struct alignas(32) item_groups_head_t
{
	/*0-15*/ _distance_type distance{ -1ll };
	/*16-23*/ long long next_item_group_index{ -1ll };
	/*24*/ item_groups_type item_group;
	/*25*/ char item_to_grab{ -1 }; //index of what item event triggered wants
	/*26*/ long long inserter_index{-1ll};
	//int n_group_data_index{ -1 };
	//int event_trigger_index{ -1 }; //index into what triggered the event
	__declspec(align(32)) item_groups_data_type item_group_data;
};

#define __BELT_SEGMENT_VECTOR_ITERATORS__
#define __BELT_SEGMENT_VECTOR_TYPE__

using _data_vector = mem::vector<item_groups_data_type, mem::Allocating_Type::ALIGNED_MALLOC, mem::allocator<item_groups_data_type, mem::Allocating_Type::ALIGNED_MALLOC>, mem::use_memcpy::force_checks_off>;
using _vector = mem::vector<item_groups_type, mem::Allocating_Type::ALIGNED_MALLOC, mem::allocator<item_groups_type, mem::Allocating_Type::ALIGNED_MALLOC>, mem::use_memcpy::force_checks_off>;
using _vector_distance = mem::vector<_distance_type, mem::Allocating_Type::ALIGNED_MALLOC, mem::allocator<_distance_type, mem::Allocating_Type::ALIGNED_MALLOC>, mem::use_memcpy::force_checks_off>;
using _vector_goal_distance = mem::vector<goal_distance, mem::Allocating_Type::ALIGNED_MALLOC, mem::allocator<goal_distance, mem::Allocating_Type::ALIGNED_MALLOC>, mem::use_memcpy::force_checks_off>;
using _vector_item_groups_head = mem::vector<item_groups_head_t, mem::Allocating_Type::ALIGNED_MALLOC, mem::allocator<item_groups_head_t, mem::Allocating_Type::ALIGNED_MALLOC>, mem::use_memcpy::force_checks_off>;

using _simple_inserter_vector = mem::vector<inserter_type, mem::Allocating_Type::ALIGNED_NEW, mem::allocator<inserter_type, mem::Allocating_Type::ALIGNED_NEW>, mem::use_memcpy::force_checks_off>;
using _vector_inserters = mem::vector<_simple_inserter_vector, mem::Allocating_Type::ALIGNED_NEW, mem::allocator<_simple_inserter_vector, mem::Allocating_Type::ALIGNED_NEW>, mem::use_memcpy::force_checks_off>;

struct inserter_group_indexes_t
{
	long long start{ -1ll };
	long long end{ -1ll };
};

using _inserter_group_indexes = mem::vector<inserter_group_indexes_t, mem::Allocating_Type::ALIGNED_NEW, mem::allocator<inserter_group_indexes_t, mem::Allocating_Type::ALIGNED_NEW>, mem::use_memcpy::force_checks_off>;

using _vector_item_groups_head_type = _vector_item_groups_head;

struct remove_iterators_
{
	typename _vector::iterator item_groups_iter{ nullptr };
	typename _data_vector::iterator item_groups_data_iter{ nullptr };
	typename _vector_distance::iterator item_groups_dist_iter{ nullptr };
	typename _vector_goal_distance::iterator item_groups_goal_dist_iter{ nullptr };
};

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