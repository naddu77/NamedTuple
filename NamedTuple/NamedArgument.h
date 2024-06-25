#pragma once

#include "StringLiterals.h"

template <StringLiteral Name, typename Value>
struct NamedArgument
{
	using ValueType = Value;

	static constexpr auto name{ Name };
	Value value{};

	template <typename T>
	constexpr auto operator=(T&& t)
	{
		return NamedArgument<Name, T>{ .value = std::forward<T>(t) };
	}
};
