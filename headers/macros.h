#pragma once

#include <type_traits>

//#define CONSTEXPR_ASSERTS
#ifdef CONSTEXPR_ASSERTS
#define CONSTEXPR_VAR constexpr
#else
#define CONSTEXPR_VAR static
#endif

#define WHAT_MM_SHUFFLE(fp0,fp1,fp2,fp3) (((fp3) << 6) | ((fp2) << 4) | ((fp1) << 2) | ((fp0)))

#ifdef _IS_ASSERT_CONSTEXPR_
#define __ASSUME__(__expr) (((__expr) || (assert(__expr), (__expr))), __assume(__expr))
#else
#define __ASSUME__(__expr) __assume(__expr)
#endif
#define ASSERT_NOT_CONSTEXPR(exp) if (std::is_constant_evaluated() == false) {assert((exp));}
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