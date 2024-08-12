// Copyright 2024 Tencent Inc. All rights reserved.
//

#include "common/utils/xsd_validator.h"

#include <functional>
#include <iostream>
#include <sstream>
#include <string>

#include "boost/filesystem.hpp"
#include "boost/filesystem/path.hpp"

namespace utils {

std::map<XsdValidator::XsdType, std::string> XsdValidator::type2name_ = {
    {XsdValidator::XsdType::kOpenScenario_1_0, "OpenSCENARIO_1_0.xsd"},
    {XsdValidator::XsdType::kOpenDrive_1_4, "OpenDRIVE_1_4.xsd"},
};

XsdValidator::~XsdValidator() { xmlCleanupParser(); }

// XsdValidator& XsdValidator::Instance() {
//   static XsdValidator ins;
//   return ins;
// }

void XsdValidator::Initialize(const std::string& xsd_path) {
  xmlInitParser();

  for (auto& kv : type2name_) {
    boost::filesystem::path p(xsd_path + "/" + kv.second);
    if (!boost::filesystem::exists(p)) {
      continue;
    }
    type2path_[kv.first] = p.string();
  }
  initGenericErrorDefaultFunc(nullptr);
}

utils::Status XsdValidator::Validate(const std::string& xml, const XsdValidator::XsdType& xsd_type) {
  if (type2path_.count(xsd_type) <= 0) {
    return Status::NotSupported("Not supported the xsd type: ", std::to_string(xsd_type));
  }

  return Validate(xml, type2path_.at(xsd_type));
}

utils::Status XsdValidator::Validate(const std::string& xml, const std::string& xsd) {
  ImplPtr impl = std::make_shared<Impl>(xsd);
  if (impl->schema_valid_ctxt == nullptr) {
    return Status::Corruption("Failed to create schema valid ctxt ptr by xsd: ", xsd);
  }

  xmlDocPtr doc = xmlParseFile(xml.c_str());
  if (!doc) {
    return Status::Corruption("Failed to parse XML file: ", xml);
  }

  SCOPED_CLEANUP({ xmlFreeDoc(doc); });

  return Validate(doc, impl->schema_valid_ctxt);
}

utils::Status XsdValidator::Validate(xmlDocPtr doc, xmlSchemaValidCtxtPtr schema_valid_ctxt) {
  if (xmlSchemaValidateDoc(schema_valid_ctxt, doc) != 0) {
    xmlErrorPtr err = xmlGetLastError();
    std::ostringstream oss;
    oss << "Failed to validate XML file. [" << err->file << ":" << err->line << "] " << err->message;
    return Status::Corruption(oss.str());
  }
  return Status::OK();
}

XsdValidator::Impl::Impl(const std::string& xsd) {
  schema_parser_ctxt = xmlSchemaNewParserCtxt(xsd.c_str());
  if (!schema_parser_ctxt) {
    std::cerr << "Failed to create schema parser context: " << xsd << std::endl;
    return;
  }

  schema = xmlSchemaParse(schema_parser_ctxt);
  if (!schema) {
    std::cerr << "Failed to parse schema: " << xsd << std::endl;
    return;
  }

  schema_valid_ctxt = xmlSchemaNewValidCtxt(schema);
  if (!schema_valid_ctxt) {
    std::cerr << "Failed to create schema validation context." << std::endl;
    return;
  }

  xmlSchemaSetValidStructuredErrors(schema_valid_ctxt, XsdValidator::ErrorHandler, nullptr);
}

XsdValidator::Impl::~Impl() {
  xmlSchemaFreeValidCtxt(schema_valid_ctxt);
  xmlSchemaFree(schema);
  xmlSchemaFreeParserCtxt(schema_parser_ctxt);
}

}  // namespace utils
