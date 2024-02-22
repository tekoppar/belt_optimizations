namespace belt_segment_helpers
{
	template<typename owner_ptr, typename group_ptr, typename group_data_ptr>
	static __forceinline constexpr void item_group_has_zero_count(owner_ptr* b_ptr, group_ptr* ptr, group_data_ptr* data_ptr) noexcept
	{
		b_ptr->item_group_has_zero_count(ptr, data_ptr);
	};
	template<typename owner_ptr, typename group_ptr, typename item_groups_data_type>
	static __forceinline constexpr void item_group_has_reached_goal(owner_ptr* b_ptr, group_ptr* ptr, item_groups_data_type& item_data) noexcept
	{
		b_ptr->item_group_has_reached_goal(ptr, item_data);
	};
	template<typename owner_ptr>
	static __forceinline constexpr long long get_end_distance_direction(owner_ptr* b_ptr) noexcept
	{
		return b_ptr->get_end_distance_direction();
	};
	template<typename owner_ptr>
	static __forceinline constexpr long long get_direction_y_value(owner_ptr* b_ptr) noexcept
	{
		return b_ptr->get_direction_y_value();
	};

	template<typename item_groups_data_type, typename owner_ptr>
	static __forceinline constexpr item_groups_data_type& get_item_data_group(owner_ptr* b_ptr, std::size_t i) noexcept
	{
		return b_ptr->get_item_data_group(i);
	};
};