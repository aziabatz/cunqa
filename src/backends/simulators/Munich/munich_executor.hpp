
#include <string>
#include "classical_channel.hpp"

namespace cunqa {
class MunichExecutor {
public:
    MunichExecutor(const int& n_qpus);
    ~MunichExecutor() = default;

    void run();
private:
    comm::ClassicalChannel classical_channel;
    std::vector<std::string> qpu_ids;
};
}