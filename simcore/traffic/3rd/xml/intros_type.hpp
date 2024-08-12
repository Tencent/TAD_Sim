#pragma once

#include <tuple>
#include <string>
#include <type_traits>
#include <boost/preprocessor.hpp>
//#include "details\intros_type_details.hpp"


#define BEGIN_INTROS_TYPE(type)										\
	BEGIN_INTROS_TYPE_USER_NAME(									\
		type,														\
		BOOST_PP_STRINGIZE(type)									\
	)


#define BEGIN_INTROS_TYPE_USER_NAME(type, name)						\
	namespace utils { namespace intros { namespace details {		\
		template<typename IsConstTag>								\
		auto get_intros_type_impl(type& val, IsConstTag tag) {		\
			return ::utils::intros::details::make_intros_type(name


#define END_INTROS_TYPE(type)										\
	);}}}}															\
	auto get_intros_type(type& val)									\
	{																\
		return utils::intros::details::get_intros_type_impl(		\
			val, ::utils::intros::details::non_const_version());	\
	}																\
	auto get_intros_type(const type& val)							\
	{																\
		using namespace utils::intros::details;						\
		return utils::intros::details::get_intros_type_impl(		\
			const_cast<type&>(val),									\
			::utils::intros::details::const_version());				\
	}


#define ADD_INTROS_ITEM(x)											\
	ADD_INTROS_ITEM_USER_NAME(x, BOOST_PP_STRINGIZE(x))


#define ADD_INTROS_ITEM_USER_NAME(x, name)							\
	, ::utils::intros::details::make_intros_item(name, val.x, tag)


namespace utils { namespace intros
{
	template<typename... TupleItems>
	struct intros_type
	{
		std::string name;
		std::tuple<TupleItems...> items;
	};

	template<typename T>
	struct intros_item
	{
		std::string name;
		T& val;
	};
}}


namespace utils {
	namespace intros {
		namespace details
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
		}
	}
}
