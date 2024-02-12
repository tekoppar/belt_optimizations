#pragma once

#include <xmmintrin.h>
#include <intrin.h>
#include <assert.h>
#include <cstdint>
#include <cstddef>

template <typename T>
__forceinline bool IsAligned(T value, std::size_t alignment)
{
	//return ((reinterpret_cast<std::uintptr_t>(value) % alignment) == 0);
	return 0 == ((std::size_t)value & (alignment - 1));
};

template <typename T>
__forceinline constexpr T DivideByMultiple(T value, std::size_t alignment)
{
	return (T)((value + alignment - 1) / alignment);
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
	case 3: _mm_stream_si128(Dest + 2, _mm_load_si128(Source + 2));	 // Fall through
	case 2: _mm_stream_si128(Dest + 1, _mm_load_si128(Source + 1));	 // Fall through
	case 1: _mm_stream_si128(Dest + 0, _mm_load_si128(Source + 0));	 // Fall through
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
	case 10: _mm_prefetch((char const*)(Source + 36), _MM_HINT_NTA);	// Fall through
	case 9:  _mm_prefetch((char const*)(Source + 32), _MM_HINT_NTA);	// Fall through
	case 8:  _mm_prefetch((char const*)(Source + 28), _MM_HINT_NTA);	// Fall through
	case 7:  _mm_prefetch((char const*)(Source + 24), _MM_HINT_NTA);	// Fall through
	case 6:  _mm_prefetch((char const*)(Source + 20), _MM_HINT_NTA);	// Fall through
	case 5:  _mm_prefetch((char const*)(Source + 16), _MM_HINT_NTA);	// Fall through
	case 4:  _mm_prefetch((char const*)(Source + 12), _MM_HINT_NTA);	// Fall through
	case 3:  _mm_prefetch((char const*)(Source + 8), _MM_HINT_NTA);	// Fall through
	case 2:  _mm_prefetch((char const*)(Source + 4), _MM_HINT_NTA);	// Fall through
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
	case 3: _mm_stream_si128(Dest + 2, _mm_load_si128(Source + 2));	 // Fall through
	case 2: _mm_stream_si128(Dest + 1, _mm_load_si128(Source + 1));	 // Fall through
	case 1: _mm_stream_si128(Dest + 0, _mm_load_si128(Source + 0));	 // Fall through
	default:
		break;
	}

	_mm_sfence();
};

void SIMDCacheLineCopy(std::size_t InitialQuadwordCount, void* __restrict _Dest, const void* __restrict _Source) noexcept;
void SIMDRemainingCacheLine(std::size_t NumQuadwords, void* __restrict _Dest, const void* __restrict _Source) noexcept;
void SIMDMemCopy256(void* __restrict _Dest, const void* __restrict _Source, std::size_t NumQuadwords) noexcept;