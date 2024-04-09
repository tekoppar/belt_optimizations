#pragma once

struct goal_distance_dead_object_t
{
	explicit goal_distance_dead_object_t() = default;
}; inline constexpr goal_distance_dead_object_t goal_distance_dead_object_v{};

class goal_distance
{
	long long* index_ptr{ nullptr };
public:
	constexpr goal_distance() noexcept
	{};
	constexpr goal_distance(long long* ptr) noexcept :
		index_ptr{ ptr }
	{};

	constexpr bool operator==(const long long* const rhs) const noexcept
	{
		return index_ptr == rhs;
	};
	constexpr bool operator!=(const long long* const rhs) const noexcept
	{
		return index_ptr != rhs;
	};
	friend constexpr bool operator==(const long long* const lhs, const goal_distance& rhs) noexcept
	{
		return lhs == rhs.get_index_ptr();
	};
	friend constexpr bool operator!=(const long long* const lhs, const goal_distance& rhs) noexcept
	{
		return lhs != rhs.get_index_ptr();
	};

	constexpr void set_checked_index_ptr(std::nullptr_t) noexcept
	{
		if (!is_offset_alive(-1ll, goal_distance_dead_object_v)) index_ptr = nullptr;
#ifdef _DEBUG
		else throw std::runtime_error("");
#endif
	};
	constexpr void set_index_ptr(std::nullptr_t) noexcept
	{
		index_ptr = nullptr;
	};
	inline constexpr bool is_offset_alive(long long offset, goal_distance_dead_object_t) const noexcept
	{
		return (*(index_ptr - offset) == 0x8fffffffffffffff);
	};
	inline constexpr bool is_offset_alive(long long offset) noexcept
	{
		if (*(index_ptr - offset) == 0x8fffffffffffffff)
		{
			*(unsigned long long*)index_ptr = 0x8fffffffffffffff;
			set_index_ptr(nullptr);
			return false;
		}
		return true;
	};
	constexpr long long get_distance() const noexcept
	{
		if (index_ptr == nullptr) return -1ll;
		return *index_ptr;
	};
	constexpr long long* const get_unsafe_index_ptr() noexcept
	{
		return index_ptr;
	};
	constexpr const long long* const get_index_ptr() const noexcept
	{
		return index_ptr;
	};
	constexpr long long get_index_from_ptr(const long long* const ptr) const noexcept
	{
#ifdef _DEBUG
		if (index_ptr == nullptr) throw std::runtime_error("");
#endif

		return index_ptr - ptr;
	};
	constexpr goal_distance get_offset_ptr(long long offset) const noexcept
	{
		if (offset == 0ll) return nullptr;
		return goal_distance{ index_ptr + offset };
	};
	constexpr void set_index_ptr(long long* new_ptr) noexcept
	{
		index_ptr = new_ptr;
	};
	constexpr void offset_ptr(long long ptr_offset) noexcept
	{
		if (is_offset_alive(ptr_offset) && index_ptr) index_ptr -= ptr_offset;
	};
	constexpr void update_goal_distance(long long new_distance) noexcept
	{
		*index_ptr = new_distance;
	};
	constexpr void subtract_goal_distance(long long offset) noexcept
	{
		*index_ptr -= offset;
	};
	constexpr void add_goal_distance(long long offset) noexcept
	{
		*index_ptr += offset;
	};
	constexpr void update_pointer_and_values(long long* begin) noexcept
	{
		if (is_offset_alive(1ll) && index_ptr != begin)
		{
			auto old_dist = *index_ptr;
			index_ptr = index_ptr - 1;
			*index_ptr = old_dist + *index_ptr;
		}
	};
	constexpr void update_pointer_and_values(long long* begin, long long offset) noexcept
	{
		if (is_offset_alive(offset) && index_ptr != begin)
		{
			auto old_dist = *index_ptr;
			auto prev_dist_value = *(index_ptr - 1ll);
			index_ptr = index_ptr - offset;
			*index_ptr = old_dist + prev_dist_value;
		}
	};
	constexpr void update_pointer_and_values(long long* begin, long long offset, goal_distance_dead_object_t) noexcept
	{
		if (is_offset_alive(offset) && index_ptr != begin)
		{
			auto old_dist = *index_ptr;
			*(unsigned long long*)index_ptr = 0x8fffffffffffffff;
			auto prev_dist_value = *(index_ptr - 1ll);
			index_ptr = index_ptr - offset;
			*index_ptr = old_dist + prev_dist_value;
		}
	};
	constexpr void update_pointer_and_values() noexcept
	{
		//auto old_dist = *index_ptr;
		if (is_offset_alive(-1ll)) index_ptr = index_ptr - 1;
		//*index_ptr = old_dist + *index_ptr;
	};
	constexpr void update_pointers_without_checks(long long offset) noexcept
	{
		index_ptr = index_ptr + offset;
	};
};