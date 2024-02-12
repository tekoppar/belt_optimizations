#pragma once

#include <new>
#include <type_traits>
#include <utility>
#include <malloc.h>
#include <Windows.h>
#include <cstring>
#include <memory>
#ifndef _PPL_H
#include <ppl.h>
#endif

#include "const_data.h"
#include "vectors.h"
#include "item.h"

namespace mem
{
	template<class T>
	concept type_has_copy = requires(T * ptr, T val)
	{
		{
			*ptr = val
		};
	};

	template<class T>
	concept type_has_move = requires(T * ptr, T val)
	{
		{
			*ptr = std::move(val)
		};
	};

	template<typename type>
	class single_list_block_node
	{
	public:
		using value_type = type;

		single_list_block_node* prev{ nullptr };
		single_list_block_node* next{ nullptr };
		type object;

		//constexpr void erase() noexcept;
	};

	template<typename type, std::size_t memory_block_size = 4096>
	class memory_block
	{
		static constexpr std::size_t number_of_times = memory_block_size / sizeof(type);

		type* start_of_block{ nullptr };
		type* end_of_block{ nullptr };
		int first_free{ 0 };
		unsigned long long start_of_block_address{ 0 };
		unsigned long long end_of_block_address{ 0 };

	public:
		memory_block() noexcept
		{
			allocate();
		};
		~memory_block() noexcept
		{
			HeapFree(GetProcessHeap(), 0, start_of_block);
		};

	private:
		long long get_ptr_as_index(type* ptr) noexcept
		{
			unsigned long long memory_address = (unsigned long long)ptr;
			long long index = memory_address - start_of_block_address;
			return index / sizeof(type);
		};
	public:
		bool is_ptr_inside(type* ptr) noexcept
		{
			unsigned long long memory_address = (unsigned long long)ptr;
			return memory_address >= start_of_block_address && memory_address < end_of_block_address;
		};
	private:
		__declspec(allocator) void allocate() noexcept
		{
			void* allocation = HeapAlloc(GetProcessHeap(), 0, memory_block_size);
			start_of_block = (type*)allocation;
			start_of_block_address = (unsigned long long)start_of_block;
			end_of_block = (type*)(((unsigned char*)start_of_block) + memory_block_size);
			end_of_block_address = (unsigned long long)end_of_block;

			if ((type*)(((unsigned char*)start_of_block) + memory_block_size) != end_of_block)
				std::terminate();
		};

		bool is_valid_type(type* ptr) noexcept
		{
			unsigned char* next_block = (unsigned char*)(ptr);
			for (int i = 0, l = sizeof(type); i < l; ++i)
			{
				unsigned char value = next_block[i];
				if (value == 0)
				{
					return false;
				}
			}

			return true;
		};
		type* find_next_memory(type* ptr)
		{
			if (is_ptr_inside(ptr) == false)
				return nullptr;

			int count = end_of_block_address - (unsigned long long)ptr;

			for (int i = 0; i < count; ++i)
			{
				if (is_valid_type(ptr) == false)
					return ptr;

				ptr = ptr + 1;
			}
		};

	public:
		__declspec(restrict)type* get_memory() noexcept
		{
			if (first_free >= number_of_times)
				return nullptr;

			type* ptr = start_of_block + first_free;
			++first_free;

			if (is_valid_type(start_of_block + first_free))
			{
				type* next_ptr = find_next_memory(start_of_block + first_free);
				if (next_ptr == nullptr)
					first_free = number_of_times;
				else
				{
					first_free = get_ptr_as_index(next_ptr);
				}
			}

			return ptr;
		};

		bool free_memory(type* ptr) noexcept
		{
			unsigned long long memory_address = (unsigned long long)ptr;
			int index = memory_address - start_of_block_address;
			index = index / sizeof(type);
			if (index < number_of_times)
			{
				ZeroMemory(ptr, sizeof(type));
				first_free = index;
				return true;
			}
			else
			{
				return false;
			}
		};

		bool has_free_memory() noexcept
		{
			return first_free != number_of_times;
		};

		type& get_object(std::size_t index) noexcept
		{
			const int l_index = index * sizeof(type);
			return start_of_block[l_index];
		};
		const type& get_object(std::size_t index) const noexcept
		{
			const int l_index = index * sizeof(type);
			return start_of_block[l_index];
		};
		inline type* get_start_of_block() const noexcept
		{
			return start_of_block;
		};
		inline type* get_last_of_block() const noexcept
		{
			return start_of_block + (first_free - 1);
		};
		inline type* get_end_of_block() const noexcept
		{
			return end_of_block;
		};
	};

	template<typename type, std::size_t memory_block_size = 4096>
	class block_allocator
	{
	public:
		static constexpr std::size_t number_of_times = memory_block_size / sizeof(type);
		static constexpr std::size_t remaining_mem = memory_block_size - (number_of_times * sizeof(type));

	private:
		memory_block<type, memory_block_size>* all_blocks{ nullptr };
		memory_block<type, memory_block_size>* active_block{ nullptr };
		long long block_index{ 0 };
		long long capacity{ 0 };

	public:
		block_allocator() noexcept :
			all_blocks{ internal_allocate() },
			active_block{ &all_blocks[0] },
			capacity{ 4 }
		{};
		~block_allocator() noexcept
		{
			for (int i = 0; i < block_index; ++i)
			{
				all_blocks[i].~memory_block<type, memory_block_size>();
			}
			free(all_blocks);
		};
		block_allocator(const block_allocator& o) noexcept :
			active_block{ o.active_block },
			all_blocks{ o.all_blocks },
			block_index{ o.block_index },
			capacity{ o.capacity }
		{};
		block_allocator(block_allocator&& o) noexcept :
			active_block{ std::move(o.active_block) },
			all_blocks{ std::exchange(o.all_blocks, nullptr) },
			block_index{ std::move(o.block_index) },
			capacity{ std::exchange(o.capacity, 0) }
		{};
		block_allocator& operator=(const block_allocator& o) noexcept
		{
			active_block = o.active_block;
			all_blocks = o.all_blocks;
			block_index = o.block_index;
			capacity = o.capacity;
		};
		block_allocator& operator=(block_allocator&& o) noexcept
		{
			active_block = std::move(o.active_block);
			all_blocks = std::exchange(o.all_blocks, nullptr);
			block_index = std::move(o.block_index);
			capacity = std::exchange(o.capacity, 0);
			return *this;
		};

		memory_block<type, memory_block_size>* internal_allocate() noexcept
		{
			memory_block<type, memory_block_size>* tmp_ptr = (memory_block<type, memory_block_size>*)malloc(sizeof(memory_block<type, memory_block_size>) * 4);
			::new (tmp_ptr) memory_block<type, memory_block_size>{};
			return tmp_ptr;
		};

		void resize() noexcept
		{
			memory_block<type, memory_block_size>* tmp = all_blocks;
			all_blocks = (memory_block<type, memory_block_size>*)malloc(capacity * 4 * sizeof(memory_block<type, memory_block_size>));
			std::memcpy(all_blocks, tmp, sizeof(memory_block<type, memory_block_size>) * capacity);
			capacity = capacity * 4;
			free(tmp);
		};

		type* allocate() noexcept
		{
			if (active_block->has_free_memory() == false)
			{
				if (block_index >= capacity) resize();

				::new (all_blocks + block_index) memory_block<type, memory_block_size>{};
				//all_blocks[block_index] = std::move(memory_block<type, memory_block_size>{});
				active_block = &all_blocks[block_index];
				++block_index;

				return active_block->get_memory();
			}
			else
			{
				type* ptr = active_block->get_memory();
				/*if (ptr == nullptr)
				{
					return allocate();
				}
				else*/
				return ptr;
			}
		};

		void m_free(type* ptr) noexcept
		{
			if (active_block->free_memory(ptr))
				return;
			else
			{
				for (int i = 0; i < capacity; ++i)
				{
					if (all_blocks[i].free_memory(ptr))
						return;
				}
			}
		};

		type& get_object(std::size_t index) noexcept
		{
			int l_block_index = std::floor(index / memory_block_size);
			if (l_block_index < block_index)
				return all_blocks[l_block_index].get_object(index % memory_block_size);
		};
		const type& get_object(std::size_t index) const noexcept
		{
			int l_block_index = std::floor(index / memory_block_size);
			if (l_block_index < block_index)
				return all_blocks[l_block_index].get_object(index % memory_block_size);
		};

		inline void get_block_index(std::size_t index, type* iterator, type* end_of_block) const noexcept
		{
			iterator = all_blocks[index].get_start_of_block();
			end_of_block = all_blocks[index].get_end_of_block();
		};
		inline long long get_block_index_from_ptr(type* ptr) const noexcept
		{
			for (long long i = 0, l = capacity; i < l; ++i)
			{
				if (all_blocks[i].is_ptr_inside(ptr)) return i;
			}

			return -1;
		};
		inline type* get_start_of_block(std::size_t index) const noexcept
		{
			return all_blocks[index].get_start_of_block();
		};
		inline type* get_last_of_block(std::size_t index) const noexcept
		{
			return all_blocks[index].get_last_of_block();
		};
		inline type* get_end_of_block(std::size_t index) const noexcept
		{
			return all_blocks[index].get_end_of_block();
		};
	};

	template<class type, class allocator = block_allocator<type, 4096>>
	class block_iterator
	{
	public:
#ifdef __cpp_lib_concepts
		using iterator_concept = std::random_access_iterator_tag;
#endif // __cpp_lib_concepts
		using iterator_category = std::random_access_iterator_tag;
		using value_type = type::value_type;
		using difference_type = std::ptrdiff_t;

		constexpr block_iterator() noexcept
		{};
		constexpr block_iterator(type* ptr, type* current_end, allocator* allocator_ptr) noexcept :
			m_ptr(ptr),
			m_current_end{ current_end },
			_allocator{ allocator_ptr }
		{};

		constexpr type* m_get(std::size_t index) noexcept
		{
			if (m_ptr + index < m_current_end)
				return m_ptr + index;
			else
			{
				std::size_t block_diff = current_block * allocator::number_of_times;
				++current_block;
				_allocator->get_block_index(current_block, m_ptr, m_current_end);
				m_ptr = m_ptr + (index - block_diff);
				return m_ptr;
			}
		};

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
		[[nodiscard]] constexpr type* operator*(block_iterator iter) const noexcept
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
		//pointer operator&() const noexcept = delete;

		[[nodiscard]] constexpr type* operator[](const std::size_t& index) noexcept
		{
			return *m_get(index);
		};
		[[nodiscard]] constexpr type* operator[](const difference_type& index) noexcept
		{
			return *m_get(index);
		};
		[[nodiscard]] constexpr const type* operator[](const std::size_t& index) const noexcept
		{
			return *m_get(index);
		};
		[[nodiscard]] constexpr const type* operator[](const difference_type& index) const noexcept
		{
			return *m_get(index);
		};

		constexpr block_iterator& operator++() noexcept
		{
			this->m_ptr = m_get(1);
			return *this;
		};
		constexpr block_iterator operator++(int) noexcept
		{
			block_iterator tmp = *this;
			++*this;
			return tmp;
		};
		constexpr block_iterator& operator+=(const difference_type& offset) noexcept
		{
			this->m_ptr = m_get(offset);
			return *this;
		};

		constexpr block_iterator& operator--() noexcept
		{
			this->m_ptr = m_get(-1);
			return *this;
		};
		constexpr block_iterator operator--(int) noexcept
		{
			block_iterator tmp = *this;
			--*this;
			return tmp;
		};
		constexpr block_iterator& operator-=(const difference_type& offset) noexcept
		{
			this->m_ptr = m_get(offset);
			return *this;
		};

		constexpr block_iterator operator+(const difference_type& offset) const noexcept
		{
			block_iterator tmp = *this;
			tmp += offset;
			return tmp;
		};
		constexpr friend block_iterator operator+(const difference_type& rhs, const block_iterator& lhs) noexcept
		{
			return block_iterator{ lhs.m_ptr + rhs };
		};

		constexpr block_iterator operator-(const difference_type& offset) const noexcept
		{
			block_iterator tmp = *this;
			tmp -= offset;
			return tmp;
		};
		constexpr friend block_iterator operator-(const difference_type& rhs, const block_iterator& lhs) noexcept
		{
			return block_iterator{ lhs.m_ptr - rhs };
		};

		constexpr friend difference_type operator+(const block_iterator& lhs, const block_iterator& rhs)
		{
			return lhs.m_ptr + rhs.m_ptr;
		};
		constexpr friend difference_type operator-(const block_iterator& lhs, const block_iterator& rhs)
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
		constexpr friend bool operator==(const block_iterator& lhs, const block_iterator& rhs)
		{
			return lhs.m_ptr == rhs.m_ptr;
		};
		constexpr friend bool operator!=(const block_iterator& lhs, const block_iterator& rhs)
		{
			return lhs.m_ptr != rhs.m_ptr;
		};
		constexpr friend bool operator<(const block_iterator& lhs, const block_iterator& rhs)
		{
			return lhs.m_ptr < rhs.m_ptr;
		};
		constexpr friend bool operator>(const block_iterator& lhs, const block_iterator& rhs)
		{
			return rhs.m_ptr < lhs.m_ptr;
		};
		constexpr friend bool operator>=(const block_iterator& lhs, const block_iterator& rhs)
		{
			return lhs.m_ptr >= rhs.m_ptr;
		};
		constexpr friend bool operator<=(const block_iterator& lhs, const block_iterator& rhs)
		{
			return rhs.m_ptr >= lhs.m_ptr;
		};
		auto operator<=>(const block_iterator&) const = default;

		constexpr auto GetValueType() const noexcept
		{
			return type{};
		};

	private:
		alignas(8) type* m_ptr{ nullptr };
		alignas(8) type* m_current_end
		{
			nullptr
		};
		alignas(8) allocator* _allocator
		{
			nullptr
		};
		alignas(8) std::size_t current_block{ 0 };
	};

	template<class type>
	class b_iterator
	{
	public:
#ifdef __cpp_lib_concepts
		using iterator_concept = std::bidirectional_iterator_tag;
#endif // __cpp_lib_concepts
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = type::value_type;

		constexpr b_iterator() = default;
		constexpr b_iterator(type* ptr) noexcept :
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
		[[nodiscard]] constexpr type* operator*(b_iterator iter) const noexcept
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
		//pointer operator&() const noexcept = delete;

		constexpr b_iterator& operator++() noexcept
		{
			m_ptr = m_ptr->prev;
			return *this;
		};
		constexpr b_iterator operator++(int) noexcept
		{
			b_iterator tmp = *this;
			++*this;
			return tmp;
		};
		constexpr b_iterator& operator--() noexcept
		{
			m_ptr = m_ptr->next;
			return *this;
		};
		constexpr b_iterator operator--(int) noexcept
		{
			b_iterator tmp = *this;
			--*this;
			return tmp;
		};

		constexpr friend std::ptrdiff_t operator+(const b_iterator& lhs, const b_iterator& rhs)
		{
			return lhs.m_ptr + rhs.m_ptr;
		};
		constexpr friend std::ptrdiff_t operator-(const b_iterator& lhs, const b_iterator& rhs)
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
		constexpr friend bool operator==(const b_iterator& lhs, const b_iterator& rhs)
		{
			return lhs.m_ptr == rhs.m_ptr;
		};
		constexpr friend bool operator!=(const b_iterator& lhs, const b_iterator& rhs)
		{
			return lhs.m_ptr != rhs.m_ptr;
		};
		constexpr friend bool operator<(const b_iterator& lhs, const b_iterator& rhs)
		{
			return lhs.m_ptr < rhs.m_ptr;
		};
		constexpr friend bool operator>(const b_iterator& lhs, const b_iterator& rhs)
		{
			return rhs.m_ptr < lhs.m_ptr;
		};
		constexpr friend bool operator>=(const b_iterator& lhs, const b_iterator& rhs)
		{
			return lhs.m_ptr >= rhs.m_ptr;
		};
		constexpr friend bool operator<=(const b_iterator& lhs, const b_iterator& rhs)
		{
			return rhs.m_ptr >= lhs.m_ptr;
		};
		auto operator<=>(const b_iterator&) const = default;

		constexpr auto GetValueType() const noexcept
		{
			return type{};
		};

	private:
		alignas(8) type* m_ptr{ nullptr };
	};

	template<typename type, class Allocator = mem::block_allocator<single_list_block_node<type>, 4096>>
	class single_list_block
	{
	public:
		static inline mem::block_allocator<single_list_block_node<type>, 4096> allocator{};

		single_list_block_node<type>* p_first{ nullptr };
		single_list_block_node<type>* p_last{ nullptr };
	public:
		using iterator = b_iterator<single_list_block_node<type>>;

		inline constexpr single_list_block() = default;
		inline constexpr ~single_list_block() noexcept
		{
			if (p_first)
			{
				single_list_block_node<type>* tmp_ptr = p_first;
				single_list_block_node<type>* nxt_ptr{ nullptr };

				while (tmp_ptr)
				{
					nxt_ptr = tmp_ptr->prev;
					tmp_ptr->object.type::~type();
					//allocator.deallocate(tmp_ptr);
					tmp_ptr = nxt_ptr;
				}
			}
		};

		constexpr single_list_block(const single_list_block& o) noexcept
		{
			p_first = o.p_first;
			p_last = o.p_last;
		};
		constexpr single_list_block& operator=(const single_list_block& o) noexcept
		{
			p_first = o.p_first;
			p_last = o.p_last;

			return *this;
		};
		constexpr single_list_block(single_list_block&& o) noexcept :
			p_first{ std::exchange(o.p_first, nullptr) },
			p_last{ std::exchange(o.p_last, nullptr) }
		{};
		constexpr single_list_block& operator=(single_list_block&& o) noexcept
		{
			p_first = std::exchange(o.p_first, nullptr);
			p_last = std::exchange(o.p_last, nullptr);

			return *this;
		};

	private:
		template<typename type>
		__forceinline constexpr single_list_block_node<type>* get_from_index(type index) noexcept
		{
			type index_count = 0;
			single_list_block_node<type>* ptr = p_first;
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
			single_list_block_node<type>* ptr = p_first;
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
			single_list_block_node<type>* ptr = p_first;
			while (ptr)
			{
				if (index_count == index) return iterator{ ptr };
				++index_count;
				ptr = ptr->next;
			}

			return iterator{ nullptr };
		};
		[[nodiscard]] constexpr const iterator operator[](long long index) const noexcept
		{
			long long index_count = 0;
			single_list_block_node<type>* ptr = p_first;
			while (ptr)
			{
				if (index_count == index) return iterator{ ptr };
				++index_count;
				ptr = ptr->next;
			}

			return iterator{ nullptr };
		};

		constexpr iterator begin() const noexcept
		{
			return iterator{ p_first };
		};
		constexpr iterator last() const noexcept
		{
			return iterator{ p_last };
		};
		constexpr iterator end() const noexcept
		{
			return iterator{ nullptr };
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
				single_list_block_node<type>* prev = p_last;
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
				//p_first->next = nullptr;
				//p_first->prev = p_last;
				//p_first->object = std::move(type{ std::forward<Types>(args)... });
				::new (p_first) single_list_block_node<type>{ nullptr, nullptr, type{ std::forward<Types>(args)... } };
				p_first->prev = p_last;
				p_last = p_first;
			}
			else
			{
				single_list_block_node<type>* prev = p_last;
				p_last = allocator.allocate();
				//p_last->prev = nullptr;
				p_first->next = p_last;
				//p_last->object = std::move(type{ std::forward<Types>(args)... });

				::new (p_last) single_list_block_node<type>{ nullptr, prev, type{ std::forward<Types>(args)... } };
				prev->prev = p_last;
				//p_last->next = prev;
			}
		};

		constexpr iterator erase(long long index) noexcept
		{
			auto it = this->operator[](index);
			single_list_block_node<type>* prev = nullptr;
			single_list_block_node<type>* next = nullptr;
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

			if (it) allocator.m_free(*it);
			return iterator{ prev };
		};
		constexpr iterator erase(iterator& it) noexcept
		{
			single_list_block_node<type>* prev = nullptr;
			single_list_block_node<type>* next = nullptr;
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

			if (it) allocator.m_free(*it);
			return iterator{ prev };
		};
	};

	/*template<typename type>
	constexpr void single_list_block_node<type>::erase() noexcept
	{
		single_list_block_node<type>* _prev = nullptr;
		single_list_block_node<type>* _next = nullptr;
		if (next)
		{
			_next = next;
		}
		if (prev)
		{
			_prev = prev;
		}

		if (_next && _prev)
		{
			prev->next = _next;
			next->prev = _prev;
		}

		single_list_block<type>::allocator.m_free(this);
	};*/
};

/*
struct poooooop_msvc
{
	mem::single_list_block<item_32> list;
	__forceinline constexpr mem::single_list_block<item_32>::iterator get_item(long long i) noexcept
	{
		return list[i];
	};
	/*__forceinline constexpr const mem::single_list_block<item_32>::iterator get_item(long long i) const noexcept
	{
		return list[i];
	};
};

static auto fuck_msvc2() noexcept
{
	poooooop_msvc arr;
	arr.list.emplace_back(belt_direction::left_right, 4000ll);
	auto iter = arr.get_item(0);
	auto success = iter->add_item(belt_item{ item_type::log }, vec2_uint{ 0ll, 0ll });
	//if (success == -1) return success;
	success = iter->add_item(belt_item{ item_type::stone }, vec2_uint{ 64ll, 0ll });
	//if (success == -1) return success;
	success = iter->add_item(belt_item{ item_type::log }, vec2_uint{ 96ll, 0ll });
	//if (success == -1) return success;
	success = iter->add_item(belt_item{ item_type::iron }, vec2_uint{ 128ll, 0ll });
	//if (success == -1) return success;

	long long total = 0;
	for (auto it = arr.list.begin(); it != arr.list.end(); ++it)
	{
		total += it->count();
	}
	return total;

	//return arr.get_item(0)->count();
};*/