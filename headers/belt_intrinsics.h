#pragma once

#include <immintrin.h>
#include <cstddef>
#include <emmintrin.h>
#include <tmmintrin.h>

namespace belt_utility
{
	__forceinline static void _mm256_add_si256__p(__m256i* a, __m256i* b) noexcept
	{
		_mm256_store_si256(a, _mm256_add_epi8(_mm256_load_si256(a), _mm256_load_si256(b)));
	};

	__forceinline static void _mm256_add64_si256__p(__m256i* a, const __m256i& b) noexcept
	{
		_mm256_store_si256(a, _mm256_add_epi64(_mm256_load_si256(a), b));
	};

	__forceinline static void _mm256_sub64_si256__p(__m256i* a, const __m256i& b) noexcept
	{
		_mm256_store_si256(a, _mm256_sub_epi64(_mm256_load_si256(a), b));
	};

	template<std::size_t shift_left>
	__forceinline static constexpr void _mm256_slli_si256__p(__m256i* a)
	{
		if constexpr (0 < shift_left && shift_left < 16) _mm256_store_si256(a, _mm256_alignr_epi8(_mm256_load_si256(a), _mm256_permute2x128_si256(_mm256_load_si256(a), _mm256_load_si256(a), _MM_SHUFFLE(0, 0, 2, 0)), 16 - shift_left));
		else if constexpr (shift_left == 0) _mm256_store_si256(a, _mm256_permute2x128_si256(_mm256_load_si256(a), _mm256_load_si256(a), _MM_SHUFFLE(0, 0, 2, 0)));
		else if constexpr (16 < shift_left && shift_left < 32) _mm256_store_si256(a, _mm256_slli_si256(_mm256_permute2x128_si256(_mm256_load_si256(a), _mm256_load_si256(a), _MM_SHUFFLE(0, 0, 2, 0)), shift_left - 16));
	};

	template<std::size_t shift_right>
	__forceinline static constexpr void _mm256_srli_si256__p(__m256i* a)
	{
		if constexpr (0 < shift_right && shift_right < 16) _mm256_store_si256(a, _mm256_alignr_epi8(_mm256_permute2x128_si256(_mm256_load_si256(a), _mm256_load_si256(a), _MM_SHUFFLE(2, 0, 0, 1)), _mm256_load_si256(a), shift_right));
		else if constexpr (shift_right == 0) _mm256_store_si256(a, _mm256_permute2x128_si256(_mm256_load_si256(a), _mm256_load_si256(a), _MM_SHUFFLE(2, 0, 0, 1)));
		else if constexpr (16 < shift_right && shift_right < 32) _mm256_store_si256(a, _mm256_srli_si256(_mm256_permute2x128_si256(_mm256_load_si256(a), _mm256_load_si256(a), _MM_SHUFFLE(2, 0, 0, 1)), shift_right - 16));
	};

	template<std::size_t shift_left>
	__forceinline static constexpr void _mm256_slli_bit__p(__m256i* a)
	{
		_rotl64(a->m256i_i64[0], shift_left);
		_rotl64(a->m256i_i64[1], shift_left);
		_rotl64(a->m256i_i64[2], shift_left);
		_rotl64(a->m256i_i64[3], shift_left);
		auto tmp = a->m256i_i64[3];

		_mm256_store_si256(a, _mm256_slli_si256(_mm256_permute2x128_si256(_mm256_load_si256(a), _mm256_load_si256(a), _MM_SHUFFLE(0, 0, 2, 0)), shift_left - 16));
	};

	template<std::size_t i>
	__forceinline static constexpr __m256i _mm256_slli_si256__(__m256i a)
	{
		return _mm256_alignr_epi8(a, _mm256_permute2x128_si256(a, a, _MM_SHUFFLE(0, 0, 2, 0)), 16 - i);
	};

	template<std::size_t i>
	__forceinline static constexpr void _mm512_slli2x256_si512__(__m256i* a)
	{
		a[1] = _mm256_alignr_epi8(a[1], _mm256_permute2x128_si256(a[1], a[1], _MM_SHUFFLE(0, 0, 2, 0)), 16 - i);
		a[1].m256i_i16[0] = a[0].m256i_i16[15];
		a[0] = _mm256_alignr_epi8(a[0], _mm256_permute2x128_si256(a[0], a[0], _MM_SHUFFLE(0, 0, 2, 0)), 16 - i);
	};

	template<std::size_t shift_right>
	__forceinline static constexpr void _mm512_srli2x256_si512__(__m256i* a)
	{
		/*a[0] = _mm256_alignr_epi8(_mm256_permute2x128_si256(a[0], a[0], _MM_SHUFFLE(2, 0, 0, 1)), a[0], shift_right);
		a[0].m256i_i16[15] = a[1].m256i_i16[0];
		a[1] = _mm256_alignr_epi8(_mm256_permute2x128_si256(a[1], a[1], _MM_SHUFFLE(2, 0, 0, 1)), a[1], shift_right);*/

		a[0] = _mm256_alignr_epi8(_mm256_permute2x128_si256(a[0], a[0], _MM_SHUFFLE(2, 0, 0, 1)), a[0], shift_right);
		const auto shuffled = _mm_shuffle_epi8(_mm256_castsi256_si128(a[1]), _mm_setr_epi8(14, 15, 12, 13, 10, 11, 8, 9, 6, 7, 4, 5, 2, 3, 0, 1));
		a[1] = _mm256_alignr_epi8(_mm256_permute2x128_si256(a[1], a[1], _MM_SHUFFLE(2, 0, 0, 1)), a[1], shift_right);
		a[0] = _mm256_inserti128_si256(a[0], _mm_blend_epi16(_mm256_extracti128_si256(a[0], 1), shuffled, 0b1000'0000), 1);
	};
};