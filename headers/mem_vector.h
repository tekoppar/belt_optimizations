#pragma once

#ifndef _PPL_H
#include <ppl.h>
#endif

#include <version>
#include <array>
#include <iterator>
#include <memory>
#include <new>
#include <tuple>
#include <type_traits>
#include <utility>
#include <malloc.h>
#include <concrt.h>
#include <vector>
#include <corecrt_terminate.h>
#include <cstring>
#ifdef _DEBUG
#include <exception>
#endif

#include "macros.h"
#include "math_utility.h"
#include "mem_utilities.h"
#include "mem_vector_concepts.h"
#include "type_conversion.h"
#include "simd_memcpy.h"
#include <exception>

#ifdef _DEBUG
#define _DEAD_DEBUG_CHECK
#endif
#include <unordered_map>
#include <execution>

namespace mem
{
	template<typename T>
	constexpr static void destroy_at(T* ptr) noexcept
	{
		ptr->~T();
	};
	namespace tuple
	{
		template<typename TupR, typename Tup = std::remove_reference_t<TupR>, auto N = std::tuple_size_v<Tup>>
		constexpr auto ReverseTuple(TupR&& t)
		{
			return[&t]<auto... I>(std::index_sequence<I...>)
			{
				constexpr std::array is{ (N - 1 - I)... };
				return std::tuple<std::tuple_element_t<is[I], Tup>...>{
					std::get<is[I]>(std::forward<TupR>(t))...
				};
			}(std::make_index_sequence<N>{});
		};

		template<typename TupR, typename Tup = std::remove_reference_t<TupR>, auto N = std::tuple_size_v<Tup>>
		constexpr auto GetTupleValue(TupR&& t)
		{
			return[&t]<auto... I>(std::index_sequence<I...>)
			{
				constexpr std::array is{ (N - 1 - I)... };
				return std::tuple<std::tuple_element_t<is[I], Tup>...>{std::get<is[I]>(std::forward<TupR>(t))...};
			}(std::make_index_sequence<N>{});
		};

		template<typename Object, typename... Types>
		constexpr auto GetTuple_Val(Types&&... args)
		{
			const std::tuple<Types...> temp{ std::forward<Types>(args)... };
			const auto tuple_reversed = mem::tuple::GetTupleValue(temp);
			Object temp_object{};
			std::memcpy(&temp_object, &tuple_reversed, sizeof(tuple_reversed));
			return temp_object;
		};

		constexpr auto org_tuple = std::tuple<int, long>{ 0, 15 };
		constexpr auto get_tuple_val_test = ReverseTuple(std::tuple<int, long>{0, 15});
		constexpr auto val_test = std::get<0>(get_tuple_val_test);

		template<std::size_t index, typename TupR, typename Tup = std::remove_reference_t<decltype(TupR()[0])>>
		constexpr auto GetIndexValue(TupR&& t)
		{
			return t[index];
		};

		template<typename type, std::size_t N>
		struct test_arr
		{
			type arr[N];

			constexpr type operator[](auto i) const noexcept
			{
				return arr[i];
			};
		};

		template<typename type, std::size_t N, template<typename, std::size_t> typename cont_type>
		constexpr auto get_values(const cont_type<type, N>& t)
		{
			return[&t]<auto... I>(std::index_sequence<I...>)
			{
				constexpr std::array is{ (N - 1 - I)... };
				return cont_type<type, N>{std::forward<type>(t[I])...};
			}(std::make_index_sequence<N>{});
		};

		constexpr std::array<int, 2> arr_test{ 0, 15 };
		constexpr test_arr<int, 2> arr_test2{ 0, 15 };
		constexpr auto test = get_values(arr_test2);
	};

	enum class mem_tuple_layout
	{
		normal,
		reverse
	};

	enum class Allocating_Type
	{
		NEW,
		CONCURRENCY,
		MALLOC,
		ALIGNED_NEW,
		ALIGNED_MALLOC,
		ALIGNED_NEW_32
	};

	enum class use_memcpy
	{
		check_based_on_type,
		force_checks_off
	};

	/*template<typename Object>
	constexpr bool get_is_trivially_copyable_v() noexcept
	{
		if constexpr (mem_vector_has_method<Object>)
		{
			return get_is_trivially_copyable_v<Object::vector_value_type>();
		}
		else
			return std::is_trivially_copyable_v<Object>;
	};*/

	template<class Object>
	class iterator
	{
	public:
#ifdef __cpp_lib_concepts
		using iterator_concept = std::random_access_iterator_tag;
#endif // __cpp_lib_concepts
		using iterator_category = std::random_access_iterator_tag;
		using value_type = typename Object::value_type;
		using difference_type = typename Object::difference_type;
		using pointer = typename Object::pointer;
		using reference = typename Object::reference;
		using const_reference = typename Object::const_reference;

		constexpr iterator() = default;
		explicit constexpr iterator(pointer ptr) noexcept :
			m_ptr(ptr)
		{};

		[[nodiscard]] constexpr reference operator*() noexcept
		{
			return *m_ptr;
		};
		[[nodiscard]] constexpr const reference operator*() const noexcept
		{
			return const_cast<reference>(*m_ptr);
		};
		[[nodiscard]] constexpr reference operator*(iterator iter) const noexcept
		{
			return *iter.m_ptr;
		};
		[[nodiscard]] constexpr pointer operator->() const noexcept
		{
			return m_ptr;
		};
		//pointer operator&() const noexcept = delete;

		[[nodiscard]] constexpr reference operator[](const std::size_t& index) const noexcept
		{
			return *(m_ptr + index);
		};
		[[nodiscard]] constexpr reference operator[](const difference_type& index) const noexcept
		{
			return *(m_ptr + index);
		};

		constexpr iterator& operator++() noexcept
		{
			++m_ptr;
			return *this;
		};
		constexpr iterator operator++(int) noexcept
		{
			iterator tmp = *this;
			++*this;
			return tmp;
		};
		constexpr iterator& operator+=(const difference_type& offset) noexcept
		{
			m_ptr += offset;
			return *this;
		};
		constexpr iterator operator+(const difference_type& offset) noexcept
		{
			iterator tmp = *this;
			tmp.m_ptr += offset;
			return tmp;
		};
		inline constexpr friend iterator operator+(const iterator& lhs, const difference_type& offset) noexcept
		{
			return iterator(lhs.m_ptr + offset);
		};
		inline constexpr friend iterator operator+(const difference_type& offset, const iterator& lhs) noexcept
		{
			return lhs + offset;
		};

		constexpr iterator& operator--() noexcept
		{
			--m_ptr;
			return *this;
		};
		constexpr iterator operator--(int) noexcept
		{
			iterator tmp = *this;
			--*this;
			return tmp;
		};
		constexpr iterator& operator-=(const difference_type& offset) noexcept
		{
			m_ptr -= offset;
			return *this;
		};
		constexpr iterator operator-(const difference_type& offset) noexcept
		{
			iterator tmp = *this;
			tmp.m_ptr -= offset;
			return tmp;
		};
		template<typename T = Object>
		inline constexpr friend iterator operator-(const iterator<T>& lhs, const difference_type& offset) noexcept
		{
			iterator tmp{ lhs.m_ptr };
			tmp.m_ptr -= offset;
			return tmp;
		};
		template<typename T = Object>
		inline constexpr friend iterator operator-(const difference_type& offset, const iterator<T>& lhs) noexcept
		{
			return iterator{ lhs - offset };
		};

		inline constexpr friend difference_type operator+(const iterator& lhs, const iterator& rhs)
		{
			return lhs.m_ptr + rhs.m_ptr;
		};
		inline constexpr friend difference_type operator-(const iterator& lhs, const iterator& rhs)
		{
			return lhs.m_ptr - rhs.m_ptr;
		};
		inline constexpr friend difference_type operator+(const iterator& lhs, const pointer& rhs)
		{
			return lhs.m_ptr + rhs;
		};
		inline constexpr friend difference_type operator-(const iterator& lhs, const pointer& rhs)
		{
			return lhs.m_ptr - rhs;
		};
		inline constexpr friend difference_type operator+(const pointer& lhs, const iterator& rhs)
		{
			return lhs + rhs.m_ptr;
		};
		inline constexpr friend difference_type operator-(const pointer& lhs, const iterator& rhs)
		{
			return lhs - rhs.m_ptr;
		};

		inline constexpr bool operator<(const iterator& rhs)
		{
			return m_ptr < rhs.m_ptr;
		};
		inline constexpr bool operator>(const iterator& rhs)
		{
			return rhs.m_ptr < m_ptr;
		};
		inline constexpr bool operator<=(const iterator& rhs)
		{
			return m_ptr <= rhs.m_ptr;
		};
		inline constexpr bool operator>=(const iterator& rhs)
		{
			return m_ptr >= rhs.m_ptr;
		};

		inline constexpr friend bool operator==(const iterator& lhs, const iterator& rhs)
		{
			return lhs.m_ptr == rhs.m_ptr;
		};
		inline constexpr friend bool operator!=(const iterator& lhs, const iterator& rhs)
		{
			return lhs.m_ptr != rhs.m_ptr;
		};
		inline constexpr friend bool operator==(const iterator& lhs, const pointer& rhs)
		{
			return lhs.m_ptr == rhs;
		};
		inline constexpr friend bool operator!=(const iterator& lhs, const pointer& rhs)
		{
			return lhs.m_ptr != rhs;
		};
		inline constexpr friend bool operator<(const iterator& lhs, const iterator& rhs)
		{
			return lhs.m_ptr < rhs.m_ptr;
		};
		inline constexpr friend bool operator>(const iterator& lhs, const iterator& rhs)
		{
			return rhs.m_ptr < lhs.m_ptr;
		};
		inline constexpr friend bool operator>=(const iterator& lhs, const iterator& rhs)
		{
			static_assert(std::is_same_v<decltype(lhs), decltype(rhs)>, "not same type");
			return std::addressof(lhs.m_ptr) >= std::addressof(rhs.m_ptr);
		};
		inline constexpr friend bool operator<=(const iterator& lhs, const iterator& rhs)
		{
			return lhs.m_ptr <= rhs.m_ptr;
		};
		auto operator<=>(const iterator&) const = default;

		constexpr auto GetValueType() const noexcept
		{
			return value_type{};
		};
		constexpr value_type GetValue() noexcept
		{
			return *m_ptr;
		};
		constexpr value_type GetValue() const noexcept
		{
			return *m_ptr;
		};
		constexpr reference GetRef() noexcept
		{
			return *m_ptr;
		};
		constexpr const reference GetConstRef() const noexcept
		{
			return const_cast<reference>(*m_ptr);
		};

	private:
		alignas(8) pointer m_ptr{ nullptr };
	};

	template <class Object, mem::Allocating_Type allocating_type = mem::Allocating_Type::NEW>
	class allocator
	{
	public:
		using value_type = Object;
		using pointer = Object*;
		using const_pointer = const Object*;

		static constexpr std::size_t size_of_value_type{ sizeof(value_type) };
		static constexpr std::size_t bad_arr_length{ static_cast<std::size_t>(-1) / size_of_value_type };

		constexpr allocator() = default;
		template<class Other>
		constexpr allocator(const allocator<Other>&) noexcept
		{};

		template <class Other>
		friend constexpr bool operator==(const allocator<Object, allocating_type>&, const allocator<Other, allocating_type>&) noexcept
		{
			return true;
		};
		template<class Other>
		constexpr bool operator!=(const allocator<Other>&) const noexcept
		{
			return false;
		};

		template <typename OtherType, mem::Allocating_Type type = allocating_type>
		struct rebind
		{
			typedef allocator<OtherType, type> other;
		};
		template <typename OtherType, mem::Allocating_Type type = allocating_type>
		using rebind_alloc = allocator<OtherType, type>;

	private:
		[[nodiscard]] __declspec(allocator) constexpr void* _allocate(const std::size_t count) const
		{
			if constexpr (mem::Allocating_Type::MALLOC == allocating_type)
			{
				auto const pv = malloc(count);
#ifdef ENABLE_CPP_EXCEPTION_THROW
				if (!pv) throw std::bad_alloc();
#else
				if (!pv) [[unlikely]] std::terminate();
#endif
				return pv;
			}
			else if constexpr (mem::Allocating_Type::CONCURRENCY == allocating_type)
			{
				auto const ptr = concurrency::Alloc(count);
#ifdef ENABLE_CPP_EXCEPTION_THROW
				if (!ptr) [[unlikely]] throw std::bad_alloc();
#else
				if (!ptr) [[unlikely]] std::terminate();
#endif
				return ptr;
			}
			else if constexpr (mem::Allocating_Type::NEW == allocating_type)
			{
				auto const ptr = ::operator new[](count, std::nothrow);
#ifdef ENABLE_CPP_EXCEPTION_THROW
				if (!ptr) [[unlikely]] throw std::bad_alloc();
#else
				if (!ptr) [[unlikely]] std::terminate();
#endif
				return ptr;
			}
			else if constexpr (mem::Allocating_Type::ALIGNED_MALLOC == allocating_type)
			{
				constexpr auto closest_alignment = mem::get_closest_power_of_2_alignment<value_type, 32>();
				auto const pv = _aligned_malloc(count, closest_alignment);
#ifdef ENABLE_CPP_EXCEPTION_THROW
				if (!pv) throw std::bad_alloc();
#else
				if (!pv) [[unlikely]] std::terminate();
#endif
				return pv;
			}
			else if constexpr (mem::Allocating_Type::CONCURRENCY == allocating_type)
			{
				auto const ptr = concurrency::Alloc(count);
#ifdef ENABLE_CPP_EXCEPTION_THROW
				if (!ptr) [[unlikely]] throw std::bad_alloc();
#else
				if (!ptr) [[unlikely]] std::terminate();
#endif
				return ptr;
			}
			else if constexpr (mem::Allocating_Type::ALIGNED_NEW_32 == allocating_type)
			{
				auto const ptr = ::operator new[](count, std::align_val_t{ 32ull }, std::nothrow);
				//if (!mem::is_aligned<T, closest_alignment>(ptr)) throw std::bad_alloc();
#ifdef ENABLE_CPP_EXCEPTION_THROW
				if (!ptr) [[unlikely]] throw std::bad_alloc();
#else
				if (!ptr) [[unlikely]] std::terminate();
#endif
				return ptr;
			}
			else
			{
				constexpr auto closest_alignment = mem::get_closest_power_of_2_alignment<value_type, 32>();
				auto const ptr = ::operator new[](count, std::align_val_t{ closest_alignment }, std::nothrow);
				//if (!mem::is_aligned<T, closest_alignment>(ptr)) throw std::bad_alloc();
#ifdef ENABLE_CPP_EXCEPTION_THROW
				if (!ptr) [[unlikely]] throw std::bad_alloc();
#else
				if (!ptr) [[unlikely]] std::terminate();
#endif
				return ptr;
			}
		};
	public:
		[[nodiscard]] __declspec(allocator, restrict) constexpr value_type* allocate(const std::size_t count) const
		{
#ifdef ENABLE_CPP_EXCEPTION_THROW
			if (count == 0 || count > bad_arr_length) throw std::bad_array_new_length();
#endif
			if (std::is_constant_evaluated()) return ::new value_type[count];
			else return static_cast<value_type*>(this->_allocate(size_of_value_type * count));
		};

		constexpr void deallocate(value_type* const memory_block, std::size_t n = 0) const noexcept //, const std::size_t new_size
		{
			if (std::is_constant_evaluated())
			{
				delete[] memory_block;
				return;
			}

			if constexpr (mem::Allocating_Type::MALLOC == allocating_type)
			{
				free(memory_block);
			}
			else if constexpr (mem::Allocating_Type::ALIGNED_MALLOC == allocating_type)
			{
				_aligned_free(memory_block);
			}
			else if constexpr (mem::Allocating_Type::CONCURRENCY == allocating_type)
			{
				concurrency::Free(memory_block);
			}
			else if constexpr (mem::Allocating_Type::NEW == allocating_type)
			{
				if (n != 0) operator delete[](memory_block, n);
				else operator delete[](memory_block);
			}
			else
			{
				constexpr auto closest_alignment = mem::get_closest_power_of_2_alignment<value_type, 32>();
				//constexpr auto closest_alignment = (mem::aligned_by<value_type, sizeof(value_type)>() == 0ull ? (mem::aligned_by<value_type, 32ull>() == 0ull ? 32ull : mem::get_closest_alignment<value_type>()) : mem::get_closest_alignment<value_type>());
				if (n != 0) operator delete[](memory_block, n, std::align_val_t{ closest_alignment });
				else operator delete[](memory_block, std::align_val_t{ closest_alignment });
			}
		};
	};

	struct expected_size {} constexpr expected_size_t;

	template <class _Value_type, class _Size_type, class _Difference_type, class _Pointer, class _Const_pointer, class _Reference, class _Const_reference>
	struct vector_iterator_types
	{
		using value_type = _Value_type;
		using size_type = _Size_type;
		using difference_type = _Difference_type;
		using pointer = _Pointer;
		using const_pointer = _Const_pointer;
		using reference = _Reference;
		using const_reference = _Const_reference;
	};

	template <class _Value_type>
	struct simple_types
	{ // wraps types from allocators with simple addressing for use in iterators
	 // and other SCARY machinery
		using value_type = _Value_type;
		using size_type = std::size_t;
		using difference_type = std::ptrdiff_t;
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using reference = _Value_type&;
		using const_reference = const _Value_type&;
	};

	template <class value_types>
	class vector_value
	{
	public:
		using value_type = typename value_types::value_type;
		using size_type = typename value_types::size_type;
		using difference_type = typename value_types::difference_type;
		using pointer = typename value_types::pointer;
		using const_pointer = typename value_types::const_pointer;
		using reference = typename value_types::reference;
		using const_reference = typename value_types::const_reference;

		constexpr vector_value() = default;
		constexpr vector_value(pointer first_, size_type capacity) noexcept :
			first{ first_ },
			last{ first_ },
			end{ first_ + capacity }
		{
			//assert(((unsigned long long)first_) + (sizeof(value_type) * capacity) == ((unsigned long long)end));
		};
		constexpr vector_value(pointer first_, size_type capacity, expected_size expected_size_t) noexcept :
			first{ first_ },
			last{ first_ + capacity },
			end{ first_ + capacity }
		{};
		constexpr vector_value(pointer first_, pointer last_, pointer end_) noexcept :
			first{ first_ },
			last{ last_ },
			end{ end_ }
		{};
		constexpr vector_value(std::nullptr_t) noexcept :
			first{ nullptr },
			last{ nullptr },
			end{ nullptr }
		{};

		constexpr void _Swap_val(vector_value& rhs) noexcept
		{
			vector_value tmp = rhs;
			rhs.first = this->first;
			rhs.last = this->last;
			rhs.end = this->end;
			this->first = tmp.first;
			this->last = tmp.last;
			this->end = tmp.end;
		};

		constexpr void _Take_contents(vector_value& rhs) noexcept
		{
			this->first = rhs.first;
			this->last = rhs.last;
			this->end = rhs.end;

			rhs.first = nullptr;
			rhs.last = nullptr;
			rhs.end = nullptr;
		};

		constexpr friend bool operator==(const vector_value& lhs, const vector_value& rhs)
		{
			return lhs.first == rhs.first && lhs.last == rhs.last && lhs.end == rhs.end;
		};
		constexpr friend bool operator!=(const vector_value& lhs, const vector_value& rhs)
		{
			return !(lhs == rhs);
		};
		constexpr friend bool operator==(const vector_value& lhs, const std::nullptr_t& rhs)
		{
			return lhs.first == rhs && lhs.last == rhs && lhs.end == rhs;
		};
		constexpr friend bool operator!=(const vector_value& lhs, const std::nullptr_t& rhs)
		{
			return !(lhs == rhs);
		};

		constexpr inline long long size() const noexcept
		{
			return last - first;//  this->size_;
		};
		constexpr inline std::size_t usize() const noexcept
		{
			return static_cast<std::size_t>(last - first);// this->size_;
		};
		constexpr inline long long get_capacity() const noexcept
		{
			return end - first;
		};
		constexpr inline std::size_t get_ucapacity() const noexcept
		{
			return static_cast<std::size_t>(end - first);
		};

		alignas(8) pointer first { nullptr }; // pointer to beginning of array
		alignas(8) pointer last { nullptr }; // pointer to current end of sequence
		alignas(8) pointer end { nullptr }; // pointer to end of array
	};

	/*static_assert(mem::concepts::is_allocator_requirements_v<mem::allocator<int>, int> == true);
	static_assert(mem::concepts::is_allocator_requirements_v<mem::allocator<long long>, long long> == true);
	static_assert(mem::concepts::is_allocator_requirements_v<mem::allocator<std::string>, std::string> == true);
	static_assert(mem::concepts::is_allocator_requirements_v<mem::allocator<std::vector<std::string>>, std::vector<std::string>> == true);

	static_assert(std::is_same<mem::allocator<Rect, mem::Allocating_Type::NEW>::rebind_alloc<Rect>, mem::allocator<Rect, mem::Allocating_Type::NEW>>::value == true);
	static_assert(std::is_same<mem::allocator<Rect, mem::Allocating_Type::NEW>::rebind_alloc<unsigned long long>, mem::allocator<Rect, mem::Allocating_Type::NEW>>::value == false);
	static_assert(mem::concepts::is_allocator_requirements_v<mem::allocator<Rect, mem::Allocating_Type::NEW>, Rect>);*/

	template <class Object, mem::Allocating_Type allocating_type = mem::Allocating_Type::NEW, class Allocator = mem::allocator<Object, allocating_type>, mem::use_memcpy memcpy_check = mem::use_memcpy::check_based_on_type>
	class vector
	{
	private:
		using _Alty = typename std::allocator_traits<Allocator>::template rebind_alloc<Object>;//std::_Rebind_alloc_t<Allocator, Object>;//
		using _Alty_traits = std::allocator_traits<_Alty>;

		//_Alty MemoryAllocator;

		static_assert(std::is_same_v<_Alty, mem::allocator<Object, allocating_type>>);

	public:
		static constexpr mem::use_memcpy use_memcpy_value = memcpy_check;
		static constexpr  mem::Allocating_Type use_allocating_type = allocating_type;
		static_assert(!_ENFORCE_MATCHING_ALLOCATORS || std::is_same_v<Object, typename Allocator::value_type>, _MISMATCHED_ALLOCATOR_MESSAGE("vector<T, Allocator>", "T"));

		using value_type = Object;
		using allocator_type = Allocator;
		using pointer = typename _Alty_traits::pointer;
		using const_pointer = typename _Alty_traits::const_pointer;
		using reference = Object&;
		using const_reference = const Object&;
		using size_type = typename _Alty_traits::size_type;
		using difference_type = typename _Alty_traits::difference_type;

		static_assert(mem::concepts::is_allocator_v<Allocator, Object>);
		static_assert(mem::concepts::is_allocator_requirements_v<_Alty, Object>);
		static_assert(mem::concepts::is_simple_alloc_v<_Alty> == true, "is not a simple alloc");

		//private:
		using scary_val = mem::vector_value<std::conditional_t<mem::concepts::is_simple_alloc_v<_Alty>, mem::simple_types<Object>,
			mem::vector_iterator_types<Object, size_type, difference_type, pointer, const_pointer, Object&, const Object&>>>;

		static_assert(std::is_same_v<Object, scary_val::value_type> == true, "not same type");

	public:
		using iterator = mem::iterator<scary_val>;

		static_assert(std::is_same_v<Object, iterator::value_type> == true, "not same type");
		static_assert(std::is_same_v<mem::iterator<mem::vector_value<mem::simple_types<Object>>>, iterator> == true, "not same iterator type");
		//static_assert(std::random_access_iterator<iterator>);

		scary_val values{ nullptr, nullptr, nullptr };
#ifdef _DEAD_DEBUG_CHECK
		bool is_dead{ false };
#endif

		inline constexpr vector() = default;
		inline constexpr vector(long long capacity, _Alty allocator = _Alty{}) noexcept : //requires(mem::is_allocator_v<Allocator, Object>&& mem::is_allocator_requirements_v<_Alty, Object>)
			values{ _Alty_traits::allocate(allocator, static_cast<unsigned long long>(capacity)), static_cast<unsigned long long>(capacity) }
		{};
		inline constexpr vector(long long capacity, expected_size expected_size_t, _Alty allocator = _Alty{}) noexcept : //requires(mem::is_allocator_v<Allocator, Object>&& mem::is_allocator_requirements_v<_Alty, Object>)
			values{ _Alty_traits::allocate(allocator, static_cast<unsigned long long>(capacity)), static_cast<unsigned long long>(capacity), expected_size_t }
		{};
		inline constexpr ~vector() noexcept
			//requires (vector_has_method<Object> == false)
		{
			if (this->values.first != nullptr && this->get_capacity() > 0)
			{
				if constexpr (std::is_trivially_destructible_v<Object> == false)
				{
					if (!std::is_constant_evaluated())
					{
						for (iterator iter = begin(); iter != last(); ++iter)
						{
							(*(iter)).Object::~Object();
						}
					}
				}
				const _Alty MemoryAllocator{};
				MemoryAllocator.deallocate(this->values.first);// , this->usize());
				values = { nullptr, nullptr, nullptr };
			}

#ifdef _DEAD_DEBUG_CHECK
			is_dead = true;
#endif
		};

		constexpr vector(const vector& o) noexcept
		{
			if (o.values != nullptr && o.size() > 0) this->emplace_resize_vector(o);
#ifdef _DEAD_DEBUG_CHECK
			this->is_dead = o.is_dead;
#endif
		};
		constexpr vector& operator=(const vector& o) noexcept
		{
			if (o.values != nullptr && o.size() > 0) this->emplace_resize_vector(o);
#ifdef _DEAD_DEBUG_CHECK
			this->is_dead = o.is_dead;
#endif
			return *this;
		};
		/*template<typename other_vector>
		constexpr vector& operator=(const other_vector& o) noexcept
			requires(!std::is_same_v<vector, other_vector>&& std::is_class_v<other_vector>)
		{
			if (o.values != nullptr && o.size() > 0) this->emplace_resize_vector(o);
#ifdef _DEAD_DEBUG_CHECK
			this->is_dead = o.is_dead;
#endif
			return *this;
		};*/
		constexpr vector(vector&& o) noexcept :
			values{ std::exchange(o.values, { nullptr, nullptr, nullptr }) }
#ifdef _DEAD_DEBUG_CHECK
			,
			is_dead{ std::exchange(o.is_dead, true) }
#endif
		{};
		constexpr vector& operator=(vector&& o) noexcept
		{
			this->values = exchange(std::move(o.values));
#ifdef _DEAD_DEBUG_CHECK
			this->is_dead = std::exchange(o.is_dead, true);
#endif
			return *this;
		};

		inline constexpr bool validate_vector() const noexcept
		{
			auto logic_check = values.first <= values.last && values.first < values.end && values.last <= values.end;
#ifdef ENABLE_CPP_EXCEPTION_THROW
			if (logic_check == false) throw std::runtime_error("");
#else
			if (logic_check == false) [[unlikely]] std::terminate();
#endif
			return logic_check;
		};
		template<typename validation_ptr>
		inline constexpr bool validate_pointer(validation_ptr ptr) const noexcept
			requires(std::is_same_v<validation_ptr, pointer>&& std::is_convertible_v<validation_ptr, pointer>)
		{
			auto logic_check = (values.first) >= ptr && ptr <= (values.last) && ptr < (values.end);
#ifdef ENABLE_CPP_EXCEPTION_THROW
			if (logic_check == false) throw std::runtime_error("");
#else
			if (logic_check == false) [[unlikely]] std::terminate();
#endif
			return logic_check;
		};
		template<typename validation_iter>
		inline constexpr bool validate_pointer(validation_iter iter) const noexcept
			requires(std::is_same_v<validation_iter, iterator>)
		{
			auto logic_check = values.first >= iter.operator->() && iter.operator->() < values.last && iter.operator->() <= values.end;
#ifdef ENABLE_CPP_EXCEPTION_THROW
			if (logic_check == false) throw std::runtime_error("");
#else
			if (logic_check == false) [[unlikely]] std::terminate();
#endif
			return logic_check;
		};

		template<typename type_size>
		[[nodiscard]] constexpr value_type& operator[](type_size index) noexcept
			requires(std::is_integral_v<type_size>&& std::is_same_v<std::size_t, type_size> == false)
		{
#ifdef ENABLE_CPP_EXCEPTION_THROW
			if (this->size() < index) throw std::runtime_error("");
			//if (this->size() == index) throw std::runtime_error("");
			if (this->size() >= index) return this->values.first[index];
			throw std::runtime_error("");
#else
			return values.first[index];
#endif
		};
		template<typename type_size>
		[[nodiscard]] constexpr value_type& operator[](type_size index) noexcept
			requires(std::is_same_v<std::size_t, type_size>)
		{
#ifdef ENABLE_CPP_EXCEPTION_THROW
			if (this->usize() < index) throw std::runtime_error("");
			//if (this->usize() == index) throw std::runtime_error("");
			if (this->usize() >= index) return this->values.first[index];
			throw std::runtime_error("");
#else
			return values.first[index];
#endif
		};

		template<typename type_size>
		[[nodiscard]] constexpr const value_type& operator[](type_size index) const noexcept
			requires(std::is_integral_v<type_size>&& std::is_same_v<std::size_t, type_size> == false)
		{
#ifdef ENABLE_CPP_EXCEPTION_THROW
			if (this->size() < index) throw std::runtime_error("");
			//if (this->size() == index) throw std::runtime_error("");
			if (this->size() >= index) return this->values.first[index];
			throw std::runtime_error("");
#else
			return values.first[index];
#endif
		};
		template<typename type_size>
		[[nodiscard]] constexpr const value_type& operator[](type_size index) const noexcept
			requires(std::is_same_v<std::size_t, type_size>)
		{
#ifdef ENABLE_CPP_EXCEPTION_THROW
			if (this->usize() < index) throw std::runtime_error("");
			//if (this->usize() == index) throw std::runtime_error("");
			if (this->usize() >= index) return this->values.first[index];
			throw std::runtime_error("");
#else
			return values.first[index];
#endif
		};

		friend inline constexpr bool operator==(const vector& lhs, const vector& rhs) noexcept
		{
			return lhs.values.first == rhs.values.first && lhs.values.last == rhs.values.last && lhs.values.end == rhs.values.end;
		};

		inline constexpr long long size() const noexcept
		{
			return this->values.last - this->values.first;//  this->size_;
		};
		inline constexpr unsigned long long usize() const noexcept
		{
			return this->values.last - this->values.first;// this->size_;
		};
		inline constexpr long long get_capacity() const noexcept
		{
			return this->values.get_capacity();// this->capacity;
		};
		inline constexpr std::size_t get_ucapacity() const noexcept
		{
			return this->values.get_ucapacity();//static_cast<std::size_t>(this->capacity);
		};
		inline constexpr bool empty() const noexcept
		{
			return this->values.first == this->values.last;
		};

		constexpr iterator begin() const noexcept
		{
			return iterator(this->values.first);
		};
		constexpr iterator last() const noexcept
		{
			return iterator(this->values.last);
		};
		constexpr iterator end() const noexcept
		{
			if (this->values.last == this->values.end) return iterator(this->values.last);
			else return iterator(this->values.last + 1);
		};
		constexpr const value_type& back() const noexcept
		{
#ifdef ENABLE_CPP_EXCEPTION_THROW
			if (values.end == values.first) throw std::runtime_error("");
#endif
			return values.first[size() - 1ll];
		};

		inline constexpr void reserve(long long size) noexcept
		{
			if (this->get_capacity() < size) this->resize(size);
		};

		__forceinline constexpr void reserve() noexcept
		{
			this->resize(this->get_capacity() * 2);
		};

		inline constexpr bool needs_resize() const noexcept
		{
			return this->values.last >= this->values.end;
		};

		inline constexpr bool needs_resize(long long min_size) const noexcept
		{
			return min_size >= get_capacity();
			return (this->values.last + min_size) >= (this->values.end - 1ll);
		};

		inline constexpr void decrease_size(long long new_size) noexcept
			requires(std::is_trivially_destructible_v<value_type> == true)
		{
			this->values.last = this->values.first + new_size;
		};

		inline constexpr void check_increase_capacity() noexcept
		{
			if (this->needs_resize()) this->resize(this->get_capacity() * 2ll);
		};

		__forceinline constexpr void increase_capacity() noexcept
		{
			this->resize(this->get_capacity() * 2ll);
		};

		__forceinline constexpr void increase_capacity(long long min_size) noexcept
		{
			this->resize((this->get_capacity() + min_size) * 2ll);
		};

		__forceinline constexpr bool inside_range(void* ptr) noexcept
		{
			return values.first <= ptr && ptr < values.end;
		};

		inline constexpr scary_val exchange(scary_val&& o) noexcept
		{
			if (values.first != nullptr && this->get_capacity() > 0)
			{
				if constexpr (std::is_trivially_destructible_v<Object> == false)
				{
					if (!std::is_constant_evaluated())
					{
						for (iterator iter = begin(); iter != last(); ++iter)
						{
							(*(iter)).Object::~Object();
						}
					}
				}
				const _Alty MemoryAllocator{};
				MemoryAllocator.deallocate(values.first);// , this->usize());
			}
			return std::exchange(o, scary_val{ nullptr, nullptr, nullptr });
		};

		__forceinline constexpr void resize() noexcept
		{
			const scary_val oldContainer{ this->values };
			this->reallocate(oldContainer);
		};

		__declspec(noinline) constexpr void resize(long long newSize) noexcept
			requires (mem::concepts::vector_has_method<value_type> == false)
		{
			scary_val oldContainer{ this->values };
			long long old_size = oldContainer.size();
			const _Alty MemoryAllocator{};
			this->values = scary_val{ MemoryAllocator.allocate(static_cast<std::size_t>(newSize)), static_cast<std::size_t>(newSize) };

			for (long long i = 0; i < old_size; ++i)
			{
				new (&this->values.first[i]) value_type{ std::move(oldContainer.first[i]) };
				//this->values.first[i] = std::move(oldContainer.first[i]);
			}
			MemoryAllocator.deallocate(oldContainer.first);
			this->values.last = this->values.first + old_size;
		};

		__declspec(noinline) constexpr void resize(long long newSize) noexcept
			requires (mem::concepts::vector_has_method<value_type> == true)
		{
			__ASSUME__(values.first != nullptr);
			const _Alty MemoryAllocator{};
			if constexpr (mem::concepts::get_is_trivially_copyable_v<value_type> == true)
			{
				if (!std::is_constant_evaluated())
				{
					const auto old_size = this->size();
					scary_val newContainer = { MemoryAllocator.allocate(tc::widen<std::size_t>(newSize)), tc::widen<std::size_t>(newSize) };

#ifdef ENABLE_CPP_EXCEPTION_THROW
					if (values.first + old_size > values.end) throw std::runtime_error("");
#else
					if (values.first + old_size > values.end) [[unlikely]] std::terminate();
#endif

					std::memcpy(newContainer.first, this->values.first, sizeof(value_type) * old_size);

					this->values = std::move(newContainer);
					this->values.last = this->values.first + old_size;
					return;
				}
			}

			scary_val oldContainer{ this->values };
			long long old_size = oldContainer.size();
			this->values = scary_val{ MemoryAllocator.allocate(static_cast<std::size_t>(newSize)), static_cast<std::size_t>(newSize) };

			for (long long i = 0; i < old_size; ++i)
			{
				new (&this->values.first[i]) value_type{ std::move(oldContainer.first[i]) };
				//this->values.first[i] = std::move(oldContainer.first[i]);
			}
			MemoryAllocator.deallocate(oldContainer.first);
			this->values.last = this->values.first + old_size;
		};


		template <typename... Types>
		constexpr void emplace_resize(Types&&... args) noexcept
			requires (std::is_move_constructible_v<value_type> == true && std::is_constructible_v<value_type, Types...> == true)
		{
			__ASSUME__(values.first != nullptr);

			this->reallocate(std::move(values));
			if (std::is_constant_evaluated() == false)
				new (values.last) value_type{ std::forward<Types>(args)... };
			else
				*values.last = value_type{ std::forward<Types>(args)... };
			++values.last;
		};

		inline constexpr void emplace_resize(const value_type& object) noexcept
		{
			__ASSUME__(values.first != nullptr);

			this->reallocate(std::move(this->values));
			new (this->values.last) value_type{ object };
			//*(this->values.last) = object;
			++this->values.last;
		};

		inline constexpr void emplace_resize(value_type&& object) noexcept
		{
			__ASSUME__(values.first != nullptr);

			this->reallocate(std::move(this->values));
			if constexpr (mem::concepts::get_is_trivially_copyable_v<value_type> == true)
			{
				if (std::is_constant_evaluated() == false)
				{
					std::memcpy(this->values.last, &object, sizeof(value_type));
					++this->values.last;

					if constexpr (mem::concepts::vector_has_method<value_type> == true) object.values.first = nullptr;

					return;
				}
			}

			new (this->values.last) value_type{ std::move(object) };
			//*(this->values.last) = std::move(object);
			++this->values.last;
		};

		template<typename vector_other>
		__forceinline constexpr void emplace_resize_vector(const vector_other& vector) noexcept
		{
			if constexpr (mem::use_memcpy::force_checks_off == memcpy_check || mem::use_memcpy::check_based_on_type == memcpy_check && mem::concepts::get_is_trivially_copyable_v<value_type> == true)
			{
				if (std::is_constant_evaluated() == false) emplace_resize_vector_m(vector);
				else emplace_resize_vector_cm(vector);
			}
			else emplace_resize_vector_cm(vector);
		};

		template<typename vector_other>
		inline constexpr void emplace_resize_vector_cm(const vector_other& vector) noexcept
			requires (mem::concepts::vector_has_method<vector_other> == true && std::is_class_v<vector_other>)
		{
			//__ASSUME__(values.first != nullptr);
			//__ASSUME__(values.first != 0);
			if (this->values.first && !this->empty()) this->clear();

			const auto new_capacity = vector.get_capacity();

			if (this->get_capacity() < new_capacity || this->values.first == nullptr)
			{
				const _Alty MemoryAllocator{};
				if (values.get_capacity() > 0ll)
				{
					MemoryAllocator.deallocate(values.first);
				}
				this->values = scary_val{ MemoryAllocator.allocate(new_capacity), vector.get_ucapacity() };
			}

			if (std::is_constant_evaluated() == false)
			{
				if constexpr (mem::concepts::get_is_trivially_copyable_v<value_type> == true)
				{
					__assume(this->values.first != nullptr);
					std::memcpy(this->values.first, vector.values.first, sizeof(value_type) * vector.size());
					this->values.last = this->values.first + vector.size();
					return;
				}
			}

			for (long long i = 0; i < vector.size(); ++i)
			{
				if (std::is_constant_evaluated() == false) new (&this->values.first[i]) value_type{ vector.values.first[i] };
				else this->values.first[i] = vector.values.first[i];
			}
			this->values.last = this->values.first + vector.size();
		};
		template<typename vector_other>
		inline constexpr void emplace_resize_vector_m(const vector_other& vector) noexcept
			requires (mem::concepts::vector_has_method<vector_other> == true && std::is_class_v<vector_other>)
		{
			__ASSUME__(values.first != nullptr);

			if (this->values.first && !this->empty()) this->clear();
			_Alty MemoryAllocator{};
			if (this->get_capacity() < vector.get_capacity())
			{
				if (values.get_capacity() > 0ll)
				{
					MemoryAllocator.deallocate(values.first);
				}
				this->values = { MemoryAllocator.allocate(vector.get_capacity()), static_cast<std::size_t>(vector.get_capacity()) };
			}

			const std::size_t type_arr_size = vector.size();

			if constexpr (mem::Allocating_Type::ALIGNED_NEW == allocating_type || mem::Allocating_Type::ALIGNED_MALLOC == allocating_type)
				mem::simd::mem_copy_256<value_type, true>((void*)values.first, (void*)vector.values.first, type_arr_size);
			else if constexpr (mem::Allocating_Type::NEW == allocating_type || mem::Allocating_Type::MALLOC == allocating_type)
				mem::simd::mem_copy_256<value_type, false>((void*)values.first, (void*)vector.values.first, type_arr_size); //mem::simd::SIMDMemCopy256((void*)this->values.first, (void*)vector.values.first, mem::divide_by_multiple(type_arr_size, 16));
			else
				std::memcpy(this->values.first, vector.values.first, sizeof(value_type) * type_arr_size);
			this->values.last = this->values.first + type_arr_size;
		};

		template<typename vector_other>
		__forceinline constexpr void emplace_resize_vector(vector_other&& vector) noexcept
		{
			if constexpr (mem::use_memcpy::force_checks_off == memcpy_check || mem::use_memcpy::check_based_on_type == memcpy_check && mem::concepts::get_is_trivially_copyable_v<value_type> == true)
			{
				if (std::is_constant_evaluated() == false) emplace_resize_vector_m(vector);
				else emplace_resize_vector_cm(vector);
			}
			else emplace_resize_vector_cm(vector);
		};

		template<typename vector_other>
		inline constexpr void emplace_resize_vector_cm(vector_other&& vector) noexcept
			requires (mem::concepts::vector_has_method<vector_other> == true)
		{
			__ASSUME__(values.first != nullptr);

			if (this->values.first && !this->empty()) this->clear();
			_Alty MemoryAllocator{};
			if (this->get_capacity() < vector.get_capacity())
			{
				if (values.get_capacity() > 0ll)
				{
					MemoryAllocator.deallocate(values.first);
				}
				this->values = { MemoryAllocator.allocate(vector.get_capacity()), static_cast<std::size_t>(vector.get_capacity()) };

			}

			this->capacity = std::exchange(vector.get_capacity(), 0);
			this->size_ = std::exchange(vector.size(), 0);

			if constexpr (mem::concepts::get_is_trivially_copyable_v<value_type> == true)
			{
				std::memcpy(&this->values.first[0], &vector.values.first[0], sizeof(value_type) * this->size_);
				this->values.last = this->values.first + this->size_;
			}
			else
			{
				for (long long i = 0; i < this->size(); ++i)
				{
					new (&this->values.first[i]) value_type{ std::move(vector.values.first[i]) };
					//this->values.first[i] = std::move(vector.values.first[i]);
				}
			}

			//MemoryAllocator.deallocate(vector.values.first);
			vector.values = { nullptr, nullptr, nullptr };
		};
		template<typename vector_other>
		inline constexpr void emplace_resize_vector_m(vector_other&& vector) noexcept
			requires (mem::concepts::vector_has_method<vector_other> == true)
		{
			__ASSUME__(values.first != nullptr);

			if (this->values.first && !this->empty()) this->clear();

			_Alty MemoryAllocator{};

			if (this->get_capacity() < vector.get_capacity())
			{
				if (values.get_capacity() > 0ll)
				{
					MemoryAllocator.deallocate(values.first);
				}
				this->values = { MemoryAllocator.allocate(vector.get_capacity()), static_cast<std::size_t>(vector.get_capacity()) };
			}

			const std::size_t type_arr_size = vector.size();

			if constexpr ((mem::Allocating_Type::ALIGNED_NEW == allocating_type || mem::Allocating_Type::ALIGNED_MALLOC == allocating_type) && (sizeof(Object) == 32ll || sizeof(Object) == 16ll))
				mem::simd::mem_copy_256<value_type, true>((void*)values.first, (void*)vector.values.first, type_arr_size);
			else if constexpr (mem::Allocating_Type::NEW == allocating_type || mem::Allocating_Type::MALLOC == allocating_type)
				mem::simd::SIMDMemCopy256((void*)this->values.first, (void*)vector.values.first, mem::divide_by_multiple(type_arr_size, 16));
			else
				std::memcpy(this->values.first, vector.values.first, sizeof(value_type) * type_arr_size);

			vector.values = { nullptr, nullptr, nullptr };
			this->values.last = this->values.first + type_arr_size;
		};

	private:
		__declspec(noinline) void reallocate_m(scary_val old_container) noexcept
		{
			const auto new_size = (old_container.get_ucapacity() == 0ull ? 8ull : old_container.get_ucapacity()) * 2ull;
			const auto old_size = old_container.usize();
			constexpr const size_t type_size = sizeof(Object);
			const std::size_t type_arr_size = type_size * old_size;
			const _Alty MemoryAllocator{};
			this->values = scary_val{ MemoryAllocator.allocate(new_size), new_size };

			if (old_container.get_capacity() > 0)
			{
				if constexpr ((mem::Allocating_Type::ALIGNED_NEW == allocating_type || mem::Allocating_Type::ALIGNED_MALLOC == allocating_type) && (sizeof(Object) == 32ll || sizeof(Object) == 16ll))
					mem::simd::mem_copy_256<value_type, true>((void*)values.first, (void*)old_container.first, old_size);
				else if constexpr (mem::Allocating_Type::NEW == allocating_type || mem::Allocating_Type::MALLOC == allocating_type)
					mem::simd::SIMDMemCopy256((void*)this->values.first, (void*)old_container.first, mem::divide_by_multiple(type_arr_size, 16));
				else
					std::memcpy(this->values.first, old_container.first, type_arr_size);
				MemoryAllocator.deallocate(old_container.first);
			}
			this->values.last = this->values.first + old_size;
		};
		__declspec(noinline) constexpr void reallocate_cm(scary_val old_container) noexcept
		{
			std::size_t new_size = (old_container.get_ucapacity() == 0ull ? 8ull : old_container.get_ucapacity()) * 2ull;
			if (values.end == values.first) new_size = (old_container.get_ucapacity() * 2ull) + 2ull;

			auto old_size = old_container.size();
			const _Alty MemoryAllocator{};

#ifdef ENABLE_CPP_EXCEPTION_THROW
			if (new_size <= 0ll) throw std::runtime_error("");
#endif

			this->values = scary_val{ MemoryAllocator.allocate(new_size), new_size };

			if (old_size > 256ll)
			{
				const long long initial_count = old_size;
				const auto loops = expr::divide_with_remainder(initial_count, 4ll);
				for (long long i = 0ll, i_loop = 0ll; i < loops.div; ++i)
				{
					/*if (std::is_constant_evaluated() == false)
					{
						auto tmp = old_container.first;
						mem::pre_fetch_cachelines<Object, 4ll, _MM_HINT_NTA>(tmp + (i_loop + 4ll));
						//auto tmp2 = values.first;
						//pre_fetch_cachelines<Object, 4ll>(tmp2 + (i_loop + 4ll));
					}*/
					//pre_fetch_cachelines<Object, 1ll, _MM_HINT_NTA>(old_container.first + (i_loop + 1ll));
					if (std::is_constant_evaluated() == true) this->values.first[i_loop] = std::move(old_container.first[i_loop]);
					else new (&this->values.first[i_loop]) value_type{ std::move(old_container.first[i_loop]) };

					//pre_fetch_cachelines<Object, 1ll, _MM_HINT_NTA>(old_container.first + (i_loop + 2ll));
					if (std::is_constant_evaluated() == true) this->values.first[i_loop + 1ll] = std::move(old_container.first[i_loop + 1ll]);
					else new (&this->values.first[i_loop + 1ll]) value_type{ std::move(old_container.first[i_loop + 1ll]) };

					//pre_fetch_cachelines<Object, 1ll, _MM_HINT_NTA>(old_container.first + (i_loop + 3ll));
					if (std::is_constant_evaluated() == true) this->values.first[i_loop + 2ll] = std::move(old_container.first[i_loop + 2ll]);
					else new (&this->values.first[i_loop + 2ll]) value_type{ std::move(old_container.first[i_loop + 2ll]) };

					//pre_fetch_cachelines<Object, 1ll, _MM_HINT_NTA>(old_container.first + (i_loop + 4ll));
					if (std::is_constant_evaluated() == true) this->values.first[i_loop + 3ll] = std::move(old_container.first[i_loop + 3ll]);
					else new (&this->values.first[i_loop + 3ll]) value_type{ std::move(old_container.first[i_loop + 3ll]) };
					i_loop += 4ll;
				}
				const long long l = loops.div * 4ll + loops.rem;
				for (long long i = loops.div * 4ll; i < l; ++i)
				{
					if (std::is_constant_evaluated() == true) this->values.first[i] = std::move(old_container.first[i]);
					else  new (&this->values.first[i]) value_type{ std::move(old_container.first[i]) };
				}
			}
			else
			{
				for (long long i = 0; i < old_size; ++i)
				{
					//if (std::is_constant_evaluated() == false && i + 1 > old_size) _mm_prefetch((char const*)(&old_container.first[i + 1]), _MM_HINT_NTA);
					if (std::is_constant_evaluated() == true) this->values.first[i] = std::move(old_container.first[i]);
					else new (&this->values.first[i]) value_type{ std::move(old_container.first[i]) };
				}
			}
			MemoryAllocator.deallocate(old_container.first);
			this->values.last = this->values.first + old_size;
		};

	public:
		__forceinline constexpr void reallocate(scary_val old_container) noexcept
		{
			if constexpr (mem::use_memcpy::force_checks_off == memcpy_check || mem::use_memcpy::check_based_on_type == memcpy_check && mem::concepts::get_is_trivially_copyable_v<value_type> == true)
			{
				if (std::is_constant_evaluated() == false) reallocate_m(old_container);
				else reallocate_cm(old_container);
			}
			else reallocate_cm(old_container);
		};

		inline constexpr void push_back(const value_type& val) noexcept
			requires (mem::concepts::vector_has_method<value_type> == false)
		{
			if (this->needs_resize()) [[unlikely]]
				this->emplace_resize(val);
			else [[likely]]
			{
				new (this->values.last) value_type{ val };
				//*(this->values.last) = val;
				++this->values.last;
			}
		};

		inline constexpr void push_back(const mem::vector<Object, allocating_type, Allocator, memcpy_check>& val) noexcept
			requires (mem::concepts::vector_has_method<value_type> == false)
		{
			if (this->needs_resize(val.size()))
				this->increase_capacity(val.size());

			auto begin_val = val.begin();
			const auto end_val = val.last();

			while (begin_val < end_val)
			{
				new (this->values.last) value_type{ *begin_val };
				//*(this->values.last) = *begin_val;
				++this->values.last;
				++begin_val;
			}
		};

		inline constexpr void push_back(value_type&& val) noexcept
			requires (mem::concepts::vector_has_method<value_type> == false)
		{
			if (this->needs_resize()) [[unlikely]]
				this->emplace_resize(std::move(val));
			else [[likely]]
			{
				if (std::is_constant_evaluated() == true) *(this->values.last) = std::move(val);
				else new (this->values.last) value_type{ std::move(val) };
				++this->values.last;
			}
		};

		inline constexpr void push_back(mem::vector<Object, allocating_type, Allocator, memcpy_check>&& val) noexcept
			requires (mem::concepts::vector_has_method<value_type> == false)
		{
			if (this->needs_resize(val.size())) this->increase_capacity(val.size());

			this->insert(this->begin() + this->size(), val.begin(), val.end());
		};

		inline constexpr void push_back(const value_type& val) noexcept
			requires (mem::concepts::vector_has_method<value_type> == true)
		{
			if constexpr (mem::concepts::get_is_trivially_copyable_v<value_type> == true)
			{
				new (this->values.last) value_type{ val };
				//*(this->values.last) = val;
				++this->values.last;

				if (this->needs_resize()) this->increase_capacity();
			}
			else
			{
				if (this->needs_resize()) [[unlikely]]
					this->emplace_resize(val);
				else [[likely]]
				{
					new (this->values.last) value_type{ val };
					//*(this->values.last) = val;
					++this->values.last;
				}
			}
		};

		inline constexpr void push_back(value_type&& val) noexcept
			requires (mem::concepts::vector_has_method<value_type> == true)
		{
			if constexpr (mem::concepts::get_is_trivially_copyable_v<value_type> == true)
			{
				std::memcpy(this->values.last, &val, sizeof(value_type));
				++this->values.last;
				val.values.first = nullptr;

				if (this->needs_resize()) this->increase_capacity();
			}
			else
			{
				if (this->needs_resize()) [[unlikely]]
					this->emplace_resize(std::move(val));
				else [[likely]]
				{
					new (this->values.last) value_type{ std::move(val) };
					//*(this->values.last) = std::move(val);
					++this->values.last;
				}

				val.values = nullptr;
			}
		};

		inline constexpr value_type& emplace_back() noexcept
			requires(mem::concepts::vector_has_method<value_type> == false && std::is_default_constructible_v<value_type> == true)
		{
			static_assert(std::is_default_constructible_v<value_type> == true);
			if (this->needs_resize()) [[unlikely]]
			{
				this->resize();
				if (std::is_constant_evaluated() == false)
					new (this->values.last) value_type{};
				else
					*values.last = value_type{};
				++this->values.last;
				return *(values.last - 1ll);
			}
			else [[likely]]
			{
				if (std::is_constant_evaluated() == false)
					new (this->values.last) value_type{};
				else
					*values.last = value_type{};
				++this->values.last;
				return *(values.last - 1ll);
			}
		};
		inline constexpr value_type& emplace_back() noexcept
			requires(mem::concepts::vector_has_method<value_type> == true && std::is_default_constructible_v<value_type> == true)
		{
			static_assert(std::is_default_constructible_v<value_type> == true);
			if (this->needs_resize()) [[unlikely]]
			{
				this->resize();
				if (std::is_constant_evaluated() == false)
					new (this->values.last) value_type{ 4 };
				else
					*values.last = value_type{ 4 };
				++this->values.last;
				return *(values.last - 1ll);
			}
			else [[likely]]
			{
				if (std::is_constant_evaluated() == false)
					new (this->values.last) value_type{ 4 };
				else
					*values.last = value_type{ 4 };
				++this->values.last;
				return *(values.last - 1ll);
			}
		};

		template <typename... Types>
		inline constexpr void emplace_back(std::tuple<Types...> args) noexcept
		{
#ifdef _DEBUG
			static_assert(sizeof(value_type) == sizeof(args), "mismatch in size");
#endif
			if (this->needs_resize()) [[unlikely]]
				this->emplace_resize<Types...>(std::forward<Types>(args)...);
			else [[likely]]
			{
				new (this->values.last) value_type{ std::forward<Types>(args)... };
				++this->values.last;
			}
		};

		template <typename... Types>
		inline constexpr value_type& emplace_back(Types&&... args) noexcept
			requires (std::is_move_constructible_v<value_type> == true && std::is_constructible_v<value_type, Types...> == true)
		{
			if (this->needs_resize()) [[unlikely]]
			{
				this->emplace_resize<Types...>(std::forward<Types>(args)...);
				return *(values.last - 1ll);
			}
			else [[likely]]
			{
				if (std::is_constant_evaluated() == false)
					new (this->values.last) value_type{ std::forward<Types>(args)... };
				else
					*values.last = value_type{ std::forward<Types>(args)... };
				++this->values.last;
				return *(values.last - 1ll);
			}
		};

		template <typename... Types>
		inline constexpr void emplace_back_values(Types... args) noexcept
			requires (std::is_trivially_constructible_v<value_type, Types...> == true)
		{
			if (this->needs_resize()) [[unlikely]]
				this->emplace_resize<Types...>(std::forward<Types>(args)...);
			else [[likely]]
			{
				new (this->values.last) value_type{ std::forward<Types>(args)... };
				++this->values.last;
			}
		};

		template <std::size_t expected_args, typename... Types>
		inline constexpr void emplace_back(std::tuple<Types...> args) noexcept
		{
#ifdef _DEBUG
			static_assert(sizeof(value_type) == sizeof(args), "mismatch in size");
#endif
			if (this->needs_resize()) this->increase_capacity();

			const auto temp = mem::tuple::ReverseTuple(args);
			std::memcpy(&(this->values.first[this->size()]), &temp, sizeof(args));
			this->values.last = this->values.first + (this->size() + (sizeof...(Types) / expected_args));
		};

		template <std::size_t expected_args, mem::mem_tuple_layout tuple_layout = mem::mem_tuple_layout::reverse, typename... Types>
		inline constexpr void emplace_back(Types&&... args) noexcept
		{
			const std::size_t min_size{ (sizeof...(args) / expected_args) };
			if (this->needs_resize(min_size)) this->increase_capacity(min_size);

			const std::tuple<Types...> temp{ std::forward<Types>(args)... };
			if constexpr (mem::mem_tuple_layout::reverse == tuple_layout)
			{
				const auto temp2 = mem::tuple::ReverseTuple(temp);
				std::memcpy(&(this->values.first[this->size()]), &temp2, sizeof(temp2));
			}
			else
			{
				std::memcpy(&(this->values.first[this->size()]), &temp, sizeof(temp));
			}

			this->values.last = this->values.first + (this->size() + min_size);
		};

		template <std::size_t expected_args, mem::mem_tuple_layout tuple_layout = mem::mem_tuple_layout::reverse, typename... Types>
		inline constexpr void emplace_back_values(Types... args) noexcept
		{
			const std::size_t min_size{ (sizeof...(args) / expected_args) };
			if (this->needs_resize(min_size)) this->increase_capacity(min_size);

			const std::tuple<Types...> temp{ std::forward<Types>(args)... };
			if constexpr (mem::mem_tuple_layout::reverse == tuple_layout)
			{
				const auto temp2 = mem::tuple::ReverseTuple(temp);
				std::memcpy(&(this->values.first[this->size_]), &temp2, sizeof(temp2));
			}
			else
			{
				std::memcpy(&(this->values.first[this->size_]), &temp, sizeof(temp));
			}
			this->values.last = this->values.first + (this->size() + min_size);
		};

	private:
		constexpr mem::vector<long long> validate_iterators_get_indexes(std::vector<iterator*>& iterators) const noexcept
		{
			mem::vector<long long> iterator_indexes(3);

			for (iterator* iterator : iterators)
			{
				if (iterator->operator->() >= this->values.first && iterator->operator->() <= this->values.end)
				{
					const auto ptr_diff = (iterator->operator->() - this->values.first);
					iterator_indexes.push_back(static_cast<long long>(ptr_diff));
				}
				else iterator_indexes.push_back(-1ll);
			}

			return iterator_indexes;
		};

		constexpr void validate_iterators_from_indexes(const mem::vector<long long>& indexes, std::vector<iterator*>& iterators)
		{
			std::size_t loop_index{ 0 };
			for (iterator* loop_iterator : iterators)
			{
				if (indexes.operator[](loop_index) != -1) *loop_iterator = iterator{ this->values.first + indexes.operator[](loop_index) };
				++loop_index;
			}
		};

	public:
		__forceinline constexpr iterator emplace(iterator iter_position) noexcept
			requires (std::is_move_constructible_v<value_type> == true && std::is_default_constructible_v<value_type> == true)
		{
			const std::size_t min_size = this->usize() + 1ull;
			if (this->needs_resize(min_size)) this->increase_capacity(static_cast<long long>(min_size));

			if (std::is_constant_evaluated() == false)
			{
				if constexpr (mem::concepts::get_is_trivially_copyable_v<value_type> == true)
				{
					const iterator tmp_start = iterator{ iter_position + 1ll };
					if (tmp_start != values.last)
					{
						iterator _end = last();
						const std::size_t count = _end - iter_position;
						std::memmove(tmp_start.operator->(), iter_position.operator->(), sizeof(value_type) * count);
					}
					else *values.last = *iter_position;

					this->values.last = this->values.last + 1ll;
					iter_position.operator*() = value_type{ };
					return iterator{ values.first + (iter_position - values.first) };
				}
			}

			if (size() > 0ll)
			{
				iterator _start = iterator{ last() };
				iterator _begin = iterator{ last() - 1ll };
				const iterator _end = iter_position;

				while (_begin > _end)
				{
					new (_start.operator->()) value_type{ std::move(*_begin) };
					//(*_start) = std::move(*_begin);
					--_start;
					--_begin;
				}

				iter_position.operator*() = value_type{ };
				this->values.last = this->values.last + 1ll;
				return iterator{ iter_position };
			}

			return end();
		};
		template <typename... Types>
		__forceinline constexpr iterator emplace(iterator iter_position, Types&&... args) noexcept
			requires (std::is_move_constructible_v<value_type> == true && std::is_constructible_v<value_type, Types...> == true)
		{
			const long long min_size = this->size() + 1ll;

			if (this->needs_resize(min_size))
			{
				//iterator tmp_start = iterator{ iter_position + 1ll };
				//iterator _end = last();
				//std::vector<iterator*> temp_v{ &tmp_start, &iter_position, &_end };
				//const auto iterator_indexes = this->validate_iterators_get_indexes(temp_v);

				this->increase_capacity(static_cast<long long>(min_size));
				//this->validate_iterators_from_indexes(iterator_indexes, temp_v);
			}

			if (std::is_constant_evaluated() == false)
			{
				if constexpr (mem::concepts::get_is_trivially_copyable_v<value_type> == true)
				{
					const iterator tmp_start = iterator{ iter_position + 1ll };
					if (tmp_start != values.last)
					{
						const iterator _end = last();
						const std::size_t count = _end - iter_position;
						std::memmove(tmp_start.operator->(), iter_position.operator->(), sizeof(value_type) * count);
					}
					else *values.last = *iter_position;

					this->values.last = this->values.last + 1ll;
					iter_position.operator*() = value_type{ std::forward<Types>(args)... };
					return iterator{ values.first + (iter_position - values.first) };
				}
			}

			if (size() > 0ll)
			{
				iterator _start = iterator{ last() };
				iterator _begin = iterator{ last() - 1ll };
				const iterator _end = iter_position;

				while (_begin > _end)
				{
					(*_start) = std::move(*_begin);
					--_start;
					--_begin;
				}

				iter_position.operator*() = value_type{ std::forward<Types>(args)... };
				this->values.last = this->values.last + 1ll;
				return iterator{ iter_position };
			}

			return end();
		};

		inline constexpr iterator insert(iterator insert_position, value_type insert_value) noexcept
		{
			const std::size_t min_size = this->usize() + 1;
			if (this->needs_resize(min_size))
			{
				auto tmp_iter_index = insert_position - values.first;
				this->increase_capacity(static_cast<long long>(min_size));
				insert_position = iterator{ values.first + tmp_iter_index };
			}

			if constexpr (std::is_trivially_copyable_v<value_type> == true) //mem::concepts::get_is_trivially_copyable_v<value_type> == true)
			{
				if (std::is_constant_evaluated() == false)
				{
					/**values.last = *insert_position;
					*insert_position = insert_value;
					++values.last;*/
					if (insert_position == values.last)
					{
						new (values.last) value_type{ insert_value };
						//*values.last = insert_value;
						++values.last;
					}
					else
					{
						const auto right_count = values.last - insert_position;
						std::memmove(insert_position.operator->() + 1ll, insert_position.operator->(), sizeof(value_type) * right_count);
						new (insert_position.operator->()) value_type{ insert_value };
						++values.last;
						//*insert_position = insert_value;
						//++values.last;
					}

					return insert_position;
				}
			}

			/**values.last = *insert_position;
			*insert_position = insert_value;
			++values.last;*/

			//are we not inserting at the end, move right side by count
			if (insert_position == values.last)
			{
				new (values.last) value_type{ insert_value };
				//*values.last = insert_value;
				++values.last;
			}
			else
			{
				if (values.last != values.first)
				{
					iterator tmp_start = iterator{ values.last - 1ll };
					iterator new_right_start = iterator{ values.last };

					while (insert_position <= tmp_start)
					{
						new (new_right_start.operator->()) value_type{ *tmp_start };
						//(*new_right_start) = std::move(*tmp_start);
						--new_right_start;
						--tmp_start;
					}
					++values.last;
				}

				/*iterator tmp_start = iterator{insert_position};
				iterator new_right_start{ insert_position + 1ll };
				auto right_count = values.last - insert_position;

				for (long long i = 0; i < right_count; ++i)
				{
					(*new_right_start) = std::move(*tmp_start);
					++new_right_start;
					++tmp_start;
				}*/

				new (insert_position.operator->()) value_type{ insert_value };
				//*insert_position = insert_value;
				//++values.last;
			}

			return insert_position;
		};

		inline constexpr void insert(iterator start, iterator begin, iterator end) noexcept
		{
			std::size_t count = end - begin;
			const std::size_t min_size = this->usize() + count;

			if (this->needs_resize(min_size))
			{
				std::vector<iterator*> temp_v{ &start, &begin, &end };
				const auto iterator_indexes = this->validate_iterators_get_indexes(temp_v);

				this->increase_capacity(static_cast<long long>(min_size));

				this->validate_iterators_from_indexes(iterator_indexes, temp_v);
			}

			if constexpr (mem::concepts::get_is_trivially_copyable_v<value_type> == true)
			{
				if (std::is_constant_evaluated() == false)
				{
					const iterator tmp_start{ start };
					const iterator new_right_start{ this->values.last };// + (count + 1)};
					const auto right_size = this->values.last - start;
					if (right_size > 0)
						std::memcpy(new_right_start.operator->(), tmp_start.operator->(), sizeof(value_type) * right_size);
					std::memcpy(start.operator->(), begin.operator->(), sizeof(value_type) * count);
					this->values.last = this->values.last + count;
					return;
				}
			}

			//are we not inserting at the end, move right side by count
			if (start.operator->() <= this->values.last)
			{
				iterator tmp_start = iterator{ start };
				iterator new_right_start{ this->values.last };
				const difference_type right_size{ this->values.last - start };

				for (long long i = 0; i < right_size; ++i)
				{
					new (new_right_start.operator->()) value_type{ std::move(*tmp_start) };
					//(*new_right_start) = std::move(*tmp_start);
					++new_right_start;
					++tmp_start;
				}
			}

			while (begin != end)
			{
				(*start) = std::move(*begin);
				++start;
				++begin;
			}

			this->values.last = this->values.last + count;
		};

		template<typename iter_begin, typename iter_end>
		inline constexpr void insert(iterator start, iter_begin begin, iter_end end) noexcept
		{
			std::size_t count = end - begin;
			const std::size_t min_size = this->usize() + count;

			if (this->needs_resize(min_size))
			{
				const auto start_index = this->values.end - start;
				std::size_t begin_index{ -1 };
				std::size_t end_index{ -1 };

				if (values.begin > begin && values.end > begin) //begin is from this and will be invalidated
					begin_index = this->values.end - begin;

				if (values.begin > end && values.end > end) //end is from this and will be invalidated
					end_index = this->values.end - end;

				this->increase_capacity(min_size);

				start = iterator{ values.start + start_index };

				if (begin_index != -1) //begin is from this and will be invalidated
					begin = iterator{ values.start + begin_index };

				if (end_index != -1) //begin is from this and will be invalidated
					end = iterator{ values.start + end_index };
			}

			if constexpr (mem::concepts::get_is_trivially_copyable_v<value_type> == true)
			{
				iterator tmp_start = iterator{ start };
				iterator new_right_start = this->values.last;
				const auto right_size = this->values.last - start;
				if (right_size > 0)
					std::memcpy(new_right_start.operator->(), tmp_start.operator->(), sizeof(value_type) * right_size);
				std::memcpy(start.operator->(), begin.operator->(), sizeof(value_type) * count);
				this->values.last = this->values.last + count;
			}
			else
			{
				//are we not inserting at the end, move right side by count
				if (start <= this->values.last)
				{
					iterator tmp_start = iterator{ start };
					iterator new_right_start = this->values.last;
					const auto right_size = this->values.last - start;

					for (long long i = 0; i < right_size; ++i)
					{
						new (new_right_start.operator->()) value_type{ std::move(*tmp_start) };
						//(*new_right_start) = std::move(*tmp_start);
						++new_right_start;
						++tmp_start;
					}
				}

				while (begin != end)
				{
					(*start) = std::move(*begin);
					++start;
					++begin;
				}

				this->values.last = this->values.last + count;
			}
		};

		constexpr iterator remove_unsafe(iterator start) noexcept
		{
			if constexpr (mem::concepts::get_is_trivially_copyable_v<value_type> == true)
			{
				const iterator first = (start + 1);
				std::memcpy(start.operator->(), first.operator->(), sizeof(value_type) * static_cast<std::ptrdiff_t>(this->values.last - first)); //shift right side by one
				_Alty MemoryAllocator{};
				_Alty_traits::destroy(MemoryAllocator, this->values.last);
			}
			else
			{
				iterator first = start + 1;
				const iterator end = this->end();
				while (first != end)
				{
					*start = std::move(*first);
					++first;
					++start;
				}
				_Alty MemoryAllocator{};
				_Alty_traits::destroy(MemoryAllocator, this->values.last);
			}

			--this->values.last;
			return start;
		};
		constexpr iterator erase(iterator start, const iterator end) noexcept
			requires(mem::concepts::get_is_trivially_copyable_v<value_type> == true)
		{
			std::size_t count{ 0 };

			iterator first = start;
			const iterator _end = this->end();
			const _Alty MemoryAllocator{};
			while (first != end)
			{
				if constexpr (std::is_trivially_destructible_v<Object> == false)
				{
					(*(first)).~Object();
					//MemoryAllocator.deallocate(first.operator->());
					//_Alty_traits::destroy(MemoryAllocator, first.operator->());
				}
				++first;
				++count;
			}

			this->values.last = this->values.last - count;
			return iterator{ this->values.last };
		};
		constexpr iterator erase(iterator start, const iterator end) noexcept
			requires(mem::concepts::get_is_trivially_copyable_v<value_type> == false)
		{
			iterator first = start;
			const iterator _end = this->end();
			_Alty MemoryAllocator{};
			if constexpr (std::is_trivially_destructible_v<Object> == false)
			{
				while (first != end)
				{
					_Alty_traits::destroy(MemoryAllocator, first.operator->());
					++first;
				}
			}
			else
			{
				while (first != end)
				{
					++first;
				}
			}
			this->values.last = start.operator->();
			return start;
		};

		constexpr void remove_unsafe(const std::size_t index) noexcept
		{
			if (this->usize() < index) return;

			if constexpr (mem::concepts::get_is_trivially_copyable_v<value_type> == true)
			{
				const auto start = this->begin() + index;
				const auto first = start + 1;
				const auto end = this->end();
				(*(start)).~Object();
				std::memcpy(start.operator->(), first.operator->(), sizeof(value_type) * static_cast<std::size_t>(end - first)); //shift right side by one
			}
			else
			{
				auto start = this->begin() + index;
				auto first = start + 1;
				const auto end = this->end();
				const auto count = end - first;

				while (first != end)
				{
					*start = std::move(*first);
					++first;
					++start;
				}

				std::destroy_at(&(end - 1));
			}

			--this->values.last;
		};

		constexpr void remove(const std::size_t index) noexcept
		{
			if (this->usize() < index) return;

			//const std::size_t right_size{ this->usize() - (index == 0 ? 1 : index) };
			const std::size_t left_size{ index - 1 < this->usize() && index - 1 >= 0 ? index - 1 : 0 };

			if (left_size < 0) return;

			std::destroy_at(&this->values.first[index]);

			if (!std::is_constant_evaluated())
			{
				if constexpr (mem::concepts::get_is_trivially_copyable_v<value_type> == true)
				{
					const auto start = this->begin() + index;
					const auto first = start + 1;
					const auto end = this->end();
					std::memcpy(start.operator->(), first.operator->(), sizeof(value_type) * static_cast<std::size_t>(end - first)); //shift right side by one
					//std::memcpy(&this->values.first[left_size], &this->values.first[left_size + 1], sizeof(value_type) * right_size); //shift right side by one
					--this->values.last;
					return;
				}
			}

			for (std::size_t i = left_size + 1, newLocation = left_size; i < this->usize(); ++i)
			{
				this->values.first[newLocation] = std::move(this->values.first[i]);
				++newLocation;
			}
			--this->values.last;
		};

		constexpr void clear() noexcept
		{
			if (this->size() == 0 || this->values.first == nullptr || this->values.last < this->values.first)
				return;

			for (long long i = 0; i < this->size(); ++i)
			{
				std::destroy_at(&this->values.first[i]);
			}
			this->values.last = this->values.first;
		};
	};

	namespace utility
	{
		template<typename vector_type, typename pointer_container>
		constexpr static inline auto validate_pointers_get_indexes(const vector_type& vector, pointer_container& pointers) noexcept
		{
			mem::vector<mem::vector<long long>> pointer_indexes(pointers.size());
			for (auto iterator = pointers.begin(); iterator != pointers.last(); ++iterator)
			{
				auto& second_vector = *iterator;
				auto& last_inserted = pointer_indexes.emplace_back();
				for (auto iterator_second = second_vector.begin(); iterator_second != second_vector.last(); ++iterator_second)
				{
					if ((*iterator_second) >= vector.values.first && (*iterator_second) <= vector.values.end)
					{
						const auto ptr_diff = ((*iterator_second) - vector.values.first);
						last_inserted.push_back(static_cast<long long>(ptr_diff));
					}
					else last_inserted.push_back(-1ll);
				}
			}

			return pointer_indexes;
		};
		template<typename vector_type, typename pointer_container>
		constexpr static inline void validate_pointers_from_indexes(const vector_type& vector, const mem::vector<mem::vector<long long>>& indexes, pointer_container& pointers)
		{
			size_t loop_index{ 0 };
			for (auto loop_iterator = pointers.begin(); loop_iterator != pointers.last(); ++loop_iterator)
			{
				size_t loop_index_2 = 0;
				for (auto iterator_second = (*loop_iterator).begin(); iterator_second != (*loop_iterator).last(); ++iterator_second)
				{
					if (indexes.operator[](loop_index).operator[](loop_index_2) != ((*iterator_second) - vector.values.first)) (*iterator_second) = (vector.values.first + indexes.operator[](loop_index).operator[](loop_index_2));
					++loop_index_2;
				}

				++loop_index;
			}
		};

		template<typename vector_type, typename iterator_container>
		constexpr static inline mem::vector<long long> validate_iterators_get_indexes(const vector_type& vector, iterator_container& iterators) noexcept
		{
			mem::vector<long long> iterator_indexes(iterators.size());

			for (auto iterator = iterators.begin(); iterator != iterators.last(); ++iterator)
			{
				if ((*iterator)->operator->() >= vector.values.first && (*iterator)->operator->() <= vector.values.end)
				{
					const auto ptr_diff = ((*iterator)->operator->() - vector.values.first);
					iterator_indexes.push_back(static_cast<long long>(ptr_diff));
				}
				else iterator_indexes.push_back(-1ll);
			}

			return iterator_indexes;
		};

		template<typename vector_type, typename iterator_container>
		constexpr static inline void validate_iterators_from_indexes(const vector_type& vector, const mem::vector<long long>& indexes, iterator_container& iterators)
		{
			std::size_t loop_index{ 0 };
			for (auto loop_iterator = iterators.begin(); loop_iterator != iterators.last(); ++loop_iterator)
			{
				if (indexes.operator[](loop_index) != -1) *(*loop_iterator) = (vector.begin() + indexes.operator[](loop_index));
				++loop_index;
			}
		};

		template<typename... args>
		static inline size_t get_iterators_sizes_from() noexcept
		{
			return (sizeof(typename args::iterator) + ...);
		};

		template<typename source_type>
		static inline void naive_memcpy(char* dest, source_type source) noexcept
		{
			const char* source_ptr = (const char*)&source;
			constexpr size_t l = sizeof(source_type);
			for (size_t i = 0; i < l; ++i)
				dest[i] = source_ptr[i];
		};
		static inline void naive_memcpy(char* dest, char* source, size_t n) noexcept
		{
			for (size_t i = 0; i < n; ++i)
				dest[i] = source[i];
		};

		template<typename first>
		static inline auto get_iterators_from(const first& f) noexcept
		{
			constexpr long long first_size = static_cast<long long>(sizeof(first::iterator));
			mem::vector<char> byte_iterators{ first_size };
			naive_memcpy((char*)&byte_iterators[0], (size_t) & (*f.begin()));
			byte_iterators.values.last += first_size;
			return byte_iterators;
		};
		template<typename first, typename... args>
		static inline auto get_iterators_from(const first& f, const args&... urgs) noexcept
		{
			constexpr long long first_size = static_cast<long long>(sizeof(first::iterator));
			mem::vector<char> byte_iterators{ first_size + static_cast<long long>(get_iterators_sizes_from<args...>()) };
			naive_memcpy((char*)&byte_iterators[0], (size_t) & (*f.begin()));
			size_t index = 0;
			(naive_memcpy((char*)&byte_iterators[index += (sizeof(typename args::iterator))], (size_t) & (*urgs.begin())), ...);
			byte_iterators.values.last += first_size + static_cast<long long>(get_iterators_sizes_from<args...>());
			return byte_iterators;
		};

		template<typename iter>
		static inline auto bytes_to_iterator(mem::vector<char>::iterator start, mem::vector<char>::iterator end) noexcept
		{
			iter tmp{ nullptr };
			naive_memcpy((char*)&tmp, (char*)&(*start), end - start);
			return tmp;
		};

		constexpr static auto test_iterator_validators() noexcept
		{
			mem::vector<int> cont_a{ 4 };
			for (int i = 0, l = 32; i < l; ++i)
			{
				cont_a.emplace_back(i);
			}

			auto start_iter = cont_a.begin();
			auto iter_5 = cont_a.begin() + 4;
			auto iter_10 = cont_a.begin() + 9;
			auto iter_24 = cont_a.begin() + 23;

			mem::vector<mem::vector<int>::iterator*> iterator_cont{ 12 };
			iterator_cont.emplace_back(&start_iter);
			iterator_cont.emplace_back(&iter_5);
			iterator_cont.emplace_back(&iter_10);
			iterator_cont.emplace_back(&iter_24);

			mem::vector<long long> iterator_indexes{ 2 };

			for (int i = 0, l = 32; i < l; ++i)
			{
				const bool cont_resize = cont_a.needs_resize();
				if (cont_resize)
				{
					iterator_indexes = mem::utility::validate_iterators_get_indexes(cont_a, iterator_cont);
				}
				cont_a.emplace_back(i);
				if (cont_resize)
				{
					mem::utility::validate_iterators_from_indexes(cont_a, iterator_indexes, iterator_cont);

					auto begin_index = iterator_indexes.begin();
					for (auto begin_ = iterator_cont.begin(); begin_ != iterator_cont.last(); ++begin_, ++begin_index)
					{
						if (cont_a[*begin_index] != *(*(*begin_))) return false;
					}
				}
			}

			return true;
		};
		static_assert(test_iterator_validators() == true, "iterator validating didn't work");

		static bool test_weird_iterator_memcpy() noexcept
		{
			mem::vector<int> cont_a{ 4 };
			cont_a.emplace_back(25);
			mem::vector<long long> iterator_indexes{ 2 };
			iterator_indexes.emplace_back(1);
			mem::vector<mem::vector<int>::iterator*> iterator_cont{ 12 };
			auto cont_a_iter_test = cont_a.begin();
			auto test_p = &(*cont_a_iter_test);
			iterator_cont.emplace_back(&cont_a_iter_test);

			auto iters = get_iterators_from(cont_a, iterator_indexes, iterator_cont);

			auto byte_iter_begin = iters.begin();
			auto byte_iter_last = byte_iter_begin + sizeof(mem::vector<int>::iterator);
			auto cont_a_iter = bytes_to_iterator<mem::vector<int>::iterator>(byte_iter_begin, byte_iter_last);
			byte_iter_begin = byte_iter_last;
			byte_iter_last += sizeof(mem::vector<long long>::iterator);
			auto iterator_indexes_iter = bytes_to_iterator<mem::vector<long long>::iterator>(byte_iter_begin, byte_iter_last);
			byte_iter_begin = byte_iter_last;
			byte_iter_last += sizeof(mem::vector<mem::vector<int>::iterator*>::iterator);
			auto iterator_cont_iter = bytes_to_iterator<mem::vector<mem::vector<int>::iterator*>::iterator>(byte_iter_begin, byte_iter_last);

			if (cont_a_iter_test != cont_a_iter || iterator_indexes.begin() != iterator_indexes_iter || iterator_cont.begin() != iterator_cont_iter)
				return false;

			return true;
		};
	};

	static_assert(std::is_same_v<decltype(mem::vector<int>{}.begin()), decltype(mem::iterator<mem::vector<int>::scary_val>{}) > == true, "no");

	CONSTEXPR_VAR auto test_mem_vector_back() noexcept
	{
		mem::vector<int> vec;
		vec.push_back(0);
		vec.push_back(1);
		vec.push_back(2);
		vec.push_back(3);
		vec.push_back(4);
		vec.push_back(5);

		return vec.back();
	};
#ifdef CONSTEXPR_ASSERTS
	static_assert(test_mem_vector_back() == 5, "no");
#endif

	template<typename iter_type>
	constexpr void update_goal_pointer_iters(iter_type iter, iter_type last_iter, long long offset) noexcept
	{
		while (iter != last_iter)
		{
			(*iter).offset_ptr(offset);
			++iter;
		}
	};

	template <typename vector_type, class __vector, typename delete_vector_type, typename vector_goal_pointer_type, typename vector_distances_between>
	constexpr delete_vector_type erase_indices(__vector& data, std::vector<delete_vector_type>& delete_iterators, vector_goal_pointer_type& goal_vector, vector_distances_between& distances_between_vector) noexcept
	{
		auto indice_iter{ delete_iterators.begin() };
		typename __vector::iterator writer_iter = indice_iter->item_groups_iter;
		typename __vector::iterator reader_iter{ writer_iter + 1 };
		const typename __vector::iterator last_iter{ data.last() };
		const typename __vector::iterator first_iter{ data.begin() };

		auto data_writer_iter = indice_iter->item_groups_data_iter;
		auto data_reader_iter{ data_writer_iter + 1 };

		auto dist_writer_iter = indice_iter->item_groups_dist_iter;
		auto dist_reader_iter{ dist_writer_iter + 1 };

		auto goal_dist_writer_iter = indice_iter->item_groups_goal_dist_iter;
		auto prev_dist_iter = dist_writer_iter;

		if (writer_iter != first_iter) prev_dist_iter = prev_dist_iter - 1;
		//*goal_dist_writer_iter = prev_dist_iter.operator->();
		//++goal_dist_writer_iter;
		//update_goal_pointer_iters(goal_dist_writer_iter, goal_vector);
		long long indice_iter_index = 0ll;
		if (writer_iter != last_iter)
		{
			if (data.size() > 256ll)
			{
				while (reader_iter != last_iter && indice_iter != delete_iterators.end())
				{
					long long initial_count = 0ll;
					if (indice_iter != delete_iterators.end())
					{
						if (delete_iterators.size() > 1)
						{
							if (indice_iter + 1 != delete_iterators.end()) initial_count = (((indice_iter + 1)->item_groups_iter - 1ll) - (indice_iter->item_groups_iter + 1ll));
							else initial_count = data.last() - (indice_iter->item_groups_iter);
						}
						else initial_count = (indice_iter->item_groups_iter) - reader_iter;
					}
					else initial_count = last_iter - reader_iter;
					if (initial_count > 0ll)
					{
						if (*goal_dist_writer_iter == distances_between_vector.begin().operator->())
							(*goal_dist_writer_iter).set_index_ptr(nullptr);
						else if ((goal_dist_writer_iter - goal_vector.begin()) > 0ll)
						{
							auto previous_goal_iter = goal_dist_writer_iter - 1ll;
							auto previous_goal_iter_ptr = (*previous_goal_iter).get_index_ptr();
							if (previous_goal_iter_ptr == nullptr || previous_goal_iter_ptr == ((*goal_dist_writer_iter).get_index_ptr() - 1ll))
								(*goal_dist_writer_iter).set_index_ptr(nullptr);
						}
						if ((*goal_dist_writer_iter).get_index_ptr() != nullptr)
							(*goal_dist_writer_iter).update_pointer_and_values(distances_between_vector.begin().operator->(), indice_iter_index + 1ll);

						if constexpr ((mem::Allocating_Type::ALIGNED_NEW == __vector::use_allocating_type || mem::Allocating_Type::ALIGNED_MALLOC == __vector::use_allocating_type) &&
							(sizeof(vector_type) == 32ll || sizeof(vector_type) == 16ll) || sizeof(vector_type) == 8ll || sizeof(vector_type) == 4ll)
						{
							mem::simd::mem_copy_256<vector_type, false>((void*)(writer_iter.operator->()), (void*)reader_iter.operator->(), initial_count);
							mem::simd::mem_copy_256<decltype(delete_vector_type::item_groups_data_iter)::value_type, true>((void*)(data_writer_iter.operator->()), (void*)data_reader_iter.operator->(), initial_count);
							mem::simd::mem_copy_256<decltype(delete_vector_type::item_groups_dist_iter)::value_type, false>((void*)(dist_writer_iter.operator->()), (void*)dist_reader_iter.operator->(), initial_count);

							auto offset = ((indice_iter + 1ll) != delete_iterators.end() ? 1ll : -1ll);

							reader_iter += initial_count + offset + 1ll;
							writer_iter += initial_count + offset;
							data_reader_iter += initial_count + offset + 1ll;
							data_writer_iter += initial_count + offset;
							dist_reader_iter += initial_count + offset + 1ll;
							dist_writer_iter += initial_count + offset;
						}
						else
						{
							//mem::pre_fetch_cachelines<vector_type, 4ll>(reader_iter.operator->());
							const auto loops = expr::divide_with_remainder(initial_count, 4ll);
							//const auto is_ptr_aligned = mem::is_aligned<vector_type, 16>(writer_iter.operator->());

							for (long long i = loops.div, i_loop = 0ll; i > 0ll; --i)
							{
								if constexpr (mem::use_memcpy::force_checks_off == __vector::use_memcpy_value)
								{
									SIMDMemCopy256<vector_type, 16ull, 4ull>((void*)(writer_iter.operator->()), (void*)reader_iter.operator->(), mem::Allocating_Type::ALIGNED_NEW == __vector::use_allocating_type);
									SIMDMemCopy256<vector_type, 16ull, 4ull>((void*)(data_writer_iter.operator->()), (void*)data_reader_iter.operator->(), mem::Allocating_Type::ALIGNED_NEW == __vector::use_allocating_type);
									//std::memcpy(writer_iter.operator->() + 1, reader_iter.operator->() + 1, sizeof(vector_type));
									//std::memcpy(writer_iter.operator->() + 2, reader_iter.operator->() + 2, sizeof(vector_type));
									//std::memcpy(writer_iter.operator->() + 3, reader_iter.operator->() + 3, sizeof(vector_type));
									//auto fetch_ptr = (reader_iter.operator->() + 3ll);
									//mem::pre_fetch_cachelines<vector_type, 4ll>(fetch_ptr + 4ll);
								}
								else
								{
									//pre_fetch_cachelines<vector_type, 1ll>(reader_iter.operator->());
									*writer_iter = std::move(*reader_iter);
									//pre_fetch_cachelines<vector_type, 1ll>(reader_iter.operator->() + 1ll);
									*(writer_iter + 1) = std::move(*(reader_iter + 1));
									//pre_fetch_cachelines<vector_type, 1ll>(reader_iter.operator->() + 2ll);
									*(writer_iter + 2) = std::move(*(reader_iter + 2));
									//pre_fetch_cachelines<vector_type, 1ll>(reader_iter.operator->() + 3ll);
									*(writer_iter + 3) = std::move(*(reader_iter + 3));

									*data_writer_iter = std::move(*data_reader_iter);
									//pre_fetch_cachelines<vector_type, 1ll>(reader_iter.operator->() + 1ll);
									*(data_writer_iter + 1) = std::move(*(data_reader_iter + 1));
									//pre_fetch_cachelines<vector_type, 1ll>(reader_iter.operator->() + 2ll);
									*(data_writer_iter + 2) = std::move(*(data_reader_iter + 2));
									//pre_fetch_cachelines<vector_type, 1ll>(reader_iter.operator->() + 3ll);
									*(data_writer_iter + 3) = std::move(*(data_reader_iter + 3));
									if (std::is_constant_evaluated() == false)
									{
										auto fetch_ptr = (reader_iter.operator->() + 3ll);
										mem::pre_fetch_cachelines<vector_type, 4ll>(fetch_ptr + 4ll);
										auto data_fetch_ptr = (data_reader_iter.operator->() + 3ll);
										mem::pre_fetch_cachelines<vector_type, 4ll>(data_fetch_ptr + 4ll);
										//_mm_prefetch((char const*)(fetch_ptr), _MM_HINT_NTA);
										//_mm_prefetch((char const*)(fetch_ptr + 1ll), _MM_HINT_NTA);
										//_mm_prefetch((char const*)(fetch_ptr + 2ll), _MM_HINT_NTA);
										//_mm_prefetch((char const*)(fetch_ptr + 3ll), _MM_HINT_NTA);
									}
								}
								writer_iter += 4ll;
								reader_iter += 4ll;
								data_writer_iter += 4ll;
								data_reader_iter += 4ll;
							}
							for (long long i = loops.div * 4ll, l = i + loops.rem; i < l; ++i)
							{
								*writer_iter = std::move(*reader_iter);
								*data_writer_iter = std::move(*data_reader_iter);
								++writer_iter;
								++reader_iter;
								++data_writer_iter;
								++data_reader_iter;
							}
						}
					}
					if (indice_iter != delete_iterators.end())
					{
						/*if (writer_iter != first_iter) prev_dist_iter = (dist_writer_iter - 1ll);
						(*goal_dist_writer_iter).set_index_ptr(prev_dist_iter.operator->());
						(*goal_dist_writer_iter).offset_goal_distance(*(*goal_dist_writer_iter).get_index_ptr());
						++goal_dist_writer_iter;
						update_goal_pointer_iters(goal_dist_writer_iter, goal_vector);*/

						if (indice_iter + 1 != delete_iterators.end())
						{
							++indice_iter;
							++goal_dist_writer_iter;
							++indice_iter_index;
							update_goal_pointer_iters(goal_dist_writer_iter, indice_iter->item_groups_goal_dist_iter, indice_iter_index);
							goal_dist_writer_iter = indice_iter->item_groups_goal_dist_iter;
						}
						else
						{
							indice_iter = delete_iterators.end();
							++goal_dist_writer_iter;
							++indice_iter_index;
							update_goal_pointer_iters(goal_dist_writer_iter, goal_vector.last(), indice_iter_index);
						}
					}
					if (initial_count == 0ll)
					{
						++reader_iter;
						++data_reader_iter;
						++dist_reader_iter;
					}
				}
			}
			else
			{
				for (; reader_iter != last_iter; ++reader_iter, ++data_reader_iter, ++dist_reader_iter)
				{
					if (indice_iter->item_groups_iter != reader_iter)
					{
						*writer_iter = std::move(*reader_iter);
						*data_writer_iter = std::move(*data_reader_iter);
						*dist_writer_iter = std::move(*dist_reader_iter);
						++writer_iter;
						++data_writer_iter;
						++dist_writer_iter;
					}
					else
					{
						if (indice_iter + 1 != delete_iterators.end())
						{
							if (writer_iter != first_iter) prev_dist_iter = (dist_writer_iter - 1ll);
							(*goal_dist_writer_iter).set_index_ptr(prev_dist_iter.operator->());
							(*goal_dist_writer_iter).subtract_goal_distance(*(*goal_dist_writer_iter).get_index_ptr());
							++indice_iter;
							++goal_dist_writer_iter;
							++indice_iter_index;
							update_goal_pointer_iters(goal_dist_writer_iter, indice_iter->item_groups_goal_dist_iter, indice_iter_index);
							goal_dist_writer_iter = indice_iter->item_groups_goal_dist_iter;
						}
					}
				}
			}
		}
		return delete_vector_type{ writer_iter, data_writer_iter, dist_writer_iter };
	};
	template <typename vector_type>
	constexpr inline auto erase_indices(vector_type& data, std::vector<std::size_t>& indicesToDelete) noexcept
	{
		auto indice_iter{ indicesToDelete.begin() };
		auto writer_iter{ data.begin() + (*indice_iter) };
		auto reader_iter{ writer_iter };
		const auto last_iter{ data.last() };

		std::size_t reader_index{ (*indice_iter) };
		if (writer_iter != last_iter)
		{
			for (; reader_iter != last_iter; ++reader_index, ++reader_iter)
			{
				if ((*indice_iter) != reader_index)
				{
					*writer_iter = std::move(*reader_iter);
					++writer_iter;
				}
				else
				{
					if (indice_iter + 1 != indicesToDelete.end()) ++indice_iter;
				}
			}
		}
		return writer_iter;
	};
	template <typename vector_type>
	constexpr inline auto erase_array_indices(vector_type data, std::size_t n, std::vector<std::size_t>& indicesToDelete) noexcept
	{
		auto indice_iter{ indicesToDelete.begin() };
		vector_type writer_iter{ data + (*indice_iter) };
		vector_type reader_iter{ writer_iter };
		vector_type last_iter{ data + n };

		std::size_t reader_index{ (*indice_iter) };
		if (writer_iter != last_iter)
		{
			for (; reader_index < n; ++reader_index, ++reader_iter)
			{
				if ((*indice_iter) != reader_index)
				{
					*writer_iter = *reader_iter;
					++writer_iter;
				}
				else
				{
					if (indice_iter + 1 != indicesToDelete.end())
						++indice_iter;
				}
			}
		}
		return writer_iter;
	};

	template <typename value_type, mem::Allocating_Type allocating_type>
	constexpr static void* _allocate_ptr(const std::size_t count)
	{
		if constexpr (mem::Allocating_Type::MALLOC == allocating_type)
		{
			auto const pv = malloc(count);
#ifdef ENABLE_CPP_EXCEPTION_THROW
			if (!pv) throw std::bad_alloc();
#endif
			return pv;
		}
		else
		{
			auto const ptr = ::operator new[](count, std::nothrow);
#ifdef ENABLE_CPP_EXCEPTION_THROW
			if (!ptr) throw std::bad_alloc();
#else
			if (!ptr) [[unlikely]] std::terminate();
#endif

			return ptr;
		}
	};
	template <typename value_type>
	constexpr static auto allocate_ptr(const std::size_t count)
	{
#ifdef ENABLE_CPP_EXCEPTION_THROW
		constexpr std::size_t bad_arr_length{ (std::numeric_limits<std::size_t>::max)() / sizeof(value_type) };
		if (std::is_constant_evaluated() == false && (count == 0 || count > bad_arr_length))
			throw std::bad_array_new_length();
#endif
		if (std::is_constant_evaluated()) return ::new value_type[count]{};
		return static_cast<value_type*>(mem::_allocate_ptr<value_type, mem::Allocating_Type::NEW>(sizeof(value_type) * count));
	};

	template <typename value_type, mem::Allocating_Type allocating_type = mem::Allocating_Type::NEW>
	constexpr static void deallocate_ptr(value_type* const memory_block, std::size_t n = 0) noexcept //, const std::size_t new_size
	{
		// if (new_size == 0) return;
		if (std::is_constant_evaluated())
		{
			delete[] memory_block;
			return;
		}

		if constexpr (mem::Allocating_Type::MALLOC == allocating_type)
		{
			free(memory_block);
		}
		else
		{
			if (n != 0) operator delete[](memory_block, n);
			else operator delete[](memory_block);
		}
	};

	template <typename value_type>
	constexpr static auto reallocate_ptr(value_type*& oldContainer, value_type*& current_container, std::size_t& c, std::size_t& n) noexcept
	{
		if (std::is_constant_evaluated() == false)
		{
			if constexpr (std::is_trivially_move_constructible_v<value_type> == true)
			{
				const std::size_t old_max_cap = n;
				std::size_t new_size = n > 0ull ? n * 2ull : 2ull;
				current_container = mem::allocate_ptr<value_type>(new_size);

				if (c > 0ull) std::memcpy(current_container, oldContainer, sizeof(value_type) * c);
				n = new_size;
				if (oldContainer && old_max_cap > 0ull) mem::deallocate_ptr<value_type>(oldContainer);

				return current_container;
			}
		}

		const std::size_t old_max_cap = n;
		std::size_t new_size = n > 0ull ? n * 2ull : 2ull;
		current_container = mem::allocate_ptr<value_type>(new_size);

		for (long long i = 0; i < static_cast<long long>(c); ++i)
		{
			current_container[i] = std::move(oldContainer[i]);
		}
		n = new_size;
		if (oldContainer && old_max_cap > 0ull) mem::deallocate_ptr<value_type>(oldContainer);

		return current_container;
	};
	template <typename return_type>
	constexpr static auto emplace_resize_ptr(return_type*& ptr, std::size_t& c, std::size_t& m) noexcept
	{
		if (ptr)
		{
			return_type* oldContainer = ptr;
			return mem::reallocate_ptr<return_type>(oldContainer, ptr, c, m);
		}
		else
		{
			m = m > 0ull ? m * 2ull : 2ull;
			return mem::allocate_ptr<return_type>(m);
		}
	};
	template <typename return_type, typename... Types>
	constexpr static void emplace_back_ptr(return_type*& ptr, std::size_t& c, std::size_t& m, Types &&...args) noexcept
	{
		if (c >= m)
		{
			ptr = mem::emplace_resize_ptr<return_type>(ptr, c, m);

			if (std::is_constant_evaluated() == false) new (ptr + c) return_type{ std::forward<Types>(args)... };
			else ptr[c] = return_type{ std::forward<Types>(args)... };
			++c;
		}
		else
		{
			if (std::is_constant_evaluated() == false) new (ptr + c) return_type{ std::forward<Types>(args)... };
			else ptr[c] = return_type{ std::forward<Types>(args)... };
			++c;
		}
	};
};

namespace mem
{
	CONSTEXPR_VAR int validate_mem_iterator()
	{
		using _Alty = typename std::allocator_traits<mem::allocator<int>>::template rebind_alloc<int>;
		using scary_val = mem::vector_value<std::conditional_t<mem::concepts::is_simple_alloc_v<_Alty>, mem::simple_types<int>, mem::vector_iterator_types<int, long long, std::ptrdiff_t, int*, const int*, int&, const int&>>>;
		int int_arr[10]{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		mem::iterator<scary_val> iter_a{ &int_arr[1] };
		mem::iterator<scary_val> iter_b{ &int_arr[5] };
		constexpr std::ptrdiff_t n{ 5ll - 1ll };
		constexpr std::ptrdiff_t x{ 1 };
		constexpr std::ptrdiff_t y{ 3 };

		auto tmp_iter_a = iter_a;
		auto tmp_iter_b = iter_b;
		auto tmp_iter_b_2 = iter_b;

		if ((iter_a + n) != iter_b) return -1;
		if (std::addressof(tmp_iter_a += n) != std::addressof(tmp_iter_a)) return -2;
		if ((iter_b + -n) != (iter_b - n)) return -3;
		tmp_iter_a = iter_a;
		if ((tmp_iter_a + n) != (tmp_iter_a += n)) return -4;
		tmp_iter_a = iter_a;
		if ((tmp_iter_a + n) != (n + tmp_iter_a)) return -5;
		tmp_iter_a = iter_a;
		if ((tmp_iter_a + (x + y)) != ((tmp_iter_a + x) + y)) return -6;
		tmp_iter_a = iter_a;
		if ((tmp_iter_a + 0) != tmp_iter_a) return -7;
		tmp_iter_b = iter_b;
		if ((iter_a + (n - 1)) != --tmp_iter_b) return -8;
		tmp_iter_b = iter_b;
		tmp_iter_b_2 = iter_b;
		if ((tmp_iter_b += -n) != iter_a && (tmp_iter_b_2 -= n) != iter_a) return -9;
		tmp_iter_b = iter_b;
		if (std::addressof(tmp_iter_b -= n) != std::addressof(tmp_iter_b)) return -10;
		tmp_iter_b = iter_b;
		tmp_iter_b_2 = iter_b;
		if ((tmp_iter_b - n) != (tmp_iter_b_2 -= n)) return -11;
		if (*iter_b != iter_a[n]) return -12;
		if (!(iter_a <= iter_b)) return -13;

		return 1;
	};
#ifdef CONSTEXPR_ASSERTS
	constexpr int validate_iterator_value = validate_mem_iterator();
	static_assert(validate_mem_iterator() == 1, "non working iterators");
#endif

	CONSTEXPR_VAR std::size_t TestMemVectorConstexpr() noexcept
	{
		/*std::vector<Rect, mem::allocator<Rect>> test_custom_allocator;
		test_custom_allocator.push_back(Rect{});
		test_custom_allocator.push_back(Rect{});
		test_custom_allocator.push_back(Rect{});*/
		mem::vector<long long> iterator_indexes_1{ 3 };

		//mem::vector<std::vector<std::string>> vector{ 3 };

		//static_assert(sizeof(vector.begin()) == 8);

		//mem::vector<long long> test_capacity = mem::vector<long long>(static_cast<long long>(test_custom_allocator.size()));
		mem::vector<long long> test_capacity = mem::vector<long long>(3ll);

		if (test_capacity.get_capacity() != 3)
			return 0;

		test_capacity.push_back(0ll);
		test_capacity.push_back(1ll);
		test_capacity.push_back(2ll);

		if (test_capacity[0ll] + test_capacity[1ll] + test_capacity[2ll] == 3)
			return 3;

		return test_capacity.get_capacity();// test_custom_allocator.size();// vector.get_ucapacity();
	};
#ifdef CONSTEXPR_ASSERTS
	constexpr std::size_t test_value = TestMemVectorConstexpr();


	static_assert(TestMemVectorConstexpr() == 3ull);
	static_assert(std::is_copy_constructible_v<mem::vector<long long>>);
	static_assert(std::is_copy_assignable_v<mem::vector<long long>>);
#endif

	CONSTEXPR_VAR auto test_insert() noexcept
	{
		mem::vector<long long> iterator_indexes_1{ 25 };
		iterator_indexes_1.emplace_back(15ll);
		iterator_indexes_1.emplace_back(250ll);
		iterator_indexes_1.emplace(iterator_indexes_1.begin(), -250ll);

		return iterator_indexes_1[1ll] == -250ll;
	};
#ifdef CONSTEXPR_ASSERTS
	static_assert(test_insert() == true, "no");
#endif

	CONSTEXPR_VAR auto test_mem_erase_indices()
	{
		mem::vector<int> v{ 5 };
		v.emplace_back(0);
		v.emplace_back(1);
		v.emplace_back(2);
		v.emplace_back(3);
		v.emplace_back(4);
		std::vector<std::size_t> is{ 2, 4 };
		if (v[is[0]] != 2) return -15ll;
		if (v[is[1]] != 4) return -16ll;
		const auto erase_iter = mem::erase_indices(v, is);
		const auto new_last = v.erase(erase_iter, v.end());
		//if (new_last != v.begin() + 3ll) return -17ll;
		auto tmp_iter = v.begin();
		auto tmp_iter1 = tmp_iter + 1ll;
		auto tmp_iter2 = tmp_iter + 2ll;

		if (*tmp_iter != 0) return -1ll;
		if (*tmp_iter1 != 1) return -2ll;
		if (*tmp_iter2 != 3) return -3ll;
		if (v[0] != 0) return -1ll;
		if (v[1] != 1) return -2ll;
		if (v[2] != 3) return -3ll;

		return v.size();
	};
#ifdef CONSTEXPR_ASSERTS
	constexpr auto mem_erase_indices_val = test_mem_erase_indices();
	static_assert(test_mem_erase_indices() == 3, "erased incorrectly");
#endif

	CONSTEXPR_VAR auto test_vector()
	{
		int* v = nullptr;
		std::size_t c_l = 0;
		std::size_t m_l = 0;

		mem::emplace_back_ptr<int>(v, c_l, m_l, 0);
		mem::emplace_back_ptr<int>(v, c_l, m_l, 1);
		mem::emplace_back_ptr<int>(v, c_l, m_l, 2);
		mem::emplace_back_ptr<int>(v, c_l, m_l, 3);
		mem::emplace_back_ptr<int>(v, c_l, m_l, 4);

		std::vector<std::size_t> is{ 2, 4 };
		mem::erase_array_indices(v, 5, is); //auto erase_iter = mem::erase_array_indices(v, 5, is);
		//v.erase(erase_iter, v.end());
		std::destroy_at(&v[3]);
		std::destroy_at(&v[4]);

		long long return_val = 3ll;
		if (v[0] != 0) return_val = -1ll;
		if (v[1] != 1) return_val = -2ll;
		if (v[2] != 3) return_val = -3ll;

		mem::deallocate_ptr<int, mem::Allocating_Type::NEW>(v);
		return return_val;
	};
#ifdef CONSTEXPR_ASSERTS
	constexpr auto vector_erase_indices_val = test_vector();
	static_assert(test_vector() == 3ll, "no");
#endif

	CONSTEXPR_VAR auto test_mem_vector_empty(int insert_count) noexcept
	{
		mem::vector<int> test{ 4 };
		for (int i = 0; i < insert_count; ++i) test.emplace_back(i);
		return test.empty();
	};
#ifdef CONSTEXPR_ASSERTS
	static_assert(test_mem_vector_empty(0) == true, "vector is not empty");
	static_assert(!test_mem_vector_empty(4) == true, "vector is not empty");
#endif


	CONSTEXPR_VAR auto test_mem_vector_begin_last(int insert_count)
	{
		mem::vector<int> test{ 4 };
		for (int i = 0; i < insert_count; ++i) test.emplace_back(i);

		int calc = 0;
		auto begin_iter = test.begin();
		const auto last_iter = test.last();
		while (begin_iter != last_iter)
		{
			calc += *begin_iter;
			++begin_iter;
		}

		return calc;
	};
#ifdef CONSTEXPR_ASSERTS
	static_assert(test_mem_vector_begin_last(0) == 0, "vector is not empty");
	static_assert(test_mem_vector_begin_last(4) == 6, "vector is not empty");
#endif
	struct non_trivial
	{
		int i{ 0 };
		non_trivial() noexcept {};
		non_trivial(int _i) noexcept : i{ _i }
		{};
		~non_trivial() noexcept {};
	};
	CONSTEXPR_VAR auto test_mem_vector_specific_insert(int insert_count)
	{
		mem::vector<non_trivial> test{ 4 };
		for (int i = 0; i < insert_count; ++i) test.emplace_back(i);

		mem::vector<non_trivial> dest{ test.size() * 3 };
		for (int i = 30; i < insert_count * 2; ++i)
			dest.emplace_back(i);

		const auto begin_iter = test.begin();
		const auto last_iter = test.last();
		dest.insert(dest.begin(), begin_iter, last_iter);

		int calc = 0;
		auto dest_iter = dest.begin();
		while (dest_iter != dest.last())
		{
			calc += (*dest_iter).i;
			++dest_iter;
		}

		return calc;
	};
#ifdef CONSTEXPR_ASSERTS
	static_assert(test_mem_vector_begin_last(0) == 0, "vector is not empty");
	static_assert(test_mem_vector_begin_last(4) == 6, "vector is not empty");
#endif

	struct ice_struct
	{
		mem::iterator<mem::vector_value<mem::simple_types<int>>> first{ nullptr };
		mem::iterator<mem::vector_value<mem::simple_types<int>>> second{ nullptr };
	};
	CONSTEXPR_VAR auto test_mem_vector_ice(int insert_count) noexcept
	{
		mem::vector<int> ice{ 4 };
		mem::vector<ice_struct> test{ 4 };
		auto field = &ice_struct::first;
		for (int i = 0; i < insert_count; ++i) ice.emplace_back(i);
		for (int i = 0; i < insert_count; ++i) test.emplace_back((ice.begin() + i), (ice.begin() + i));

		int calc = 0;
		if (!test.empty())
		{
			auto begin_iter = test.begin();
			const auto last_iter = test.last();

			//auto& field_1 = begin_iter.operator*();
			auto field_val = *(begin_iter.GetValue().*field);
			calc += field_val;
			++begin_iter;
		}
		return calc;
	};
#ifdef CONSTEXPR_ASSERTS
	static_assert(test_mem_vector_ice(0) == 0, "vector is not empty");
	static_assert(test_mem_vector_ice(4) == 0, "vector is not empty");
#endif
	struct is_trivial
	{
		char i{ 0 };
		constexpr is_trivial() noexcept {};
		constexpr is_trivial(char _i) noexcept :
			i{ _i }
		{};
		constexpr ~is_trivial() noexcept {};
		constexpr operator char() const noexcept
		{
			return i;
		};
	};
	template<typename type>
	static void __declspec(noinline) hidden_copy(type* lhs, const type& rhs) noexcept
	{
		volatile void* const vid = lhs;
		type* ptr = (type*)vid;
		*ptr = rhs;
	};
	static auto __declspec(noinline) test_different_templates_mem() noexcept
	{
		constexpr long long capacity = 111111111;
		mem::vector<is_trivial, mem::Allocating_Type::ALIGNED_NEW, mem::allocator<is_trivial, mem::Allocating_Type::ALIGNED_NEW>, mem::use_memcpy::force_checks_off> v_1{ capacity, mem::expected_size_t };
		for (size_t i = 0; i < capacity; ++i)
			v_1[i] = i;
		//v_1.values.last += capacity;

		decltype(v_1) copy;
		hidden_copy(&copy, v_1);

		return *(copy.values.last - 1ll);
	};
	static auto __declspec(noinline) test_different_templates_std() noexcept
	{
		constexpr size_t capacity = 111111111;
		std::vector<is_trivial> v_1;
		v_1.resize(capacity);
		for (size_t i = 0; i < capacity; ++i)
			v_1[i] = i;

		decltype(v_1) copy;
		hidden_copy(&copy, v_1);

		return *(copy.end() - 1ull);
	};
	static auto __declspec(noinline) test_mem_vector_speed(const size_t r_i) noexcept
	{
		constexpr long long size{ 111'111'111 };
		mem::vector<is_trivial, mem::Allocating_Type::ALIGNED_NEW, mem::allocator<is_trivial, mem::Allocating_Type::ALIGNED_NEW>> v{ size, mem::expected_size_t };
		for (long long i = 0; i < size; ++i)
			v[i] = i;

		decltype(v) v2;
		hidden_copy(&v2, v);
		return v2[r_i];
	};
	static auto __declspec(noinline) test_std_vector_speed(const size_t r_i) noexcept
	{
		constexpr long long size{ 111'111'111 };
		std::vector<is_trivial> v;
		v.resize(size);
		for (size_t i = 0; i < size; ++i)
			v[i] = i;

		std::vector<is_trivial> v2;
		hidden_copy(&v2, v);
		return v2[r_i];
	};
};