#pragma once

#include <cstddef>
#include <memory>
#include <vector>
#include <type_traits>
#include <version>
#include <iterator>
#include <utility>

#include "mem_vector.h"
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

	inline constexpr friend std::ptrdiff_t operator+(const index_iterator& lhs, const index_iterator& rhs) noexcept
	{
		return lhs.index + rhs.index;
	};
	inline constexpr friend std::ptrdiff_t operator-(const index_iterator& lhs, const index_iterator& rhs) noexcept
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
	inline constexpr friend bool operator==(const index_iterator& lhs, const index_iterator& rhs) noexcept
	{
		return lhs.index == rhs.index;
	};
	inline constexpr friend bool operator!=(const index_iterator& lhs, const index_iterator& rhs) noexcept
	{
		return lhs.index != rhs.index;
	};
	inline constexpr friend bool operator<(const index_iterator& lhs, const index_iterator& rhs) noexcept
	{
		return lhs.index < rhs.index;
	};
	inline constexpr friend bool operator>(const index_iterator& lhs, const index_iterator& rhs) noexcept
	{
		return rhs.index < lhs.index;
	};
	inline constexpr friend bool operator>=(const index_iterator& lhs, const index_iterator& rhs) noexcept
	{
		return lhs.index >= rhs.index;
	};
	inline constexpr friend bool operator<=(const index_iterator& lhs, const index_iterator& rhs) noexcept
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
	inline constexpr std::size_t get_index() const noexcept
	{
		return index;
	};
	inline constexpr void set_index(std::size_t new_index) noexcept
	{
		index = new_index;
	};
	inline constexpr auto get_vector_begin() const noexcept
	{
		return vector->begin();
	};
	inline constexpr auto get_vector_last() const noexcept
	{
		return vector->last();
	};
	inline constexpr long long get_vector_size() const noexcept
	{
		return vector->size();
	};

private:
	alignas(8) std::size_t index{ 0ull };
	alignas(8) vector_type* vector { nullptr };
};

static_assert(index_iterator<int, std::vector<int>>{} == nullptr, "should be nullptr");
static_assert(index_iterator<int, mem::vector<int>>{} == nullptr, "should be nullptr");

template<class type, typename vector_type>
class double_index_iterator
{
public:
#ifdef __cpp_lib_concepts
	using iterator_concept = std::bidirectional_iterator_tag;
#endif // __cpp_lib_concepts
	using iterator_category = std::bidirectional_iterator_tag;
	using value_type = type;
	using reference = value_type&;
	using const_reference = const reference;

	constexpr double_index_iterator() = default;
	constexpr double_index_iterator(vector_type* _vector) noexcept :
		vector{ _vector }
	{};
	constexpr double_index_iterator(std::size_t _index, std::size_t _nested_index, vector_type* _vector) noexcept :
		index{ _index },
		nested_index{ _nested_index },
		vector{ _vector }
	{};
	constexpr double_index_iterator(std::size_t _index, double_index_iterator iter) noexcept :
		index{ _index },
		vector{ iter.vector }
	{};
	constexpr double_index_iterator(double_index_iterator iter, last_index_iterator_t) noexcept :
		index{ iter.vector->size() - 1ull },
		vector{ iter.vector }
	{};
	constexpr ~double_index_iterator() noexcept
	{};

	constexpr double_index_iterator(const double_index_iterator& iter) noexcept :
		index{ iter.index },
		nested_index{ iter.nested_index },
		vector{ iter.vector }
	{};
	constexpr double_index_iterator(double_index_iterator&& iter) noexcept :
		index{ std::exchange(iter.index, 0ull) },
		nested_index{ std::exchange(iter.nested_index, 0ull) },
		vector{ std::exchange(iter.vector, nullptr) }
	{};
	constexpr double_index_iterator& operator=(const double_index_iterator& iter) noexcept
	{
		index = iter.index;
		nested_index = iter.nested_index;
		vector = iter.vector;

		return *this;
	};
	constexpr double_index_iterator& operator=(double_index_iterator&& iter) noexcept
	{
		index = std::exchange(iter.index, 0ull);
		nested_index = std::exchange(iter.nested_index, 0ull);
		vector = std::exchange(iter.vector, nullptr);

		return *this;
	};

	constexpr operator bool() const noexcept
	{
		return index < vector->usize();
	};
	constexpr operator reference () noexcept
	{
#ifdef _DEBUG
		if (!vector) throw std::runtime_error("");
		if (vector->size() <= index) throw std::runtime_error("");
		if (vector->operator[](index).size() <= nested_index) throw std::runtime_error("");
#endif
		return vector->operator[](index).operator[](nested_index);
	};
	constexpr operator const_reference () const noexcept
	{
#ifdef _DEBUG
		if (!vector) throw std::runtime_error("");
		if (vector->size() <= index) throw std::runtime_error("");
		if (vector->operator[](index).size() <= nested_index) throw std::runtime_error("");
#endif
		return vector->operator[](index).operator[](nested_index);
	};

	[[nodiscard]] constexpr reference operator*() noexcept
	{
#ifdef _DEBUG
		if (!vector) throw std::runtime_error("");
		if (vector->size() <= index) throw std::runtime_error("");
		if (vector->operator[](index).size() <= nested_index) throw std::runtime_error("");
#endif
		return vector->operator[](index).operator[](nested_index);
	};
	[[nodiscard]] constexpr const_reference operator*() const noexcept
	{
#ifdef _DEBUG
		if (!vector) throw std::runtime_error("");
		if (vector->size() <= index) throw std::runtime_error("");
		if (vector->operator[](index).size() <= nested_index) throw std::runtime_error("");
#endif
		return vector->operator[](index).operator[](nested_index);
	};
	[[nodiscard]] constexpr reference operator*(double_index_iterator iter) const noexcept
	{
#ifdef _DEBUG
		if (!iter.vector) throw std::runtime_error("");
		if (iter.vector->size() <= iter.index) throw std::runtime_error("");
		if (iter.vector->operator[](iter.index).size() <= iter.nested_index) throw std::runtime_error("");
#endif
		return iter.vector->operator[](iter.index).operator[](iter.nested_index);
	};
	[[nodiscard]] constexpr reference operator->() noexcept
	{
#ifdef _DEBUG
		if (!vector) throw std::runtime_error("");
		if (vector->size() <= index) throw std::runtime_error("");
		if (vector->operator[](index).size() <= nested_index) throw std::runtime_error("");
#endif
		return vector->operator[](index).operator[](nested_index);
	};
	[[nodiscard]] constexpr const_reference operator->() const noexcept
	{
#ifdef _DEBUG
		if (!vector) throw std::runtime_error("");
		if (vector->size() <= index) throw std::runtime_error("");
		if (vector->operator[](index).size() <= nested_index) throw std::runtime_error("");
#endif
		return vector->operator[](index).operator[](nested_index);
	};
	//pointer operator&() const noexcept = delete;

	/*constexpr double_index_iterator& operator++() noexcept
	{
		++index;
		return *this;
	};
	constexpr double_index_iterator operator++(int) noexcept
	{
		double_index_iterator tmp = *this;
		++*this;
		return tmp;
	};
	constexpr double_index_iterator operator+(const double_index_iterator& rhs) const
	{
		return { index + rhs.index, vector };
	};
	template<typename integer>
	constexpr double_index_iterator operator+(integer rhs) const
		requires(std::is_integral_v<integer>)
	{
		return { index + rhs, vector };
	};
	constexpr double_index_iterator& operator--() noexcept
	{
		--index;
		return *this;
	};
	constexpr double_index_iterator operator--(int) noexcept
	{
		double_index_iterator tmp = *this;
		--*this;
		return tmp;
	};
	constexpr double_index_iterator operator-(const double_index_iterator& rhs) const
	{
		return { index - rhs.index, vector };
	};
	template<typename integer>
	constexpr double_index_iterator operator-(integer rhs) const
		requires(std::is_integral_v<integer>)
	{
		return { index - rhs, vector };
	};*/
	inline constexpr void increment() noexcept
	{
		++index;
	};
	inline constexpr void nested_increment() noexcept
	{
		++nested_index;
	};
	inline constexpr double_index_iterator add(const double_index_iterator& rhs) const noexcept
	{
		return { index + rhs.index, nested_index, vector };
	};
	inline constexpr double_index_iterator nested_add(const double_index_iterator& rhs) const noexcept
	{
		return { index, nested_index + rhs.nested_index, vector };
	};

	inline constexpr void decrement() noexcept
	{
		--index;
	};
	inline constexpr void nested_decrement() noexcept
	{
		--nested_index;
	};
	inline constexpr double_index_iterator sub(const double_index_iterator& rhs) const noexcept
	{
		return { index - rhs.index, nested_index, vector };
	};
	inline constexpr double_index_iterator nested_sub(const double_index_iterator& rhs) const noexcept
	{
		return { index, nested_index - rhs.nested_index, vector };
	};

	/*inline constexpr friend std::ptrdiff_t operator+(const double_index_iterator& lhs, const double_index_iterator& rhs) noexcept
	{
		return lhs.index + rhs.index;
	};
	inline constexpr friend std::ptrdiff_t operator-(const double_index_iterator& lhs, const double_index_iterator& rhs) noexcept
	{
		return lhs.index - rhs.index;
	};*/

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
	inline constexpr friend bool operator==(const double_index_iterator& lhs, const double_index_iterator& rhs) noexcept
	{
		return lhs.index == rhs.index && lhs.nested_index == rhs.nested_index;
	};
	inline constexpr friend bool operator!=(const double_index_iterator& lhs, const double_index_iterator& rhs) noexcept
	{
		return lhs.index != rhs.index && lhs.nested_index != rhs.nested_index;
	};
	inline constexpr friend bool operator<(const double_index_iterator& lhs, const double_index_iterator& rhs) noexcept
	{
		return lhs.index < rhs.index && lhs.nested_index < rhs.nested_index;
	};
	inline constexpr friend bool operator>(const double_index_iterator& lhs, const double_index_iterator& rhs) noexcept
	{
		return rhs.index < lhs.index && rhs.nested_index < lhs.nested_index;
	};
	inline constexpr friend bool operator>=(const double_index_iterator& lhs, const double_index_iterator& rhs) noexcept
	{
		return lhs.index >= rhs.index && lhs.nested_index >= rhs.nested_index;
	};
	inline constexpr friend bool operator<=(const double_index_iterator& lhs, const double_index_iterator& rhs) noexcept
	{
		return rhs.index >= lhs.index && rhs.nested_index >= lhs.nested_index;
	};
	auto operator<=>(const double_index_iterator&) const = default;

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
	inline constexpr std::size_t get_index() const noexcept
	{
		return index;
	};
	inline constexpr void set_index(std::size_t new_index) noexcept
	{
		index = new_index;
	};
	inline constexpr auto get_vector_begin() const noexcept
	{
		return vector->begin();
	};
	inline constexpr auto get_vector_last() const noexcept
	{
		return vector->last();
	};
	inline constexpr long long get_vector_size() const noexcept
	{
		return vector->size();
	};

	//nested variants
	inline constexpr bool is_nested_next_valid() const noexcept
	{
		return nested_index + 1 < vector->operator[](index).usize();
	};
	inline constexpr bool is_nested_prev_valid() const noexcept
	{
		return nested_index - 1 < vector->operator[](index).usize();
	};
	inline constexpr bool nested_vector_empty() const noexcept
	{
		return  vector->operator[](index).empty();
	};
	inline constexpr std::size_t get_nested_index() const noexcept
	{
		return nested_index;
	};
	inline constexpr void set_nested_index(std::size_t new_index) noexcept
	{
		nested_index = new_index;
	};
	inline constexpr auto get_nested_vector_begin() const noexcept
	{
		return vector->operator[](index).begin();
	};
	inline constexpr auto get_nested_vector_last() const noexcept
	{
		return  vector->operator[](index).last();
	};
	inline constexpr long long get_nested_vector_size() const noexcept
	{
		return vector->operator[](index).size();
	};

private:
	alignas(8) std::size_t index{ 0ull };
	alignas(8) std::size_t nested_index{ 0ull };
	alignas(8) vector_type* vector { nullptr };
};