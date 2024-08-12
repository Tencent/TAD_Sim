#include "c_terrain_common.h"

#define TO_LOWER(c) (('A' <= (c) && (c) <= 'Z') ? ((c) + 0x20) : (c))

namespace terrain {
// split string
void stringSplit(const std::string &strIn, const std::string &spliter, std::vector<std::string> &splitedStrs) {
  splitedStrs.clear();

  std::string::size_type pos1 = 0, pos2 = strIn.find(spliter);

  while (std::string::npos != pos2) {
    splitedStrs.push_back(strIn.substr(pos1, pos2 - pos1));

    pos1 = pos2 + spliter.size();
    pos2 = strIn.find(spliter, pos1);
  }

  if (pos1 != strIn.length()) splitedStrs.push_back(strIn.substr(pos1));
}

void getFileExtension(const std::string &filePath, std::string &extension) {
  std::vector<std::string> pathList;
  terrain::stringSplit(filePath, ".", pathList);

  extension = ".";

  if (pathList.size() > 0) {
    extension += pathList.back();
  }

  // extension to lowercase
  for (auto i = 0ul; i < extension.size(); ++i) {
    extension.at(i) = TO_LOWER(extension.at(i));
  }
}

CTimeElapsed::CTimeElapsed(const std::string &prefix) {
  m_prefix = prefix;
  m_t_start = std::chrono::high_resolution_clock::now();
}
CTimeElapsed::~CTimeElapsed() {
  m_t_end = std::chrono::high_resolution_clock::now();
  double duration_ms = std::chrono::duration_cast<std::chrono::microseconds>(m_t_end - m_t_start).count() * 0.001;
  LOG(INFO) << m_prefix << " elapsed[ms]:" << duration_ms;
}
}  // namespace terrain
