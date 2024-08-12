/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/
#pragma once
#ifndef _IXML_VALIDATION_H_
#  define _IXML_VALIDATION_H_

#  include <iostream>
#  include <list>
#  include <string>
#  include <xercesc/dom/DOM.hpp>
#  include <xercesc/framework/XMLGrammarDescription.hpp>
#  include <xercesc/parsers/XercesDOMParser.hpp>
#  include <xercesc/sax/ErrorHandler.hpp>
#  include <xercesc/sax/SAXParseException.hpp>
#  include <xercesc/util/OutOfMemoryException.hpp>

using std::cerr;
using std::endl;

XERCES_CPP_NAMESPACE_USE

class StrX {
  char* fLocalForm;

 public:
  explicit StrX(const XMLCh* const toTranscode) { fLocalForm = XMLString::transcode(toTranscode); }
  ~StrX() { XMLString::release(&fLocalForm); }
  const char* localForm() const { return fLocalForm; }
};

class DOMTreeErrorReporter : public ErrorHandler {
 public:
  DOMTreeErrorReporter();
  virtual ~DOMTreeErrorReporter();
  void warning(const SAXParseException& toCatch) {}
  void resetErrors() {}
  void error(const SAXParseException& toCatch);
  void fatalError(const SAXParseException& toCatch);
  bool getErrFlag() { return m_errFlag; }
  std::string getErrMsg() { return m_errMsg; }

 private:
  std::string m_errMsg;
  bool m_errFlag;
};

class IXMLValidation {
 public:
  explicit IXMLValidation(std::string& grammar);
  ~IXMLValidation();
  int validation(std::string& xmlStr, std::string& errMsg);

 private:
  std::string m_grammar;
};

#endif
