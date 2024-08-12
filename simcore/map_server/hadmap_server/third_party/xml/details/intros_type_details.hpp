#pragma once

#include <tuple>
#include <string>
#include "intros_type.hpp"

namespace utils { namespace intros { namespace details
{
	struct const_version {};
	struct non_const_version {};

	template<typename T>
	auto make_intros_item(const std::string& name, T& x, non_const_version)
	{
		return intros::intros_item<T>{name, x};
	}

	template<typename T>
	auto make_intros_item(const std::string& name, T& x, const_version)
	{
		return intros_item<const T>{name, x};
	}

	template<typename... T>
	auto make_intros_type(const std::string& name, T... params)
	{
		return intros_type<decltype(params)...>{name, std::tuple<decltype(params)...>{params...}};
	}
}}}
