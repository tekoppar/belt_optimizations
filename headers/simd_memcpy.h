#pragma once

#include <xmmintrin.h>
#include <intrin.h>
#include <type_traits>
#include <emmintrin.h>
#include <cassert>
#include <immintrin.h>

#include "mem_utilities.h"

namespace mem
{
	template <typename T>
	inline constexpr bool is_aligned(T value, long long alignment)
	{
		return 0 == ((long long)value & (alignment - 1));
	};

	template <typename T>
	inline constexpr T divide_by_multiple(T value, long long alignment)
	{
		return (T)((value + alignment - 1) / alignment);
	};

	namespace simd
	{
		inline void cache_line_copy(long long InitialQuadwordCount, void* __restrict _Dest, const void* __restrict _Source) noexcept
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
		inline void cache_line_copy_8(long long InitialDoublewordCount, void* __restrict _Dest, const void* __restrict _Source) noexcept
		{
			__m256i* __restrict s_Dest = (__m256i * __restrict)_Dest;
			const __m256i* __restrict s_Source = (const __m256i * __restrict)_Source;

			const long long l = InitialDoublewordCount;
			for (long long i = 0; i < l; ++i)
			{
				_mm_stream_si64x(&s_Dest->m256i_i64[i], s_Source->m256i_i64[i]);
			}
		};
		inline void cache_line_copy_4(long long InitialWordCount, int* __restrict _Dest, const int* __restrict _Source) noexcept
		{
			int* __restrict s_Dest = (int* __restrict)_Dest;
			const int* __restrict s_Source = (const int* __restrict)_Source;

			for (long long i = 0; i < InitialWordCount; ++i)
			{
				_mm_stream_si32(s_Dest + i, *(s_Source + i));
			}
		};

		template<long long trailing_bytes>
		inline void remaining_trailing_bytes(const long long bytes, void* __restrict _Dest, const void* __restrict _Source) noexcept
		{
			switch (bytes - trailing_bytes)
			{
				case 3: ((short* __restrict)(((char* __restrict)_Dest) + trailing_bytes))[0] = ((short* __restrict)(((char* __restrict) _Source) + trailing_bytes))[0];
					(((char* __restrict)_Dest) + trailing_bytes)[3] = (((char* __restrict) _Source) + trailing_bytes)[3];
					return;
				case 2: ((short* __restrict)(((char* __restrict)_Dest) + trailing_bytes))[0] = ((short* __restrict)(((char* __restrict) _Source) + trailing_bytes))[0];
					return;
				case 1: (((char* __restrict)_Dest) + trailing_bytes)[0] = (((char* __restrict) _Source) + trailing_bytes)[0];
				default:
					return;
			}
			throw 0;
			/*char* __restrict _dest = ((char* __restrict)_Dest) + trailing_bytes;
			const char* __restrict _source = ((char* __restrict) _Source) + trailing_bytes;
			for (long long i = trailing_bytes; i < bytes; ++i)
				_dest[i] = _source[i];*/
		};
		inline void remaining_cache_line_aligned(long long bytes, void* __restrict _Dest, const void* __restrict _Source) noexcept
		{
			__m256i* __restrict s_Dest = (__m256i * __restrict)_Dest;
			const __m256i* __restrict s_Source = (const __m256i * __restrict)_Source;

			static_assert(64 / 4 == 16, "no");
			static_assert(61 / 4 == 15, "no");

			const long long byte_words = bytes / 4;
			switch (byte_words)
			{
				case 15:
				{
					_mm256_stream_si256(s_Dest, _mm256_load_si256(s_Source)); //0 - 7
					_mm_stream_si128((__m128i*) & (s_Dest + 1)->m256i_i32[0], _mm_load_si128((const __m128i*) & (s_Source + 1)->m256i_i32[0])); // 8 - 11
					_mm_stream_si64x((long long*)&(s_Dest + 1)->m256i_i32[4], (long long)(s_Source + 1)->m256i_i32[4]); // 12 - 13
					_mm_stream_si32(&(s_Dest + 1)->m256i_i32[6], (s_Source + 1)->m256i_i32[6]); // 14
					remaining_trailing_bytes<32ll + 16 + 8 + 4>(bytes, s_Dest, s_Source);
				}
				break;
				case 14:
				{
					_mm256_stream_si256(s_Dest, _mm256_load_si256(s_Source)); //0 - 7
					_mm_stream_si128((__m128i*) & (s_Dest + 1)->m256i_i32[0], _mm_load_si128((const __m128i*) & (s_Source + 1)->m256i_i32[0])); // 8 - 11
					_mm_stream_si64x((long long*)&(s_Dest + 1)->m256i_i32[4], (long long)(s_Source + 1)->m256i_i32[4]); // 12 - 13
					remaining_trailing_bytes<32ll + 16 + 8>(bytes, s_Dest, s_Source);
				}
				break;
				case 13:
				{
					_mm256_stream_si256(s_Dest, _mm256_load_si256(s_Source)); //0 - 7
					_mm_stream_si128((__m128i*) & (s_Dest + 1)->m256i_i32[0], _mm_load_si128((const __m128i*) & (s_Source + 1)->m256i_i32[0])); // 8 - 11
					_mm_stream_si32(&(s_Dest + 1)->m256i_i32[4], (s_Source + 1)->m256i_i32[4]); // 12
					remaining_trailing_bytes<32ll + 16 + 4>(bytes, s_Dest, s_Source);
				}
				break;
				case 12:
				{
					_mm256_stream_si256(s_Dest, _mm256_load_si256(s_Source)); //0 - 7
					_mm_stream_si128((__m128i*) & (s_Dest + 1)->m256i_i32[0], _mm_load_si128((const __m128i*) & (s_Source + 1)->m256i_i32[0])); // 8 - 11
					remaining_trailing_bytes<32ll + 16>(bytes, s_Dest, s_Source);
				}
				break;
				case 11:
				{
					_mm256_stream_si256(s_Dest, _mm256_load_si256(s_Source)); //0 - 7
					_mm_stream_si64x(&(s_Dest + 1)->m256i_i64[0], (s_Source + 1)->m256i_i64[0]); // 8 - 9
					_mm_stream_si32(&(s_Dest + 1)->m256i_i32[3], (s_Source + 1)->m256i_i32[3]); // 10
					remaining_trailing_bytes<32ll + 8 + 4>(bytes, s_Dest, s_Source);
				}
				break;
				case 10:
				{
					_mm256_stream_si256(s_Dest, _mm256_load_si256(s_Source)); //0 - 7
					_mm_stream_si64x(&(s_Dest + 1)->m256i_i64[0], (s_Source + 1)->m256i_i64[0]); // 8 - 9
					remaining_trailing_bytes<32ll + 8>(bytes, s_Dest, s_Source);
				}
				break;
				case 9:
				{
					_mm256_stream_si256(s_Dest, _mm256_load_si256(s_Source)); //0 - 7
					_mm_stream_si32(&(s_Dest + 1)->m256i_i32[0], (s_Source + 1)->m256i_i32[0]); // 8
					remaining_trailing_bytes<32ll + 4>(bytes, s_Dest, s_Source);
				}
				break;
				case 8:
				{
					_mm256_stream_si256(s_Dest, _mm256_load_si256(s_Source)); //0 - 7
					remaining_trailing_bytes<32ll>(bytes, s_Dest, s_Source);
				}
				break;
				case 7:
				{
					_mm_stream_si128((__m128i*) & (s_Dest)->m256i_i32[0], _mm_load_si128((const __m128i*) & (s_Source)->m256i_i32[0])); // 0 - 3
					_mm_stream_si64x((long long*)&(s_Dest)->m256i_i32[4], (long long)(s_Source)->m256i_i32[4]); // 4 - 5
					_mm_stream_si32(&(s_Dest)->m256i_i32[6], (s_Source)->m256i_i32[6]); // 6
					remaining_trailing_bytes<16ll + 8 + 4>(bytes, s_Dest, s_Source);
				}
				break;
				case 6:
				{
					_mm_stream_si128((__m128i*) & (s_Dest)->m256i_i32[0], _mm_load_si128((const __m128i*) & (s_Source)->m256i_i32[0])); // 0 - 3
					_mm_stream_si64x((long long*)&(s_Dest)->m256i_i32[4], (long long)(s_Source)->m256i_i32[4]); // 4 - 5
					remaining_trailing_bytes<16ll + 8>(bytes, s_Dest, s_Source);
				}
				break;
				case 5:
				{
					_mm_stream_si128((__m128i*) & (s_Dest)->m256i_i32[0], _mm_load_si128((const __m128i*) & (s_Source)->m256i_i32[0])); // 0 - 3
					_mm_stream_si32(&(s_Dest)->m256i_i32[4], (s_Source)->m256i_i32[4]); // 4
					remaining_trailing_bytes<16ll + 4>(bytes, s_Dest, s_Source);
				}
				break;
				case 4:
				{
					_mm_stream_si128((__m128i*) & (s_Dest)->m256i_i32[0], _mm_load_si128((const __m128i*) & (s_Source)->m256i_i32[0])); // 0 - 3
					remaining_trailing_bytes<16ll>(bytes, s_Dest, s_Source);
				}
				break;
				case 3:
				{
					_mm_stream_si64x((long long*)&(s_Dest)->m256i_i32[0], (long long)(s_Source)->m256i_i32[0]); // 0 - 1
					_mm_stream_si32(&(s_Dest)->m256i_i32[3], (s_Source)->m256i_i32[3]); // 3
					remaining_trailing_bytes<8ll + 4>(bytes, s_Dest, s_Source);
				}
				break;
				case 2:
				{
					_mm_stream_si64x((long long*)&(s_Dest)->m256i_i32[0], (long long)(s_Source)->m256i_i32[0]); // 0 - 1
					remaining_trailing_bytes<8ll>(bytes, s_Dest, s_Source);
				}
				break;
				case 1:
				{
					_mm_stream_si32(&(s_Dest)->m256i_i32[0], (s_Source)->m256i_i32[0]); // 0
					remaining_trailing_bytes<4ll>(bytes, s_Dest, s_Source);
				}
				break;
				default:
					remaining_trailing_bytes<0>(bytes, s_Dest, s_Source);
					break;
			}
		};
		inline void remaining_cache_line_unaligned(long long bytes, void* __restrict _Dest, const void* __restrict _Source) noexcept
		{
			__m256i* __restrict s_Dest = (__m256i * __restrict)_Dest;
			const __m256i* __restrict s_Source = (const __m256i * __restrict)_Source;

			static_assert(64 / 4 == 16, "no");
			static_assert(61 / 4 == 15, "no");

			const long long byte_words = bytes / 4;
			switch (byte_words)
			{
				case 15:
				{
					_mm256_stream_si256(s_Dest, _mm256_loadu_si256(s_Source)); //0 - 7
					_mm_stream_si128((__m128i*) & (s_Dest + 1)->m256i_i32[0], _mm_loadu_si128((const __m128i*) & (s_Source + 1)->m256i_i32[0])); // 8 - 11
					_mm_stream_si64x((long long*)&(s_Dest + 1)->m256i_i32[4], (long long)(s_Source + 1)->m256i_i32[4]); // 12 - 13
					_mm_stream_si32(&(s_Dest + 1)->m256i_i32[6], (s_Source + 1)->m256i_i32[6]); // 14
					remaining_trailing_bytes<32ll + 16 + 8 + 4>(bytes, s_Dest, s_Source);
				}
				break;
				case 14:
				{
					_mm256_stream_si256(s_Dest, _mm256_loadu_si256(s_Source)); //0 - 7
					_mm_stream_si128((__m128i*) & (s_Dest + 1)->m256i_i32[0], _mm_loadu_si128((const __m128i*) & (s_Source + 1)->m256i_i32[0])); // 8 - 11
					_mm_stream_si64x((long long*)&(s_Dest + 1)->m256i_i32[4], (long long)(s_Source + 1)->m256i_i32[4]); // 12 - 13
					remaining_trailing_bytes<32ll + 16 + 8>(bytes, s_Dest, s_Source);
				}
				break;
				case 13:
				{
					_mm256_stream_si256(s_Dest, _mm256_loadu_si256(s_Source)); //0 - 7
					_mm_stream_si128((__m128i*) & (s_Dest + 1)->m256i_i32[0], _mm_loadu_si128((const __m128i*) & (s_Source + 1)->m256i_i32[0])); // 8 - 11
					_mm_stream_si32(&(s_Dest + 1)->m256i_i32[4], (s_Source + 1)->m256i_i32[4]); // 12
					remaining_trailing_bytes<32ll + 16 + 4>(bytes, s_Dest, s_Source);
				}
				break;
				case 12:
				{
					_mm256_stream_si256(s_Dest, _mm256_loadu_si256(s_Source)); //0 - 7
					_mm_stream_si128((__m128i*) & (s_Dest + 1)->m256i_i32[0], _mm_loadu_si128((const __m128i*) & (s_Source + 1)->m256i_i32[0])); // 8 - 11
					remaining_trailing_bytes<32ll + 16>(bytes, s_Dest, s_Source);
				}
				break;
				case 11:
				{
					_mm256_stream_si256(s_Dest, _mm256_loadu_si256(s_Source)); //0 - 7
					_mm_stream_si64x(&(s_Dest + 1)->m256i_i64[0], (s_Source + 1)->m256i_i64[0]); // 8 - 9
					_mm_stream_si32(&(s_Dest + 1)->m256i_i32[3], (s_Source + 1)->m256i_i32[3]); // 10
					remaining_trailing_bytes<32ll + 8 + 4>(bytes, s_Dest, s_Source);
				}
				break;
				case 10:
				{
					_mm256_stream_si256(s_Dest, _mm256_loadu_si256(s_Source)); //0 - 7
					_mm_stream_si64x(&(s_Dest + 1)->m256i_i64[0], (s_Source + 1)->m256i_i64[0]); // 8 - 9
					remaining_trailing_bytes<32ll + 8>(bytes, s_Dest, s_Source);
				}
				break;
				case 9:
				{
					_mm256_stream_si256(s_Dest, _mm256_loadu_si256(s_Source)); //0 - 7
					_mm_stream_si32(&(s_Dest + 1)->m256i_i32[0], (s_Source + 1)->m256i_i32[0]); // 8
					remaining_trailing_bytes<32ll + 4>(bytes, s_Dest, s_Source);
				}
				break;
				case 8:
				{
					_mm256_stream_si256(s_Dest, _mm256_loadu_si256(s_Source)); //0 - 7
					remaining_trailing_bytes<32ll>(bytes, s_Dest, s_Source);
				}
				break;
				case 7:
				{
					_mm_stream_si128((__m128i*) & (s_Dest)->m256i_i32[0], _mm_loadu_si128((const __m128i*) & (s_Source)->m256i_i32[0])); // 0 - 3
					_mm_stream_si64x((long long*)&(s_Dest)->m256i_i32[4], (long long)(s_Source)->m256i_i32[4]); // 4 - 5
					_mm_stream_si32(&(s_Dest)->m256i_i32[6], (s_Source)->m256i_i32[6]); // 6
					remaining_trailing_bytes<16ll + 8 + 4>(bytes, s_Dest, s_Source);
				}
				break;
				case 6:
				{
					_mm_stream_si128((__m128i*) & (s_Dest)->m256i_i32[0], _mm_loadu_si128((const __m128i*) & (s_Source)->m256i_i32[0])); // 0 - 3
					_mm_stream_si64x((long long*)&(s_Dest)->m256i_i32[4], (long long)(s_Source)->m256i_i32[4]); // 4 - 5
					remaining_trailing_bytes<16ll + 8>(bytes, s_Dest, s_Source);
				}
				break;
				case 5:
				{
					_mm_stream_si128((__m128i*) & (s_Dest)->m256i_i32[0], _mm_loadu_si128((const __m128i*) & (s_Source)->m256i_i32[0])); // 0 - 3
					_mm_stream_si32(&(s_Dest)->m256i_i32[4], (s_Source)->m256i_i32[4]); // 4
					remaining_trailing_bytes<16ll + 4>(bytes, s_Dest, s_Source);
				}
				break;
				case 4:
				{
					_mm_stream_si128((__m128i*) & (s_Dest)->m256i_i32[0], _mm_loadu_si128((const __m128i*) & (s_Source)->m256i_i32[0])); // 0 - 3
					remaining_trailing_bytes<16ll>(bytes, s_Dest, s_Source);
				}
				break;
				case 3:
				{
					_mm_stream_si64x((long long*)&(s_Dest)->m256i_i32[0], (long long)(s_Source)->m256i_i32[0]); // 0 - 1
					_mm_stream_si32(&(s_Dest)->m256i_i32[3], (s_Source)->m256i_i32[3]); // 3
					remaining_trailing_bytes<8ll + 4>(bytes, s_Dest, s_Source);
				}
				break;
				case 2:
				{
					_mm_stream_si64x((long long*)&(s_Dest)->m256i_i32[0], (long long)(s_Source)->m256i_i32[0]); // 0 - 1
					remaining_trailing_bytes<8ll>(bytes, s_Dest, s_Source);
				}
				break;
				case 1:
				{
					_mm_stream_si32(&(s_Dest)->m256i_i32[0], (s_Source)->m256i_i32[0]); // 0
					remaining_trailing_bytes<4ll>(bytes, s_Dest, s_Source);
				}
				break;
				default:
					remaining_trailing_bytes<0>(bytes, s_Dest, s_Source);
					break;
			}
		};
		inline void remaining_cache_line(long long NumQuadwords, void* __restrict _Dest, const void* __restrict _Source) noexcept
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
		inline void remaining_cache_line_8(long long NumDoublewords, void* __restrict _Dest, const void* __restrict _Source) noexcept
		{
			__m256i* __restrict s_Dest = (__m256i * __restrict)_Dest;
			const __m256i* __restrict s_Source = (const __m256i * __restrict)_Source;

			const auto rem = NumDoublewords & 7;
			const long long l = rem;
			for (long long i = 0; i < l; ++i)
			{
				_mm_stream_si64x(&s_Dest->m256i_i64[i], s_Source->m256i_i64[i]);
			}
		};
		inline void remaining_cache_line_4(long long InitialWordCount, void* __restrict _Dest, const void* __restrict _Source) noexcept
		{
			__m256i* __restrict s_Dest = (__m256i * __restrict)_Dest;
			const __m256i* __restrict s_Source = (const __m256i * __restrict)_Source;

			const auto rem = InitialWordCount & 15;
			const long long l = rem;
			for (long long i = 0; i < l; ++i)
			{
				_mm_stream_si32(&s_Dest->m256i_i32[i], s_Source->m256i_i32[i]);
			}
		};

		template<typename T>
		inline void mem_copy_256_loop(__m256i* __restrict Dest, const __m256i* __restrict Source, const long long CacheLines) noexcept
		{
			constexpr long long cache_lines_per_object = mem::cache_lines_for<T>();
			constexpr long long cache_lines_for_t = expr::max(10ll / cache_lines_per_object, 1ll);
			constexpr long long objects_to_prefetch = 4ll;
			constexpr long long pre_fetch_limit = objects_to_prefetch * cache_lines_for_t;

			if (is_aligned(Source, 32ll) && is_aligned(Dest, 32ll))
			{
				// Do four quadwords per loop to minimize stalls.
				long long i = CacheLines;
				for (; i > 0ll; --i)
				{
					// If this is a large copy, start prefetching future cache lines.  This also prefetches the
					// trailing quadwords that are not part of a whole cache line.
					_mm_prefetch((char const*)(Source + pre_fetch_limit), _MM_HINT_NTA);
					_mm256_stream_si256(Dest, _mm256_stream_load_si256(Source));
					_mm256_stream_si256(Dest + 1ll, _mm256_stream_load_si256(Source + 1ll));
					Dest += 2ll;
					Source += 2ll;
				}

				__assume(i > 0ll);
				for (; i > 0ll; --i)
				{
					_mm256_stream_si256(Dest, _mm256_stream_load_si256(Source));
					_mm256_stream_si256(Dest + 1ll, _mm256_stream_load_si256(Source + 1ll));
					Dest += 2ll;
					Source += 2ll;
				}
			}
			else
			{
				// Do four quadwords per loop to minimize stalls.
				long long i = CacheLines;
				for (; i > pre_fetch_limit; --i)
				{
					// If this is a large copy, start prefetching future cache lines.  This also prefetches the
					// trailing quadwords that are not part of a whole cache line.
					_mm_prefetch((char const*)(Source + pre_fetch_limit), _MM_HINT_NTA);
					_mm256_stream_si256(Dest, _mm256_loadu_si256(Source));
					_mm256_stream_si256(Dest + 1ll, _mm256_loadu_si256(Source + 1ll));
					Dest += 2ll;
					Source += 2ll;
				}

				__assume(i > 0ll);
				for (; i > 0ll; --i)
				{
					_mm256_stream_si256(Dest, _mm256_loadu_si256(Source));
					_mm256_stream_si256(Dest + 1ll, _mm256_loadu_si256(Source + 1ll));
					Dest += 2ll;
					Source += 2ll;
				}
			}
		};

		[[deprecated]] inline void SIMDMemCopy(void* __restrict _Dest, const void* __restrict _Source, long long NumQuadwords) noexcept
		{
			assert(is_aligned(_Dest, 16));
			assert(is_aligned(_Source, 16));

			__m128i* __restrict Dest = (__m128i * __restrict)_Dest;
			const __m128i* __restrict Source = (const __m128i * __restrict)_Source;

			// Discover how many quadwords precede a cache line boundary.  Copy them separately.
			long long InitialQuadwordCount = (4 - ((long long)Source >> 4) & 3) & 3;
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

			const long long CacheLines = NumQuadwords >> 2;

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
					for (long long i = CacheLines; i > 0; --i)
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
					break;

				case 0: break;	// No whole cache lines to read
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

		inline void SIMDMemCopy256(void* __restrict _Dest, const void* __restrict _Source, long long NumQuadwords) noexcept
		{
			assert(is_aligned(_Dest, 16));
			assert(is_aligned(_Source, 16));

			__m256i* __restrict Dest = (__m256i * __restrict)_Dest;
			const __m256i* __restrict Source = (const __m256i * __restrict)_Source;

			// Discover how many quadwords precede a cache line boundary.  Copy them separately.
			long long InitialQuadwordCount = ((long long)Dest % 64) / 16;
			if (InitialQuadwordCount > NumQuadwords)
				InitialQuadwordCount = NumQuadwords;

			cache_line_copy(InitialQuadwordCount, _Dest, _Source);
			if (NumQuadwords == InitialQuadwordCount) return;

			Dest = (__m256i*)((__m128i*)Dest + InitialQuadwordCount);
			Source = (__m256i*)((__m128i*)Source + InitialQuadwordCount);
			NumQuadwords -= InitialQuadwordCount;

			const long long CacheLines = NumQuadwords >> 2;
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

					//if ((((long long)Source % 64) / 32) == 0)
					if ((((long long)Source) & 31) == 0)
					{
						// Do four quadwords per loop to minimize stalls.
						for (long long i = CacheLines; i > 0; --i)
						{
							// If this is a large copy, start prefetching future cache lines.  This also prefetches the
							// trailing quadwords that are not part of a whole cache line.
							if (i >= 10) _mm_prefetch((char const*)(Source + 20), _MM_HINT_NTA);

							_mm256_stream_si256(Dest + 0, _mm256_load_si256(Source + 0));
							_mm256_stream_si256(Dest + 1, _mm256_load_si256(Source + 1));

							Dest += 2;
							Source += 2;
						}
					}
					else
					{
						// Do four quadwords per loop to minimize stalls.
						for (long long i = CacheLines; i > 0; --i)
						{
							// If this is a large copy, start prefetching future cache lines.  This also prefetches the
							// trailing quadwords that are not part of a whole cache line.
							if (i >= 10)
								_mm_prefetch((char const*)(Source + 20), _MM_HINT_NTA);

							_mm256_stream_si256(Dest + 0, _mm256_loadu_si256(Source + 0));
							_mm256_stream_si256(Dest + 1, _mm256_loadu_si256(Source + 1));

							Dest += 2;
							Source += 2;
						}
					}
					break;

				case 0: break;	// No whole cache lines to read
			}

			// Copy the remaining quadwords
			remaining_cache_line(NumQuadwords, Dest, Source);

			_mm_sfence();
		};

		template<typename T, long long alignment, long long objects_to_copy>
		[[deprecated]] inline void SIMDMemCopy256(void* __restrict _Dest, const void* __restrict _Source, const bool is_ptr_aligned) noexcept
		{
			static_assert(mem::aligned_by<T, alignment>() == 0ll, "type is not 16 byte aligned");
			constexpr long long NumQuadwords = divide_by_multiple(sizeof(T) * objects_to_copy, alignment);
			long long local_NumQuadwords = NumQuadwords;

			__m256i* __restrict Dest = (__m256i * __restrict)_Dest;
			const __m256i* __restrict Source = (const __m256i * __restrict)_Source;

			if (is_ptr_aligned == false)
			{
				if (std::is_constant_evaluated() == false)
				{
					assert(is_aligned(_Dest, 32));
					assert(is_aligned(_Source, 32));

					// Discover how many quadwords precede a cache line boundary.  Copy them separately.
					long long InitialQuadwordCount = ((long long)Dest % 64) / 16;
					if (InitialQuadwordCount > NumQuadwords)
						InitialQuadwordCount = NumQuadwords;

					cache_line_copy(InitialQuadwordCount, _Dest, _Source);
					if (NumQuadwords == InitialQuadwordCount) return;

					Dest = (__m256i*)((__m128i*)Dest + InitialQuadwordCount);
					Source = (__m256i*)((__m128i*)Source + InitialQuadwordCount);
					local_NumQuadwords -= InitialQuadwordCount;
				}
			}

			const long long CacheLines = std::is_constant_evaluated() ? NumQuadwords >> 2 : local_NumQuadwords >> 2;
			constexpr long long cachelines_per_object = mem::cache_lines_for<T>();
			constexpr long long cache_lines_for_t = 10ll / cachelines_per_object;
			constexpr long long objects_to_prefetch = (expr::min)(CacheLines, cache_lines_for_t);
			constexpr auto __m256i_in_t = mem::types_in_ptr<T, __m256i>();
			mem::pre_fetch_cachelines<T, objects_to_prefetch, _MM_HINT_NTA>(Source);

			if ((((long long)Source) & 31) == 0)
			{
				// Do four quadwords per loop to minimize stalls.
				for (long long i = CacheLines; i > 0; --i)
				{
					// If this is a large copy, start prefetching future cache lines.  This also prefetches the
					// trailing quadwords that are not part of a whole cache line.
					if (i >= objects_to_prefetch * cache_lines_for_t) _mm_prefetch((char const*)(Source + (objects_to_prefetch * cache_lines_for_t * 2)), _MM_HINT_NTA);

					_mm256_stream_si256(&Dest[0], _mm256_stream_load_si256(Source + 0));
					_mm256_stream_si256(&Dest[1], _mm256_stream_load_si256(Source + 1));

					Dest += 2;
					Source += 2;
				}
			}
			else
			{
				// Do four quadwords per loop to minimize stalls.
				for (long long i = CacheLines; i > 0; --i)
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

			if (is_ptr_aligned == false) remaining_cache_line(local_NumQuadwords, Dest, Source);
			_mm_sfence();
		};

		namespace no_direct_calls
		{
			//don't directly call this function instead call SIMDMemCopy256 since these functions depend on pre-requisites from said function
			template<typename T, bool is_ptr_aligned>
			inline void mem_copy_256_32(__m256i* __restrict Dest, const __m256i* __restrict Source, long long objects_to_copy, long long NumQuadwords) noexcept
			{
				static_assert(mem::aligned_by<T, 32ll>() == 0ll, "type is not 32 byte aligned");
				if constexpr (is_ptr_aligned == false)
				{
					if (std::is_constant_evaluated() == false)
					{
#ifdef _DEBUG
						assert(is_aligned(Dest, 32));
						assert(is_aligned(Source, 32));
#endif
						// Discover how many quadwords precede a cache line boundary.  Copy them separately.
						long long InitialQuadwordCount = ((long long)Dest % 64ll) / 16ll;

						cache_line_copy(InitialQuadwordCount, Dest, Source);
						Dest = (__m256i*)((__m128i*)Dest + InitialQuadwordCount);
						Source = (__m256i*)((__m128i*)Source + InitialQuadwordCount);
						NumQuadwords -= InitialQuadwordCount;
					}
				}

				const long long cache_lines = NumQuadwords >> 2;
				mem_copy_256_loop<T>(Dest, Source, cache_lines);

				if constexpr (is_ptr_aligned == false) remaining_cache_line(NumQuadwords, Dest, Source);
				_mm_sfence();
			};
			//don't directly call this function instead call SIMDMemCopy256 since these functions depend on pre-requisites from said function
			template<typename T, bool is_ptr_aligned>
			inline void mem_copy_256_16(__m256i* __restrict Dest, const __m256i* __restrict Source, long long objects_to_copy, long long NumQuadwords) noexcept
			{
				static_assert(mem::aligned_by<T, 16ll>() == 0ll, "type is not 16 byte aligned");
				if constexpr (is_ptr_aligned == false)
				{
					if (std::is_constant_evaluated() == false)
					{
#ifdef _DEBUG
						assert(is_aligned(Dest, sizeof(T)));
						assert(is_aligned(Source, sizeof(T)));
#endif
						// Discover how many quadwords precede a cache line boundary.  Copy them separately.
						long long InitialQuadwordCount = ((long long)Dest % 64) / 16;

						cache_line_copy(InitialQuadwordCount, Dest, Source);
						Dest = (__m256i*)((__m128i*)Dest + InitialQuadwordCount);
						Source = (__m256i*)((__m128i*)Source + InitialQuadwordCount);
						NumQuadwords -= InitialQuadwordCount;
					}
				}

				const long long cache_lines = NumQuadwords >> 2;
				mem_copy_256_loop<T>(Dest, Source, cache_lines);

				if constexpr (is_ptr_aligned == false) remaining_cache_line(NumQuadwords, Dest, Source);
				_mm_sfence();
			};
			//don't directly call this function instead call SIMDMemCopy256 since these functions depend on pre-requisites from said function
			template<typename T, bool is_ptr_aligned>
			inline void mem_copy_256_8(__m256i* __restrict Dest, const __m256i* __restrict Source, long long objects_to_copy, long long NumQuadwords) noexcept
			{
				static_assert(mem::aligned_by<T, 8ll>() == 0ll, "type is not 8 byte aligned");
				if constexpr (is_ptr_aligned == false)
				{
					if (std::is_constant_evaluated() == false)
					{
#ifdef _DEBUG
						assert(is_aligned(Dest, sizeof(T)));
						assert(is_aligned(Source, sizeof(T)));
#endif
						// Discover how many quadwords precede a cache line boundary.  Copy them separately.
						auto aligned_ptr_number = 64ll - ((long long)Dest % 64ll);
						long long InitialQuadwordCount = ((0ll ^ aligned_ptr_number) & (64ll ^ aligned_ptr_number)) / 8ll;

						cache_line_copy_8(InitialQuadwordCount, Dest, Source);
						Dest = (__m256i*)(((long long*)Dest) + InitialQuadwordCount);
						Source = (__m256i*)(((long long*)Source) + InitialQuadwordCount);
						NumQuadwords -= InitialQuadwordCount;
					}
				}

				const long long cache_lines = NumQuadwords / 8;
				mem_copy_256_loop<T>(Dest, Source, cache_lines);

				if constexpr (is_ptr_aligned == false) remaining_cache_line_8(NumQuadwords, Dest, Source);
				_mm_sfence();
			};
			//don't directly call this function instead call SIMDMemCopy256 since these functions depend on pre-requisites from said function
			template<typename T, bool is_ptr_aligned>
			inline void mem_copy_256_4(__m256i* __restrict Dest, const __m256i* __restrict Source, long long objects_to_copy, long long NumDoublewords) noexcept
			{
				static_assert(mem::aligned_by<T, 4ll>() == 0ll, "type is not 8 byte aligned");
				if constexpr (is_ptr_aligned == false)
				{
					if (std::is_constant_evaluated() == false)
					{
#ifdef _DEBUG
						assert(is_aligned(Dest, sizeof(T)));
						assert(is_aligned(Source, sizeof(T)));
#endif
						// Discover how many doublewords precede a cache line boundary.  Copy them separately.
						auto aligned_ptr_number = 64ll - ((long long)Dest % 64ll);
						long long InitialDoublewordCount = ((0ll ^ aligned_ptr_number) & (64ll ^ aligned_ptr_number)) / 4ll;//aligned_ptr_number == 64 || aligned_ptr_number == 0 ? 0 : (64 - aligned_ptr_number) / 4;

						cache_line_copy_4(InitialDoublewordCount, (int*)Dest, (const int*)Source);
						Dest = (__m256i*)(((int*)Dest) + InitialDoublewordCount);
						Source = (__m256i*)(((int*)Source) + InitialDoublewordCount);
						NumDoublewords -= InitialDoublewordCount;
					}
				}

				const long long cache_lines = NumDoublewords / 16;
				mem_copy_256_loop<T>(Dest, Source, cache_lines);

				if constexpr (is_ptr_aligned == false) remaining_cache_line_4(NumDoublewords, Dest, Source);
				_mm_sfence();
			};
			//don't directly call this function instead call SIMDMemCopy256 since these functions depend on pre-requisites from said function
			template<typename T, bool is_ptr_aligned>
			inline void mem_copy_256_1(__m256i* __restrict Dest, const __m256i* __restrict Source, long long objects_to_copy, long long NumDoublewords, long long remaining_chars) noexcept
			{
				static_assert(sizeof(T) == 1ull, "type is not 8 byte aligned");
				if constexpr (is_ptr_aligned == false)
				{
					if (std::is_constant_evaluated() == false)
					{
#ifdef _DEBUG
						assert(is_aligned(Dest, sizeof(T)));
						assert(is_aligned(Source, sizeof(T)));
#endif
						// Discover how many doublewords precede a cache line boundary.  Copy them separately.
						auto aligned_ptr_number = 64ll - ((long long)Dest % 64ll);
						long long InitialDoublewordCount = ((0ll ^ aligned_ptr_number) & (64ll ^ aligned_ptr_number)) / 4ll;//aligned_ptr_number == 64 || aligned_ptr_number == 0 ? 0 : (64 - aligned_ptr_number) / 4;

						cache_line_copy_4(InitialDoublewordCount, (int*)Dest, (const int*)Source);
						Dest = (__m256i*)(((int*)Dest) + InitialDoublewordCount);
						Source = (__m256i*)(((int*)Source) + InitialDoublewordCount);
						NumDoublewords -= InitialDoublewordCount;
					}
				}

				const long long cache_lines = NumDoublewords >> 2;
				mem_copy_256_loop<T>(Dest, Source, cache_lines);

				if (remaining_chars > 0)
				{
					if constexpr (is_ptr_aligned == false) remaining_cache_line_unaligned(remaining_chars, Dest + cache_lines, Source + cache_lines);
					else remaining_cache_line_aligned(remaining_chars, Dest + cache_lines, Source + cache_lines);
				}
				/*if (remaining_chars > 0)
				{
					remaining_cache_line_m();

					auto offset = cache_lines * 64;
					char* _dest = ((char*)Dest) + offset;
					const char* _source = ((char*)Source) + offset;

					for (long long i = 0; i < remaining_chars; ++i)
						_dest[i] = _source[i];
				}*/

				if constexpr (is_ptr_aligned == false)
					remaining_cache_line_4(NumDoublewords, Dest, Source);
				_mm_sfence();
			};
		};

		template<typename T, bool is_ptr_aligned>
		constexpr inline void mem_copy_256(void* __restrict _Dest, const void* __restrict _Source, const long long objects_to_copy) noexcept
		{
			__m256i* __restrict Dest = (__m256i * __restrict)_Dest;
			const __m256i* __restrict Source = (const __m256i * __restrict)_Source;

			if constexpr (mem::aligned_by<T, 4ll>() == 0ll)
			{
				long long NumDoublewords = divide_by_multiple(sizeof(T) * objects_to_copy, 4ll);
				if (NumDoublewords < 16) cache_line_copy_4(NumDoublewords, (int*)_Dest, (const int*)_Source);
				else no_direct_calls::mem_copy_256_4<T, is_ptr_aligned>(Dest, Source, objects_to_copy, NumDoublewords);
			}
			else if constexpr (mem::aligned_by<T, 8ll>() == 0ll)
			{
				long long NumQuadwords = divide_by_multiple(sizeof(T) * objects_to_copy, 8ll);
				if (NumQuadwords < 8) cache_line_copy_8(NumQuadwords, _Dest, _Source);
				else no_direct_calls::mem_copy_256_8<T, is_ptr_aligned>(Dest, Source, objects_to_copy, NumQuadwords);
			}
			else if constexpr (mem::aligned_by<T, 16ll>() == 0ll)
			{
				long long NumQuadwords = divide_by_multiple(sizeof(T) * objects_to_copy, 16ll);
				if (NumQuadwords < 4) cache_line_copy(NumQuadwords, _Dest, _Source);
				else no_direct_calls::mem_copy_256_16<T, is_ptr_aligned>(Dest, Source, objects_to_copy);
			}
			else if constexpr (mem::aligned_by<T, 32ll>() == 0ll)
			{
				long long NumQuadwords = divide_by_multiple(sizeof(T) * objects_to_copy, 16ll);
				if (NumQuadwords < 4) cache_line_copy(NumQuadwords, _Dest, _Source);
				else no_direct_calls::mem_copy_256_32<T, is_ptr_aligned>(Dest, Source, objects_to_copy);
			}
			else if constexpr (sizeof(T) == 1ull)
			{
				long long NumQuadwords = divide_by_multiple(sizeof(T) * objects_to_copy, 16ll);
				long long remaining = objects_to_copy % 64;
				if (NumQuadwords < 4) cache_line_copy(NumQuadwords, _Dest, _Source);
				else no_direct_calls::mem_copy_256_1<T, is_ptr_aligned>(Dest, Source, objects_to_copy, NumQuadwords, remaining);
			}
			else static_assert(mem::aligned_by<T, -1ll>() == 0ll, "T is not aligned on 4, 8, 16, 32 or 64 bytes");
		};
	};
};