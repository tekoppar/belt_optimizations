#pragma once

#include <version>
#include <new>
#include <type_traits>
#include <utility>
#include <malloc.h>
#include <Windows.h>
#include <cstring>
#include <heapapi.h>
#include <corecrt_terminate.h>

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
	int get_ptr_as_index(type* ptr) noexcept
	{
		unsigned long long memory_address = (unsigned long long)ptr;
		int index = memory_address - start_of_block_address;
		return index / sizeof(type);
	};
	bool is_ptr_inside(type* ptr) noexcept
	{
		unsigned long long memory_address = (unsigned long long)ptr;
		return memory_address >= start_of_block_address && memory_address < end_of_block_address;
	};

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
	inline type* get_start_of_block(std::size_t index) const noexcept
	{
		return all_blocks[index].get_start_of_block();
	};
	inline type* get_end_of_block(std::size_t index) const noexcept
	{
		return all_blocks[index].get_end_of_block();
	};
};

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

template<class type, class allocator = block_allocator<type, 4096>>
class block_iterator
{
public:
#ifdef __cpp_lib_concepts
	using iterator_concept = std::random_access_iterator_tag;
#endif // __cpp_lib_concepts
	using iterator_category = std::random_access_iterator_tag;
	using value_type = type;
	using difference_type = std::ptrdiff_t;
	using pointer = type*;
	using reference = value_type&;

	constexpr block_iterator() noexcept
	{};
	constexpr block_iterator(pointer ptr, pointer current_end, allocator* allocator_ptr) noexcept :
		m_ptr(ptr),
		m_current_end{ current_end },
		_allocator{ allocator_ptr }
	{};

	constexpr pointer m_get(std::size_t index) noexcept
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

	[[nodiscard]] constexpr reference operator*() const noexcept
	{
		return *this->m_ptr;
	};
	[[nodiscard]] constexpr reference operator*(block_iterator iter) const noexcept
	{
		return *iter.m_ptr;
	};
	[[nodiscard]] constexpr pointer operator->() const noexcept
	{
		return this->m_ptr;
	};
	//pointer operator&() const noexcept = delete;

	[[nodiscard]] constexpr reference operator[](const std::size_t& index) noexcept
	{
		return *m_get(index);
	};
	[[nodiscard]] constexpr reference operator[](const difference_type& index) noexcept
	{
		return *m_get(index);
	};
	[[nodiscard]] constexpr const reference operator[](const std::size_t& index) const noexcept
	{
		return *m_get(index);
	};
	[[nodiscard]] constexpr const reference operator[](const difference_type& index) const noexcept
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
		return value_type{};
	};

private:
	alignas(8) pointer m_ptr{ nullptr };
	alignas(8) pointer m_current_end{nullptr};
	alignas(8) allocator* _allocator{nullptr};
	alignas(8) std::size_t current_block{ 0 };
};

template<typename type>
class block_container
{
	block_allocator<type, 4096> memory_allocator;
	std::size_t count{ 0 };

public:
	[[nodiscard]] constexpr type& operator[](std::size_t index) noexcept
	{
		return memory_allocator.get_object(index);
	};
	[[nodiscard]] constexpr const type& operator[](std::size_t index) const noexcept
	{
		return memory_allocator.get_object(index);
	};
	inline constexpr std::size_t size() const noexcept
	{
		return count;
	};

	inline constexpr void push_back(const type& val) noexcept
		requires(type_has_copy<type>)
	{
		type* ptr = memory_allocator.allocate();
		*ptr = val;
	};
	inline constexpr void push_back(type&& val) noexcept
		requires(type_has_move<type>)
	{
		type* ptr = memory_allocator.allocate();
		*ptr = std::move(val);
	};

	template <typename... Types>
	inline constexpr void emplace_back(Types&&... args) noexcept
		requires (std::is_move_constructible_v<type> == true && std::is_constructible_v<type, Types...> == true)
	{
		type* ptr = memory_allocator.allocate();
		::new (ptr) type{ std::forward<Types>(args)... };
	};

	constexpr block_iterator<type, block_allocator<type, 4096>> begin() noexcept
	{
		return block_iterator<type, block_allocator<type, 4096>>{ memory_allocator.get_start_of_block(0), memory_allocator.get_end_of_block(0), &memory_allocator };
	};
	constexpr block_iterator<type, block_allocator<type, 4096>> end() noexcept
	{
		return block_iterator<type, block_allocator<type, 4096>>{ memory_allocator.get_end_of_block(0), memory_allocator.get_end_of_block(0), &memory_allocator };
	};
};