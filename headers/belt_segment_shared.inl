namespace belt_segment_helpers
{
	template<typename owner_ptr, typename group_ptr, typename group_data_ptr>
	static inline constexpr void item_group_has_zero_count(owner_ptr* b_ptr, group_ptr* ptr, group_data_ptr* data_ptr) noexcept
	{
		b_ptr->item_group_has_zero_count(ptr, data_ptr);
	};
	template<typename owner_ptr, typename group_ptr>
	static inline constexpr void item_group_has_reached_goal(owner_ptr* b_ptr, group_ptr* ptr) noexcept
	{
		b_ptr->item_group_has_reached_goal(ptr);
	};
};