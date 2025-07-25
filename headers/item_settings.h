#pragma once

#include <limits>

struct item_settings
{
	constexpr static const short single_belt_length = 128;
	constexpr static const short belt_length = 128 * 8;
	constexpr static const inline long long max_item_count = 16;
	constexpr static const long long belt_item_size = 32;
	constexpr static const int max_distance_between_items = (std::numeric_limits<short>::max)() - belt_item_size;

	static constexpr auto max(auto lhs, auto rhs) noexcept
	{
		return lhs < rhs ? rhs : lhs;
	};

	enum class item_removal_result
	{
		item_not_removed,
		item_removed,
		item_removed_zero_remains
	};

	struct index_item_position_return
	{
		long long found_index{ -1ll };
		long long item_distance_position{ -1ll };
		//long long event_trigger_index{ -1 };

		friend inline constexpr bool operator==(const index_item_position_return& lhs, const index_item_position_return& rhs)
		{
			return lhs.found_index == rhs.found_index && lhs.item_distance_position == rhs.item_distance_position;//&& lhs.event_trigger_index == rhs.event_trigger_index;
		};
		friend inline constexpr bool operator!=(const index_item_position_return& lhs, const index_item_position_return& rhs)
		{
			return !(lhs == rhs);
		};
	};
};