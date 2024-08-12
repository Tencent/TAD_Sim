#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"
#include "boost/filesystem/path.hpp"

namespace tx_sim {
namespace test {
namespace fs = boost::filesystem;

// TODO(nemo): the resource path should be passed through cmdline args or env vars.
fs::path GetTestResourceDirPath();

}  // namespace test
}  // namespace tx_sim
