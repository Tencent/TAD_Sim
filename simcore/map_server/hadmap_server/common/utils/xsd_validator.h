// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "common/utils/scoped_cleanup.h"
#include "common/utils/singleton.h"
#include "common/utils/status.h"

#include "libxml/encoding.h"
#include "libxml/xmlreader.h"
#include "libxml/xmlwriter.h"

namespace utils {

class XsdValidator : public Singleton<XsdValidator> {
 public:
  enum XsdType {
    kNone = 0,
    kOpenScenario_1_0,
    kOpenDrive_1_4,
  };

  virtual ~XsdValidator();

  void Initialize(const std::string& xsd_path);

  utils::Status Validate(const std::string& xml, const XsdType& xsd_type);

  static utils::Status Validate(const std::string& xml, const std::string& xsd);

 private:
  static utils::Status Validate(xmlDocPtr doc, xmlSchemaValidCtxtPtr schema_valid_ctxt);

  static void ErrorHandler(void* arg, xmlErrorPtr err) {}

  struct Impl {
    explicit Impl(const std::string& xsd);
    ~Impl();
    xmlSchemaParserCtxtPtr schema_parser_ctxt = nullptr;
    xmlSchemaPtr schema = nullptr;
    xmlSchemaValidCtxtPtr schema_valid_ctxt = nullptr;
  };
  using ImplPtr = std::shared_ptr<Impl>;

 private:
  std::map<XsdType, std::string> type2path_;
  static std::map<XsdType, std::string> type2name_;
};

}  // namespace utils
