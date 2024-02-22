#pragma once

#include <limits>
#include <type_traits>
#include <source_location>

namespace tc
{
	[[nodiscard]] constexpr double floor(double a) noexcept
	{
		/*if (a >= 9223372036854775807.0)
			return 9223372036854775807.0;
		else if (a <= -9223372036854775808.0)
			return -9223372036854775808.0;*/

		if (a >= 9223372036854775807.0)
			return static_cast<double>(9223372036854775807ll);
		else if (a <= -9223372036854775807.0)
			return static_cast<double>(-9223372036854775807ll);

		if (a < 0)
			return static_cast<double>(static_cast<long long>(a) - 1);
		else
			return static_cast<double>(static_cast<long long>(a));

		//return static_cast<double>(static_cast<long long>(a));
	};

	template<typename left, typename right>
	concept is_type_narrowing = (std::numeric_limits<left>::lowest)() >= (std::numeric_limits<right>::lowest)() &&
		(std::numeric_limits<left>::max)() <= (std::numeric_limits<right>::max)();

	template<typename left, typename right>
	concept is_type_widening = (std::numeric_limits<left>::lowest)() <= (std::numeric_limits<right>::lowest)() &&
		(std::numeric_limits<left>::max)() >= (std::numeric_limits<right>::max)();

	template<typename left, typename right>
	concept is_type_conversion = (std::is_integral_v<left> && std::is_floating_point_v<right>) ||
		(std::is_integral_v<right> && std::is_floating_point_v<left>) ||
		(std::is_integral_v<left> && std::is_integral_v<right> && (std::is_unsigned_v<left> || std::is_unsigned_v<right>));

	template<typename return_type, typename input_type>
	constexpr return_type narrow(input_type value) noexcept
		requires(tc::is_type_narrowing<return_type, input_type> && !tc::is_type_widening<return_type, input_type>)
	{
		if constexpr (std::is_integral_v<return_type> && std::is_floating_point_v<input_type>)
		{
			constexpr auto min_val_r = (std::numeric_limits<return_type>::lowest)();
			constexpr auto max_val_r = (std::numeric_limits<return_type>::max)();
			constexpr auto min_val = static_cast<input_type>(min_val_r);
			constexpr auto max_val = static_cast<input_type>(max_val_r);
			return static_cast<return_type>(std::max<input_type>(min_val, std::min<input_type>(max_val, static_cast<input_type>(tc::floor(value)))));
		}
		else if constexpr (std::is_integral_v<input_type> && std::is_floating_point_v<return_type>)
		{
			constexpr auto min_val = (std::numeric_limits<return_type>::lowest)();
			constexpr auto max_val = (std::numeric_limits<return_type>::max)();
			return static_cast<return_type>(std::max<return_type>(min_val, std::min<return_type>(max_val, static_cast<return_type>(value))));
		}
		else
		{
			constexpr auto min_val_r = std::is_integral_v<input_type> && std::is_unsigned_v<input_type> ? (std::numeric_limits<input_type>::lowest)() : (std::numeric_limits<return_type>::lowest)();
			constexpr auto max_val_r = (std::numeric_limits<return_type>::max)();
			constexpr auto min_val = std::is_integral_v<input_type> && std::is_unsigned_v<input_type> ? min_val_r : static_cast<input_type>(min_val_r);
			constexpr auto max_val = static_cast<input_type>(max_val_r);
			return static_cast<return_type>(std::max<input_type>(min_val, std::min<input_type>(max_val, value)));
		}
	};

	template<typename return_type, typename input_type>
	constexpr return_type widen(input_type value) noexcept
		requires(!tc::is_type_narrowing<return_type, input_type>&& tc::is_type_widening<return_type, input_type>)
	{
		if constexpr (std::is_integral_v<return_type> && std::is_floating_point_v<input_type>)
		{
			constexpr auto min_val = (std::numeric_limits<return_type>::lowest)();
			constexpr auto max_val = (std::numeric_limits<return_type>::max)();
			return static_cast<return_type>(std::max<return_type>(min_val, std::min<return_type>(max_val, static_cast<return_type>(tc::floor(value)))));
		}
		else if constexpr (std::is_integral_v<input_type> && std::is_floating_point_v<return_type>)
		{
			constexpr auto min_val = (std::numeric_limits<return_type>::lowest)();
			constexpr auto max_val = (std::numeric_limits<return_type>::max)();
			return std::max<return_type>(min_val, std::min<return_type>(max_val, static_cast<return_type>(value)));
		}
		else
		{
			constexpr auto min_val_r = std::is_integral_v<return_type> && std::is_unsigned_v<return_type> ? (std::numeric_limits<return_type>::lowest)() : (std::numeric_limits<input_type>::lowest)();
			constexpr auto max_val_r = (std::numeric_limits<input_type>::max)();
			constexpr auto min_val = std::is_integral_v<return_type> && std::is_unsigned_v<return_type> ? min_val_r : static_cast<return_type>(min_val_r);
			constexpr auto max_val = static_cast<return_type>(max_val_r);
			return std::max<return_type>(min_val, std::min<return_type>(max_val, static_cast<return_type>(value)));
		}
	};

	template<typename return_type, typename input_type>
	constexpr return_type sign(input_type value, const std::source_location location = std::source_location::current())
		requires(std::is_signed_v<return_type> == true && std::is_unsigned_v<input_type> == true)
	{
		constexpr auto max_return = (std::numeric_limits<return_type>::max)();
		constexpr auto lowest_return = std::numeric_limits<return_type>::lowest();
		constexpr auto max_input = (std::numeric_limits<input_type>::max)();
		constexpr auto lowest_input = std::numeric_limits<input_type>::lowest();
		constexpr auto lowest_signed_input = std::numeric_limits<std::make_signed_t<input_type>>::lowest();

		if constexpr (max_return >= max_input && lowest_return <= lowest_input)
			return static_cast<return_type>(value);

		if (value == static_cast<input_type>(0)) return static_cast<return_type>(0);
		if constexpr (static_cast<std::size_t>(max_return) < static_cast<std::size_t>(max_input)) //max_input is greater then max_return
		{
			if (value > static_cast<input_type>(0) && static_cast<std::size_t>(max_return) <= static_cast<std::size_t>(value)) return max_return;
		}
		if (static_cast<std::size_t>(max_return) >= static_cast<std::size_t>(value))
		{
			return static_cast<return_type>(value);
		}
		else
		{
			if (value > static_cast<input_type>(0) && static_cast<std::size_t>(max_return) < static_cast<std::size_t>(value))
			{
				//if (std::is_constant_evaluated() == false) throw std::runtime_error("return type's max range is less then input");
				return max_return;
			}
			if (lowest_signed_input >= static_cast<std::make_signed_t<input_type>>(value)) //should never occur since unsigned values can't be negative
			{
				//if (std::is_constant_evaluated() == false) throw std::runtime_error("return type's lowest range is greater then input");
				return lowest_return;
			}
		}

		//if (std::is_constant_evaluated() == false) throw std::runtime_error(std::string(location.file_name()) + "(" + std::to_string(location.line()) + "," + std::to_string(location.column()) + "): " + location.function_name() + " warning: missing return value");
		return 0;
	};
	template<typename input_type>
	constexpr std::make_signed_t<input_type> sign(input_type value, const std::source_location location = std::source_location::current())
		requires(std::is_unsigned_v<input_type> == true)
	{
		return sign<std::make_signed_t<input_type>>(value, location);
	};

	template<typename return_type, typename input_type>
	constexpr return_type unsign(input_type value, const std::source_location location = std::source_location::current())
		requires(std::is_unsigned_v<return_type> == true && std::is_signed_v<input_type> == true)
	{
		constexpr auto max_return = (std::numeric_limits<return_type>::max)();
		constexpr auto lowest_return = std::numeric_limits<return_type>::lowest();
		constexpr auto max_input = (std::numeric_limits<input_type>::max)();
		constexpr auto lowest_input = std::numeric_limits<input_type>::lowest();

		if (value == static_cast<input_type>(0)) return static_cast<return_type>(0);
		if constexpr (static_cast<std::size_t>(max_return) < static_cast<std::size_t>(max_input)) //max_input is greater then max_return
		{
			if (value > static_cast<input_type>(0) && static_cast<std::size_t>(max_return) <= static_cast<std::size_t>(value)) return max_return;
		}
		if (value > static_cast<input_type>(0) && static_cast<std::size_t>(max_return) >= static_cast<std::size_t>(value))
		{
			return static_cast<return_type>(value);
		}
		else
		{
			if (value > static_cast<input_type>(0) && static_cast<std::size_t>(max_return) < static_cast<std::size_t>(value))
			{
				//if (std::is_constant_evaluated() == false) throw std::runtime_error("return type's max range is less then input");
				return max_return;
			}
			if (value < static_cast<input_type>(0) || static_cast<long long>(lowest_return) >= value)
			{
				//if (std::is_constant_evaluated() == false) throw std::runtime_error("return type's lowest range is greater then input");
				return lowest_return;
			}
		}

		//if (std::is_constant_evaluated() == false) throw std::runtime_error(std::string(location.file_name()) + "(" + std::to_string(location.line()) + "," + std::to_string(location.column()) + "): " + location.function_name() + " warning: missing return value");
		return (std::numeric_limits<return_type>::max)();
	};
	template<typename input_type>
	constexpr std::make_unsigned_t<input_type> unsign(input_type value, const std::source_location location = std::source_location::current())
		requires(std::is_signed_v<input_type> == true)
	{
		return unsign<std::make_unsigned_t<input_type>>(value, location);
	};

	template<typename return_type, typename input_type>
	constexpr return_type conversion(input_type value) noexcept
		requires(tc::is_type_conversion<return_type, input_type>)
	{
		if constexpr (std::is_integral_v<return_type> && std::is_floating_point_v<input_type>)
		{
			constexpr auto min_val_r = (std::numeric_limits<return_type>::lowest)();
			constexpr auto max_val_r = (std::numeric_limits<return_type>::max)();
			constexpr auto min_val = static_cast<input_type>(min_val_r);
			constexpr auto max_val = static_cast<input_type>(max_val_r);
			const auto float_result = std::max<input_type>(min_val, std::min<input_type>(max_val, static_cast<input_type>(tc::floor(value))));

			if (float_result == min_val)
				return (std::numeric_limits<return_type>::lowest)();
			else if (float_result == max_val)
				return (std::numeric_limits<return_type>::max)();
			else
				return static_cast<return_type>(value);
		}
		else if constexpr (std::is_integral_v<input_type> && std::is_floating_point_v<return_type>)
		{
			constexpr auto min_val = (std::numeric_limits<return_type>::lowest)();
			constexpr auto max_val = (std::numeric_limits<return_type>::max)();
			return std::max<return_type>(min_val, std::min<return_type>(max_val, static_cast<return_type>(value)));
		}
		else if constexpr (std::is_integral_v<return_type> && std::is_integral_v<input_type> && std::is_unsigned_v<return_type>)
		{
			constexpr auto min_val = (std::numeric_limits<return_type>::lowest)();
			constexpr auto max_val = (std::numeric_limits<input_type>::max)();
			return std::max<return_type>(min_val, std::min<return_type>(max_val, static_cast<return_type>(value)));
		}
		else if constexpr (std::is_integral_v<return_type> && std::is_integral_v<input_type> && std::is_unsigned_v<input_type>)
		{
			constexpr auto min_val = (std::numeric_limits<input_type>::lowest)();
			constexpr auto max_val = (std::numeric_limits<return_type>::max)();

			input_type temp_val{ (std::numeric_limits<return_type>::max)() };
			if (value >= temp_val)
				return (std::numeric_limits<return_type>::max)();

			//constexpr auto min_val = static_cast<return_type>(min_val_r);
			return std::max<return_type>(min_val, std::min<return_type>(max_val, value));
		}
		else
		{
			constexpr auto min_val_r = (std::numeric_limits<return_type>::lowest)();
			constexpr auto max_val_r = (std::numeric_limits<return_type>::max)();
			constexpr auto min_val = static_cast<input_type>(min_val_r);
			constexpr auto max_val = static_cast<input_type>(max_val_r);
			return static_cast<return_type>(std::max<input_type>(min_val, std::min<input_type>(max_val, value)));
		}
	};
};

static_assert(tc::narrow<int>(std::numeric_limits<double>::lowest()) == std::numeric_limits<int>::lowest());
static_assert(tc::narrow<int>((std::numeric_limits<double>::max)()) == (std::numeric_limits<int>::max)());
static_assert(tc::narrow<int>(std::numeric_limits<std::size_t>::lowest()) == 0);
static_assert(tc::narrow<int>((std::numeric_limits<std::size_t>::max)()) == (std::numeric_limits<int>::max)());
static_assert(tc::narrow<int>(std::numeric_limits<long long>::lowest()) == std::numeric_limits<int>::lowest());
static_assert(tc::narrow<int>((std::numeric_limits<long long>::max)()) == (std::numeric_limits<int>::max)());

static_assert(tc::narrow<char>(std::numeric_limits<std::size_t>::lowest()) == 0);
static_assert(tc::narrow<char>((std::numeric_limits<std::size_t>::max)()) == 127);
static_assert(tc::narrow<char>(std::numeric_limits<long long>::lowest()) == -128);
static_assert(tc::narrow<char>((std::numeric_limits<long long>::max)()) == 127);
static_assert(tc::narrow<char>(std::numeric_limits<double>::lowest()) == -128);
static_assert(tc::narrow<char>((std::numeric_limits<double>::max)()) == 127);

static_assert(tc::narrow<unsigned char>(std::numeric_limits<long long>::lowest()) == 0);
static_assert(tc::narrow<unsigned char>((std::numeric_limits<long long>::max)()) == 255);
static_assert(tc::narrow<unsigned char>(std::numeric_limits<std::size_t>::lowest()) == 0);
static_assert(tc::narrow<unsigned char>((std::numeric_limits<std::size_t>::max)()) == 255);
static_assert(tc::narrow<unsigned char>(std::numeric_limits<float>::lowest()) == 0);
static_assert(tc::narrow<unsigned char>((std::numeric_limits<float>::max)()) == 255);

static_assert(tc::narrow<float>(std::numeric_limits<double>::lowest()) == std::numeric_limits<float>::lowest());
static_assert(tc::narrow<float>((std::numeric_limits<double>::max)()) == (std::numeric_limits<float>::max)());

static_assert(tc::conversion<float>(std::numeric_limits<std::size_t>::lowest()) == std::numeric_limits<std::size_t>::lowest());
static_assert(tc::conversion<float>((std::numeric_limits<std::size_t>::max)()) == (std::numeric_limits<std::size_t>::max)());
static_assert(tc::conversion<float>(std::numeric_limits<long long>::lowest()) == std::numeric_limits<long long>::lowest());
static_assert(tc::conversion<float>((std::numeric_limits<long long>::max)()) == (std::numeric_limits<long long>::max)());

static_assert(tc::conversion<long long>(std::numeric_limits<float>::lowest()) == std::numeric_limits<long long>::lowest());
static_assert(tc::conversion<long long>((std::numeric_limits<float>::max)()) == (std::numeric_limits<long long>::max)());
static_assert(tc::conversion<long long>(std::numeric_limits<double>::lowest()) == std::numeric_limits<long long>::lowest());
static_assert(tc::conversion<long long>((std::numeric_limits<double>::max)()) == (std::numeric_limits<long long>::max)());
static_assert(tc::conversion<long long>(std::numeric_limits<std::size_t>::lowest()) == static_cast<long long>(std::numeric_limits<std::size_t>::lowest()));
static_assert(tc::conversion<long long>((std::numeric_limits<std::size_t>::max)()) == (std::numeric_limits<long long>::max)());

static_assert(tc::widen<float>(std::numeric_limits<int>::lowest()) == std::numeric_limits<int>::lowest());
static_assert(tc::widen<float>((std::numeric_limits<int>::max)()) == (std::numeric_limits<int>::max)());
static_assert(tc::widen<float>(std::numeric_limits<long long>::lowest()) == std::numeric_limits<long long>::lowest());
static_assert(tc::widen<float>((std::numeric_limits<long long>::max)()) == (std::numeric_limits<long long>::max)());
static_assert(tc::widen<float>(std::numeric_limits<unsigned long long>::lowest()) == std::numeric_limits<unsigned long long>::lowest());
static_assert(tc::widen<float>((std::numeric_limits<unsigned long long>::max)()) == (std::numeric_limits<unsigned long long>::max)());

static_assert(tc::widen<unsigned long long>(512) == 512);
static_assert(tc::widen<long long>(512u) == 512);

static_assert(tc::unsign<unsigned int>(55555555ll) == 55555555u, "no");
static_assert(tc::unsign<unsigned int>(555555555555ll) == (std::numeric_limits<unsigned int>::max)(), "no");
static_assert(tc::unsign<std::size_t>(-5555555555555ll) == 0ull, "no");

static_assert(tc::sign<int>(55555555ull) == 55555555, "no");
static_assert(tc::sign<int>(555555555555ull) == (std::numeric_limits<int>::max)(), "no");