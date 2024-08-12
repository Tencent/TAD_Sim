#pragma once

#include <type_traits>
#include <boost/preprocessor.hpp>
#include "pp_variadic_transform.hpp"

#define ADD_TYPENAME(s, data, elem) typename elem

#define EXPAND_TYPE_PARAMS(...)	\
	UTILS_PP_VARIADIC_TRANSFORM(ADD_TYPENAME, ~, __VA_ARGS__)

#define EXPR_TEST_NAME_IMPL2(name) \
	BOOST_PP_CAT(name, _impl2)

#define EXPR_TEST_NAME_IMPL1(name)	\
	BOOST_PP_CAT(name, _impl1)

#define IMPLEMENT_EXPR_TEST_IMPL(name, test, ...)							\
	namespace exprtest														\
	{																		\
		namespace details													\
		{																	\
			template<EXPAND_TYPE_PARAMS(__VA_ARGS__), typename = test>		\
			std::true_type EXPR_TEST_NAME_IMPL2(name)();					\
			template<EXPAND_TYPE_PARAMS(__VA_ARGS__)>						\
			decltype(EXPR_TEST_NAME_IMPL2(name)<__VA_ARGS__>())				\
				EXPR_TEST_NAME_IMPL1(name)(void*);							\
			template<EXPAND_TYPE_PARAMS(__VA_ARGS__)>						\
			std::false_type EXPR_TEST_NAME_IMPL1(name)(...);				\
		}																	\
	}																		\
	template<EXPAND_TYPE_PARAMS(__VA_ARGS__)>								\
	struct name : decltype(													\
		exprtest::details::EXPR_TEST_NAME_IMPL1(name)<__VA_ARGS__>(nullptr))\
	{};

#define GET_TEST(ind_test, ...)	ind_test<__VA_ARGS__>
