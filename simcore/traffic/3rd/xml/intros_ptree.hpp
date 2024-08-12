#pragma once


#include "details/intros_ptree_details.hpp"
#include <boost/property_tree/ptree.hpp>


#define MAKE_USER_NAME(name, scope, is_attribute)		\
	(													\
		(												\
			boost::property_tree::path(scope)			\
				/= (is_attribute ? "<xmlattr>" : ""))	\
				/= name									\
	).dump()


namespace utils { namespace intros_ptree 
{
	template<typename T>
	boost::property_tree::ptree make_ptree(const T& in)
	{
		return details::make_ptree_impl(in);
	}

	template<typename T>
	T make_intros_object(boost::property_tree::ptree& tree)
	{
		return details::make_intros_object_impl<T>(tree, details::object_write_category<T>());
	}
}}
	