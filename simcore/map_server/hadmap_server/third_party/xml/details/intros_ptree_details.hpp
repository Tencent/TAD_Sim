#pragma once

#include <tuple>
#include <string>
#include <algorithm>
#include <type_traits>
#include <boost/property_tree/ptree.hpp>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include "intros_type.hpp"
#include "util_traits.hpp"
#include "is_valid_expression.hpp"

// forward declarations begin
namespace utils { namespace intros_ptree { namespace details 
{
	template<typename T>
	void intros_to_ptree_impl(boost::property_tree::ptree& tree, const T& in);

	template<typename... T>
	void intros_from_ptree_impl(std::tuple<T...>& out, boost::property_tree::ptree& tree);
} } }
// forward declarations end

// item tag conditions
namespace utils { namespace intros_ptree { namespace details
{
	namespace tags
	{
		// item tags
		struct item_has_intros {};
		struct item_is_array {};
		struct item_can_stream_insert {};
		struct item_has_input_iterator {};
		struct item_can_stream_extract {};
		struct item_can_insert_at_end {};
		struct item_not_supported {};
	}

	namespace obj_tags
	{
		// object tags
		struct has_no_intros {};
		struct has_no_def_ctor {};
		struct proper_object {};
	}

	template<typename T>
	using has_input_iterator_check = std::enable_if_t<
		std::is_base_of<
		std::input_iterator_tag,
		typename std::iterator_traits<typename T::iterator>::iterator_category
		>::value>;

	IMPLEMENT_EXPR_TEST(has_intros, decltype(get_intros_type(std::declval<T&>())), T)
	IMPLEMENT_EXPR_TEST(can_stream_insert, decltype(std::declval<std::ostream&>() << std::declval<T&>()), T)
	IMPLEMENT_EXPR_TEST(can_stream_extract, decltype(std::declval<std::istream&>() >> std::declval<T&>()), T)
	IMPLEMENT_EXPR_TEST_INDIRECT(has_input_iterator, has_input_iterator_check, T)
	IMPLEMENT_EXPR_TEST(can_insert_at_end, decltype(std::inserter(std::declval<T&>(), std::end(std::declval<T&>()))), T)
	IMPLEMENT_EXPR_TEST(has_empty, decltype(std::declval<T&>().empty()), T)

	template<typename T>
	using item_category_read_intros = traits::get_tag<
		has_intros<T>,			tags::item_has_intros,
		std::is_array<T>,		tags::item_is_array,
		can_stream_insert<T>,	tags::item_can_stream_insert,
		has_input_iterator<T>,	tags::item_has_input_iterator,
		tags::item_not_supported
	>;

	template<typename T>
	using item_category_write_intros = traits::get_tag<
		has_intros<T>,			tags::item_has_intros,
		std::is_array<T>,		tags::item_is_array,
		can_stream_extract<T>,	tags::item_can_stream_extract,
		can_insert_at_end<T>,	tags::item_can_insert_at_end,
		tags::item_not_supported
	>;

	template<typename T>
	using object_write_category = traits::get_tag<
		utils::traits::negation<has_intros<T>>,						obj_tags::has_no_intros,
		utils::traits::negation<std::is_default_constructible<T>>,	obj_tags::has_no_def_ctor,
		obj_tags::proper_object
	>;

	namespace add_to_tree
	{
		template<typename T, std::enable_if_t<has_empty<T>::value, int> = 0>
		void add_to_tree_impl(boost::property_tree::ptree& tree, const std::string& name, const T& val, tags::item_has_intros)
		{
			if (val.empty())
				return;
			boost::property_tree::ptree subtree;
			intros_to_ptree_impl(subtree, get_intros_type(val));
			tree.add_child(name, subtree);
		}
		template<typename T, std::enable_if_t<!has_empty<T>::value, int> = 0>
		void add_to_tree_impl(boost::property_tree::ptree& tree, const std::string& name, const T& val, tags::item_has_intros)
		{
			boost::property_tree::ptree subtree;
			intros_to_ptree_impl(subtree, get_intros_type(val));
			tree.add_child(name, subtree);
		}
		template<typename T>
		void add_to_tree_impl(boost::property_tree::ptree& tree, const std::string& name, const T& val, tags::item_is_array)
		{
			static_assert(!std::is_void<T>::value, "Array not supported for intros_type");
		}
		template<typename T>
		void add_to_tree_impl(boost::property_tree::ptree& tree, const std::string& name, const T& val, tags::item_can_stream_insert)
		{
			tree.add(name, val);
		}
		template<typename T>
		void add_to_tree_impl(boost::property_tree::ptree& tree, const std::string& name, const T& val, tags::item_has_input_iterator)
		{
			std::for_each(val.begin(), val.end(), [&val, &tree, &name](const auto& x) {
				add_to_tree_impl(tree, name, x, item_category_read_intros<typename T::value_type>());
			});
		}
		template<typename T>
		void add_to_tree_impl(boost::property_tree::ptree& tree, const std::string& name, const T& val, tags::item_not_supported)
		{
			static_assert(!std::is_void<T>::value, "Unsupported type for intros_type");
		}
		template<typename T>
		void add_to_tree(boost::property_tree::ptree& tree, const T& x)
		{
			using t = std::remove_reference_t<decltype(std::remove_reference_t<T>::val)>;
			add_to_tree_impl(tree, x.name, x.val, item_category_read_intros<t>());
		}
	}

	namespace add_to_intros
	{
		template<typename T>
		T get_path_last_but(T path, std::string& last_path)
		{
			boost::property_tree::path last_but;

			while (!path.single())
				last_but /= path.reduce();

			last_path = path.reduce();

			return last_but;
		}

		template<typename T>
		void add_to_intros_impl(T& val, const std::string& name, boost::property_tree::ptree& tree, tags::item_has_intros)
		{
#if 1
			auto& subtree = (name == "" ? tree :
				(boost::none == tree.get_child_optional(name) ? (tree.put(name, T()), tree.get_child(name)) : tree.get_child(name))
				);
#else
			auto& subtree = (name == "" ? tree : tree.get_child(name));
#endif
			auto intros_type = get_intros_type(val);
			intros_from_ptree_impl(intros_type.items, subtree);
		}
		template<typename T>
		void add_to_intros_impl(T& val, const std::string& name, boost::property_tree::ptree& tree, tags::item_is_array)
		{
			static_assert(!std::is_void<T>::value, "Array not supported for intros_type");
		}
		template<typename T>
		void add_to_intros_impl(T& val, const std::string& name, boost::property_tree::ptree& tree, tags::item_can_stream_extract)
		{
			val = tree.get(name, T());
		}
		template<typename T>
		void add_to_intros_impl(T& val, const std::string& name, boost::property_tree::ptree& tree, tags::item_can_insert_at_end)
		{
			std::string last_path;
			boost::property_tree::ptree empty_tree;
			auto source = tree.get_child(
							get_path_last_but(boost::property_tree::path(name), last_path), 
							empty_tree);

			for (auto source_it = source.begin(); source_it != source.end(); ++source_it)
			{
				if (source_it->first == last_path)
				{
					typename T::value_type item;
					add_to_intros_impl(item, "", source_it->second, item_category_write_intros<typename T::value_type>());
					std::inserter(val, val.end()) = item;
				}
			}
		}
		template<typename T>
		void add_to_intros_impl(T& val, const std::string& name, boost::property_tree::ptree& tree, tags::item_not_supported)
		{
			static_assert(!std::is_void<T>::value, "Unsupported type for intros_type");
		}
		template<typename T>
		void add_to_intros(T& x, boost::property_tree::ptree& tree)
		{
			using t = std::remove_reference_t<decltype(std::remove_reference_t<T>::val)>;
			add_to_intros_impl(x.val, x.name, tree, item_category_write_intros<t>());
		}
	}

	template<typename T>
	void intros_to_ptree_impl(boost::property_tree::ptree& tree, const T& in)
	{
		boost::fusion::for_each(in.items,
			[&tree](auto& x) {
			add_to_tree::add_to_tree(tree, x);
		});
	}

	template<typename... T>
	void intros_from_ptree_impl(std::tuple<T...>& out, boost::property_tree::ptree& tree)
	{
		boost::fusion::for_each(out,
			[&tree](auto& x) {
			add_to_intros::add_to_intros(x, tree); });
	}

	template<typename T>
	T make_intros_object_impl(boost::property_tree::ptree& tree, obj_tags::has_no_intros)
	{
		static_assert(!std::is_void<T>::value, "Object has no intros support");
        return T();
	}

	template<typename T>
	T make_intros_object_impl(boost::property_tree::ptree& tree, obj_tags::has_no_def_ctor)
	{
		static_assert(!std::is_void<T>::value, "Cannot make object with no default constructor");
        return T();
	}

	template<typename T>
	T make_intros_object_impl(boost::property_tree::ptree& tree, obj_tags::proper_object)
	{
		T ret;

		auto ret_intros = get_intros_type(ret);

		details::intros_from_ptree_impl(ret_intros.items, tree.get_child(ret_intros.name));

		return ret;
	}
	template<typename T, std::enable_if_t<!details::has_intros<T>::value, int> = 0>
	boost::property_tree::ptree make_ptree_impl(T)
	{
		static_assert(!std::is_void<T>::value, "Use intros_type for introspection support");
        return boost::property_tree::ptree();
	}

	template<typename T, std::enable_if_t<details::has_intros<T>::value, int> = 0>
	boost::property_tree::ptree make_ptree_impl(const T& in)
	{
		auto intros_type = get_intros_type(in);
		boost::property_tree::ptree tree, subtree;

		details::intros_to_ptree_impl(subtree, intros_type);

		tree.add_child(intros_type.name, subtree);
		return tree;
	}
}}}
