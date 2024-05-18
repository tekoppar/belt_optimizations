#pragma once

struct vec2
{
	float x;
	float y;

	constexpr vec2 operator+(const vec2& rhs) const noexcept
	{
		return vec2{ x + rhs.x, y + rhs.y };
	};
	constexpr vec2& operator+=(const vec2& rhs) noexcept
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	};
	constexpr vec2 operator-(const vec2& rhs) const noexcept
	{
		return vec2{ x - rhs.x, y - rhs.y };
	};
	constexpr vec2& operator-=(const vec2& rhs) noexcept
	{
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	};
	constexpr vec2 operator*(const vec2& rhs) const noexcept
	{
		return vec2{ x * rhs.x, y * rhs.y };
	};
	constexpr vec2& operator*=(const vec2& rhs) noexcept
	{
		x *= rhs.x;
		y *= rhs.y;
		return *this;
	};
};

struct vec2_uint
{
	long long x;
	long long y;

	constexpr vec2_uint operator+(const vec2_uint& rhs) const noexcept
	{
		return vec2_uint{ x + rhs.x, y + rhs.y };
	};
	constexpr vec2_uint& operator+=(const vec2_uint& rhs) noexcept
	{
		x += rhs.x;
		y += rhs.y;
		return *this;
	};
	constexpr vec2_uint operator-(const vec2_uint& rhs) const noexcept
	{
		return vec2_uint{ x - rhs.x, y - rhs.y };
	};
	constexpr vec2_uint& operator-=(const vec2_uint& rhs) noexcept
	{
		x -= rhs.x;
		y -= rhs.y;
		return *this;
	};
	constexpr vec2_uint operator*(const vec2_uint& rhs) const noexcept
	{
		return vec2_uint{ x * rhs.x, y * rhs.y };
	};
	constexpr vec2_uint& operator*=(const vec2_uint& rhs) noexcept
	{
		x *= rhs.x;
		y *= rhs.y;
		return *this;
	};

	constexpr bool operator==(const vec2_uint& rhs) const noexcept
	{
		return x == rhs.x && y == rhs.y;
	};
	constexpr bool operator!=(const vec2_uint& rhs) const noexcept
	{
		return !(*this == rhs);
	};
	constexpr bool operator>(const vec2_uint& rhs) const noexcept
	{
		return x > rhs.x && y > rhs.y;
	};
	constexpr bool operator<(const vec2_uint& rhs) const noexcept
	{
		return rhs > *this;
	};
	constexpr bool operator>=(const vec2_uint& rhs) const noexcept
	{
		return x >= rhs.x && y >= rhs.y;
	};
	constexpr bool operator<=(const vec2_uint& rhs) const noexcept
	{
		return rhs >= *this;
	};
};