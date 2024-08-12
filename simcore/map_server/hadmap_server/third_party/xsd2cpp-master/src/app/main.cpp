#include <fstream>
#include <xsd2cpp.h>
#include <string>
#include <sstream>

int main(int argc, char *argv[]) {
  using namespace xsd2cpp;

  //
  std::string RES_DIR =
      "D:\\UGit\\Tadsim3.0\\tadsim\\simcore\\map_server\\hadmap_server\\third_party\\xsd2cpp-master\\src\\app";
  // create file name
  std::string filename(RES_DIR);
  filename += "\\OpenSCENARIO_1_0_0.xsd";

  // create header filename
  std::string headerFilename(RES_DIR);
  headerFilename += "\\header.txt";

  // lib name and version
  std::string libName = "openscenario";
  std::string libVersion = "1_0";

  //
  std::string outPathPublic(RES_DIR);
  std::string outPathPrivate(RES_DIR);

  // create config
  xsd2cpp::Config config{};
  config.year = "2024";
  config.libName = libName;
  config.libKey = "osc";
  // create outputs
  std::string structHeaderFile = outPathPublic + "\\" + libName + ".h";
  std::string headerFile = outPathPrivate + "\\" + libName + ".hpp";
  std::string sourceFile = outPathPrivate + "\\" + libName + ".cc";

  // create files
  std::fstream fsp(structHeaderFile, std::ios::out);
  std::fstream fsh(headerFile, std::ios::out);
  std::fstream fss(sourceFile, std::ios::out);

  // run lib
  xsd2cpp::xsd2cpp(filename, fsp, fsh, fss, headerFilename, config);

  return 0;
}
