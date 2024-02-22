#pragma once

#include <intrin.h>
#include <limits>
#include <vadefs.h>
#include <type_traits>

#include "math_utility.h"

namespace mem
{
	template<long long l_n_cache_size_kb, long long c_cache_line_size, long long l_n_n_ways>
	class cpu_cache_info
	{
		constexpr static inline const long long cache_kb_size = 1024ll;
	public:
		constexpr static inline const long long cache_total_size_kb = l_n_cache_size_kb;
		constexpr static inline const long long cache_line_size = c_cache_line_size;
		constexpr static inline const long long cache_n_ways = l_n_n_ways;
		constexpr static inline const long long cache_size = cpu_cache_info::cache_kb_size * cache_total_size_kb;
		constexpr static inline const long long cache_set_size = (cache_line_size * cache_n_ways);
		constexpr static inline const long long cache_number_of_sets = (cache_size / cache_n_ways) / cache_line_size;
		constexpr static inline const long long cache_one_way_size = (cache_number_of_sets * cache_line_size);
	};
	class cpu_info
	{
	public:
		constexpr static inline const mem::cpu_cache_info<32ll, 64ll, 8ll> l1d_info{};
	};

	struct cache_set_counter
	{
		long long set_counter[mem::cpu_info::l1d_info.cache_number_of_sets]{};
	};

	struct set_loops_before_info
	{
		long long loops{ 0ll };
		long long smallest_bytes_that_can_be_read{ 0ll };
	};
	struct set_loop_step_info
	{
		set_loops_before_info l_info{ 0ll, 64ll };
		long long step_rate{ 0ll };
	};

	constexpr bool is_power_of_2_to_64(long long number, long long minimum_check) noexcept
	{
		if (minimum_check <= 64ll && number % 64ll == 0ll) return true;
		if (minimum_check <= 32ll && number % 32ll == 0ll) return true;
		if (minimum_check <= 16ll && number % 16ll == 0ll) return true;
		if (minimum_check <= 8ll && number % 8ll == 0ll) return true;
		if (minimum_check <= 4ll && number % 4ll == 0ll) return true;
		if (minimum_check <= 2ll && number % 2ll == 0ll) return true;
		return false;
	};

	template<typename lhs, typename rhs, long long offset_count>
	constexpr static inline long long ptr_type_offset() noexcept
	{
		return (sizeof(lhs) / sizeof(rhs)) * offset_count;
	};
	static_assert(ptr_type_offset<long long, int, 4ll>() == 8ll);
	static_assert(ptr_type_offset<long long, char, 4ll>() == 32ll);

	template<typename lhs, typename rhs>
	constexpr static inline long long types_in_ptr() noexcept
	{
		return (sizeof(lhs) / sizeof(rhs));
	};
	static_assert(types_in_ptr<long long, int>() == 2ll);

	struct aligned_allocation_t
	{
		explicit aligned_allocation_t() = default;
	}; inline constexpr aligned_allocation_t aligned_allocation_v{};

	template<typename T>
	static inline size_t get_alignment(const void* ptr) noexcept
	{
		return (std::uintptr_t)ptr % sizeof(T);
	};
	template<typename T, long long alignment>
	static inline bool is_aligned(const void* ptr) noexcept
	{
		return (std::uintptr_t)ptr % alignment == 0ull;
	};
	template<typename T, std::size_t alignment>
	constexpr inline std::size_t aligned_by() noexcept
	{
		constexpr double align = sizeof(T) % alignment;
		return align == 0.0 ? static_cast<std::size_t>(align) : 1ull;
	};
	template<typename T>
	constexpr inline std::size_t cache_lines_for() noexcept
	{
		if constexpr (sizeof(T) >= 64ull) return expr::max<std::size_t>((sizeof(T) / 64ull), 1ull);
		else return expr::max<std::size_t>(64ull / (sizeof(T)), 1ull);
	};

	template<long long n>
	struct can_be_aligned_s
	{
		constexpr static inline bool check() noexcept
		{
			if constexpr (n == 1ll) return true;
			constexpr double test_n = static_cast<double>(n);
			constexpr double div_n = test_n / 2.0;
			constexpr bool has_rem = !(div_n == static_cast<long long>(div_n));
			constexpr long long new_n = has_rem == false ? static_cast<long long>(div_n) : -1ll;
			if constexpr (new_n == 0ll) return true;
			else if constexpr (new_n < 0ll) return false;
			else if constexpr (new_n == 1ll) return true;
			else return can_be_aligned_s<new_n>{}.check();
		};
	};
	template<>
	struct can_be_aligned_s<0ll>
	{
		constexpr static inline bool check() noexcept
		{
			return true;
		};
	};
	template<typename T>
	constexpr static inline bool can_be_aligned() noexcept
	{
		return can_be_aligned_s<sizeof(T)>{}.check();
	};
	static_assert(mem::can_be_aligned<int>());
	static_assert(mem::can_be_aligned<bool>());

	template<typename T>
	constexpr static inline long long get_closest_alignmnet() noexcept
	{
		constexpr long long size = sizeof(T);
		long long align_size = 1ll;
		while (align_size < size)
		{
			align_size += align_size;
		}

		return align_size;
	};
	static_assert(mem::get_closest_alignmnet<int>() == 4ll);
	static_assert(mem::get_closest_alignmnet<int*>() == 8ll);

	//#define _GUARD_CONSTEXPR_CALLS
	template <long long n, long long n_total, typename T, int prefetch_hint>
	struct prefetch
	{
		__forceinline constexpr void ___mm_prefetch___(T* ptr) noexcept
		{
			constexpr long long nn = n_total - n;
			if (std::is_constant_evaluated() == false) _mm_prefetch((char const*)(ptr + nn), prefetch_hint);
			prefetch<(n - 1ll), n_total, T, prefetch_hint>{}.___mm_prefetch___(ptr);
		};
	};
	template <long long n_total, typename T, int prefetch_hint>
	struct prefetch<0ll, n_total, T, prefetch_hint>
	{
		__forceinline constexpr void ___mm_prefetch___(T* ptr) noexcept
		{
			if (std::is_constant_evaluated() == false) _mm_prefetch((char const*)(ptr + n_total), prefetch_hint);
		};
	};

	template <long long n, long long n_total, long long offset, typename T, int prefetch_hint>
	struct prefetch_offset
	{
		__forceinline constexpr void ___mm_prefetch___(T const* ptr) noexcept
		{
			constexpr long long calc_offset = (offset * (n_total - n));
			char const* tmp_ptr = ((char const*)(ptr));
			if (std::is_constant_evaluated() == false) _mm_prefetch(tmp_ptr + calc_offset, prefetch_hint);
			prefetch_offset<(n - 1ll), n_total, offset, T, prefetch_hint>{}.___mm_prefetch___(ptr);
		};
	};
	template <long long n_total, long long offset, typename T, int prefetch_hint>
	struct prefetch_offset<0ll, n_total, offset, T, prefetch_hint>
	{
		__forceinline constexpr void ___mm_prefetch___(T const* ptr) noexcept
		{
			constexpr long long calc_offset = offset * n_total;
			char const* tmp_ptr = ((char const*)(ptr));
			if (std::is_constant_evaluated() == false) _mm_prefetch(tmp_ptr + calc_offset, prefetch_hint);
		};
	};

	template<typename T, long long objects_to_pre_fetch = 32ll, int prefetch_hint = 0, typename in_T = T>
	__forceinline static void pre_fetch_cachelines(in_T* ptr)
	{
		if constexpr (sizeof(T) > 64ull)
		{
			constexpr long long cache_lines_per_object = expr::max<long long>(static_cast<long long>(sizeof(T) / 64ll), 1ll);
			constexpr long long prefetch_calls = objects_to_pre_fetch * cache_lines_per_object;

			prefetch_offset<prefetch_calls - 1ll, prefetch_calls - 1ll, 64ll, T, prefetch_hint>{}.___mm_prefetch___((T*)ptr);
		}
		else
		{
			constexpr long long objects_per_cache_line = expr::max<long long>(static_cast<long long>(sizeof(T) <= 64ll ? 64ll / sizeof(T) : mem::types_in_ptr<T, in_T>()), 1ll);
			constexpr long long prefetch_calls = expr::max<long long>(objects_to_pre_fetch / objects_per_cache_line, 1ll);

			prefetch_offset<prefetch_calls - 1ll, prefetch_calls - 1ll, 64ll, T, prefetch_hint>{}.___mm_prefetch___((T*)ptr);
		}
	};
	template<typename T, long long objects_to_pre_fetch = 32ll, int prefetch_hint = 0, typename in_T = T>
	__forceinline static void pre_fetch_cacheline_ptrs(in_T* ptr)
	{
		if constexpr (static_cast<long long>(sizeof(in_T)) > 64ll)
		{
			constexpr long long cache_lines_per_object = expr::max<long long>(64ll / static_cast<long long>(sizeof(T)), 1ll);
			constexpr long long prefetch_calls = objects_to_pre_fetch * cache_lines_per_object;

			prefetch<prefetch_calls - 1ll, prefetch_calls - 1ll, T, prefetch_hint>{}.___mm_prefetch___((T*)ptr);
		}
		else
		{
			constexpr long long objects_per_cache_line = expr::max<long long>(static_cast<long long>(sizeof(T) <= 64ll ? (sizeof(T) == 64ull ? 1ll : (64ll / sizeof(T)) * sizeof(T)) : mem::types_in_ptr<T, char>()), 1ll);
			constexpr long long prefetch_calls = expr::max<long long>(sizeof(T) <= 64ll ? objects_to_pre_fetch / objects_per_cache_line : objects_to_pre_fetch, 1ll);

			prefetch_offset<prefetch_calls - 1ll, prefetch_calls - 1ll, objects_per_cache_line, T, prefetch_hint>{}.___mm_prefetch___((T*)ptr);
		}
	};

	constexpr static long long find_cache_index(long long byte_ptr)
	{
		return byte_ptr / 64ll;
	};
	constexpr static long long find_cache_set(long long byte_ptr)
	{
		return find_cache_index(byte_ptr) % 64ll;
	};
	static long long find_cache_index(void* ptr)
	{
		return ((long long)ptr / 64ll);
	};
	static long long find_cache_set(void* ptr)
	{
		return find_cache_index(ptr) % 64ll;
	};

	constexpr static set_loops_before_info find_loops_before_set_collision(long long object_size, long long step_size = 1ll)
	{
		if (object_size < 1ll) return { -1ll, -1ll };
		long long byte_ptr = 0ll;
		long long active_set = mem::find_cache_set(byte_ptr);
		mem::cache_set_counter counter{};
		for (int i = 0, l = mem::cpu_info::l1d_info.cache_number_of_sets; i < l; ++i) counter.set_counter[i] = 0;

		++counter.set_counter[active_set];
		long long loop_counter{ 0ll };
		long long smallest_bytes_that_can_be_read{ 64ll };
		while (true)
		{
			byte_ptr += object_size * step_size;
			auto bytes_that_can_be_read = byte_ptr % 64ll;
			long long hit_set = mem::find_cache_set(byte_ptr);
			if (bytes_that_can_be_read < 0ll || object_size * step_size > 64ll && active_set == hit_set || active_set != hit_set)
			{
				if (counter.set_counter[hit_set] > 0) return { loop_counter, smallest_bytes_that_can_be_read };

				++counter.set_counter[hit_set];
			}
			if (bytes_that_can_be_read < smallest_bytes_that_can_be_read && bytes_that_can_be_read > 0ll) smallest_bytes_that_can_be_read = bytes_that_can_be_read;
			++loop_counter;
			active_set = hit_set;

			if (loop_counter > 100000ll) return { -1ll, -1ll };
		}

		return { -1ll, -1ll };
	};
	static_assert(mem::find_loops_before_set_collision(192ll, 1ll).loops == 63ll, "fewest collisions should be 63 loops");
	static_assert(mem::find_loops_before_set_collision(8ll, 1ll).loops == 511ll, "fewest collisions should be 63 loops");

	constexpr static set_loops_before_info find_loops_before_set_eviction(long long object_size, long long step_size = 1ll)
	{
		if (object_size < 1ll) return { -1ll, -1ll };
		auto ret_val = mem::find_loops_before_set_collision(object_size, step_size);
		return { ((ret_val.loops + 1) * 7ll) + ret_val.loops, ret_val.smallest_bytes_that_can_be_read };
		long long byte_ptr = 0ll;
		long long active_set = mem::find_cache_set(byte_ptr);
		mem::cache_set_counter counter{};
		for (int i = 0, l = mem::cpu_info::l1d_info.cache_number_of_sets; i < l; ++i) counter.set_counter[i] = 0;

		++counter.set_counter[active_set];
		long long loop_counter{ 0ll };
		long long smallest_bytes_that_can_be_read{ 64ll };
		while (true)
		{
			byte_ptr += object_size * step_size;
			auto bytes_that_can_be_read = byte_ptr % 64ll;
			long long hit_set = mem::find_cache_set(byte_ptr);
			if (bytes_that_can_be_read < 0ll || object_size * step_size > 64ll && active_set == hit_set || active_set != hit_set)
			{
				if (counter.set_counter[hit_set] > 8) return { loop_counter, smallest_bytes_that_can_be_read };
				++counter.set_counter[hit_set];
			}
			if (bytes_that_can_be_read < smallest_bytes_that_can_be_read && bytes_that_can_be_read > 0ll) smallest_bytes_that_can_be_read = bytes_that_can_be_read;
			++loop_counter;
			active_set = hit_set;

			if (loop_counter > 100000ll) return { -1ll, -1ll };
		}

		return { -1ll, -1ll };
	};
	static_assert(mem::find_loops_before_set_eviction(192ll, 1ll).loops == 511ll, "fewest collisions should be 63 loops");
	static_assert(mem::find_loops_before_set_eviction(8, 1ll).loops == 4095ll, "fewest collisions should be 63 loops");

	constexpr static set_loop_step_info find_step_rate_with_fewest_collisions(long long object_size, long long max_step_rate = 16ll)
	{
		if (object_size < 1ll) return { -1ll, -1ll };
		long long byte_ptr = 0ll;
		long long first_set = mem::find_cache_set(byte_ptr);
		mem::cache_set_counter counter{};
		for (int i = 0, l = mem::cpu_info::l1d_info.cache_number_of_sets; i < l; ++i) counter.set_counter[i] = 0;

		++counter.set_counter[first_set];
		set_loops_before_info loop_counter{ 0ll };
		set_loops_before_info highest_loop_counter{ 0ll };
		long long highest_step_rate{ 0ll };
		for (long long i = 0ll; i < max_step_rate + 1ll; ++i)
		{
			loop_counter = mem::find_loops_before_set_collision(object_size, i);
			if (loop_counter.loops > highest_loop_counter.loops || loop_counter.loops == highest_loop_counter.loops && loop_counter.smallest_bytes_that_can_be_read > highest_loop_counter.smallest_bytes_that_can_be_read)
			{
				highest_loop_counter = loop_counter;
				highest_step_rate = i;
			}
		}

		return { highest_loop_counter, highest_step_rate };
	};
	//static_assert(mem::find_step_rate_with_fewest_collisions(192ll, 4ll).loops == 63ll);

	constexpr static set_loop_step_info find_step_rate_with_fewest_evictions(long long object_size, long long max_step_rate = 16ll)
	{
		if (object_size < 1ll) return { -1ll, -1ll };
		long long byte_ptr = 0ll;
		long long first_set = mem::find_cache_set(byte_ptr);
		mem::cache_set_counter counter{};
		for (int i = 0, l = mem::cpu_info::l1d_info.cache_number_of_sets; i < l; ++i) counter.set_counter[i] = 0;

		++counter.set_counter[first_set];
		set_loops_before_info loop_counter{ 0ll };
		set_loops_before_info highest_loop_counter{ 0ll };
		long long highest_step_rate{ 0ll };
		for (long long i = 0ll; i < max_step_rate + 1ll; ++i)
		{
			loop_counter = mem::find_loops_before_set_eviction(object_size, i);
			if (loop_counter.loops > highest_loop_counter.loops || loop_counter.loops == highest_loop_counter.loops && loop_counter.smallest_bytes_that_can_be_read > highest_loop_counter.smallest_bytes_that_can_be_read)
			{
				highest_loop_counter = loop_counter;
				highest_step_rate = i;
			}
		}

		return { highest_loop_counter, highest_step_rate };
	};
	//static_assert(mem::find_step_rate_with_fewest_evictions(192ll, 4ll).loops == 575ll);
	//static_assert(mem::find_step_rate_with_fewest_evictions(256ll, 4ll).loops == 143ll);
	struct test_loop_stride_1
	{
		unsigned char a[32]{};
		unsigned char b[32]{};
		alignas(64) unsigned char p[12]{};
	};

	template<typename object_type>
	consteval static long long find_loop_stride() noexcept
	{
		constexpr long long object_bytes = static_cast<long long>(sizeof(object_type));

		if constexpr (object_bytes <= mem::cpu_info::l1d_info.cache_line_size)
		{
			return 1ll;
		}
		else
		{
			if constexpr (!(object_bytes % mem::cpu_info::l1d_info.cache_line_size) & 1ll)
			{
				if constexpr (object_bytes % mem::cpu_info::l1d_info.cache_number_of_sets != 0ll) return mem::cpu_info::l1d_info.cache_line_size / (object_bytes % mem::cpu_info::l1d_info.cache_number_of_sets);
				else return object_bytes / mem::cpu_info::l1d_info.cache_number_of_sets;
			}
			else
			{
				constexpr long long trailing_bytes = object_bytes % mem::cpu_info::l1d_info.cache_line_size;
				if constexpr (expr::is_odd(trailing_bytes)) return mem::cpu_info::l1d_info.cache_line_size;
				else
				{
					constexpr long long tb_mod = mem::cpu_info::l1d_info.cache_line_size % trailing_bytes;
					constexpr long long tb_div = tb_mod == 0ll ? 1ll : expr::round_div(mem::cpu_info::l1d_info.cache_line_size, tb_mod);
					constexpr long long tb_div_floor = expr::floor_div(mem::cpu_info::l1d_info.cache_line_size, trailing_bytes);
					return (tb_div * tb_div_floor) % mem::cpu_info::l1d_info.cache_line_size;
				}
			}
		}

		return -1ll;
	};
	constexpr auto tmp123456 = mem::find_loop_stride<test_loop_stride_1>();

	struct test_ss
	{
		long long a[8]{};
		char g[17];
	};

	template<typename object_type>
	consteval static bool check_object_alignment_stride()
	{
		//all info is based on that the storage is aligned to the object so a accessing index 0 could be written as 0x0
		//if the storage is misaligned accessing index 0 could look like 0x8 and thus any alignmnet is void
		constexpr long long object_byte_size = static_cast<long long>(sizeof(object_type));
		constexpr long long object_alignment = static_cast<long long>(alignof(object_type));

		if constexpr (object_byte_size == object_alignment && object_byte_size <= mem::cpu_info::l1d_info.cache_line_size) return true; //catches power of 2 aligned objects at 64 and below
		if constexpr (object_byte_size % mem::cpu_info::l1d_info.cache_line_size == 0ll && mem::cpu_info::l1d_info.cache_line_size == object_alignment) return true;
		if constexpr (expr::divide_with_remainder(mem::cpu_info::l1d_info.cache_line_size, object_alignment).rem == 0ll && expr::divide_with_remainder(mem::cpu_info::l1d_info.cache_line_size, object_byte_size).rem == 0ll) return true;

		constexpr auto loop_stride = mem::find_loop_stride<object_type>();
		constexpr auto trailing_bytes = object_byte_size % mem::cpu_info::l1d_info.cache_line_size;

		/*
		* if you plan to read the entire object like copying it to another buffer ignore this.
		* But if you plan to iterate over it and accessing anything with an offset less then 64 bytes - it's alignment
		* trailing bytes can cause issues as each object will be offset by said amount in memory.
		*
		* from buffer[0+n] to buffer[loop_stride] the object being accessed won't be
		* loaded in at the start of a cache line, but will be offset by
		* cache_line_size - (((trailing_bytes(0x0) * n) + offset) % cache_line_size)
		*
		* if trailing bytes is odd the cache stride will always be 64 (occurs with 1 byte alignments)
		*
		* the reason this can be bad is that if you try access the object with an offset that's greater
		* than the remaining bytes in the cache line is that a new cache line will need to be loaded.
		* this means any instructions will still for that to happen but you will also start using up
		* more cache lines and potentially evicting existing ones faster than if you changed alignment
		*
		* you might think to use the easy solution by aligning the object to the size of the cache line.
		* but a ptr can only belong the a specific set of cache lines. if you had an object that had a
		* size of 96 bytes and aligned that to 64. it would occupy 2 cache lines, and if you only ever
		* read the first 64 bytes you would only make use of half the cache lines
		*/
		//static_assert(trailing_bytes == 0ll, "warning object has trailing bytes, for n iteration less bytes will be loaded");

		constexpr auto object_cache_size_div = expr::divide_with_remainder(mem::cpu_info::l1d_info.cache_line_size, object_byte_size);
		//static_assert(object_cache_size_div.div != 0ll, "object is larger then 64bytes trails cache lines");
		//static_assert(object_cache_size_div.div > 0ll && object_cache_size_div.rem == 0ll, "object trails cache lines");

		constexpr auto fewest_evictions = mem::find_loops_before_set_eviction(object_byte_size, 1ll);

		return false;
	};
	//constexpr auto tmp1234511116 = mem::check_object_alignment_stride<test_ss>();
};