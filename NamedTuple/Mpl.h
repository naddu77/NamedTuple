#pragma once

#include "StringLiterals.h"

namespace Internal
{
	template <
		typename TDefault,
		StringLiteral,
		template <StringLiteral, typename> typename
	>
	auto MapLookup(...) -> TDefault;

	template <
		typename,
		StringLiteral TKey,
		template <StringLiteral, typename> typename TArg,
		typename TValue
	>
	auto MapLookup(TArg<TKey, TValue>*) -> TArg<TKey, TValue>;

	template <
		typename TDefault,
		typename,
		template <typename, typename> typename
	>
	auto MapLookup(...) -> TDefault;

	template <
		typename,
		typename TKey,
		template <typename, typename> typename TArg,
		typename TValue
	>
	auto MapLookup(TArg<TKey, TValue>*) -> TArg<TKey, TValue>;
}

template <typename T, StringLiteral TKey, typename TDefault, template <StringLiteral, typename> typename TArg>
using MapLookup = decltype(Internal::MapLookup<TDefault, TKey, TArg>(static_cast<T*>(nullptr)));

template <typename... Ts>
struct Inherit
	: Ts...
{

};
