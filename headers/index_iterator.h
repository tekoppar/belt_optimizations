#pragma once

#include <cstddef>
#include <memory>
#include <vector>
#include "mem_vector.h"
#include <type_traits>
#include <version>
#include <iterator>
#include <utility>

#include "const_data.h"

struct last_index_iterator_t
{
	explicit last_index_iterator_t() = default;
};
inline constexpr last_index_iterator_t last_index_iterator{};

template<class type, typename vector_type>
class index_iterator
{
public:
#ifdef __cpp_lib_concepts
	using iterator_concept = std::bidirectional_iterator_tag;
#endif // __cpp_lib_concepts
	using iterator_category = std::bidirectional_iterator_tag;
	using value_type = type;
	using reference = value_type&;
	using const_reference = const reference;

	constexpr index_iterator() = default;
	constexpr index_iterator(vector_type* _vector) noexcept :
		vector{ _vector }
	{};
	constexpr index_iterator(std::size_t _index, vector_type* _vector) noexcept :
		index{ _index },
		vector{ _vector }
	{};
	constexpr index_iterator(std::size_t _index, index_iterator iter) noexcept :
		index{ _index },
		vector{ iter.vector }
	{};
	constexpr index_iterator(index_iterator iter, last_index_iterator_t) noexcept :
		index{ iter.vector->size() - 1ull },
		vector{ iter.vector }
	{};
	constexpr ~index_iterator() noexcept
	{};

	constexpr index_iterator(const index_iterator& iter) noexcept :
		index{ iter.index },
		vector{ iter.vector }
	{};
	constexpr index_iterator(index_iterator&& iter) noexcept :
		index{ std::exchange(iter.index, 0ull) },
		vector{ std::exchange(iter.vector, nullptr) }
	{};
	constexpr index_iterator& operator=(const index_iterator& iter) noexcept
	{
		index = iter.index;
		vector = iter.vector;

		return *this;
	};
	constexpr index_iterator& operator=(index_iterator&& iter) noexcept
	{
		index = std::exchange(iter.index, 0ull);
		vector = std::exchange(iter.vector, nullptr);

		return *this;
	};

	constexpr operator bool() const noexcept
	{
		return index < vector->usize();
	};
	constexpr operator reference () noexcept
	{
		return vector->operator[](index);
	};
	constexpr operator const_reference () const noexcept
	{
		return vector->operator[](index);
	};

	[[nodiscard]] constexpr reference operator*() noexcept
	{
		return vector->operator[](index);
	};
	[[nodiscard]] constexpr const_reference operator*() const noexcept
	{
		return vector->operator[](index);
	};
	[[nodiscard]] constexpr reference operator*(index_iterator iter) const noexcept
	{
		return iter.vector->operator[](iter.index);
	};
	[[nodiscard]] constexpr reference operator->() noexcept
	{
		return vector->operator[](index);
	};
	[[nodiscard]] constexpr const_reference operator->() const noexcept
	{
		return vector->operator[](index);
	};
	//pointer operator&() const noexcept = delete;

	constexpr index_iterator& operator++() noexcept
	{
		++index;
		return *this;
	};
	constexpr index_iterator operator++(int) noexcept
	{
		index_iterator tmp = *this;
		++*this;
		return tmp;
	};
	constexpr index_iterator operator+(const index_iterator& rhs) const
	{
		return { index + rhs.index, vector };
	};
	template<typename integer>
	constexpr index_iterator operator+(integer rhs) const
		requires(std::is_integral_v<integer>)
	{
		return { index + rhs, vector };
	};
	constexpr index_iterator& operator--() noexcept
	{
		--index;
		return *this;
	};
	constexpr index_iterator operator--(int) noexcept
	{
		index_iterator tmp = *this;
		--*this;
		return tmp;
	};
	constexpr index_iterator operator-(const index_iterator& rhs) const
	{
		return { index - rhs.index, vector };
	};
	template<typename integer>
	constexpr index_iterator operator-(integer rhs) const
		requires(std::is_integral_v<integer>)
	{
		return { index - rhs, vector };
	};

	constexpr friend std::ptrdiff_t operator+(const index_iterator& lhs, const index_iterator& rhs) noexcept
	{
		return lhs.index + rhs.index;
	};
	constexpr friend std::ptrdiff_t operator-(const index_iterator& lhs, const index_iterator& rhs) noexcept
	{
		return lhs.index - rhs.index;
	};

	constexpr bool operator==(const std::nullptr_t) noexcept
	{
		return !vector;
	};
	constexpr bool operator!=(const std::nullptr_t) noexcept
	{
		return vector;
	};
	constexpr bool operator==(const std::nullptr_t) const noexcept
	{
		return !vector;
	};
	constexpr bool operator!=(const std::nullptr_t) const noexcept
	{
		return !!vector;
	};
	constexpr friend bool operator==(const index_iterator& lhs, const index_iterator& rhs) noexcept
	{
		return lhs.index == rhs.index;
	};
	constexpr friend bool operator!=(const index_iterator& lhs, const index_iterator& rhs) noexcept
	{
		return lhs.index != rhs.index;
	};
	constexpr friend bool operator<(const index_iterator& lhs, const index_iterator& rhs) noexcept
	{
		return lhs.index < rhs.index;
	};
	constexpr friend bool operator>(const index_iterator& lhs, const index_iterator& rhs) noexcept
	{
		return rhs.index < lhs.index;
	};
	constexpr friend bool operator>=(const index_iterator& lhs, const index_iterator& rhs) noexcept
	{
		return lhs.index >= rhs.index;
	};
	constexpr friend bool operator<=(const index_iterator& lhs, const index_iterator& rhs) noexcept
	{
		return rhs.index >= lhs.index;
	};
	auto operator<=>(const index_iterator&) const = default;

	constexpr auto GetValueType() const noexcept
	{
		return type{};
	};

	inline constexpr bool is_next_valid() const noexcept
	{
		return index + 1 < vector->usize();
	};
	inline constexpr bool is_prev_valid() const noexcept
	{
		return index - 1 < vector->usize();
	};
	inline constexpr bool vector_empty() const noexcept
	{
		return vector->empty();
	};

private:
	alignas(8) std::size_t index{ 0ull };
	alignas(8) vector_type* vector{ nullptr };
};

static_assert(index_iterator<int, std::vector<int>>{} == nullptr, "should be nullptr");
static_assert(index_iterator<int, mem::vector<int>>{} == nullptr, "should be nullptr");