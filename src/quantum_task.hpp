
#include "utils/json.hpp"

namespace cunqa {

class QuantumTask {
public:
    JSON circuit;
    JSON config;

    QuantumTask() = default;

    void update_circuit(const std::string& quantum_task);

private:
    bool is_parametric_ = false;
    
    void update_params_(const std::vector<double> params);
}

} // End of cunqa namespace