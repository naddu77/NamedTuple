#pragma once

#include "NamedArgument.h"
#include "Mpl.h"

#include <any>
#include <string>
#include <iostream>
#include <functional>
#include <sstream>

static_assert(std::is_same_v<void, MapLookup<Inherit<NamedArgument<"price", double>, NamedArgument<"size", int>>, "unknown", void, NamedArgument>>);
static_assert(std::is_same_v<NamedArgument<"price", double>, MapLookup<Inherit<NamedArgument<"price", double>, NamedArgument<"size", int>>, "price", void, NamedArgument>>);
static_assert(std::is_same_v<NamedArgument<"size", int>, MapLookup<Inherit<NamedArgument<"price", double>, NamedArgument<"size", int>>, "size", void, NamedArgument>>);

struct Any
	: std::any
{
	constexpr Any() noexcept = default;

	template <typename T>
	constexpr explicit(false) Any(T const& a)
		: std::any{ a }
		, print{ [](std::ostream& os, std::any const& a) -> std::ostream& {
			if constexpr (requires { os << std::any_cast<T>(a); })
			{
				os << std::any_cast<T>(a);
			}
			else if constexpr (requires {
				std::begin(std::any_cast<T>(a));
				std::end(std::any_cast<T>(a));
			})
			{
				auto obj{ std::any_cast<T>(a) };

				for (auto first{ true };
					auto const& e : obj)
				{
					if (not std::exchange(first, false))
					{
						os << ',';
					}

					os << e;
				}
			}
			else if constexpr (requires { os << std::any_cast<std::decay_t<T const&>>(a); })
			{
				os << std::any_cast<std::decay_t<T const&>>(a);
			}
			else
			{
				os << a.type().name();
			}

			return os;
		} }
	{

	}

	template <typename T>
	constexpr explicit(false) operator T() const
	{
		return std::any_cast<T>(*this);
	}

	friend std::ostream& operator<<(std::ostream& os, Any const& a)
	{
		return a.print(os, a);
	}

private:
	std::ostream&(*print)(std::ostream&, std::any const&){};
};

namespace std
{
	template <>
	struct formatter<Any>
	{
		template <typename ParseContext>
		constexpr typename ParseContext::iterator parse(ParseContext& ctx) const
		{
			return std::begin(ctx);
		}

		template <typename FormatContext>
		constexpr typename FormatContext::iterator format(Any const& a, FormatContext& ctx) const
		{
			auto out{ ctx.out() };

			std::stringstream ss;

			ss << a;

			out = std::format_to(out, "{}", ss.str());

			return out;
		}
	};
}

template <StringLiteral Name>
constexpr auto operator""_t()
{
	return NamedArgument<Name, Any>{};
}

template <StringLiteral Name, typename... Ts>
struct NamedTuple
	: Ts...
{
	static constexpr auto tag_name{ Name };
	
	static constexpr auto size() noexcept
	{
		return sizeof...(Ts);
	}

	constexpr NamedTuple(Ts&&... ts)
		: Ts{ std::forward<Ts>(ts) }...
	{

	}

	template <typename... OtherTs>
	constexpr NamedTuple(OtherTs&&... ts)
		: Ts{ std::forward<OtherTs>(ts) }...
	{

	}

	template <typename... OtherTs>
	constexpr NamedTuple& operator=(NamedTuple<Name, OtherTs...>&& other) noexcept
	{
		(..., (static_cast<Ts&>(*this) = std::move(other)));

		return *this;
	}

	template <typename... OtherTs>
	constexpr NamedTuple& operator=(NamedTuple<Name, OtherTs...> const& other)
	{
		(..., (static_cast<Ts&>(*this) = other));

		return *this;
	}

	template <typename T, typename TArg = MapLookup<NamedTuple, T::name, void, NamedArgument>>
	constexpr auto const& operator[](T const) const
		requires (not std::is_void_v<TArg>)
	{
		return static_cast<TArg const&>(*this).value;
	}

	template <typename T, typename TArg = MapLookup<NamedTuple, T::name, void, NamedArgument>>
	constexpr auto& operator[](T const)
		requires (not std::is_void_v<TArg>)
	{
		return static_cast<TArg&>(*this).value;
	}

	template <typename... OtherTs>
	auto& Assign(OtherTs&&... ts)
	{
		if constexpr ((..., (requires {
			ts.name;
			ts.value;
		})))
		{
			(..., (static_cast<MapLookup<NamedTuple, ts.name, void, NamedArgument>&>(*this).value = ts.value));
		}
		else
		{
			(..., (static_cast<Ts&>(*this).value = ts));
		}

		return *this;
	}

	template <StringLiteral... tag_names, typename Func>
	void Apply(Func&& func) const
	{
		std::invoke(std::forward<Func>(func), static_cast<MapLookup<NamedTuple, tag_names, void, NamedArgument> const&>(*this).value...);
	}

	template <typename Func>
	void ApplyAll(Func&& func) const
	{
		[func = std::forward<Func>(func), this] <auto... Ns>(std::index_sequence<Ns...>) mutable {
			std::invoke(std::forward<Func>(func), get<Ns>()...);
		}(std::make_index_sequence<size()>{});
	}

	template <std::size_t N>
	auto& get()
	{
		auto id_type{ []<auto... Ns>(std::index_sequence<Ns...>) {
			return Inherit<std::pair<std::integral_constant<std::size_t, Ns>, Ts>...>{};
		}(std::make_index_sequence<sizeof...(Ts)>{}) };

		using T = typename decltype(Internal::MapLookup<void, std::integral_constant<std::size_t, N>, std::pair>(&id_type))::second_type;

		return static_cast<T&>(*this);
	}

	template <std::size_t N>
	auto const& get() const
	{
		auto id_type{ []<auto... Ns>(std::index_sequence<Ns...>) {
			return Inherit<std::pair<std::integral_constant<std::size_t, Ns>, Ts>...>{};
		}(std::make_index_sequence<sizeof...(Ts)>{}) };

		using T = typename decltype(Internal::MapLookup<void, std::integral_constant<std::size_t, N>, std::pair>(&id_type))::second_type;

		return static_cast<T const&>(*this);
	}

	friend std::ostream& operator<<(std::ostream& os, NamedTuple const& nt)
	{
		os << std::string_view{ Name } << '{';

		[&]<auto... Ns>(std::index_sequence<Ns...>) {
			(..., ((os << (Ns ? "," : "") << std::string_view{ Ts::name } << ":" << static_cast<MapLookup<NamedTuple, Ts::name, void, NamedArgument> const&>(nt).value)));
		}(std::make_index_sequence<sizeof...(Ts)>{});

		os << '}';

		return os;
	}
};

template <typename... Ts>
NamedTuple(Ts...) -> NamedTuple<"", Ts...>;

template <StringLiteral Name = "", typename... Ts>
constexpr auto MakeNamedTuple(Ts&&... ts)
{
	return NamedTuple<Name, std::remove_cvref_t<Ts>...>(std::forward<Ts>(ts)...);
}

namespace std
{
	template <StringLiteral Name, typename... Ts>
	struct tuple_size<NamedTuple<Name, Ts...>>
		: std::integral_constant<std::size_t, sizeof...(Ts)>
	{

	};

	template <std::size_t N, StringLiteral Name, typename... Ts>
	struct tuple_element<N, NamedTuple<Name, Ts...>>
	{
		using type = decltype(std::declval<NamedTuple<Name, Ts...>>().template get<N>());
	};

	template <std::size_t N, StringLiteral Name, typename... Ts>
	decltype(auto) get(NamedTuple<Name, Ts...>&& nt) noexcept
	{
		return nt.template get<N>();
	}
}

template <typename T, StringLiteral... Names>
concept Extends = (... and requires(T t, NamedArgument<Names, Any> name) {
	t[name];
});
