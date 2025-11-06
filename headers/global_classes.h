#pragma once

struct alignas(16) item_groups_distance
{
	long long distance_between{ 0ll };
	long long distance_to_end{ 0ll };

	constexpr friend bool operator==(const item_groups_distance& lhs, const item_groups_distance& rhs) noexcept
	{
		return lhs.distance_between == rhs.distance_between && lhs.distance_to_end == rhs.distance_to_end;
	};
	constexpr friend bool operator!=(const item_groups_distance& lhs, const item_groups_distance& rhs) noexcept
	{
		return !(lhs.distance_between == rhs.distance_between && lhs.distance_to_end == rhs.distance_to_end);
	};
};