/*******************************************************************************
 * Copyright (c) 2024, Tencent Inc.
 * All rights reserved.
 * Project:  hadmap_server
 * Modify history:
 ******************************************************************************/
#include "IXMLValidation.h"
#include <fstream>
#include <memory>
#include <sstream>
#include <xercesc/framework/MemBufInputSource.hpp>

std::ostream& operator<<(std::ostream& target, const StrX& toDump) {
  target << toDump.localForm();
  return target;
}

DOMTreeErrorReporter::DOMTreeErrorReporter() { m_errFlag = false; }
DOMTreeErrorReporter::~DOMTreeErrorReporter() {}

void DOMTreeErrorReporter::error(const SAXParseException& toCatch) {
  // cerr << "Error at file /"" << StrX(toCatch.getSystemId())
  // << "/", line " << toCatch.getLineNumber()
  // << ", column " << toCatch.getColumnNumber() << endl
  // << " Message: " << StrX(toCatch.getMessage()) << endl;
  std::ostringstream os;
  os << "Error at " << StrX(toCatch.getSystemId()) << ". line " << toCatch.getLineNumber() << ", column "
     << toCatch.getColumnNumber() << ", Message: " << StrX(toCatch.getMessage()) << endl;
  m_errMsg += os.str();
  m_errFlag = true;
}
void DOMTreeErrorReporter::fatalError(const SAXParseException& toCatch) {
  // cerr << "Fatal Error at file /"" << StrX(toCatch.getSystemId())
  // << "/", line " << toCatch.getLineNumber()
  // << ", column " << toCatch.getColumnNumber() << endl
  // << " Message: " << StrX(toCatch.getMessage()) << endl;
  std::ostringstream os;
  os << "Fatal Error at " << StrX(toCatch.getSystemId()) << ". line " << toCatch.getLineNumber() << ", column "
     << toCatch.getColumnNumber() << ", Message: " << StrX(toCatch.getMessage()) << endl;
  m_errMsg += os.str();
  m_errFlag = true;
}

IXMLValidation::IXMLValidation(std::string& grammar) : m_grammar(grammar) {
  try {
    XMLPlatformUtils::Initialize();
  } catch (const XMLException& e) {
    StrX tmp_e(e.getMessage());
    cerr << "Xerces initialization error: " << tmp_e.localForm() << endl;
    throw;  // return 2;
  }
}
IXMLValidation::~IXMLValidation() {}

int IXMLValidation::validation(std::string& xmlStr, std::string& errMsg) {
  int result;

  std::shared_ptr<XercesDOMParser> parser(new XercesDOMParser);
  std::shared_ptr<DOMTreeErrorReporter> errReporter(new DOMTreeErrorReporter);
  parser->setErrorHandler(&(*errReporter));
  parser->setDoNamespaces(true);
  parser->setCreateEntityReferenceNodes(true);
  parser->useCachedGrammarInParse(true);
  parser->setDoSchema(true);
  parser->setValidationScheme(AbstractDOMParser::Val_Always);
  parser->setValidationSchemaFullChecking(true);

  MemBufInputSource inputSrc(reinterpret_cast<XMLByte*>(const_cast<char*>(m_grammar.c_str())),
                             (XMLSize_t)m_grammar.length(), "xsd");
  if (parser->loadGrammar(inputSrc, Grammar::SchemaGrammarType, true) == 0) {
    errMsg = "Error loading grammar ";
    return 4;
  }
  char xml_fname[100];
  result = 1;
  try {
    MemBufInputSource inputXml(reinterpret_cast<XMLByte*>(const_cast<char*>(xmlStr.c_str())),
                               (XMLSize_t)xmlStr.length(), "xml");
    parser->parse(inputXml);
    result = errReporter->getErrFlag() ? 9 : 0;  // 0 成功, !0 失败
    errMsg = errReporter->getErrMsg();
  } catch (const OutOfMemoryException&) {
    errMsg = "Out of memory exception.";
  } catch (const XMLException& e) {
    std::ostringstream os;
    os << "An error occurred during parsing, Message: " << StrX(e.getMessage());
    errMsg = os.str();
  } catch (const DOMException& e) {
    const int kMaxChars = 2048;
    XMLCh errText[kMaxChars];
    std::ostringstream os;
    os << "DOM Exception code is: " << e.code << ". ";
    if (DOMImplementation::loadDOMExceptionMsg(e.code, errText, kMaxChars)) os << "Message is: " << StrX(errText);
    errMsg = os.str();
  } catch (...) {
    errMsg = "An error occurred during parsing.";
  }
  return result;
}
