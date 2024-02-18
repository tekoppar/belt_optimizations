#pragma once

namespace belt_segment_helpers
{
	template<typename owner_ptr, typename group_ptr, typename group_data_ptr>
	static __forceinline constexpr void item_group_has_zero_count(owner_ptr* b_ptr, group_ptr* ptr, group_data_ptr* data_ptr) noexcept;
	template<typename owner_ptr, typename group_ptr>
	static __forceinline constexpr void item_group_has_reached_goal(owner_ptr* b_ptr, group_ptr* ptr) noexcept;
};

#include "belt_segment_shared.inl"