#pragma once

#include <type_traits>
#include <utility>

#include "belt_segment_shared.h"

#include "const_data.h"

#include "vectors.h"
#include "index_iterator.h"
#include "item.h"
#include "item_32.h"
#include "item_256.h"
#include "belt_utility_data.h"

class belt_segment;

class index_inserter
{
public:
	static inline constexpr long long inserter_grid_size{ 32ll };
	static inline long long grabbed_items{ 0ll };

private:
	vec2_uint position{ 0, 0 };
	index_iterator<item_groups_type, _vector> item_group{ 0ull, nullptr };
	index_iterator<item_groups_data_type, _data_vector> item_group_data{ 0ull, nullptr };
	index_iterator<goal_distance, _vector_goal_distance> item_group_distance{ 0ull, nullptr };
	_vector_distance const* index_calculation_vector{ nullptr };
	item_type item_need_types[8]{ item_type::pink_square, item_type::pink_square, item_type::pink_square, item_type::pink_square,
	item_type::pink_square, item_type::pink_square, item_type::pink_square, item_type::pink_square };
	__declspec(align(32)) item_uint item;
#ifdef _DEBUG
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
		item_group{ o.item_group },
		item_group_data{ o.item_group_data },
		item_group_distance{ o.item_group_distance },
		index_calculation_vector{ o.index_calculation_vector },
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
		item_group{ std::exchange(o.item_group, {0ull, nullptr}) },
		item_group_data{ std::exchange(o.item_group_data, {0ull, nullptr}) },
		item_group_distance{ std::exchange(o.item_group_distance, {0ull, nullptr}) },
		index_calculation_vector{ std::exchange(o.index_calculation_vector, nullptr) },
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
		item_group = o.item_group;
		item_group_data = o.item_group_data;
		item_group_distance = o.item_group_distance;
		index_calculation_vector = o.index_calculation_vector;
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
		item_group = std::exchange(o.item_group, { 0ull, nullptr });
		item_group_data = std::exchange(o.item_group_data, { 0ull, nullptr });
		item_group_distance = std::exchange(o.item_group_distance, { 0ull, nullptr });
		index_calculation_vector = std::exchange(o.index_calculation_vector, nullptr);
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
	inline constexpr bool has_linked_list_data() const noexcept
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
	constexpr bool check_if_indexes_matches(long long segment_end_direction) noexcept
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
				auto inserter_dist = (segment_end_direction - position.x);
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
	};
public:
	constexpr bool update_linked_list_group_data(long long segment_end_direction) noexcept
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
				auto inserter_dist = (segment_end_direction - position.x);
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
	};
public:
	constexpr void set_linked_list_data(const index_iterator<item_groups_type, _vector>& iter, const index_iterator<item_groups_data_type, _data_vector> data_iter, const index_iterator<goal_distance, _vector_goal_distance>& dist_iter, _vector_distance const* dist_ptr) noexcept
	{
		item_group = iter;
		item_group_data = data_iter;
		item_group_distance = dist_iter;
		index_calculation_vector = dist_ptr;
	};
	inline constexpr void grab_item(const item_uint& _item) noexcept
	{
		item = _item;
	};
	inline constexpr item_uint& get_item() noexcept
	{
		return item;
	};
	inline constexpr index_iterator<item_groups_type, _vector> get_item_group() const noexcept
	{
		return item_group;
	};

	constexpr void update_linked_data(long long segment_end_direction) noexcept
	{
		long long index_ptr = (*item_group_distance).get_index_ptr() - &index_calculation_vector->operator[](0);
		if (std::is_constant_evaluated() == false)
		{
			if (item_group_distance.get_index() > 0ll)
			{
				auto tmp = (item_group_distance - 1);
				auto tmp1 = (*(item_group - 1)).get_distance_to_last_item(*(item_group_data - 1));
				auto dist_to_last_item = (*item_group).get_distance_to_last_item(*item_group_data);
				auto inserter_dist = (segment_end_direction - position.x);

				//current set pointer is past us, go to tmp
				if ((*item_group_distance).get_distance() < inserter_dist && (*item_group_distance).get_distance() + dist_to_last_item < inserter_dist && (*tmp).get_distance() > inserter_dist && (*tmp).get_distance() + tmp1 > inserter_dist)
				{
					(*item_group_distance).update_pointer_and_values();
				}
			}
		}
	};
	constexpr bool linked_data_validation(long long segment_end_direction) noexcept
	{
		if (check_if_indexes_matches(segment_end_direction) == false) return update_linked_list_group_data(segment_end_direction);
		return true;
	};
	constexpr short update(belt_utility::belt_direction direction, long long segment_end_direction, long long segment_y_direction, belt_segment* segment_ptr) noexcept
	{
		if ((*item_group_distance).get_index_ptr() != nullptr)
		{
			//if (check_if_indexes_matches(segment_end_direction) == false) update_linked_list_group_data(segment_end_direction);

			auto inserter_dist = (segment_end_direction - position.x);
			auto dist_to_last_item = (*item_group).get_distance_to_last_item(*item_group_data);
			const auto found_index = (*item_group).get_first_item_of_type_before_position(direction, segment_end_direction, (*item_group_distance).get_distance(), *item_group_data, get_item_type(0), position);
			if (found_index != -1)
			{
				auto item_distance_position = (*item_group).get_item_direction_position(direction, segment_end_direction, (*item_group_distance).get_distance(), *item_group_data, found_index);
				//if (item_group_data.get_index() == 615) __debugbreak();
#ifdef _DEBUG
				if (position.x - item_distance_position > 2048ll)
					throw std::runtime_error("");
#endif
				if (position.x == item_distance_position)
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
	};
};