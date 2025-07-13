#pragma once

#include <CRTDBG.h>
#include <type_traits>

#ifdef _DEBUG
static constinit const bool _BOUNDS_CHECKING_ = true;
static constinit const bool __DEBUG_BUILD = true;
#else
static constinit const bool __DEBUG_BUILD = false;
static constinit const bool _BOUNDS_CHECKING_ = false;
#endif

//#define CPLUSPLUS_EXCEPTIONS_THROWING
//#define CONSTEXPR_ASSERTS
#ifdef CONSTEXPR_ASSERTS
#define CONSTEXPR_VAR constexpr
#else
#define CONSTEXPR_VAR static
#endif

#ifdef CPLUSPLUS_EXCEPTIONS_THROWING
static constinit const bool ENABLE_CPP_EXCEPTION_THROW = true;
#else
static constinit const bool ENABLE_CPP_EXCEPTION_THROW = false;
#endif

#define WHAT_MM_SHUFFLE(fp0,fp1,fp2,fp3) (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | ((fp0)))

#ifdef _IS_ASSERT_CONSTEXPR_
#define __ASSUME__(__expr) (((__expr) || (assert(__expr), (__expr))), __assume(__expr))
#else
#define __ASSUME__(__expr) __assume(__expr)
#endif
//#define ASSERT_NOT_CONSTEXPR(exp) if (std::is_constant_evaluated() == false) {assert((exp));}

//for debug builds to assert
template<bool enable_assert>
static constexpr void ASSERT_NOT_CONSTEXPR(bool exp)
	requires(enable_assert)
{
	if (std::is_constant_evaluated() == false)
	{
		_ASSERT((exp));
	}
};

//for release builds to prevent build errors
template<bool enable_assert>
static constexpr void ASSERT_NOT_CONSTEXPR(bool)
	requires(!enable_assert)
{}

#define RELEASE_ASSERT_NOT_CONSTEXPR(__expr) if (std::is_constant_evaluated() == false) {if (__expr) __assume(1); else __assume(0);}
/*template<class T> struct dependent_false : std::false_type {};
template<typename T>
static constexpr bool test_static_assert(T expr)
{
	if (expr)
	{
		return true;
	}
	else
	{
		static_assert(dependent_false<T>::value, ASSERT_CONSTEXPR_MESSAGE);
	}
	return false;
};
#define ASSERT_CONSTEXPR_LAMBDA_MESSAGE(message) ([=]() -> auto {return message;})
#define ASSERT_CONSTEXPR(expr_evaluation, message) []<typename T>(T _expr) -> void { if (_expr) return; else static_assert(dependent_false<T>::value, message);}(expr_evaluation);
#define macro_test(expr_evaluation, message) constexpr auto ASSERT_CONSTEXPR_MESSAGE = message; static_assert(test_static_assert(expr_evaluation), message);*/