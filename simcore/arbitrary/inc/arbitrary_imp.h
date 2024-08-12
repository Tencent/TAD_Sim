#pragma once

#include <string>

#ifdef _WIN32
#  ifdef BUILD_ARBITRARY_DLL
#    define Arbitrary_API __declspec(dllexport)
#  else
#    define Arbitrary_API _descspec(dllimport)
#  endif  // BUILD_ARBITRARY_DLL
#else
#  define Arbitrary_API
#endif

namespace arbitrary {
class CArbitraryImp {
 public:
  CArbitraryImp();
  virtual ~CArbitraryImp();

 public:
  std::string GetArbitrary_ControlPayload() const;
  bool DoArbitrary_Control(const std::string &ctrl_alg_payload, const std::string &ctrl_driver_payload);

 protected:
  std::string m_arbitrary_ctrl_payload;
};
}  // namespace arbitrary
