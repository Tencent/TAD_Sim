/**
 * @file read_xml.cpp
 * @author kekesong (kekesong@tencent.com)
 * @brief
 * @version 0.1
 * @date 2024-05-16
 *
 * @copyright Copyright (c) 2024 Tencent Inc.  All rights reserved.
 *
 */
#include "read_xml.h"
#include <sstream>
#include <stdexcept>

using namespace tinyxml2;

// using explict namespace prefix to avoid error C2872: 'XMLDocument': ambiguous
// symbol on Windows(msxml.h).
XMLElement *GetXMLRoot(const std::string &path, tinyxml2::XMLDocument &doc) {
  if (doc.LoadFile(path.c_str()) != XML_SUCCESS)
    throw std::invalid_argument("cannot open file: " + path + " " + doc.ErrorStr());
  XMLElement *root = doc.RootElement();
  if (root == nullptr) throw std::invalid_argument("no root element found in " + path);
  return root;
}

/**
 * @brief Get the Node Attribute With Default object
 *
 * @param node
 * @param attr_name
 * @param defaultValue
 * @return std::string
 */
std::string GetNodeAttributeWithDefault(const XMLElement *node, const char *attr_name,
                                        const std::string &defaultValue) {
  const char *value = node->Attribute(attr_name);
  std::string ret;
  if (value == nullptr) {
    ret = defaultValue;
  } else {
    ret = value;
  }
  return ret;
}

/**
 * @brief Get the Child Node object
 *
 * @param f
 * @param name
 * @return const XMLElement*
 */
const XMLElement *GetChildNode(const XMLElement *f, const char *name) {
  const XMLElement *child = f->FirstChildElement(name);
  if (child == nullptr) throw std::invalid_argument("no " + std::string(name) + " element found");
  return child;
}

/**
 * @brief Get the Child Node object
 *
 * @param f
 * @param name
 * @param key
 * @param value
 * @return const XMLElement*
 */
const XMLElement *GetChildNode(const XMLElement *f, const char *name, const char *key, const char *value) {
  const XMLElement *pNode = f->FirstChildElement(name);
  while (pNode) {
    std::string sValue = GetNodeAttributeWithDefault(pNode, key, "");
    if (sValue == value) return pNode;
    pNode = pNode->NextSiblingElement();
  }
  return nullptr;
}

/**
 * @brief Get the Child Node object
 *
 * @param f
 * @param name
 * @return const XMLElement*
 */
const XMLElement *TryGetChildNode(const XMLElement *f, const char *name) {
  if (f == nullptr) return nullptr;
  return f->FirstChildElement(name);
}

/**
 * @brief Get the Node Attribute object
 *
 * @param node
 * @param attr_name
 * @param node_name
 * @return std::string
 */
std::string GetNodeAttribute(const XMLElement *node, const char *attr_name, const std::string &node_name) {
  const char *p = node->Attribute(attr_name);
  if (p == nullptr)
    throw std::invalid_argument("no " + std::string(attr_name) + " attribute on node " + node_name + " specified.");
  return p;
}

/**
 * @brief Get the Node Text object
 *
 * @param node
 * @param node_name
 * @return std::string
 */
std::string GetNodeText(const XMLElement *node, const std::string &node_name) {
  const char *p = node->GetText();
  if (p == nullptr) throw std::invalid_argument("no text on node " + node_name + " specified.");
  return p;
}

/**
 * @brief  Get node with attribute In List
 *
 * @param current_node
 * @param attr_name
 * @param attr_value
 * @return const XMLElement*
 */
const XMLElement *TryGetNodeWithAttributeInList(const XMLElement *current_node, const char *attr_name,
                                                const char *attr_value) {
  const char *node_name = current_node->Name();
  do {
    if (GetNodeAttribute(current_node, attr_name, node_name) == attr_value) return current_node;
    current_node = current_node->NextSiblingElement(node_name);
  } while (current_node);
  return nullptr;
}

/**
 * @brief Get the Node With Attribute In List object
 *
 * @param current_node
 * @param attr_name
 * @param attr_value
 * @param log_name
 * @return const XMLElement*
 */
const XMLElement *GetNodeWithAttributeInList(const XMLElement *current_node, const char *attr_name,
                                             const char *attr_value, const std::string &log_name) {
  auto ret = TryGetNodeWithAttributeInList(current_node, attr_name, attr_value);
  if (ret == nullptr) {
    std::ostringstream ss;
    ss << "there's no " << log_name << " with attribute " << attr_name << "=" << attr_value << " found.";
    throw std::invalid_argument(ss.str());
  }
  return ret;
}

/**
 * @brief Get the Grand Child In List object
 *
 * @param current_node
 * @param child_name
 * @param log_name
 * @return const XMLElement*
 */
const XMLElement *GetGrandChildInList(const XMLElement *current_node, const char *child_name,
                                      const std::string &log_name) {
  const char *node_name = current_node->Name();
  bool found = (current_node->FirstChildElement(child_name) != nullptr);
  while (!found) {
    current_node = current_node->NextSiblingElement(node_name);
    if (!current_node) break;
    found = (current_node->FirstChildElement(child_name) != nullptr);
  }
  if (!found) {
    std::ostringstream ss;
    ss << "there's no " << log_name << " with first child named " << child_name << " found.";
    throw std::invalid_argument(ss.str());
  }
  return current_node->FirstChildElement(child_name);
}
