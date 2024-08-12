// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include "tx_header.h"

TX_NAMESPACE_OPEN(TypeTraits)

struct is_kv_container_map : public std::true_type {};
struct is_kv_container_set : public std::false_type {};
struct is_seq_container : public std::false_type {};

template <class, typename = void>
struct is_map : std::false_type {};

template <class Map>
struct is_map<Map, void /*std::void_t<typename Map::mapped_type>*/> : std::true_type {};

/**
 * @brief 检查给定的容器是否为键值容器
 * @tparam Set 容器模板类型
 * @tparam K 键类型
 * @return 如果容器是键值容器，则返回 true；否则返回 false
 */
template <template <typename...> class Set, typename K, std::enable_if_t<!is_map<Set<K>>::value, int> = 0>
constexpr auto is_kv_container_impl(const Set<K>&) -> is_kv_container_set {
  return is_kv_container_set{};
}

/**
 * @brief 检查给定的容器是否为键值容器
 * @tparam Map 容器模板类型
 * @tparam K 键类型
 * @tparam V 值类型
 * @return 如果容器是键值容器，则返回 true；否则返回 false
 */
template <template <typename...> class Map, typename K, typename V, std::enable_if_t<is_map<Map<K, V>>::value, int> = 0>
constexpr auto is_kv_container_impl(const Map<K, V>&) -> is_kv_container_map {
  return is_kv_container_map{};
}

/**
 * @brief 类型特性，根据模板参数判断给定类型是否为容器类型
 * @tparam T 待判断的类型
 * @tparam Enable_T 默认为 std::void_t
 * @return is_seq_container<T> 如果给定类型为容器类型，则返回 is_seq_container 类型，否则返回其他类型
 */
inline constexpr auto container_type_reflex_impl(...) -> is_seq_container { return is_seq_container{}; }

/**
 * @brief 模板函数，用于检查给定容器类型是否为关联容器
 * @tparam C 给定容器类型
 * @tparam Key 关联容器中的键类型
 * @param c 需要检查的容器指针
 * @return 若给定容器类型为关联容器，则返回 `is_kv_container_impl(*c)`，否则返回其他类型
 */
template <typename C, typename = typename C::key_type>
constexpr auto container_type_reflex_impl(C const* c) -> decltype(is_kv_container_impl(*c)) {
  return is_kv_container_impl(*c);
}

/**
 * @brief 返回给定容器的类型特性
 *
 * 模板函数，用于检查给定容器类型是否具有特定的特性。
 *
 * @tparam C 容器类型
 * @param c 给定的容器
 * @return 如果给定容器具有特定特性，则返回特性，否则返回其他类型
 */
template <typename C>
constexpr auto container_type_reflex(C const& c) -> decltype(container_type_reflex_impl(&c)) {
  return container_type_reflex_impl(&c);
}

/**
 * @brief 打印给定容器的内容
 *
 * 针对不同类型的容器，该函数根据容器的特性调用不同的打印方式。
 * 如果容器是序列容器，打印序列容器的内容；如果容器是关联容器，打印关联容器的内容。
 *
 * @tparam C 容器类型
 * @param c 给定的容器
 */
template <typename C>
void print_container(C const& c, is_seq_container /*is_associative*/) {
  std::cout << "is_seq_container." << std::endl;
}

/**
 * @brief 打印给定容器的内容
 *
 * 如果给定的容器是关联容器，打印关联容器的键值对；
 * 如果给定的容器是序列容器，打印序列容器的元素。
 *
 * @tparam C 容器类型
 * @param c 给定的容器
 */
template <typename C>
void print_container(C const& c, is_kv_container_map /*is_associative*/) {
  std::cout << "is_kv_container_map." << std::endl;
}

/**
 * @brief 打印给定容器的内容
 *
 * 根据给定容器的类型，打印对应类型的内容。
 * 如果给定的容器是关联容器，打印关联容器的键值对；
 * 如果给定的容器是序列容器，打印序列容器的元素。
 *
 * @tparam C 容器类型
 * @param c 给定的容器
 */
template <typename C>
void print_container(C const& c, is_kv_container_set /*is_associative*/) {
  std::cout << "is_kv_container_set." << std::endl;
}

/**
 * @brief 打印给定容器的内容
 *
 * 该函数根据给定容器的类型，打印对应类型的内容。
 * 如果给定的容器是关联容器，打印关联容器的键值对；
 * 如果给定的容器是序列容器，打印序列容器的元素。
 *
 * @tparam C 容器类型
 * @param c 给定的容器
 */
template <typename C>
void print_container(C const& c) {
  return print_container(c, container_type_reflex(c));
}

TX_NAMESPACE_CLOSE(TypeTraits)
