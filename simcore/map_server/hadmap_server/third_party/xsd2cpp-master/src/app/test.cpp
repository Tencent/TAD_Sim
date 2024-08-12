#include "openscenario.h"
#include "private/openscenario.hpp"

void main() {
  std::string RES_DIR = "D:\\GIT\\TAD_Sim\\xsd2cpp-master\\src\\app\\";
  tinyxml2::XMLDocument xml_doc;
  tinyxml2::XMLError eResult = xml_doc.LoadFile((RES_DIR + "Sim_Accident_40_1.xosc").c_str());
  if (eResult != tinyxml2::XML_SUCCESS) return;

  auto OpenSCENARIO = xml_doc.FirstChildElement("OpenSCENARIO");
  if (OpenSCENARIO == nullptr) return;
  xosc::OpenScenario openScenario;

  xosc::__parse__OpenScenario(OpenSCENARIO, openScenario);

  // save
  tinyxml2::XMLDocument save_doc;
  auto newScenario = save_doc.NewElement("OpenSCENARIO");
  xosc::__save__OpenScenario(openScenario, newScenario, save_doc);
  save_doc.InsertFirstChild(newScenario);
  save_doc.SaveFile((RES_DIR + "Sim_Accident_40_1_save.xosc").c_str());
}
