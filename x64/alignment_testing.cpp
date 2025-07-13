#include "alignment_testing.h"

#include <vector>
#include <Windows.h>
#include <chrono>
#include <handleapi.h>
#include <synchapi.h>
#include <iostream>

static void align_test()
{
	std::cout << "CACHE SIZE: " << cache_test::CACHE_SIZE / 1024 << " KB" << std::endl;
	std::cout << "CACHE LINE SIZE: " << cache_test::CACHE_LINE_SIZE << " bytes" << std::endl;
	std::cout << "CACHE LINES PER SET: " << cache_test::CACHE_WAYS << std::endl;
	std::cout << "SET SIZE: " << cache_test::SET_SIZE << " bytes" << std::endl;
	std::cout << "NUMBER OF SETS: " << cache_test::NUM_OF_SETS << std::endl;
	std::cout << "BUFFER SIZE: " << cache_test::BUFFER_SIZE / 1024ll / 1024ll << " MB" << std::endl;
	std::cout << "STEP SIZE: " << cache_test::STEP << " bytes" << std::endl;
	std::cout << "NUMBER OF REPS: " << cache_test::REPS + cache_test::REPS_REM << std::endl;
	std::cout << "STEP TEST: " << cache_test::useCriticalStep << std::endl;

	/*for (long long i = 1ll, l = 1024ll + 1ll; i < l; ++i)
	{
		auto tmp = mem::find_step_rate_with_fewest_evictions(i, l-1);
		std::cout << "Bytes: " << i  << ", loops: " << tmp.l_info.loops << ", smallest byte: " << tmp.l_info.smallest_bytes_that_can_be_read << ", steprate: " << tmp.step_rate << "\n";
	}*/
	std::vector<HANDLE> Threads_;
	Threads_.reserve(1);
	const auto t1 = std::chrono::high_resolution_clock::now();
	Threads_.push_back((HANDLE)_beginthreadex(nullptr, 0, &cache_test::thread_test, nullptr, 0, nullptr));
	Threads_.push_back((HANDLE)_beginthreadex(nullptr, 0, &cache_test::thread_test, nullptr, 0, nullptr));
	Threads_.push_back((HANDLE)_beginthreadex(nullptr, 0, &cache_test::thread_test, nullptr, 0, nullptr));
	Threads_.push_back((HANDLE)_beginthreadex(nullptr, 0, &cache_test::thread_test, nullptr, 0, nullptr));
	Threads_.push_back((HANDLE)_beginthreadex(nullptr, 0, &cache_test::thread_test, nullptr, 0, nullptr));
	Threads_.push_back((HANDLE)_beginthreadex(nullptr, 0, &cache_test::thread_test, nullptr, 0, nullptr));
	Threads_.push_back((HANDLE)_beginthreadex(nullptr, 0, &cache_test::thread_test, nullptr, 0, nullptr));
	Threads_.push_back((HANDLE)_beginthreadex(nullptr, 0, &cache_test::thread_test, nullptr, 0, nullptr));

	for (int i = 0; i < 1; ++i)
	{
		WaitForSingleObject(Threads_[i], INFINITE);
		CloseHandle(Threads_[i]);
	}
	const auto t2 = std::chrono::high_resolution_clock::now();
	auto ms_int = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1);
	std::cout << "EXECUTION TIME: " << ms_int.count() << "micro" << std::endl;

	return;
};