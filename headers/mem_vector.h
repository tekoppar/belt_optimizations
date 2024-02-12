#pragma once

#include <ppl.h>

#include <array>
#include <cassert>
#include <cstring>
#include <iterator>
#include <memory>
#include <new>
#include <stdexcept>
#include <stdlib.h>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <xmemory>
#include <corecrt_malloc.h>

#include "macros.h"
#include "math_utility.h"
#include "mem_utilities.h"

#include "mem_vector_concepts.h"
#include "type_conversion.h"

#include "simd_memcpy.h"

#include "item_32.h"

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

	enum Allocating_Type
	{
		NEW,
		CONCURRENCY,
		MALLOC,
		ALIGNED_NEW,
		ALIGNED_MALLOC
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
		using reference = value_type&;

		constexpr iterator() = default;
		constexpr iterator(pointer ptr) noexcept :
			m_ptr(ptr)
		{};

		[[nodiscard]] constexpr reference operator*() const noexcept
		{
			return *this->m_ptr;
		};
		[[nodiscard]] constexpr reference operator*(iterator iter) const noexcept
		{
			return *iter.m_ptr;
		};
		[[nodiscard]] constexpr pointer operator->() const noexcept
		{
			return this->m_ptr;
		};
		//pointer operator&() const noexcept = delete;

		[[nodiscard]] constexpr reference operator[](const std::size_t& index) const noexcept
		{
			return *(this->m_ptr + index);
		};
		[[nodiscard]] constexpr reference operator[](const difference_type& index) const noexcept
		{
			return *(this->m_ptr + index);
		};

		constexpr iterator& operator++() noexcept
		{
			++this->m_ptr;
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
			this->m_ptr += offset;
			return *this;
		};
		constexpr iterator operator+(const difference_type& offset) noexcept
		{
			iterator tmp = *this;
			tmp.m_ptr += offset;
			return tmp;
		};
		constexpr friend iterator operator+(const iterator& lhs, const difference_type& offset) noexcept
		{
			iterator tmp = lhs;
			tmp.m_ptr += offset;
			return tmp;
		};
		constexpr friend iterator operator+(const difference_type& offset, const iterator& lhs) noexcept
		{
			return lhs + offset;
		};

		constexpr iterator& operator--() noexcept
		{
			--this->m_ptr;
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
			this->m_ptr -= offset;
			return *this;
		};
		constexpr iterator operator-(const difference_type& offset) noexcept
		{
			iterator tmp = *this;
			tmp.m_ptr -= offset;
			return tmp;
		};
		constexpr friend iterator operator-(const iterator& lhs, const difference_type& offset) noexcept
		{
			iterator tmp = lhs;
			tmp.m_ptr -= offset;
			return tmp;
		};
		constexpr friend iterator operator-(const difference_type& offset, const iterator& lhs) noexcept
		{
			return lhs - offset;
		};

		constexpr friend difference_type operator+(const iterator& lhs, const iterator& rhs)
		{
			return lhs.m_ptr + rhs.m_ptr;
		};
		constexpr friend difference_type operator-(const iterator& lhs, const iterator& rhs)
		{
			return lhs.m_ptr - rhs.m_ptr;
		};

		inline constexpr friend bool operator==(const iterator& lhs, const iterator& rhs)
		{
			return lhs.m_ptr == rhs.m_ptr;
		};
		inline constexpr friend bool operator!=(const iterator& lhs, const iterator& rhs)
		{
			return lhs.m_ptr != rhs.m_ptr;
		};
		constexpr friend bool operator<(const iterator& lhs, const iterator& rhs)
		{
			return lhs.m_ptr < rhs.m_ptr;
		};
		constexpr friend bool operator>(const iterator& lhs, const iterator& rhs)
		{
			return rhs.m_ptr < lhs.m_ptr;
		};
		constexpr friend bool operator>=(const iterator& lhs, const iterator& rhs)
		{
			return lhs.m_ptr >= rhs.m_ptr;
		};
		constexpr friend bool operator<=(const iterator& lhs, const iterator& rhs)
		{
			return lhs.m_ptr <= rhs.m_ptr;
		};
		auto operator<=>(const iterator&) const = default;

		constexpr auto GetValueType() const noexcept
		{
			return value_type{};
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
		__declspec(allocator) [[nodiscard]] constexpr void* _allocate(const std::size_t count) const
		{
			if constexpr (mem::Allocating_Type::MALLOC == allocating_type)
			{
				auto const pv = malloc(count);
#ifdef _DEBUG
				if (!pv) throw std::bad_alloc();
#endif
				return pv;
			}
			else if constexpr (mem::Allocating_Type::CONCURRENCY == allocating_type)
			{
				auto const ptr = concurrency::Alloc(count);
				if (!ptr) [[unlikely]] throw std::bad_alloc();

				return ptr;
			}
			else if constexpr (mem::Allocating_Type::NEW == allocating_type)
			{
				auto const ptr = ::operator new[](count, std::nothrow);
				if (!ptr) [[unlikely]] throw std::bad_alloc();

				return ptr;
			}
			else if constexpr (mem::Allocating_Type::ALIGNED_MALLOC == allocating_type)
			{
				constexpr auto closest_alignmnet = mem::get_closest_alignmnet<value_type>();
				auto const pv = _aligned_malloc(count, closest_alignmnet);
#ifdef _DEBUG
				if (!pv) throw std::bad_alloc();
#endif
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
				constexpr auto closest_alignmnet = mem::get_closest_alignmnet<value_type>();
				auto const ptr = ::operator new[](count, std::align_val_t{ closest_alignmnet }, std::nothrow);
				//if (!mem::is_aligned<T, closest_alignmnet>(ptr)) throw std::bad_alloc();
				if (!ptr) [[unlikely]] throw std::bad_alloc();

				return ptr;
			}
		};
	public:
		__declspec(allocator) [[nodiscard]] constexpr value_type* allocate(const std::size_t count) const
		{
#ifdef _DEBUG
			if (count == 0 || count > bad_arr_length) throw std::bad_array_new_length();
#endif
			if (std::is_constant_evaluated()) return ::new Object[count]{};
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
				constexpr auto closest_alignmnet = mem::get_closest_alignmnet<value_type>();
				if (n != 0) operator delete[](memory_block, n, std::align_val_t{ closest_alignmnet });
				else operator delete[](memory_block, std::align_val_t{ closest_alignmnet });
			}
		};
	};

	template <class _Value_type, class _Size_type, class _Difference_type, class _Pointer, class _Const_pointer, class _Reference, class _Const_reference>
	struct vector_iterator_types
	{
		using value_type = _Value_type;
		using size_type = _Size_type;
		using difference_type = _Difference_type;
		using pointer = _Pointer;
		using const_pointer = _Const_pointer;
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
		using reference = value_type&;
		using const_reference = const value_type&;

		alignas(8) pointer first
		{
			nullptr
		}; // pointer to beginning of array
		alignas(8) pointer last
		{
			nullptr
		}; // pointer to current end of sequence
		alignas(8) pointer end
		{
			nullptr
		}; // pointer to end of array

		constexpr vector_value() = default;
		constexpr vector_value(pointer first_, size_type capacity) noexcept :
			first{ first_ },
			last{ first_ },
			end{ first_ + capacity }
		{
			//assert(((unsigned long long)first_) + (sizeof(value_type) * capacity) == ((unsigned long long)end));
		};
		constexpr vector_value(pointer first_, pointer last_, pointer end_) noexcept :
			first{ first_ },
			last{ last_ },
			end{ end_ }
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

		constexpr inline difference_type size() const noexcept
		{
			return last - first;//  this->size_;
		};
		constexpr inline std::size_t usize() const noexcept
		{
			return static_cast<std::size_t>(last - first);// this->size_;
		};
		constexpr inline difference_type get_capacity() const noexcept
		{
			return end - first;
		};
		constexpr inline std::size_t get_ucapacity() const noexcept
		{
			return static_cast<std::size_t>(end - first);
		};
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

		_Alty MemoryAllocator;

		static_assert(std::is_same_v<_Alty, mem::allocator<Object, allocating_type>>);

	public:
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

	private:
		using scary_val = mem::vector_value<std::conditional_t<mem::concepts::is_simple_alloc_v<_Alty>, mem::simple_types<Object>,
			mem::vector_iterator_types<Object, size_type, difference_type, pointer, const_pointer, Object&, const Object&>>>;

	public:
		using iterator = mem::iterator<scary_val>;
		static_assert(std::random_access_iterator<iterator>);

		scary_val values{ nullptr, nullptr, nullptr };

		inline constexpr vector() = default;
		inline constexpr vector(long long capacity) noexcept : //requires(mem::is_allocator_v<Allocator, Object>&& mem::is_allocator_requirements_v<_Alty, Object>)
			values{ this->MemoryAllocator.allocate(static_cast<unsigned long long>(capacity)), static_cast<unsigned long long>(capacity) }
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
						for (iterator iter = begin(); iter != end(); ++iter)
						{
							(*(iter)).~Object();
						}
					}
				}
				this->MemoryAllocator.deallocate(this->values.first);// , this->usize());
			}
		};

		constexpr vector(const vector& o) noexcept
		{
			if (o.values != nullptr && o.size() > 0) this->emplace_resize_vector(o);
		};
		constexpr vector& operator=(const vector& o) noexcept
		{
			if (o.values != nullptr && o.size() > 0) this->emplace_resize_vector(o);

			return *this;
		};
		constexpr vector(vector&& o) noexcept :
			values{ std::exchange(o.values, scary_val{nullptr, nullptr, nullptr}) }
		{};
		constexpr vector& operator=(vector&& o) noexcept
		{
			this->values = std::exchange(o.values, scary_val{ nullptr, nullptr, nullptr });

			return *this;
		};

		template<typename type_size>
		[[nodiscard]] constexpr value_type& operator[](type_size index) noexcept
			requires(std::is_integral_v<type_size>&& std::is_same_v<std::size_t, type_size> == false)
		{
#ifdef _DEBUG
			if (this->size() >= index) return *(this->values.first + index);
			else return *this->values.first;
#else
			return values.first[index];
#endif
		};
		template<typename type_size>
		[[nodiscard]] constexpr value_type& operator[](type_size index) noexcept
			requires(std::is_same_v<std::size_t, type_size>)
		{
#ifdef _DEBUG
			if (this->usize() >= index) return *(this->values.first + index);
			else return *this->values.first;
#else
			return values.first[index];
#endif
		};

		template<typename type_size>
		[[nodiscard]] constexpr const value_type& operator[](type_size index) const noexcept
			requires(std::is_integral_v<type_size>&& std::is_same_v<std::size_t, type_size> == false)
		{
#ifdef _DEBUG
			if (this->size() >= index) return *(this->values.first + index);
			else return *this->values.first;
#else
			return values.first[index];
#endif
		};
		template<typename type_size>
		[[nodiscard]] constexpr const value_type& operator[](type_size index) const noexcept
			requires(std::is_same_v<std::size_t, type_size>)
		{
#ifdef _DEBUG
			if (this->usize() >= index) return *(this->values.first + index);
			else return *this->values.first;
#else
			return values.first[index];
#endif
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
			if (this->values.last == this->values.end)
				return iterator(this->values.last);
			else
				return iterator(this->values.last + 1);
		};

		inline constexpr void reserve(long long size) noexcept
		{
			if (this->get_capacity() < size) this->resize(size);
		};

		inline constexpr void reserve() noexcept
		{
			this->resize(this->get_capacity() * 2);
		};

		inline constexpr bool needs_resize() const noexcept
		{
			return this->values.last >= this->values.end;
		};

		inline constexpr bool needs_resize(long long min_size) const noexcept
		{
			return (this->values.last + min_size) >= (this->values.end - 1);
		};

		inline constexpr void check_increase_capacity() noexcept
		{
			if (this->needs_resize()) this->resize(this->get_capacity() * 2ll);
		};

		inline constexpr void increase_capacity() noexcept
		{
			this->resize(this->get_capacity() * 2ll);
		};

		inline constexpr void increase_capacity(long long min_size) noexcept
		{
			this->resize((this->get_capacity() + min_size) * 2ll);
		};

		inline constexpr void resize(long long newSize) noexcept
			requires (mem::concepts::vector_has_method<value_type> == false)
		{
			scary_val oldContainer{ this->values };
			long long old_size = oldContainer.size();
			this->values = scary_val{ this->MemoryAllocator.allocate(static_cast<std::size_t>(newSize)), static_cast<std::size_t>(newSize) };

			for (long long i = 0; i < old_size; ++i)
			{
				this->values.first[i] = std::move(oldContainer.first[i]);
			}
			this->MemoryAllocator.deallocate(oldContainer.first);
			this->values.last = this->values.first + old_size;
		};

		inline constexpr void resize(long long newSize) noexcept
			requires (mem::concepts::vector_has_method<value_type> == true)
		{
			__ASSUME__(values.first != nullptr);

			if constexpr (mem::concepts::get_is_trivially_copyable_v<value_type> == true)
			{
				const auto old_size = this->size();
				scary_val newContainer = { this->MemoryAllocator.allocate(tc::widen<std::size_t>(newSize)), tc::widen<std::size_t>(newSize) };

				if (values.first + old_size > values.end) throw std::runtime_error("");

				std::memcpy(newContainer.first, this->values.first, sizeof(value_type) * old_size);

				this->values = std::move(newContainer);
				this->values.last = this->values.first + old_size;
			}
			else
			{
				scary_val oldContainer{ this->values };
				this->capacity = newSize;
				this->values = { this->MemoryAllocator.allocate(newSize), static_cast<std::size_t>(newSize) };

				for (long long i = 0; i < oldContainer.size(); ++i)
				{
					this->values.first[i] = std::move(oldContainer.first[i]);
				}

				this->values.last = this->values.first + oldContainer.size();
				this->MemoryAllocator.deallocate(oldContainer.first);
			}
		};


		template <typename... Types>
		inline constexpr void emplace_resize(Types&&... args) noexcept
			requires (std::is_move_constructible_v<value_type> == true && std::is_constructible_v<value_type, Types...> == true)
		{
			__ASSUME__(values.first != nullptr);

			this->reallocate(std::move(this->values));
			new (this->values.last) value_type{ std::forward<Types>(args)... };
			++this->values.last;
		};

		inline constexpr void emplace_resize(const value_type& object) noexcept
		{
			__ASSUME__(values.first != nullptr);

			this->reallocate(std::move(this->values));
			*(this->values.last) = object;
			++this->values.last;
		};

		inline constexpr void emplace_resize(value_type&& object) noexcept
		{
			__ASSUME__(values.first != nullptr);

			this->reallocate(std::move(this->values));
			if constexpr (mem::concepts::get_is_trivially_copyable_v<value_type> == true)
			{
				std::memcpy(this->values.last, &object, sizeof(value_type));
				++this->values.last;

				if constexpr (mem::concepts::vector_has_method<value_type> == true) object.values.first = nullptr;
			}
			else
			{
				*(this->values.last) = std::move(object);
				++this->values.last;
			}
		};

		template<typename vector_other>
		inline constexpr void emplace_resize_vector(const vector_other& vector) noexcept
			requires (mem::concepts::vector_has_method<vector_other> == true)
		{
			__ASSUME__(values.first != nullptr);
			__ASSUME__(values.first != 0);

			if (this->values.first && !this->empty()) this->clear();

			const auto new_capacity = vector.get_capacity();
			if (this->get_capacity() < new_capacity || this->values.first == nullptr)
				this->values = scary_val{ this->MemoryAllocator.allocate(new_capacity), vector.get_ucapacity() };

			if constexpr (mem::concepts::get_is_trivially_copyable_v<value_type> == true)
			{
				std::memcpy(this->values.first, vector.values.first, sizeof(value_type) * vector.size());
				this->values.last = this->values.first + vector.size();
			}
			else
			{
				for (long long i = 0; i < vector.size(); ++i)
				{
					this->values.first[i] = vector.values.first[i];
				}
			}
		};

		template<typename vector_other>
		inline constexpr void emplace_resize_vector(vector_other&& vector) noexcept
			requires (mem::concepts::vector_has_method<vector_other> == true)
		{
			__ASSUME__(values.first != nullptr);

			if (this->values.first && !this->empty()) this->clear();

			if (this->get_capacity() < vector.get_capacity())
				this->values = { this->MemoryAllocator.allocate(vector.get_capacity()), static_cast<std::size_t>(vector.get_capacity()) };

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
					this->values.first[i] = std::move(vector.values.first[i]);
				}
			}

			vector.MemoryAllocator.deallocate(vector.values.first);
			vector.values = nullptr;
		};

		inline constexpr void reallocate(scary_val oldContainer) noexcept
		{
			if constexpr (mem::use_memcpy::force_checks_off == memcpy_check || mem::use_memcpy::check_based_on_type == memcpy_check && mem::concepts::get_is_trivially_copyable_v<value_type> == true)
			{
				auto new_size = oldContainer.get_ucapacity() * 2ull;
				auto old_size = oldContainer.size();
				constexpr const size_t type_size = sizeof(Object);
				std::size_t type_arr_size = type_size * old_size;
				this->values = scary_val{ this->MemoryAllocator.allocate(new_size), new_size };

				if constexpr (mem::Allocating_Type::NEW == allocating_type || mem::Allocating_Type::MALLOC == allocating_type) SIMDMemCopy256(this->values.first, oldContainer.first, DivideByMultiple(type_arr_size, 16));
				else std::memcpy(this->values.first, oldContainer.first, type_arr_size);
				this->MemoryAllocator.deallocate(oldContainer.first);
				this->values.last = this->values.first + old_size;
			}
			else
			{
				auto new_size = oldContainer.get_ucapacity() * 2ull;
				auto old_size = oldContainer.size();
				this->values = scary_val{ this->MemoryAllocator.allocate(new_size), new_size };

				if (old_size > 256ll)
				{
					long long initial_count = old_size;
					auto loops = expr::divide_with_remainder(initial_count, 4ll);
					for (long long i = 0ll, i_loop = 0ll; i < loops.div; ++i)
					{
						if (std::is_constant_evaluated() == false)
						{
							auto tmp = oldContainer.first;
							mem::pre_fetch_cachelines<Object, 4ll, _MM_HINT_NTA>(tmp + (i_loop + 4ll));
							//auto tmp2 = values.first;
							//pre_fetch_cachelines<Object, 4ll>(tmp2 + (i_loop + 4ll));
						}
						//pre_fetch_cachelines<Object, 1ll, _MM_HINT_NTA>(oldContainer.first + (i_loop + 1ll));
						this->values.first[i_loop] = std::move(oldContainer.first[i_loop]);

						//pre_fetch_cachelines<Object, 1ll, _MM_HINT_NTA>(oldContainer.first + (i_loop + 2ll));
						this->values.first[i_loop + 1ll] = std::move(oldContainer.first[i_loop + 1ll]);

						//pre_fetch_cachelines<Object, 1ll, _MM_HINT_NTA>(oldContainer.first + (i_loop + 3ll));
						this->values.first[i_loop + 2ll] = std::move(oldContainer.first[i_loop + 2ll]);

						//pre_fetch_cachelines<Object, 1ll, _MM_HINT_NTA>(oldContainer.first + (i_loop + 4ll));
						this->values.first[i_loop + 3ll] = std::move(oldContainer.first[i_loop + 3ll]);
						i_loop += 4ll;
					}
					for (long long i = loops.div * 4ll, l = i + loops.rem; i < l; ++i)
					{
						this->values.first[i] = std::move(oldContainer.first[i]);
					}
				}
				else
				{
					for (long long i = 0; i < old_size; ++i)
					{
						//if (std::is_constant_evaluated() == false && i + 1 > old_size) _mm_prefetch((char const*)(&oldContainer.first[i + 1]), _MM_HINT_NTA);
						this->values.first[i] = std::move(oldContainer.first[i]);
					}
				}
				this->MemoryAllocator.deallocate(oldContainer.first);
				this->values.last = this->values.first + old_size;
			}
		};

		inline constexpr void push_back(const value_type& val) noexcept
			requires (mem::concepts::vector_has_method<value_type> == false)
		{
			if (this->needs_resize()) [[unlikely]]
				this->emplace_resize(val);
			else [[likely]]
				{
					*(this->values.last) = val;
					++this->values.last;
				}
		};

		inline constexpr void push_back(const mem::vector<value_type, allocating_type>& val) noexcept
			requires (mem::concepts::vector_has_method<value_type> == false)
		{
			if (this->needs_resize(val.size()))
				this->increase_capacity(val.size());

			auto begin_val = val.begin();
			const auto end_val = val.end();

			while (begin_val < end_val)
			{
				*(this->values.last) = *begin_val;
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
					*(this->values.last) = std::move(val);
					++this->values.last;
				}
		};

		inline constexpr void push_back(mem::vector<value_type, allocating_type>&& val) noexcept
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
				*(this->values.last) = val;
				++this->values.last;

				if (this->needs_resize()) this->increase_capacity();
			}
			else
			{
				if (this->needs_resize()) [[unlikely]]
					this->emplace_resize(val);
				else [[likely]]
					{
						*(this->values.last) = val;
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
						*(this->values.last) = std::move(val);
						++this->values.last;
					}

					val.values = nullptr;
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
			std::memmove(&(this->values.first[this->size()]), &temp, sizeof(args));
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
				std::memmove(&(this->values.first[this->size()]), &temp2, sizeof(temp2));
			}
			else
			{
				std::memmove(&(this->values.first[this->size()]), &temp, sizeof(temp));
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
				std::memmove(&(this->values.first[this->size_]), &temp2, sizeof(temp2));
			}
			else
			{
				std::memmove(&(this->values.first[this->size_]), &temp, sizeof(temp));
			}
			this->values.last = this->values.first + (this->size() + min_size);
		};

	private:
		inline constexpr mem::vector<long long> validate_iterators_get_indexes(std::vector<iterator*>& iterators) const noexcept
		{
			mem::vector<long long> iterator_indexes(3);

			for (iterator* iterator : iterators)
			{
				if (*iterator >= this->values.first && *iterator < this->values.end)
				{
					const auto ptr_diff = (iterator->operator->() - this->values.first);
					iterator_indexes.push_back(static_cast<long long>(ptr_diff));
				}
				else iterator_indexes.push_back(-1ll);
			}

			return iterator_indexes;
		};

		inline constexpr void validate_iterators_from_indexes(const mem::vector<long long>& indexes, std::vector<iterator*>& iterators)
		{
			std::size_t loop_index{ 0 };
			for (iterator* loop_iterator : iterators)
			{
				if (indexes.operator[](loop_index) != -1) *loop_iterator = iterator{ this->values.first + indexes[loop_index] };
				++loop_index;
			}
		};

	public:
		template <typename... Types>
		inline constexpr iterator emplace(iterator iter_position, Types&&... args) noexcept
			requires (std::is_move_constructible_v<value_type> == true && std::is_constructible_v<value_type, Types...> == true)
		{
			const std::size_t min_size = this->usize() + 1ull;

			if (this->needs_resize(min_size))
			{
				iterator tmp_start = iterator{ iter_position + 1ll };
				iterator _end = last();
				std::vector<iterator*> temp_v{ &tmp_start, &iter_position, &_end };
				const auto iterator_indexes = this->validate_iterators_get_indexes(temp_v);

				this->increase_capacity(static_cast<long long>(min_size));
				this->validate_iterators_from_indexes(iterator_indexes, temp_v);
			}

			if (std::is_constant_evaluated() == false)
			{
				if constexpr (mem::concepts::get_is_trivially_copyable_v<value_type> == true)
				{
					const iterator tmp_start = iterator{ iter_position + 1ll };
					const iterator _end = last();
					const std::size_t count = iter_position - _end;

					std::memmove(tmp_start.operator->(), iter_position.operator->(), sizeof(value_type) * count);
					this->values.last = this->values.last + 1ll;
					iter_position.operator*() = value_type{ std::forward<Types>(args)... };
					return iterator{ values.first + (values.first - iter_position) };
				}
			}

			iterator _start = iterator{ last() };
			iterator _begin = iterator{ last() - 1ll };
			const iterator _end = iter_position;

			while (_begin != _end)
			{
				(*_start) = std::move(*_begin);
				--_start;
				--_begin;
			}
			*(iter_position + 1ll) = value_type{ std::forward<Types>(args)... };
			this->values.last = this->values.last + 1ll;
			return iterator{ iter_position };
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
				const iterator tmp_start = iterator{ start };
				const iterator new_right_start = this->values.last + (count + 1);
				const auto right_size = this->values.last - start;
				if (right_size > 0)
					std::memmove(new_right_start.operator->(), tmp_start.operator->(), sizeof(value_type) * right_size);
				std::memmove(start.operator->(), begin.operator->(), sizeof(value_type) * count);
				this->values.last = this->values.last + count;
			}
			else
			{
				//are we not inserting at the end, move right side by count
				if (start <= this->values.last)
				{
					iterator tmp_start = iterator{ this->values.last };
					iterator new_right_start = this->values.last + (count + 1);
					const auto right_size = this->values.last - start;

					for (long long i = 0; i < right_size; ++i)
					{
						(*new_right_start) = std::move(*tmp_start);
						++new_right_start;
						--tmp_start;
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

				if (this->values.begin > begin && this->values.end > begin) //begin is from this and will be invalidated
					begin_index = this->values.end - begin;

				if (this->values.begin > end && this->values.end > end) //end is from this and will be invalidated
					end_index = this->values.end - end;

				this->increase_capacity(min_size);

				start = iterator{ this->values.start + start_index };

				if (begin_index != -1) //begin is from this and will be invalidated
					begin = iterator{ this->values.start + begin_index };

				if (end_index != -1) //begin is from this and will be invalidated
					end = iterator{ this->values.start + end_index };
			}

			if constexpr (mem::concepts::get_is_trivially_copyable_v<value_type> == true)
			{
				iterator tmp_start = iterator{ start };
				iterator new_right_start = this->values.last + (count + 1);
				const auto right_size = this->values.last - start;
				std::memcpy(new_right_start.operator->(), tmp_start.operator->(), sizeof(value_type) * right_size);
				std::memcpy(start.operator->(), begin.operator->(), sizeof(value_type) * count);
			}
			else
			{
				//are we not inserting at the end, move right side by count
				if (start <= this->values.last)
				{
					iterator tmp_start = iterator{ this->values.last };
					iterator new_right_start = this->values.last + (count + 1);
					const auto right_size = this->values.last - start;

					for (long long i = 0; i < right_size; ++i)
					{
						(*new_right_start) = std::move(*tmp_start);
						++new_right_start;
						--tmp_start;
					}
				}

				while (begin != end)
				{
					(*start) = std::move(*begin);
					++start;
					++begin;
				}
			}
		};

		constexpr iterator remove_unsafe(iterator start) noexcept
		{
			if constexpr (mem::concepts::get_is_trivially_copyable_v<value_type> == true)
			{
				const iterator first = (start + 1);
				std::memmove(start.operator->(), first.operator->(), sizeof(value_type) * static_cast<std::ptrdiff_t>(this->values.last - first)); //shift right side by one
				_Alty_traits::destroy(this->MemoryAllocator, (this->values.last));
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
				_Alty_traits::destroy(this->MemoryAllocator, this->values.last);
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
			while (first != end)
			{
				if constexpr (std::is_trivially_destructible_v<Object> == false)
					_Alty_traits::destroy(this->MemoryAllocator, first.operator->());
				++first;
				++count;
			}

			this->values.last = this->values.last - count;
			return { this->values.last };
		};
		constexpr iterator erase(iterator start, const iterator end) noexcept
			requires(mem::concepts::get_is_trivially_copyable_v<value_type> == false)
		{
			if constexpr (std::is_trivially_destructible_v<Object> == false)
			{
				iterator first = start;
				const iterator _end = this->end();
				while (first != end)
				{
					_Alty_traits::destroy(this->MemoryAllocator, first.operator->());
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
				std::memmove(start.operator->(), first.operator->(), sizeof(value_type) * static_cast<std::size_t>(end - first)); //shift right side by one
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

			const std::size_t right_size{ this->usize() - (index == 0 ? 1 : index) };
			const std::size_t left_size{ index - 1 < this->usize() && index - 1 >= 0 ? index - 1 : 0 };

			if (left_size < 0) return;

			std::destroy_at(&this->values.first[index]);

			if constexpr (mem::concepts::get_is_trivially_copyable_v<value_type> == true)
				std::memmove(&this->values.first[left_size], &this->values.first[left_size + 1], sizeof(value_type) * right_size); //shift right side by one
			else
			{
				for (std::size_t i = left_size + 1, newLocation = left_size; i < this->usize(); ++i)
				{
					this->values.first[newLocation] = std::move(this->values.first[i]);
					++newLocation;
				}
			}
			--this->values.last;
		};

		constexpr void clear() noexcept
		{
			if (this->size() == 0 || this->values.first == nullptr)
				return;

			for (long long i = 0; i < this->size(); ++i)
			{
				std::destroy_at(&this->values.first[i]);
			}
			this->values.last = this->values.first;
		};
	};

	template <typename vector_type>
	__declspec(noinline) constexpr mem::vector<vector_type>::iterator erase_indices(mem::vector<vector_type>& data, std::vector<typename mem::vector<vector_type>::iterator>& delete_iterators) noexcept
	{
		auto indice_iter{ delete_iterators.begin() };
		typename mem::vector<vector_type>::iterator writer_iter = *indice_iter;
		typename mem::vector<vector_type>::iterator reader_iter{ writer_iter };
		typename mem::vector<vector_type>::iterator last_iter{ data.last() };

		if (writer_iter != last_iter)
		{
			if (data.size() > 256ll)
			{
				while (reader_iter != last_iter)
				{
					const long long initial_count = (indice_iter != delete_iterators.end() ? (*indice_iter) - reader_iter : last_iter - reader_iter);
					if (initial_count > 0ll)
					{
						pre_fetch_cachelines<vector_type, 4ll>(reader_iter.operator->());
						const auto loops = expr::divide_with_remainder(initial_count, 4ll);
						for (long long i = loops.div, i_loop = 0ll; i > 0ll; --i)
						{
							//pre_fetch_cachelines<vector_type, 1ll>(reader_iter.operator->());
							*writer_iter = std::move(*reader_iter);
							//pre_fetch_cachelines<vector_type, 1ll>(reader_iter.operator->() + 1ll);
							*(writer_iter + 1) = std::move(*(reader_iter + 1));
							//pre_fetch_cachelines<vector_type, 1ll>(reader_iter.operator->() + 2ll);
							*(writer_iter + 2) = std::move(*(reader_iter + 2));
							//pre_fetch_cachelines<vector_type, 1ll>(reader_iter.operator->() + 3ll);
							*(writer_iter + 3) = std::move(*(reader_iter + 3));
							if (std::is_constant_evaluated() == false)
							{
								auto fetch_ptr = (reader_iter.operator->() + 3ll);
								pre_fetch_cachelines<vector_type, 4ll>(fetch_ptr + 4ll);
								//_mm_prefetch((char const*)(fetch_ptr), _MM_HINT_NTA);
								//_mm_prefetch((char const*)(fetch_ptr + 1ll), _MM_HINT_NTA);
								//_mm_prefetch((char const*)(fetch_ptr + 2ll), _MM_HINT_NTA);
								//_mm_prefetch((char const*)(fetch_ptr + 3ll), _MM_HINT_NTA);
							}
							writer_iter += 4ll;
							reader_iter += 4ll;
						}
						for (long long i = loops.div * 4ll, l = i + loops.rem; i < l; ++i)
						{
							*writer_iter = std::move(*reader_iter);
							++writer_iter;
							++reader_iter;
						}
					}
					if (indice_iter != delete_iterators.end())
					{
						if (indice_iter + 1 != delete_iterators.end()) ++indice_iter;
						else indice_iter = delete_iterators.end();
					}
				}
			}
			else
			{
				for (; reader_iter != last_iter; ++reader_iter)
				{
					if (*indice_iter != reader_iter)
					{
						*writer_iter = std::move(*reader_iter);
						++writer_iter;
					}
					else
					{
						if (indice_iter + 1 != delete_iterators.end()) ++indice_iter;
					}
				}
			}
		}
		return writer_iter;
	};
	template <typename vector_type>
	constexpr inline auto erase_indices(vector_type& data, std::vector<std::size_t>& indicesToDelete) noexcept
	{
		auto indice_iter{ indicesToDelete.begin() };
		auto writer_iter{ data.begin() + (*indice_iter) };
		auto reader_iter{ writer_iter };
		auto last_iter{ data.last() };

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
#ifdef _DEBUG
			if (!pv) throw std::bad_alloc();
#endif
			return pv;
		}
		else
		{
			auto const ptr = ::operator new[](count, std::nothrow);
			if (!ptr) throw std::bad_alloc();

			return ptr;
		}
		};
	template <typename value_type>
	constexpr static auto allocate_ptr(const std::size_t count)
	{
#ifdef _DEBUG
		constexpr std::size_t bad_arr_length{ std::numeric_limits<std::size_t>::max() / sizeof(value_type) };
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
				std::size_t old_max_cap = n;
				std::size_t new_size = n > 0ull ? n * 2ull : 2ull;
				current_container = mem::allocate_ptr<value_type>(new_size);

				if (c > 0ull) std::memcpy(current_container, oldContainer, sizeof(value_type) * c);
				n = new_size;
				if (oldContainer && old_max_cap > 0ull) mem::deallocate_ptr<value_type>(oldContainer);

				return current_container;
			}
		}

		std::size_t old_max_cap = n;
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

namespace mem {
	constexpr int validate_mem_iterator()
	{
		using _Alty = typename std::allocator_traits<mem::allocator<int>>::template rebind_alloc<int>;
		using scary_val = mem::vector_value<std::conditional_t<mem::concepts::is_simple_alloc_v<_Alty>, mem::simple_types<int>, mem::vector_iterator_types<int, long long, std::ptrdiff_t, int*, const int*, int&, const int&>>>;
		int int_arr[10]{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		mem::iterator<scary_val> iter_a{ &int_arr[1] };
		mem::iterator<scary_val> iter_b{ &int_arr[5] };
		constexpr std::ptrdiff_t n{ 5 - 1 };
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
	constexpr int validate_iterator_value = validate_mem_iterator();
	static_assert(validate_mem_iterator() == 1, "non working iterators");

	static constexpr std::size_t TestMemVectorConstexpr() noexcept
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
	constexpr std::size_t test_value = TestMemVectorConstexpr();

	static_assert(TestMemVectorConstexpr() == 3ull);
	static_assert(std::is_copy_constructible_v<mem::vector<long long>>);
	static_assert(std::is_copy_assignable_v<mem::vector<long long>>);

	constexpr auto test_insert() noexcept
	{
		mem::vector<long long> iterator_indexes_1{ 25 };
		iterator_indexes_1.emplace_back(15ll);
		iterator_indexes_1.emplace_back(250ll);
		iterator_indexes_1.emplace(iterator_indexes_1.begin(), -250ll);

		return iterator_indexes_1[1ll] == -250ll;
	};
	static_assert(test_insert() == true, "no");

	static constexpr auto test_mem_erase_indices()
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
		auto erase_iter = mem::erase_indices(v, is);
		auto new_last = v.erase(erase_iter, v.end());
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
	constexpr auto mem_erase_indices_val = test_mem_erase_indices();
	static_assert(test_mem_erase_indices() == 3, "erased incorrectly");

	constexpr static auto test_vector()
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
		auto erase_iter = mem::erase_array_indices(v, 5, is);
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
	constexpr auto vector_erase_indices_val = test_vector();
	static_assert(test_vector() == 3ll, "no");
};