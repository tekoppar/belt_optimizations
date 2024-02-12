#include "simd_memcpy.h"

void SIMDCacheLineCopy(size_t InitialQuadwordCount, void* __restrict _Dest, const void* __restrict _Source) noexcept
{
	__m128i* __restrict s_Dest = (__m128i * __restrict)_Dest;
	const __m128i* __restrict s_Source = (const __m128i * __restrict)_Source;

	switch (InitialQuadwordCount)
	{
	case 3: _mm_stream_si128(s_Dest + 2, _mm_load_si128(s_Source + 2));	 // Fall through
	case 2: _mm_stream_si128(s_Dest + 1, _mm_load_si128(s_Source + 1));	 // Fall through
	case 1: _mm_stream_si128(s_Dest + 0, _mm_load_si128(s_Source + 0));	 // Fall through
	default:
		break;
	}
}

void SIMDRemainingCacheLine(size_t NumQuadwords, void* __restrict _Dest, const void* __restrict _Source) noexcept
{
	__m128i* __restrict s_Dest = (__m128i * __restrict)_Dest;
	const __m128i* __restrict s_Source = (const __m128i * __restrict)_Source;

	// Copy the remaining quadwords
	switch (NumQuadwords & 3)
	{
	case 3: _mm_stream_si128(s_Dest + 2, _mm_load_si128(s_Source + 2));	 // Fall through
	case 2: _mm_stream_si128(s_Dest + 1, _mm_load_si128(s_Source + 1));	 // Fall through
	case 1: _mm_stream_si128(s_Dest + 0, _mm_load_si128(s_Source + 0));	 // Fall through
	default:
		break;
	}
}

void SIMDMemCopy256(void* __restrict _Dest, const void* __restrict _Source, size_t NumQuadwords) noexcept
{
	assert(IsAligned(_Dest, 16));
	assert(IsAligned(_Source, 16));

	__m256i* __restrict Dest = (__m256i * __restrict)_Dest;
	const __m256i* __restrict Source = (const __m256i * __restrict)_Source;

	// Discover how many quadwords precede a cache line boundary.  Copy them separately.
	size_t InitialQuadwordCount = ((size_t)Dest % 64) / 16;
	if (InitialQuadwordCount > NumQuadwords)
		InitialQuadwordCount = NumQuadwords;

	/*switch (InitialQuadwordCount)
	{
		//case 3: _mm_stream_si128(s_Dest + 2, _mm_load_si128(s_Source + 2));	 // Fall through
		//case 2: _mm_stream_si128(s_Dest + 1, _mm_load_si128(s_Source + 1));	 // Fall through
		case 1: _mm256_stream_si256(Dest + 0, _mm256_load_si256(Source + 0));	 // Fall through
		default:
			break;
	}*/
	SIMDCacheLineCopy(InitialQuadwordCount, _Dest, _Source);

	if (NumQuadwords == InitialQuadwordCount)
		return;

	Dest = (__m256i*)((__m128i*)Dest + InitialQuadwordCount);
	Source = (__m256i*)((__m128i*)Source + InitialQuadwordCount);
	NumQuadwords -= InitialQuadwordCount;

	const size_t CacheLines = NumQuadwords >> 2;

	switch (CacheLines)
	{
	default:
	case 10: _mm_prefetch((char const*)(Source + 18), _MM_HINT_NTA);	// Fall through
	case 9:  _mm_prefetch((char const*)(Source + 16), _MM_HINT_NTA);	// Fall through
	case 8:  _mm_prefetch((char const*)(Source + 14), _MM_HINT_NTA);	// Fall through
	case 7:  _mm_prefetch((char const*)(Source + 12), _MM_HINT_NTA);	// Fall through
	case 6:  _mm_prefetch((char const*)(Source + 10), _MM_HINT_NTA);	// Fall through
	case 5:  _mm_prefetch((char const*)(Source + 8), _MM_HINT_NTA);	// Fall through
	case 4:  _mm_prefetch((char const*)(Source + 6), _MM_HINT_NTA);	// Fall through
	case 3:  _mm_prefetch((char const*)(Source + 4), _MM_HINT_NTA);		// Fall through
	case 2:  _mm_prefetch((char const*)(Source + 2), _MM_HINT_NTA);		// Fall through
	case 1:  _mm_prefetch((char const*)(Source + 0), _MM_HINT_NTA);		// Fall through

		//if ((((size_t)Source % 64) / 32) == 0)
		if ((((size_t)Source) & 31) == 0)
		{
			// Do four quadwords per loop to minimize stalls.
			for (size_t i = CacheLines; i > 0; --i)
			{
				// If this is a large copy, start prefetching future cache lines.  This also prefetches the
				// trailing quadwords that are not part of a whole cache line.
				if (i >= 10)
					_mm_prefetch((char const*)(Source + 20), _MM_HINT_NTA);

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
			for (size_t i = CacheLines; i > 0; --i)
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
}