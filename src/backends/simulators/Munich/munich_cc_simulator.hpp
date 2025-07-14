#pragma once

#include <chrono>

#include "quantum_task.hpp"
#include "classical_channel.hpp"
#include "backends/cc_backend.hpp"
#include "backends/simulators/simulator_strategy.hpp"

#include "munich_helpers.hpp"

#include "utils/json.hpp"


namespace cunqa {
namespace sim {

class MunichCCSimulator final : public SimulatorStrategy<CCBackend> {
public:
    MunichCCSimulator();
    ~MunichCCSimulator() = default;

    inline std::string get_name() const override {return "MunichSimulator";}
    JSON execute(const CCBackend& backend, const QuantumTask& circuit) override;


    comm::ClassicalChannel classical_channel;
private:
    
};

} // End namespace sim
} // End namespace cunqa