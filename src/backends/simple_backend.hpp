

#include "backend.hpp"

struct SimpleConfig {
    std::string name = "SimpleSimulator";
    std::string version = "0.0.1";
    int n_qubits = 32;
    std::string description = "Simple backend with no communications.";
    std::vector<std::vector<int>> coupling_map;
    std::vector<std::string> basis_gates;
    std::string custom_instructions;
    std::vector<std::string> gates;
    json noise_model;
}

class SimpleBackend final : Backend {
public:
    SimpleConfig config;
    
    SimpleBackend(SimpleConfig config, SimulatorStrategy simulator) : 
        config{config},
        simulator_{std::make_unique<SimulatorStrategy>(simulator)}
    { }

    inline JSON execute(QuantumTask circuit) override
    {
        return simulator_->execute(circuit);
    } 

private:
    std::unique_ptr<SimulatorStrategy> simulator_;

    friend void to_json(JSON &j, SimpleBackend obj) {
        //
    }

    friend void from_json(JSON j, SimpleBackend &obj) {
        //
    }
}