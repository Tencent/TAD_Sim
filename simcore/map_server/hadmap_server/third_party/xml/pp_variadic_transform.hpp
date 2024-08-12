#pragma once

#include <boost/preprocessor.hpp>

#define UTILS_PP_VARIADIC_TRANSFORM(op, data, ...)			\
BOOST_PP_SEQ_ENUM(											\
	BOOST_PP_SEQ_TRANSFORM(									\
		op, data, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)))
