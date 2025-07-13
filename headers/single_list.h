#pragma once

#include <memory>
#ifndef _PPL_H
#include <ppl.h>
#endif

#include "const_data.h"
#include "vectors.h"
#include "item.h"
#include "item_32.h"
#include <type_traits>

namespace mem
{
#ifndef MEM_VECTOR_UTILITY
	enum class Allocating_Type
	{
		NEW,
		CONCURRENCY,
		MALLOC
	};
#endif
	struct test_struct
	{
		constexpr bool get_bool() noexcept
		{
			return true;
		};
	};

	template<typename type>
	class single_list_node
	{
	public:
		using value_type = type;

		single_list_node* prev{ nullptr };
		single_list_node* next{ nullptr };
		type object;
	};

	template<class type>
	class sl_iterator
	{
	public:
#ifdef __cpp_lib_concepts
		using iterator_concept = std::bidirectional_iterator_tag;
#endif // __cpp_lib_concepts
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = type::value_type;

		constexpr sl_iterator() = default;
		constexpr sl_iterator(type* ptr) noexcept :
			m_ptr(ptr)
		{};

		constexpr operator bool() const noexcept
		{
			return m_ptr != nullptr;
		};
		constexpr operator type* () const noexcept
		{
			return m_ptr;
		};

		[[nodiscard]] constexpr type* operator*() const noexcept
		{
			return m_ptr;
		};
		[[nodiscard]] constexpr type* operator*(sl_iterator iter) const noexcept
		{
			return iter.m_ptr;
		};
		[[nodiscard]] constexpr value_type* operator->() noexcept
		{
			return &m_ptr->object;
		};
		[[nodiscard]] constexpr const value_type* operator->() const noexcept
		{
			return &m_ptr->object;
		};

		constexpr sl_iterator& operator++() noexcept
		{
			m_ptr = m_ptr->prev;
			return *this;
		};
		constexpr sl_iterator operator++(int) noexcept
		{
			sl_iterator tmp = *this;
			++*this;
			return tmp;
		};
		constexpr sl_iterator& operator--() noexcept
		{
			m_ptr = m_ptr->next;
			return *this;
		};
		constexpr sl_iterator operator--(int) noexcept
		{
			sl_iterator tmp = *this;
			--*this;
			return tmp;
		};

		constexpr friend std::ptrdiff_t operator+(const sl_iterator& lhs, const sl_iterator& rhs)
		{
			return lhs.m_ptr + rhs.m_ptr;
		};
		constexpr friend std::ptrdiff_t operator-(const sl_iterator& lhs, const sl_iterator& rhs)
		{
			return lhs.m_ptr - rhs.m_ptr;
		};

		constexpr bool operator==(const std::nullptr_t)
		{
			return m_ptr == nullptr;
		};
		constexpr bool operator!=(const std::nullptr_t)
		{
			return m_ptr != nullptr;
		};
		constexpr friend bool operator==(const sl_iterator& lhs, const sl_iterator& rhs)
		{
			return lhs.m_ptr == rhs.m_ptr;
		};
		constexpr friend bool operator!=(const sl_iterator& lhs, const sl_iterator& rhs)
		{
			return lhs.m_ptr != rhs.m_ptr;
		};
		constexpr friend bool operator<(const sl_iterator& lhs, const sl_iterator& rhs)
		{
			return lhs.m_ptr < rhs.m_ptr;
		};
		constexpr friend bool operator>(const sl_iterator& lhs, const sl_iterator& rhs)
		{
			return rhs.m_ptr < lhs.m_ptr;
		};
		constexpr friend bool operator>=(const sl_iterator& lhs, const sl_iterator& rhs)
		{
			return lhs.m_ptr >= rhs.m_ptr;
		};
		constexpr friend bool operator<=(const sl_iterator& lhs, const sl_iterator& rhs)
		{
			return rhs.m_ptr >= lhs.m_ptr;
		};
		auto operator<=>(const sl_iterator&) const = default;

		constexpr auto GetValueType() const noexcept
		{
			return type{};
		};

	private:
		alignas(8) type* m_ptr{ nullptr };
	};

	static_assert(sl_iterator<single_list_node<item_32>>{} == nullptr, "not working");

	template <typename type, mem::Allocating_Type allocating_type = mem::Allocating_Type::NEW>
	class sl_allocator
	{
	public:
		using value_type = type;

		static constexpr std::size_t size_of_value_type{ sizeof(type) };
		static constexpr std::size_t bad_arr_length{ static_cast<std::size_t>(-1) / size_of_value_type };

		constexpr sl_allocator() = default;
		template<class Other>
		constexpr sl_allocator(const sl_allocator<Other>&) noexcept
		{};

		template <class Other>
		friend constexpr bool operator==(const sl_allocator<type, allocating_type>&, const sl_allocator<Other, allocating_type>&) noexcept
		{
			return true;
		};
		template<class Other>
		constexpr bool operator!=(const sl_allocator<Other>&) const noexcept
		{
			return false;
		};

		template <typename OtherType, mem::Allocating_Type alloc_type = allocating_type>
		struct rebind
		{
			typedef sl_allocator<OtherType, alloc_type> other;
		};
		template <typename OtherType, mem::Allocating_Type alloc_type = allocating_type>
		using rebind_alloc = sl_allocator<OtherType, alloc_type>;

		__declspec(allocator) [[nodiscard]] constexpr type* allocate() const
		{
			if (std::is_constant_evaluated()) return ::new type();
			else return static_cast<type*>(this->_allocate(size_of_value_type));
		};

	private:
		__declspec(allocator) [[nodiscard]] constexpr void* _allocate(const std::size_t count) const
		{
			if constexpr (mem::Allocating_Type::MALLOC == allocating_type)
			{
				auto const pv = malloc(count);
				return pv;
			}
			else if constexpr (mem::Allocating_Type::CONCURRENCY == allocating_type)
			{
				auto const ptr = concurrency::Alloc(count);
				if (!ptr) [[unlikely]] throw std::bad_alloc();

				return ptr;
			}
			else
			{
				auto ptr = ::new type();
				if (!ptr) [[unlikely]] throw std::bad_alloc();

				return ptr;
			}
		};

	public:
		constexpr void deallocate(type* const memory_block, std::size_t n = 0) const noexcept
		{
			if (std::is_constant_evaluated())
			{
				::delete memory_block;
				return;
			}

			if constexpr (mem::Allocating_Type::MALLOC == allocating_type)
			{
				free(memory_block);
			}
			else if constexpr (mem::Allocating_Type::CONCURRENCY == allocating_type)
			{
				concurrency::Free(memory_block);
			}
			else
			{
				if (n != 0)
				{
					::delete memory_block;
				}
				else
				{
					::delete memory_block;
				}
			}
		};
	};

	template<typename type, mem::Allocating_Type allocating_type = mem::Allocating_Type::NEW, class Allocator = mem::sl_allocator<type, allocating_type>>
	class single_list
	{
	private:
		using _Alty = typename std::allocator_traits<Allocator>::template rebind_alloc<single_list_node<type>>;
		using _Alty_traits = std::allocator_traits<_Alty>;

	public:
		mem::sl_allocator<single_list_node<type>, allocating_type> allocator{};

		single_list_node<type>* p_first{ nullptr };
		single_list_node<type>* p_last{ nullptr };

		using iterator = sl_iterator<single_list_node<type>>;
	public:
		inline constexpr single_list() = default;
		inline constexpr ~single_list() noexcept
		{
			if (p_first)
			{
				single_list_node<type>* tmp_ptr = p_first;
				single_list_node<type>* nxt_ptr{ nullptr };

				while (tmp_ptr)
				{
					nxt_ptr = tmp_ptr->prev;
					allocator.deallocate(tmp_ptr);
					tmp_ptr = nxt_ptr;
				}
			}
		};

		constexpr single_list(const single_list& o) noexcept
		{
			p_first = o.p_first;
			p_last = o.p_last;
		};
		constexpr single_list& operator=(const single_list& o) noexcept
		{
			p_first = o.p_first;
			p_last = o.p_last;

			return *this;
		};
		constexpr single_list(single_list&& o) noexcept :
			p_first{ std::exchange(o.p_first, nullptr) },
			p_last{ std::exchange(o.p_last, nullptr) }
		{};
		constexpr single_list& operator=(single_list&& o) noexcept
		{
			p_first = std::exchange(o.p_first, nullptr);
			p_last = std::exchange(o.p_last, nullptr);

			return *this;
		};

	private:
		template<typename type>
		__forceinline constexpr single_list_node<type>* get_from_index(type index) noexcept
		{
			type index_count = 0;
			single_list_node<type>* ptr = p_first;
			while (ptr->next)
			{
				if (index_count == index) return ptr;
				else
				{
					++index_count;
					ptr = ptr->next;
				}
			}

			return nullptr;
		};

	public:
		__forceinline constexpr bool empty() const noexcept
		{
			return p_first == nullptr;
		};
		__forceinline constexpr long long size() const noexcept
		{
			long long index_count = 0;
			single_list_node<type>* ptr = p_first;
			while (ptr)
			{
				++index_count;
				ptr = ptr->next;
			}

			return index_count;
		};

		[[nodiscard]] constexpr iterator operator[](long long index) noexcept
		{
			long long index_count = 0;
			single_list_node<type>* ptr = p_first;
			while (ptr)
			{
				if (index_count == index) return iterator(ptr);
				++index_count;
				ptr = ptr->next;
			}

			return nullptr;
		};
		[[nodiscard]] constexpr const iterator operator[](long long index) const noexcept
		{
			long long index_count = 0;
			single_list_node<type>* ptr = p_first;
			while (ptr)
			{
				if (index_count == index) return iterator(ptr);
				++index_count;
				ptr = ptr->next;
			}

			return nullptr;
		};

		constexpr iterator begin() const noexcept
		{
			return iterator(p_first);
		};
		constexpr iterator last() const noexcept
		{
			return iterator(p_last);
		};
		constexpr std::nullptr_t end() const noexcept
		{
			return nullptr;
		};

		inline constexpr void push_back(type object) noexcept
		{
			if (p_first == nullptr)
			{
				p_first = allocator.allocate();
				p_first->object = object;
				p_first->next = nullptr;
				p_first->prev = p_last;
				p_last = p_first;
			}
			else
			{
				single_list_node<type>* prev = p_last;
				p_last = allocator.allocate();
				if (prev != nullptr)
				{
					prev->prev = p_last;
					p_last->next = prev;
				}
				p_last->prev = nullptr;
				p_first->next = p_last;
				p_last->object = object;
			}
		};

		template <typename... Types>
		inline constexpr void emplace_back(Types&&... args) noexcept
			requires (std::is_move_constructible_v<type> == true && std::is_constructible_v<type, Types...> == true)
		{
			if (p_first == nullptr)
			{
				p_first = allocator.allocate();
				p_first->next = nullptr;
				p_first->prev = p_last;
				p_first->object = std::move(type{ std::forward<Types>(args)... });
				p_last = p_first;
			}
			else
			{
				single_list_node<type>* prev = p_last;
				p_last = allocator.allocate();
				if (prev != nullptr)
				{
					prev->prev = p_last;
					p_last->next = prev;
				}
				p_last->prev = nullptr;
				p_first->next = p_last;
				p_last->object = std::move(type{ std::forward<Types>(args)... });
			}
		};

		constexpr iterator erase(long long index) noexcept
		{
			auto it = this->operator[](index);
			single_list_node<type>* prev = nullptr;
			single_list_node<type>* next = nullptr;
			if ((*it)->next)
			{
				next = (*it)->next;
			}
			if ((*it)->prev)
			{
				prev = (*it)->prev;
			}

			if (next && prev)
			{
				prev->next = next;
				next->prev = prev;
			}

			if ((*it) == p_first)
			{
				p_first = prev;
				p_first->next = p_last;
			}

			if (it) allocator.deallocate(*it);
			return prev;
		};
		constexpr iterator erase(iterator& it) noexcept
		{
			single_list_node<type>* prev = nullptr;
			single_list_node<type>* next = nullptr;
			if ((*it)->next)
			{
				next = (*it)->next;
			}
			if ((*it)->prev)
			{
				prev = (*it)->prev;
			}

			if (next && prev)
			{
				prev->next = next;
				next->prev = prev;
			}

			if ((*it) == p_first)
			{
				p_first = prev;
				if (p_first) p_first->next = p_last;
			}

			if (it) allocator.deallocate(*it);
			return prev;
		};
	};
};

struct get_item_struct
{
	mem::single_list<item_32> list;
	__forceinline constexpr mem::sl_iterator<mem::single_list_node<item_32>> get_item(long long i) noexcept
	{
		if (list[i])
			return list[i];
		return { nullptr };
	};
	__forceinline constexpr const mem::sl_iterator<mem::single_list_node<item_32>> get_item(long long i) const noexcept
	{
		return list[i];
	};
};

constexpr auto test_get_item_method() noexcept
{
	get_item_struct arr;
	arr.list.emplace_back(4000ll);
	return 4ll;
};
constexpr auto test_get_item_method_value = test_get_item_method();
static_assert(test_get_item_method() == 4, "fuck msvc");