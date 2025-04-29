

namespace cunqa {
namespace sim {


// TODO: Completar esto para que se pueda usar en el JSON de qpus.json
struct AerConfig {
    std::vector<std::vector<int>> coupling_map;
    std::vector<std::string> basis_gates;
    std::string custom_instructions;
    std::vector<std::string> gates;
}

inline JSON quantum_task_to_AER(QuantumTask quantum_task) 
{
    return {{"instructions", quantum_task.circuit}};
}

}
}