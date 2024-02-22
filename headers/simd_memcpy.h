#pragma once

#include <xmmintrin.h>
#include <intrin.h>
#include <assert.h>
#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <emmintrin.h>
#include <cassert>
#include <immintrin.h>
#include <smmintrin.h>

#include "mem_utilities.h"

template <typename T>
inline constexpr bool IsAligned(T value, std::size_t alignment)
{
	//return ((reinterpret_cast<std::uintptr_t>(value) % alignment) == 0);
	return 0 == ((std::size_t)value & (alignment - 1));
};

template <typename T>
inline constexpr T DivideByMultiple(T value, std::size_t alignment)
{
	return (T)((value + alignment - 1) / alignment);
};

inline void SIMDCacheLineCopy(std::size_t InitialQuadwordCount, void* __restrict _Dest, const void* __restrict _Source) noexcept
{
	__m128i* __restrict s_Dest = (__m128i * __restrict)_Dest;
	const __m128i* __restrict s_Source = (const __m128i * __restrict)_Source;

	switch (InitialQuadwordCount)
	{
		case 3: _mm_stream_si128(s_Dest + 2, _mm_load_si128(s_Source + 2)); [[fallthrough]];	 // Fall through
		case 2: _mm_stream_si128(s_Dest + 1, _mm_load_si128(s_Source + 1)); [[fallthrough]];	 // Fall through
		case 1: _mm_stream_si128(s_Dest + 0, _mm_load_si128(s_Source + 0)); [[fallthrough]];	 // Fall through
		default:
			break;
	}
};

inline void SIMDRemainingCacheLine(std::size_t NumQuadwords, void* __restrict _Dest, const void* __restrict _Source) noexcept
{
	__m128i* __restrict s_Dest = (__m128i * __restrict)_Dest;
	const __m128i* __restrict s_Source = (const __m128i * __restrict)_Source;

	// Copy the remaining quadwords
	switch (NumQuadwords & 3)
	{
		case 3: _mm_stream_si128(s_Dest + 2, _mm_load_si128(s_Source + 2)); [[fallthrough]];	 // Fall through
		case 2: _mm_stream_si128(s_Dest + 1, _mm_load_si128(s_Source + 1)); [[fallthrough]];	 // Fall through
		case 1: _mm_stream_si128(s_Dest + 0, _mm_load_si128(s_Source + 0)); [[fallthrough]];	 // Fall through
		default:
			break;
	}
};

__forceinline void SIMDMemCopy(void* __restrict _Dest, const void* __restrict _Source, std::size_t NumQuadwords) noexcept
{
	assert(IsAligned(_Dest, 16));
	assert(IsAligned(_Source, 16));

	__m128i* __restrict Dest = (__m128i * __restrict)_Dest;
	const __m128i* __restrict Source = (const __m128i * __restrict)_Source;

	// Discover how many quadwords precede a cache line boundary.  Copy them separately.
	std::size_t InitialQuadwordCount = (4 - ((std::size_t)Source >> 4) & 3) & 3;
	if (InitialQuadwordCount > NumQuadwords)
		InitialQuadwordCount = NumQuadwords;

	switch (InitialQuadwordCount)
	{
		case 3: _mm_stream_si128(Dest + 2, _mm_load_si128(Source + 2)); [[fallthrough]];	 // Fall through
		case 2: _mm_stream_si128(Dest + 1, _mm_load_si128(Source + 1)); [[fallthrough]];	 // Fall through
		case 1: _mm_stream_si128(Dest + 0, _mm_load_si128(Source + 0)); [[fallthrough]];	 // Fall through
		default:
			break;
	}

	if (NumQuadwords == InitialQuadwordCount)
		return;

	Dest += InitialQuadwordCount;
	Source += InitialQuadwordCount;
	NumQuadwords -= InitialQuadwordCount;

	std::size_t CacheLines = NumQuadwords >> 2;

	switch (CacheLines)
	{
		default:
		case 10: _mm_prefetch((char const*)(Source + 36), _MM_HINT_NTA); [[fallthrough]];	// Fall through
		case 9:  _mm_prefetch((char const*)(Source + 32), _MM_HINT_NTA); [[fallthrough]];	// Fall through
		case 8:  _mm_prefetch((char const*)(Source + 28), _MM_HINT_NTA); [[fallthrough]];	// Fall through
		case 7:  _mm_prefetch((char const*)(Source + 24), _MM_HINT_NTA); [[fallthrough]];	// Fall through
		case 6:  _mm_prefetch((char const*)(Source + 20), _MM_HINT_NTA); [[fallthrough]];	// Fall through
		case 5:  _mm_prefetch((char const*)(Source + 16), _MM_HINT_NTA); [[fallthrough]];	// Fall through
		case 4:  _mm_prefetch((char const*)(Source + 12), _MM_HINT_NTA); [[fallthrough]];	// Fall through
		case 3:  _mm_prefetch((char const*)(Source + 8), _MM_HINT_NTA); [[fallthrough]];	// Fall through
		case 2:  _mm_prefetch((char const*)(Source + 4), _MM_HINT_NTA); [[fallthrough]];	// Fall through
		case 1:  _mm_prefetch((char const*)(Source + 0), _MM_HINT_NTA);	// Fall through

			// Do four quadwords per loop to minimize stalls.
			for (std::size_t i = CacheLines; i > 0; --i)
			{
				// If this is a large copy, start prefetching future cache lines.  This also prefetches the
				// trailing quadwords that are not part of a whole cache line.
				if (i >= 10)
					_mm_prefetch((char const*)(Source + 40), _MM_HINT_NTA);

				_mm_stream_si128(Dest + 0, _mm_load_si128(Source + 0));
				_mm_stream_si128(Dest + 1, _mm_load_si128(Source + 1));
				_mm_stream_si128(Dest + 2, _mm_load_si128(Source + 2));
				_mm_stream_si128(Dest + 3, _mm_load_si128(Source + 3));

				Dest += 4;
				Source += 4;
			}

		case 0:	// No whole cache lines to read
			break;
	}

	// Copy the remaining quadwords
	switch (NumQuadwords & 3)
	{
		case 3: _mm_stream_si128(Dest + 2, _mm_load_si128(Source + 2)); [[fallthrough]];	 // Fall through
		case 2: _mm_stream_si128(Dest + 1, _mm_load_si128(Source + 1)); [[fallthrough]];	 // Fall through
		case 1: _mm_stream_si128(Dest + 0, _mm_load_si128(Source + 0)); [[fallthrough]];	 // Fall through
		default:
			break;
	}

	_mm_sfence();
};

__forceinline void SIMDMemCopy256(void* __restrict _Dest, const void* __restrict _Source, std::size_t NumQuadwords) noexcept
{
	assert(IsAligned(_Dest, 16));
	assert(IsAligned(_Source, 16));

	__m256i* __restrict Dest = (__m256i * __restrict)_Dest;
	const __m256i* __restrict Source = (const __m256i * __restrict)_Source;

	// Discover how many quadwords precede a cache line boundary.  Copy them separately.
	std::size_t InitialQuadwordCount = ((std::size_t)Dest % 64) / 16;
	if (InitialQuadwordCount > NumQuadwords)
		InitialQuadwordCount = NumQuadwords;

	SIMDCacheLineCopy(InitialQuadwordCount, _Dest, _Source);
	if (NumQuadwords == InitialQuadwordCount) return;

	Dest = (__m256i*)((__m128i*)Dest + InitialQuadwordCount);
	Source = (__m256i*)((__m128i*)Source + InitialQuadwordCount);
	NumQuadwords -= InitialQuadwordCount;

	const std::size_t CacheLines = NumQuadwords >> 2;
	switch (CacheLines)
	{
		default:
		case 10: _mm_prefetch((char const*)(Source + 18), _MM_HINT_NTA); [[fallthrough]];	// Fall through
		case 9:  _mm_prefetch((char const*)(Source + 16), _MM_HINT_NTA); [[fallthrough]];	// Fall through
		case 8:  _mm_prefetch((char const*)(Source + 14), _MM_HINT_NTA); [[fallthrough]];	// Fall through
		case 7:  _mm_prefetch((char const*)(Source + 12), _MM_HINT_NTA); [[fallthrough]];	// Fall through
		case 6:  _mm_prefetch((char const*)(Source + 10), _MM_HINT_NTA); [[fallthrough]];	// Fall through
		case 5:  _mm_prefetch((char const*)(Source + 8), _MM_HINT_NTA); [[fallthrough]];	// Fall through
		case 4:  _mm_prefetch((char const*)(Source + 6), _MM_HINT_NTA); [[fallthrough]];	// Fall through
		case 3:  _mm_prefetch((char const*)(Source + 4), _MM_HINT_NTA); [[fallthrough]];		// Fall through
		case 2:  _mm_prefetch((char const*)(Source + 2), _MM_HINT_NTA); [[fallthrough]];	// Fall through
		case 1:  _mm_prefetch((char const*)(Source + 0), _MM_HINT_NTA);		// Fall through

			//if ((((size_t)Source % 64) / 32) == 0)
			if ((((std::size_t)Source) & 31) == 0)
			{
				// Do four quadwords per loop to minimize stalls.
				for (std::size_t i = CacheLines; i > 0; --i)
				{
					// If this is a large copy, start prefetching future cache lines.  This also prefetches the
					// trailing quadwords that are not part of a whole cache line.
					if (i >= 10) _mm_prefetch((char const*)(Source + 20), _MM_HINT_NTA);

					_mm256_stream_si256(Dest + 0, _mm256_load_si256(Source + 0));
					_mm256_stream_si256(Dest + 1, _mm256_load_si256(Source + 1));
					//_mm256_stream_si256(Dest + 2, _mm256_load_si256(Source + 2));
					//_mm256_stream_si256(Dest + 3, _mm256_load_si256(Source + 3));

					Dest += 2;
					Source += 2;
				}
			}
			else
			{
				// Do four quadwords per loop to minimize stalls.
				for (std::size_t i = CacheLines; i > 0; --i)
				{
					// If this is a large copy, start prefetching future cache lines.  This also prefetches the
					// trailing quadwords that are not part of a whole cache line.
					if (i >= 10)
						_mm_prefetch((char const*)(Source + 20), _MM_HINT_NTA);

					_mm256_stream_si256(Dest + 0, _mm256_loadu_si256(Source + 0));
					_mm256_stream_si256(Dest + 1, _mm256_loadu_si256(Source + 1));
					//_mm256_stream_si256(Dest + 2, _mm256_load_si256(Source + 2));
					//_mm256_stream_si256(Dest + 3, _mm256_load_si256(Source + 3));

					Dest += 2;
					Source += 2;
				}
			}

		case 0:	// No whole cache lines to read
			break;
	}

	// Copy the remaining quadwords
	/*switch (NumQuadwords & 1)
	{
		//case 3: _mm_stream_si128(s_Dest + 2, _mm_load_si128(s_Dest + 2));	 // Fall through
		//case 2: _mm_stream_si128(s_Dest + 1, _mm_load_si128(s_Dest + 1));	 // Fall through
		case 1: _mm256_stream_si256(Dest + 0, _mm256_load_si256(Dest + 0));	 // Fall through
		default:
			break;
	}*/
	SIMDRemainingCacheLine(NumQuadwords, Dest, Source);

	_mm_sfence();
};

template<typename T, bool is_ptr_aligned>
__forceinline void SIMDMemCopy256_32(void* __restrict _Dest, const void* __restrict _Source, std::size_t objects_to_copy) noexcept
{
	static_assert(mem::aligned_by<T, 32ull>() == 0ull, "type is not 16 byte aligned");
	std::size_t NumQuadwords = DivideByMultiple(sizeof(T) * objects_to_copy, 16ull);

	__m256i* __restrict Dest = (__m256i * __restrict)_Dest;
	const __m256i* __restrict Source = (const __m256i * __restrict)_Source;

	if constexpr (is_ptr_aligned == false)
	{
		if (std::is_constant_evaluated() == false)
		{
			assert(IsAligned(_Dest, 32));
			assert(IsAligned(_Source, 32));

			// Discover how many quadwords precede a cache line boundary.  Copy them separately.
			std::size_t InitialQuadwordCount = ((std::size_t)Dest % 64) / 16;
			if (InitialQuadwordCount > NumQuadwords)
				InitialQuadwordCount = NumQuadwords;

			SIMDCacheLineCopy(InitialQuadwordCount, _Dest, _Source);
			if (NumQuadwords == InitialQuadwordCount) return;

			Dest = (__m256i*)((__m128i*)Dest + InitialQuadwordCount);
			Source = (__m256i*)((__m128i*)Source + InitialQuadwordCount);
			NumQuadwords -= InitialQuadwordCount;
		}
	}

	const std::size_t CacheLines = NumQuadwords >> 2;
	constexpr std::size_t cachelines_per_object = mem::cache_lines_for<T>();
	constexpr std::size_t cache_lines_for_t = 10ull / cachelines_per_object;
	constexpr std::size_t objects_to_prefetch = 2ll;
	constexpr auto __m256i_in_t = mem::types_in_ptr<T, __m256i>();
	mem::pre_fetch_cachelines<T, objects_to_prefetch, _MM_HINT_NTA>(Source);

	if constexpr (is_ptr_aligned)
	{
		// Do four quadwords per loop to minimize stalls.
		for (std::size_t i = CacheLines; i > 0; --i)
		{
			// If this is a large copy, start prefetching future cache lines.  This also prefetches the
			// trailing quadwords that are not part of a whole cache line.
			if (i >= objects_to_prefetch * cache_lines_for_t) _mm_prefetch((char const*)(Source + (objects_to_prefetch * cache_lines_for_t * 2)), _MM_HINT_NTA);

			_mm256_stream_si256(&Dest[0], _mm256_stream_load_si256(Source + 0));
			_mm256_stream_si256(&Dest[1], _mm256_stream_load_si256(Source + 1));
			//_mm256_stream_si256(Dest + 0, _mm256_load_si256(Source + 0));
			//_mm256_stream_si256(Dest + 1, _mm256_load_si256(Source + 1));

			Dest += 2;
			Source += 2;
		}
	}
	else
	{
		// Do four quadwords per loop to minimize stalls.
		for (std::size_t i = CacheLines; i > 0; --i)
		{
			// If this is a large copy, start prefetching future cache lines.  This also prefetches the
			// trailing quadwords that are not part of a whole cache line.
			if (i >= objects_to_prefetch * cache_lines_for_t) _mm_prefetch((char const*)(Source + (objects_to_prefetch * cache_lines_for_t * 2)), _MM_HINT_NTA);

			_mm256_stream_si256(Dest + 0, _mm256_loadu_si256(Source + 0));
			_mm256_stream_si256(Dest + 1, _mm256_loadu_si256(Source + 1));

			Dest += 2;
			Source += 2;
		}
	}

	if constexpr (is_ptr_aligned == false) SIMDRemainingCacheLine(NumQuadwords, Dest, Source);
	_mm_sfence();
};
template<typename T, bool is_ptr_aligned>
__forceinline void SIMDMemCopy256_16(void* __restrict _Dest, const void* __restrict _Source, std::size_t objects_to_copy) noexcept
{
	static_assert(mem::aligned_by<T, 16ull>() == 0ull, "type is not 16 byte aligned");
	std::size_t NumQuadwords = DivideByMultiple(sizeof(T) * objects_to_copy, 16ull);

	__m128i* __restrict Dest = (__m128i * __restrict)_Dest;
	const __m128i* __restrict Source = (const __m128i * __restrict)_Source;

	if constexpr (is_ptr_aligned == false)
	{
		if (std::is_constant_evaluated() == false)
		{
			assert(IsAligned(_Dest, 16));
			assert(IsAligned(_Source, 16));

			// Discover how many quadwords precede a cache line boundary.  Copy them separately.
			std::size_t InitialQuadwordCount = ((std::size_t)Dest % 64) / 16;
			if (InitialQuadwordCount > NumQuadwords)
				InitialQuadwordCount = NumQuadwords;

			SIMDCacheLineCopy(InitialQuadwordCount, _Dest, _Source);
			if (NumQuadwords == InitialQuadwordCount) return;

			Dest = (__m128i*)((__m128i*)Dest + InitialQuadwordCount);
			Source = (__m128i*)((__m128i*)Source + InitialQuadwordCount);
			NumQuadwords -= InitialQuadwordCount;
		}
	}

	const std::size_t CacheLines = NumQuadwords >> 2;
	constexpr std::size_t cachelines_per_object = mem::cache_lines_for<T>();
	constexpr std::size_t cache_lines_for_t = 10ull / cachelines_per_object;
	constexpr std::size_t objects_to_prefetch = 4ll;
	constexpr auto __m256i_in_t = mem::types_in_ptr<T, __m128i>();
	mem::pre_fetch_cachelines<T, objects_to_prefetch* cachelines_per_object, _MM_HINT_NTA>(Source);

	if (IsAligned(Source, 32) && IsAligned(Dest, 32))
	{
		// Do four quadwords per loop to minimize stalls.
		for (std::size_t i = CacheLines; i > 0; --i)
		{
			// If this is a large copy, start prefetching future cache lines.  This also prefetches the
			// trailing quadwords that are not part of a whole cache line.
			//if (i >= objects_to_prefetch * cachelines_per_object) _mm_prefetch((char const*)(Source + (objects_to_prefetch * cachelines_per_object)), _MM_HINT_NTA);

			/*__m256i tmp_a = _mm256_stream_load_si256((__m256i*)(Source + 0));
			__m256i tmp_b = _mm256_stream_load_si256((__m256i*)(Source + 2));
			_mm256_stream_si256((__m256i*)Dest, tmp_a);
			_mm256_stream_si256((__m256i*)(Dest + 2), tmp_b);*/
			_mm256_stream_si256((__m256i*)Dest, _mm256_stream_load_si256((__m256i*)(Source + 0)));
			_mm256_stream_si256((__m256i*)(Dest + 2), _mm256_stream_load_si256((__m256i*)(Source + 2)));
			//_mm256_store_si256((__m256i*)(Dest + 0), _mm256_load_si256((__m256i*)(Source + 0)));
			//_mm256_store_si256((__m256i*)(Dest + 2), _mm256_load_si256((__m256i*)(Source + 2)));

			Dest += objects_to_prefetch;
			Source += objects_to_prefetch;
		}
	}
	else
	{
		// Do four quadwords per loop to minimize stalls.
		for (std::size_t i = CacheLines; i > 0; --i)
		{
			// If this is a large copy, start prefetching future cache lines.  This also prefetches the
			// trailing quadwords that are not part of a whole cache line.
			if (i >= objects_to_prefetch * cache_lines_for_t) _mm_prefetch((char const*)(Source + (objects_to_prefetch * cache_lines_for_t * 2)), _MM_HINT_NTA);

			_mm256_stream_si256((__m256i*)(Dest + 0), _mm256_loadu_si256((__m256i*)(Source + 0)));
			_mm256_stream_si256((__m256i*)(Dest + 2), _mm256_loadu_si256((__m256i*)(Source + 2)));

			Dest += objects_to_prefetch;
			Source += objects_to_prefetch;
		}
	}

	if constexpr (is_ptr_aligned == false) SIMDRemainingCacheLine(NumQuadwords, Dest, Source);
	_mm_sfence();
};
template<typename T, std::size_t alignment, std::size_t objects_to_copy>
__forceinline void SIMDMemCopy256(void* __restrict _Dest, const void* __restrict _Source, const bool is_ptr_aligned) noexcept
{
	static_assert(mem::aligned_by<T, alignment>() == 0ull, "type is not 16 byte aligned");
	constexpr std::size_t NumQuadwords = DivideByMultiple(sizeof(T) * objects_to_copy, alignment);
	std::size_t local_NumQuadwords = NumQuadwords;

	__m256i* __restrict Dest = (__m256i * __restrict)_Dest;
	const __m256i* __restrict Source = (const __m256i * __restrict)_Source;

	if (is_ptr_aligned == false)
	{
		if (std::is_constant_evaluated() == false)
		{
			assert(IsAligned(_Dest, 32));
			assert(IsAligned(_Source, 32));

			// Discover how many quadwords precede a cache line boundary.  Copy them separately.
			std::size_t InitialQuadwordCount = ((std::size_t)Dest % 64) / 16;
			if (InitialQuadwordCount > NumQuadwords)
				InitialQuadwordCount = NumQuadwords;

			SIMDCacheLineCopy(InitialQuadwordCount, _Dest, _Source);
			if (NumQuadwords == InitialQuadwordCount) return;

			Dest = (__m256i*)((__m128i*)Dest + InitialQuadwordCount);
			Source = (__m256i*)((__m128i*)Source + InitialQuadwordCount);
			local_NumQuadwords -= InitialQuadwordCount;
		}
	}

	const std::size_t CacheLines = std::is_constant_evaluated() ? NumQuadwords >> 2 : local_NumQuadwords >> 2;
	constexpr std::size_t cachelines_per_object = mem::cache_lines_for<T>();
	constexpr std::size_t cache_lines_for_t = 10ull / cachelines_per_object;
	constexpr std::size_t objects_to_prefetch = (expr::min)(CacheLines, cache_lines_for_t);
	constexpr auto __m256i_in_t = mem::types_in_ptr<T, __m256i>();
	mem::pre_fetch_cachelines<T, objects_to_prefetch, _MM_HINT_NTA>(Source);

	if ((((std::size_t)Source) & 31) == 0)
	{
		// Do four quadwords per loop to minimize stalls.
		for (std::size_t i = CacheLines; i > 0; --i)
		{
			// If this is a large copy, start prefetching future cache lines.  This also prefetches the
			// trailing quadwords that are not part of a whole cache line.
			if (i >= objects_to_prefetch * cache_lines_for_t) _mm_prefetch((char const*)(Source + (objects_to_prefetch * cache_lines_for_t * 2)), _MM_HINT_NTA);

			_mm256_stream_si256(&Dest[0], _mm256_stream_load_si256(Source + 0));
			_mm256_stream_si256(&Dest[1], _mm256_stream_load_si256(Source + 1));
			//_mm256_stream_si256(Dest + 0, _mm256_load_si256(Source + 0));
			//_mm256_stream_si256(Dest + 1, _mm256_load_si256(Source + 1));

			Dest += 2;
			Source += 2;
		}
	}
	else
	{
		// Do four quadwords per loop to minimize stalls.
		for (std::size_t i = CacheLines; i > 0; --i)
		{
			// If this is a large copy, start prefetching future cache lines.  This also prefetches the
			// trailing quadwords that are not part of a whole cache line.
			if (i >= objects_to_prefetch * cache_lines_for_t) _mm_prefetch((char const*)(Source + (objects_to_prefetch * cache_lines_for_t * 2)), _MM_HINT_NTA);

			_mm256_stream_si256(Dest + 0, _mm256_loadu_si256(Source + 0));
			_mm256_stream_si256(Dest + 1, _mm256_loadu_si256(Source + 1));

			Dest += 2;
			Source += 2;
		}
	}

	if (is_ptr_aligned == false) SIMDRemainingCacheLine(local_NumQuadwords, Dest, Source);
	_mm_sfence();
};