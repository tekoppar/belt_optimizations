#pragma once

#include <cstddef>
#include <utility>

#include "vectors.h"

enum class item_type : short
{
	pink_square = 0,
	wood,
	stone,
	copper,
	iron,
	brick,
	log
};

class belt_item
{
public:
	item_type type{ item_type::pink_square };
};

class item
{
	constexpr static short item_length = 32;

public:
	__declspec(align(8)) item_type type{ item_type::pink_square };
	__declspec(align(8)) vec2 position{ 0.0f, 0.0f };

	constexpr operator belt_item() noexcept
	{
		return belt_item{ type };
	};
};

template<std::size_t N>
class item_array
{
public:
	constexpr item operator[](std::size_t i) noexcept
	{
		return v[i];
	};
	constexpr const item& operator[](std::size_t i) const noexcept
	{
		return v[i];
	};

	constexpr std::size_t size() const noexcept
	{
		return N;
	}

	item v[N];
};

template<class... all>
item_array(all...) -> item_array<sizeof...(all)>;

class item_uint
{
	constexpr static short item_length = 32;

public:
	__declspec(align(8)) item_type type{ item_type::pink_square };
	__declspec(align(8)) vec2_int64 position{ 0, 0 };

	constexpr item_uint() noexcept
	{};
	constexpr item_uint(item_type _type) noexcept :
		type{ _type }
	{};
	constexpr item_uint(item_type _type, vec2_int64 pos) noexcept :
		type{ _type },
		position{ pos }
	{};
	constexpr ~item_uint() noexcept
	{};

	constexpr item_uint(const item_uint& o) noexcept :
		type{ o.type },
		position{ o.position }
	{};
	constexpr item_uint(item_uint&& o) noexcept :
		type{ std::exchange(o.type, item_type::pink_square) },
		position{ std::exchange(o.position, vec2_int64{}) }
	{};
	constexpr item_uint& operator=(const item_uint& o) noexcept
	{
		this->type = o.type;
		this->position = o.position;

		return *this;
	};
	constexpr item_uint& operator=(item_uint&& o) noexcept
	{
		this->type = std::exchange(o.type, item_type::pink_square);
		this->position = std::exchange(o.position, vec2_int64{});

		return *this;
	};

	friend constexpr bool operator==(const item_uint& lhs, const item_uint& rhs) noexcept
	{
		return lhs.type == rhs.type && lhs.position == rhs.position;
	};

	constexpr operator belt_item() noexcept
	{
		return belt_item{ type };
	};
	constexpr operator belt_item() const noexcept
	{
		return belt_item{ type };
	};
};

template<std::size_t N>
class item_array_uint
{
public:
	constexpr item_uint operator[](std::size_t i) noexcept
	{
		return v[i];
	};
	constexpr const item_uint& operator[](std::size_t i) const noexcept
	{
		return v[i];
	};

	constexpr std::size_t size() const noexcept
	{
		return N;
	}

	item_uint v[N];
};

template<class... all>
item_array_uint(all...) -> item_array_uint<sizeof...(all)>;