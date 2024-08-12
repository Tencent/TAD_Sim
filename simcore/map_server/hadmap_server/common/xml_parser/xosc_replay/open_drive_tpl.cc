/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/

#include "common/xml_parser/xosc_replay/open_drive_tpl.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "intros_ptree.hpp"
#include "util_traits.hpp"

#include <exception>
#include <iostream>
#include <string>
#include <xercesc/sax2/Attributes.hpp>
#include <xercesc/sax2/DefaultHandler.hpp>
#include <xercesc/sax2/SAX2XMLReader.hpp>
#include <xercesc/sax2/XMLReaderFactory.hpp>
#include <xercesc/util/XMLString.hpp>

using namespace xercesc;

using namespace utils::intros_ptree;
using namespace boost::property_tree;

TX_NAMESPACE_OPEN(SIM)
TX_NAMESPACE_OPEN(ODR)

BEGIN_INTROS_TYPE_USER_NAME(xmlHeader, "header")
ADD_INTROS_ITEM_USER_NAME(north, MAKE_USER_NAME("north", "", true))
ADD_INTROS_ITEM_USER_NAME(south, MAKE_USER_NAME("south", "", true))
ADD_INTROS_ITEM_USER_NAME(east, MAKE_USER_NAME("east", "", true))
ADD_INTROS_ITEM_USER_NAME(west, MAKE_USER_NAME("west", "", true))
ADD_INTROS_ITEM(geoReference)
END_INTROS_TYPE(xmlHeader)

BEGIN_INTROS_TYPE_USER_NAME(xmlOpenDRIVE, "OpenDRIVE")
ADD_INTROS_ITEM_USER_NAME(header, MAKE_USER_NAME("header", "", false))
END_INTROS_TYPE(xmlOpenDRIVE)

class XStr {
 public:
  explicit XStr(const char* const str) { data = XMLString::transcode(str); }
  ~XStr() { XMLString::release(&data); }
  const XMLCh* Data() const { return data; }

 private:
  XMLCh* data;
};

#define X(str) XStr(str).Data()

class HeaderHandler : public DefaultHandler {
 public:
  explicit HeaderHandler(xmlHeader* header, const XMLCh* tag_name = XMLString::transcode("header"))
      : header_(header), tag_name_(tag_name) {}

  void startElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname,
                    const Attributes& attrs) override {
    if (XMLString::equals(last_element_, tag_name_)) {
      throw std::runtime_error("Node found");
    }
    current_element_ = localname;
    for (int32_t i = 0; i < attrs.getLength(); i++) {
#define SET_FIELD(key)                                                 \
  if (XMLString::equals(attrs.getLocalName(i), X(#key))) {             \
    header_->key = std::atof(XMLString::transcode(attrs.getValue(i))); \
  }
      SET_FIELD(north);
      SET_FIELD(south);
      SET_FIELD(east);
      SET_FIELD(west);
    }
  }

  void endElement(const XMLCh* const uri, const XMLCh* const localname, const XMLCh* const qname) override {
    current_element_ = nullptr;
    last_element_ = localname;
  }

  void characters(const XMLCh* const chars, const XMLSize_t length) override {
    if (!current_element_ || !XMLString::equals(current_element_, X("geoReference"))) {
      return;
    }
    header_->geoReference = XMLString::transcode(chars);
  }

 private:
  const XMLCh* tag_name_ = nullptr;
  const XMLCh* current_element_ = nullptr;
  const XMLCh* last_element_ = nullptr;
  xmlHeader* header_;
};

OpenDrivePtr Load(std::istream& stream) {
  if (!stream) {
    return nullptr;
  }
  ptree tree;
  read_xml(stream, tree);
  return std::make_shared<xmlOpenDRIVE>(make_intros_object<xmlOpenDRIVE>(tree));
}

OpenDrivePtr Load(const std::string& file) {
  auto ptr = std::make_shared<xmlOpenDRIVE>();

  SAX2XMLReader* parser = XMLReaderFactory::createXMLReader();
  HeaderHandler handler(&(ptr->header));
  parser->setContentHandler(&handler);
  parser->setErrorHandler(&handler);
  try {
    parser->parse(file.c_str());
  } catch (const std::runtime_error& ex) {
  } catch (...) {
  }
  delete parser;

  return ptr;
}

TX_NAMESPACE_CLOSE(ODR)
TX_NAMESPACE_CLOSE(SIM)