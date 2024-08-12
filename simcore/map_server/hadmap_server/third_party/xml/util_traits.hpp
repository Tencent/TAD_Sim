#pragma once

#include <type_traits>

namespace utils { namespace traits
{
	//<Available from C++17>
	template <bool B>
	using bool_constant = std::integral_constant<bool, B>;

	template<class B>
	struct negation : bool_constant<!B::value> { };

	template<class...> struct disjunction : std::false_type { };
	template<class B1> struct disjunction<B1> : B1 { };
	template<class B1, class... Bn>
	struct disjunction<B1, Bn...> : std::conditional_t<B1::value != false, B1, disjunction<Bn...>> { };

	template<class...> struct conjunction : std::true_type { };
	template<class B1> struct conjunction<B1> : B1 { };
	template<class B1, class... Bn>
	struct conjunction<B1, Bn...> : std::conditional_t<B1::value != false, conjunction<Bn...>, B1> {};
	//</Available from C++17>

	template<class...> struct get_tag {};
	template<class def_tag> struct get_tag<def_tag> : def_tag {};
	template<class cond1, class tag1, class... Bn>
	struct get_tag<cond1, tag1, Bn...> : std::conditional_t<cond1::value != false, tag1, get_tag<Bn...>> {};
}}
