#pragma once

namespace belt_segment_helpers
{
	template<typename owner_ptr, typename group_ptr>
	static inline constexpr void item_group_has_zero_count(owner_ptr* b_ptr, group_ptr* ptr) noexcept;
	template<typename owner_ptr, typename group_ptr>
	static inline constexpr void item_group_has_reached_goal(owner_ptr* b_ptr, group_ptr* ptr) noexcept;
};

#include "belt_segment_shared.inl"