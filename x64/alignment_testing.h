#pragma once

#include <iostream>
#include <ctime>
#include <new>
#include <emmintrin.h>
#include <mutex>
#include <process.h>
#include <intrin.h>

#include "math_utility.h"
#include <vector>

namespace cache_test
{
	constexpr long long pow(long long base, long long exp)
	{
		return ((exp == 0) ? 1 : base * pow(base, exp - 1));
	};

	constexpr long long CACHE_SIZE_IN_KB = 32ll;
	constexpr long long CACHE_SIZE = 1024ll * CACHE_SIZE_IN_KB;
	constexpr long long CACHE_LINE_SIZE = 64ll;
	constexpr long long CACHE_WAYS = 8ll;
	constexpr long long SET_SIZE = (CACHE_LINE_SIZE * CACHE_WAYS);
	constexpr long long NUM_OF_SETS = (CACHE_SIZE / CACHE_WAYS) / CACHE_LINE_SIZE;
	constexpr long long ONE_WAY_SIZE = (NUM_OF_SETS * CACHE_LINE_SIZE);

	static_assert(SET_SIZE* NUM_OF_SETS == CACHE_SIZE);

	//#define _return_cache_info_
	constexpr int useCriticalStep = 0;
	constexpr bool onlyWriteToCache = true;

	constexpr long long OBJECT_SIZE = 128ll;
	constexpr long long BUFFER_SIZE = OBJECT_SIZE * 1024ll * 1140ll;
	constexpr long long BUFFER_SIZE_MB = BUFFER_SIZE / 1024ll / 1024ll;
	static_assert(BUFFER_SIZE % 192ll == 0ll);
	constexpr long long CRITICAL_STRIDE = (NUM_OF_SETS * CACHE_LINE_SIZE);
	constexpr long long INSTRUCTIONS_IN_LOOP = 4ll;

	constexpr long long object_bytes = 144ll;
	constexpr long long number_of_sets = 64ll;
	constexpr long long cache_line_size = 64ll;
	constexpr long long decent_loop_step_rate = (object_bytes <= cache_line_size ? 1ll : (!((object_bytes % cache_line_size) & 1) ? (object_bytes % number_of_sets != 0ll ? cache_line_size / (object_bytes % number_of_sets) : 1ll) : cache_line_size));

	constexpr auto OBJECTS_PER_WAY = expr::divide_with_remainder(ONE_WAY_SIZE, OBJECT_SIZE);

	constexpr long long STEP = useCriticalStep == 0 ? OBJECT_SIZE : useCriticalStep == 1 ? (256 * 5) - 1 : OBJECT_SIZE * 7;// (256 * 5) - 1;  //4 * CACHE_LINE_SIZE + (useCriticalStep ? 0 : sizeof(char) * 32 * 9);
	constexpr double STEPPED_OVER_CACHE_LINES = (double)STEP / (double)CACHE_LINE_SIZE - 1.0;
	constexpr long long OBJECTS_PER_STRIDE = (CRITICAL_STRIDE / STEP);
	constexpr double BYTES_PER_STRIDE = OBJECTS_PER_STRIDE * (double)STEP;

	consteval auto GET_REPS_LOOPS()
	{
		if constexpr (useCriticalStep == 0)
		{
			constexpr auto REPS_LOOPS = expr::divide_with_remainder(BUFFER_SIZE, STEP);
			return decltype(REPS_LOOPS){ REPS_LOOPS.div, REPS_LOOPS.rem / OBJECT_SIZE };

		}
		else if constexpr (useCriticalStep == 1)
		{
			constexpr auto REPS_LOOPS = expr::divide_with_remainder(BUFFER_SIZE, STEP);
			constexpr auto REPS_MUL_DIV = expr::multiply_division_with_remainder(REPS_LOOPS.div, STEP / OBJECT_SIZE, INSTRUCTIONS_IN_LOOP); //798700
			constexpr decltype(REPS_MUL_DIV) REPS_COPY{ REPS_MUL_DIV.div, (REPS_LOOPS.rem / OBJECT_SIZE) + REPS_MUL_DIV.rem };
			return REPS_COPY;
		}
		else
		{
			constexpr auto REPS_LOOPS = expr::divide_with_remainder(BUFFER_SIZE, STEP);
			constexpr auto REPS_MUL_DIV = expr::multiply_division_with_remainder(REPS_LOOPS.div, STEP / OBJECT_SIZE, INSTRUCTIONS_IN_LOOP); //798700
			constexpr decltype(REPS_MUL_DIV) REPS_COPY{ REPS_MUL_DIV.div, (REPS_LOOPS.rem / OBJECT_SIZE) + REPS_MUL_DIV.rem };
			return REPS_COPY;
		}
	};

	constexpr auto REPS_LOOPS = GET_REPS_LOOPS();
	constexpr long long REPS = REPS_LOOPS.div;
	constexpr long long REPS_REM = REPS_LOOPS.rem;

	constexpr long long byte_ptr_index = 85ll;
	constexpr long long byte_ptr = byte_ptr_index * STEP;
	constexpr double modulod_ptr = (byte_ptr / CACHE_LINE_SIZE) % NUM_OF_SETS;

	struct cache_info
	{
		long long sc[NUM_OF_SETS]{};
	};

	static cache_info cache_test(unsigned char* buffer)
	{
		cache_info set_counter{};
		for (int i = 0; i < NUM_OF_SETS; ++i) set_counter.sc[i] = 0;

		const clock_t t1 = clock();

		if constexpr (useCriticalStep < 2)
		{
			std::size_t index = 0;
			for (std::size_t i = 0; i < REPS; i += 4)
			{
				if (index + (STEP * 3) >= BUFFER_SIZE)
					index = 0;
#ifdef _return_cache_info_
				const auto ptr_set1 = find_cache_set(buffer + index);
				++set_counter[ptr_set1];
				const auto ptr_set2 = find_cache_set(buffer + (index + STEP));
				++set_counter[ptr_set2];
				const auto ptr_set3 = find_cache_set(buffer + (index + (STEP * 2)));
				++set_counter[ptr_set3];
				const auto ptr_set4 = find_cache_set(buffer + (index + (STEP * 3)));
				++set_counter[ptr_set4];
#endif
				if (onlyWriteToCache)
				{
					buffer[index] = (unsigned char)(index % 255);
					buffer[index + STEP] = (unsigned char)((index + STEP) % 255);
					buffer[index + (STEP * 2)] = (unsigned char)((index + (STEP * 2)) % 255);
					buffer[index + (STEP * 3)] = (unsigned char)((index + (STEP * 3)) % 255);
				}
				else
				{
					buffer[index] = (unsigned char)(buffer[index] % 255);
					buffer[index + STEP] = (unsigned char)(buffer[index + STEP] % 255);
					buffer[index + (STEP * 2)] = (unsigned char)(buffer[index + (STEP * 2)] % 255);
					buffer[index + (STEP * 3)] = (unsigned char)(buffer[index + (STEP * 3)] % 255);
				}
				index += (STEP * 4);
			}
			index = REPS * OBJECT_SIZE;
			if constexpr (REPS_REM > 0)
			{
				for (std::size_t i = 0; i < REPS_REM; ++i)
				{
#ifdef _return_cache_info_
					const auto ptr_set1 = find_cache_set(buffer + index);
					++set_counter.sc[ptr_set1];
#endif
					if (onlyWriteToCache)
					{
						buffer[index] = (unsigned char)(index % 255);
					}
					else
					{
						buffer[index] = (unsigned char)(buffer[index] % 255);
					}
					index += OBJECT_SIZE;
				}
			}
		}
		else
		{
			std::size_t index = 0;
			std::size_t index_loops = OBJECT_SIZE;
			for (std::size_t i = 0; i < REPS; i += 4)
			{
				if (index + (STEP * 3) >= BUFFER_SIZE)
				{
					index = index_loops;
					index_loops += OBJECT_SIZE;
				}
#ifdef _return_cache_info_
				const auto ptr_set1 = find_cache_set(buffer + index1);
				++set_counter.sc[ptr_set1];
				const auto ptr_set2 = find_cache_set(buffer + index1 + index);
				++set_counter.sc[ptr_set2];
				const auto ptr_set3 = find_cache_set(buffer + index1 + index * 2);
				++set_counter.sc[ptr_set3];
				const auto ptr_set4 = find_cache_set(buffer + index1 + index * 3);
				++set_counter.sc[ptr_set4];
#endif
				if (onlyWriteToCache)
				{
					buffer[index] = (unsigned char)(index % 255);
					buffer[index + (STEP * 1)] = (unsigned char)((index + (STEP * 1)) % 255);
					buffer[index + (STEP * 2)] = (unsigned char)((index + (STEP * 2)) % 255);
					buffer[index + (STEP * 3)] = (unsigned char)((index + (STEP * 3)) % 255);
				}
				else
				{
					buffer[index] = (unsigned char)(buffer[index] % 255);
					buffer[index + STEP] = (unsigned char)(buffer[index + STEP] % 255);
					buffer[index + (STEP * 2)] = (unsigned char)(buffer[index + (STEP * 2)] % 255);
					buffer[index + (STEP * 3)] = (unsigned char)(buffer[index + (STEP * 3)] % 255);
				}
				index += (STEP * 4);
			}
			index = REPS * OBJECT_SIZE;
			if constexpr (REPS_REM > 0)
			{
				for (std::size_t i = 0; i < REPS_REM; ++i)
				{
#ifdef _return_cache_info_
					const auto ptr_set1 = find_cache_set(buffer + index);
					++set_counter.sc[ptr_set1];
#endif
					if (onlyWriteToCache)
					{
						buffer[index] = (unsigned char)(index % 255);
					}
					else
					{
						buffer[index] = (unsigned char)(buffer[index] % 255);
					}
					index += OBJECT_SIZE;
				}
			}
		}

		const clock_t t2 = clock();

		//======================================================================
		// Print the execution time (in clock ticks) and cleanup resources
		//======================================================================

		const double executionTime = ((double)(t2) - t1) / CLOCKS_PER_SEC;
		std::cout << "EXECUTION TIME: " << executionTime << "s" << std::endl;

		return set_counter;
	};
	static inline std::mutex cout_lock;
	static cache_info cache_size_test(long long* buffer) noexcept
	{
		cache_info set_counter{};
		for (int i = 0; i < NUM_OF_SETS; ++i) set_counter.sc[i] = 0;

		//auto t1 = std::chrono::high_resolution_clock::now();

		if constexpr (useCriticalStep < 2)
		{
			const int xl = 32;
			for (int x = 0; x < xl; ++x)
			{
				std::size_t index = 0;
				for (std::size_t i = 0; i < REPS; i += 4)
				{
					if (index + (STEP * 4) >= BUFFER_SIZE)
						index = 0;
#ifdef _return_cache_info_
					const auto ptr_set1 = find_cache_set(buffer + index);
					++set_counter[ptr_set1];
#endif
					if (onlyWriteToCache)
					{
						//_mm_prefetch((const char*)&(((unsigned char*)buffer)[index + (STEP * 10)]), _MM_HINT_NTA);
						_mm_stream_si64((long long*)&(((unsigned char*)buffer)[index]), (index % 255));
						//_mm_prefetch((const char*)&(((unsigned char*)buffer)[index + (STEP * 11)]), _MM_HINT_NTA);
						_mm_stream_si64((long long*)&(((unsigned char*)buffer)[index + STEP]), (index % 255));
						//_mm_prefetch((const char*)&(((unsigned char*)buffer)[index + (STEP * 12)]), _MM_HINT_NTA);
						_mm_stream_si64((long long*)&(((unsigned char*)buffer)[index + (STEP * 2)]), (index % 255));
						//_mm_prefetch((const char*)&(((unsigned char*)buffer)[index + (STEP * 13)]), _MM_HINT_NTA);
						_mm_stream_si64((long long*)&(((unsigned char*)buffer)[index + (STEP * 3)]), (index % 255));
						//_mm_prefetch((const char*)&(((unsigned char*)buffer)[index + (STEP * 14)]), _MM_HINT_NTA);
						//_mm_stream_si64((long long*)&(((unsigned char*)buffer)[index + (STEP * 4)]), (index % 255));
						//_mm_prefetch((const char*)&(((unsigned char*)buffer)[index + (STEP * 15)]), _MM_HINT_NTA);
						//_mm_stream_si64((long long*)&(((unsigned char*)buffer)[index + (STEP * 5)]), (index % 255));
						//_mm_prefetch((const char*)&(((unsigned char*)buffer)[index + (STEP * 16)]), _MM_HINT_NTA);
						//_mm_stream_si64((long long*)&(((unsigned char*)buffer)[index + (STEP * 6)]), (index % 255));
						//_mm_prefetch((const char*)&(((unsigned char*)buffer)[index + (STEP * 17)]), _MM_HINT_NTA);
						//_mm_stream_si64((long long*)&(((unsigned char*)buffer)[index + (STEP * 7)]), (index % 255));
						//buffer[index] = (index % 255);
						//*((long long*)&(((unsigned char*)buffer)[index])) = (index % 255);
						//*((long long*)&(((unsigned char*)buffer)[index + (STEP * 1)])) = (index % 255);
						//*((long long*)&(((unsigned char*)buffer)[index + (STEP * 2)])) = (index % 255);
						//*((long long*)&(((unsigned char*)buffer)[index + (STEP * 3)])) = (index % 255);
					}
					else
					{
						buffer[index] = (buffer[index] % 255);
					}
					index += STEP * 4;
				}
				index = REPS * OBJECT_SIZE;
				if constexpr (REPS_REM > 0)
				{
					for (std::size_t i = 0; i < REPS_REM; ++i)
					{
#ifdef _return_cache_info_
						const auto ptr_set1 = find_cache_set(buffer + index);
						++set_counter.sc[ptr_set1];
#endif
						if (onlyWriteToCache)
						{
							buffer[index] = (index % 255);
						}
						else
						{
							buffer[index] = (buffer[index] % 255);
						}
						index += OBJECT_SIZE;
					}
				}
			}
		}

		//auto t2 = std::chrono::high_resolution_clock::now();

		//======================================================================
		// Print the execution time (in clock ticks) and cleanup resources
		//======================================================================

		//auto ms_int = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1);
		//auto executionTime = ms_int.count();
		//const std::lock_guard<std::mutex> lock(cout_lock);
		//std::cout << "EXECUTION TIME: " << executionTime << "s" << std::endl;

		return set_counter;
	};

	static unsigned int thread_test(void* ignored)
	{
		cache_test::cache_info infos[15]{};
		long long* buffer = (long long*)::operator new[](cache_test::BUFFER_SIZE + (cache_test::SET_SIZE * 16), std::align_val_t{ 64 }, std::nothrow_t{});
		for (int i = 0, l = 15; i < l; ++i)
		{
			infos[i] = cache_test::cache_size_test(buffer);
		}
		operator delete[](buffer, std::align_val_t{ 64 });

		for (int i2 = 0, l2 = 10; i2 < l2; ++i2)
		{
			for (int i = 0, l = cache_test::NUM_OF_SETS; i < l; ++i)
			{
				if (infos[i2].sc[i] > 0) std::cout << "SET" << i << ": " << infos[i2].sc[i] << "\n";
			}
		}

		_endthreadex(0);
		return 0;
	};
};