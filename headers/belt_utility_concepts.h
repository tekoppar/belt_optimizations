#pragma once

#include "vectors.h"

namespace belt_utility
{
	template<class T>
	concept element_has_member = requires(T t)
	{
		{
			t.x
		};
	};

	template<class T>
	concept class_has_iterator = requires(T t)
	{
		{
			t.begin()
		};
		{
			t.end()
		};
		{
			t.end() - 1
		};
	};

	template<typename type>
	concept type_has_get_position = requires(type x)
	{
		{
			x.get_position()
		};
		{
			vec2_int64{} == x.get_position()
		};
	};
	template<typename type>
	concept type_has_get_distance_position = requires(type x)
	{
		{
			x.get_distance_position_plus()
		};
		{
			std::declval<long long>() == x.get_distance_position_plus()
		};
	};
};