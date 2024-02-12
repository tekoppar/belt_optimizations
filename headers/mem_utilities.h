#pragma once

#include <intrin.h>

#include "math_utility.h"

namespace mem
{
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
			_mm_prefetch((char const*)(ptr + nn), prefetch_hint);
			prefetch<(n - 1ll), n_total, T, prefetch_hint>{}.___mm_prefetch___(ptr);
		};
	};
	template <long long n_total, typename T, int prefetch_hint>
	struct prefetch<0ll, n_total, T, prefetch_hint>
	{
		__forceinline constexpr void ___mm_prefetch___(T* ptr) noexcept
		{
			_mm_prefetch((char const*)(ptr + n_total), prefetch_hint);
		};
	};

	template <long long n, long long n_total, long long offset, typename T, int prefetch_hint>
	struct prefetch_offset
	{
		__forceinline constexpr void ___mm_prefetch___(T const* ptr) noexcept
		{
			constexpr long long calc_offset = (offset * (n_total - n));
			char const* tmp_ptr = ((char const*)(ptr));
			_mm_prefetch(tmp_ptr + calc_offset, prefetch_hint);
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
			_mm_prefetch(tmp_ptr + calc_offset, prefetch_hint);
		};
	};

	template<typename T, long long objects_to_pre_fetch = 32ll, int prefetch_hint = 0, typename in_T = T>
	__forceinline static void pre_fetch_cachelines(in_T* ptr)
	{
		if constexpr (sizeof(in_T) > 64ull)
		{
			constexpr long long cache_lines_per_object = expr::max<long long>(64ll / static_cast<long long>(sizeof(T)), 1ll);
			constexpr long long prefetch_calls = objects_to_pre_fetch * cache_lines_per_object;

			prefetch_offset<prefetch_calls - 1ll, prefetch_calls - 1ll, 64ll, T, prefetch_hint>{}.___mm_prefetch___((T*)ptr);
		}
		else
		{
			constexpr long long objects_per_cache_line = expr::max<long long>(static_cast<long long>(sizeof(T) <= 64ll ? 64ll / sizeof(T) : types_in_ptr<T, in_T>()), 1ll);
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
			constexpr long long objects_per_cache_line = expr::max<long long>(static_cast<long long>(sizeof(T) <= 64ll ? (64ll / sizeof(T)) * sizeof(T) : types_in_ptr<T, char>()), 1ll);
			constexpr long long prefetch_calls = expr::max<long long>(sizeof(T) <= 64ll ? objects_to_pre_fetch / objects_per_cache_line : objects_to_pre_fetch, 1ll);

			prefetch_offset<prefetch_calls - 1ll, prefetch_calls - 1ll, objects_per_cache_line, T, prefetch_hint>{}.___mm_prefetch___((T*)ptr);
		}
	};
};