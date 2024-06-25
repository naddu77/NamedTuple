#include "NamedTuple.h"

#include <iostream>
#include <iomanip>
#include <print>
#include <cassert>

using namespace std::string_view_literals;
using namespace std::string_literals;

int main()
{
    // allow empty
	{
		auto const nt{ MakeNamedTuple() };

		static_assert(not [](auto t) { return requires { t[""_t]; }; }(nt));
	}

	// support direct initialization
	{
		auto nt{ MakeNamedTuple<"Trade">("price"_t = 42, "size"_t = 100) };

		std::cout << nt << std::endl;

		assert(nt["price"_t] == 42);
		assert(nt["size"_t] == 100);
	}

	// support extends
	{
		using Record = decltype(MakeNamedTuple("price"_t = int{}, "size"_t = std::size_t{}));
		constexpr Record record{ 42, 100ul };

		assert(100ul == record["size"_t]);
		static_assert(not Extends<Record, "quantity">);
		static_assert(not Extends<Record, "price", "quantity">);
		static_assert(not Extends<Record, "price", "size", "value">);

		static_assert(Extends<Record, "price", "size">);
		static_assert(Extends<Record, "size", "price">);

		constexpr auto empty{ MakeNamedTuple() };

		static_assert(not Extends<decltype(empty), "name">);

		auto name{ MakeNamedTuple(empty, "name"_t = 42) };

		static_assert(Extends<decltype(name), "name">);

		constexpr auto get_name{ [](Extends<"name"> auto const& t) {
			return t["name"_t];
		} };

		assert(42 == get_name(name));
	}

	// support assignment
	{
		auto nt{ MakeNamedTuple("price"_t = int{}, "size"_t = std::size_t{}) };

		assert(0 == nt["price"_t] and 0ul == nt["size"_t]);

		nt.Assign(42, 99ul);

		assert(42 == nt["price"_t] and 99ul == nt["size"_t]);

		nt.Assign("price"_t = 11, "size"_t = 1234);

		assert(11 == nt["price"_t] and std::size_t{ 1234 } == nt["size"_t]);
	}

	// support modification
	{
		auto nt{ MakeNamedTuple("price"_t = int{}, "size"_t = std::size_t{}) };

		nt["price"_t] = 12;
		nt["size"_t] = 34u;

		assert(12 == nt["price"_t] and 34u == nt["size"_t]);
	}

	// support composition
	{
		auto n1{ MakeNamedTuple("quantity"_t = 42) };
		auto n2{ MakeNamedTuple("value"_t = 100u) };
		auto nt{ MakeNamedTuple<"Msg">(n1, "price"_t, "size"_t, n2) };

		nt["price"_t] = 12;
		nt["size"_t] = 34u;

		assert(12 == static_cast<int>(nt["price"_t]) and 34u == static_cast<unsigned>(nt["size"_t]) and 42 == nt["quantity"_t]);
	}

	// support nesting
	{
		auto nt1{ MakeNamedTuple("first"_t, "last"_t) };
		auto nt2{ MakeNamedTuple<"Attendee">("name"_t = nt1, "position"_t) };

		nt2["name"_t]["first"_t] = "Lho"sv;
		nt2["name"_t]["last"_t] = "Hyung-Suk"sv;
		nt2["position"_t] = "CEO"sv;

		std::cout << nt2 << std::endl;
	}

	// get by value
	{
		auto nt{ MakeNamedTuple("price"_t = 100, "size"_t = 42u) };

		assert(100 == nt.get<0>().value and 42u == nt.get<1>().value);
	}

	// support decomposition
	{
		auto nt{ MakeNamedTuple("price"_t = 100, "size"_t = 42u) };
		auto& [price, size] { nt };

		assert(100 == price.value and 42u == size.value);

		price.value = 50;
		size.value = 40u;

		std::cout << nt << std::endl;
	}

	// pack the tuple
	{
		auto nt{ MakeNamedTuple("_1"_t = char{}, "_2"_t = int{}, "_3"_t = char{}) };

		static_assert(12u == sizeof(nt));
	}

	// support array
	{
		auto nt{ MakeNamedTuple<"Person">("name"_t = std::string{}, "children"_t) };

		nt.Assign("name"_t = "Hyung-Suk"s, "children"_t = Any{ std::array{ "Yoon-Jung", "BM" } });

		std::cout << nt << std::endl;

		nt.Assign("Mike", std::array{ "John" });

		std::cout << nt << std::endl;
	}

	// apply
	{
		auto nt{ MakeNamedTuple("price"_t = 42, "size"_t = 100, "name"_t = "Item"s) };
		auto f{ [](auto const&... args) {
			auto first{ true };

			std::println("{{{}}}", (... + std::format("\"{}\": {}{}", std::string_view{ args.name }, args.value, std::exchange(first, false) ? "" : ",")));
		} };

		[&] <auto... Ns>(std::index_sequence<Ns...> ns) {
			std::invoke(f, nt.template get<Ns>()...);
		}(std::make_index_sequence<nt.size()>{});
	}

	// showcase
	{
		auto employee{ MakeNamedTuple<"Employee">("name"_t, "age"_t, "title"_t) };
		std::vector<decltype(employee)> employees{
			{ "Hyung-Suk"s, 20, "Dad"s },
			{ "Yoon-Jung"s, 3, "Baby"s }
		};

		auto age{ 30 };

		employees[0].Assign("age"_t = age);

		auto const to_json{ [](std::ostream& os, auto const& xs) {
			os << "[{\n";

			for (auto const& x : xs)
			{
				os << std::format("\t\"{}\": {{\n", std::string_view{ x.tag_name });

				//x.Apply<"name", "age", "title">([](auto const& name, auto age, auto const& title) {
				//	std::println("\t\t\"name\" : {},", name);
				//	std::println("\t\t\"age\" : {},", age);
				//	std::println("\t\t\"title\" : {}", title);
				//});

				x.ApplyAll([&](auto&&... elements) {
					auto first{ true };

					(..., (os << (std::exchange(first, false) ? "" : ",\n") << "\t\t" << std::quoted(std::string_view{ elements.name }) << " : " << elements.value));
				});

				//[&]<auto... Ns>(std::index_sequence<Ns...>) {
				//	((os << (Ns ? ",\n" : "") << "\t\t" << std::quoted(std::string_view{ x.template get<Ns>().name }) << " : " << x.template get<Ns>().value), ...);
				//}(std::make_index_sequence<x.size()>{});

				os << "\n\t}\n";
			}

			os << "}]\n";
		} };

		to_json(std::cout, employees);
	}
}
