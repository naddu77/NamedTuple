#pragma once

#include <ranges>
#include <algorithm>
#include <string>
#include <string_view>

template <std::size_t N>
struct StringLiteral
{
	char buf[N + 1]{};

	constexpr StringLiteral() = default;
	constexpr StringLiteral(char const (&sl)[N + 1])
	{
		std::ranges::copy(sl, std::begin(buf));
	}

	constexpr StringLiteral(std::string_view s)
	{
		std::ranges::copy(s, std::begin(buf));
	}

	constexpr StringLiteral(std::string_view s1, std::string_view s2)
	{
		std::ranges::copy(s1, std::begin(buf));
		std::ranges::copy(s2, std::next(std::begin(buf), std::size(s1)));
	}

	explicit constexpr StringLiteral(std::string const& s)
	{
		std::ranges::copy(s, std::begin(buf));
	}

	auto operator<=>(StringLiteral const&) const = default;

	constexpr explicit operator std::string_view() const
	{
		return { std::data(buf), N };
	}

	constexpr std::string_view ToStringView() const
	{
		return { &buf[0], N };
	}

	constexpr std::size_t size() const noexcept
	{
		return N;
	}

	template <auto sl>
	constexpr bool contains() const noexcept
	{
		return not std::empty(std::ranges::search(std::string_view{ buf }, std::string_view{ sl.buf }));
	}

	constexpr auto operator[](std::size_t i) const
	{
		return buf[i];
	}
};

template <std::size_t N>
StringLiteral(char const (&)[N]) -> StringLiteral<N - 1>;

template <std::size_t N>
StringLiteral(StringLiteral<N>) -> StringLiteral<N>;

template <std::size_t N1, std::size_t N2>
constexpr auto operator+(StringLiteral<N1> const& sl1, StringLiteral<N2> const& sl2)
{
	StringLiteral<N1 + N2> result{ sl1.ToStringView() };

	std::ranges::copy_n(sl2.buf, N2, result.buf + N1);

	return result;
}

template <std::size_t N1, std::size_t N2>
constexpr auto operator+(StringLiteral<N1> const& sl1, char const(&str)[N2])
{
	StringLiteral<N1 + N2 - 1> result{ sl1.ToStringView() };

	std::ranges::copy_n(str, N2 - 1, result.buf + N1);

	return result;
}

namespace Literals
{
	template <StringLiteral sl>
	constexpr auto operator""_sl()
	{
		return sl;
	}

	using namespace std::string_view_literals;

	static_assert(""sv == ""_sl);
	static_assert("name"sv == "name"_sl);
}
