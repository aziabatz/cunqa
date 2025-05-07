
#include <memory>

#include "munich_ccomm_simulator.hpp"



int main() {


    std::unique_ptr<cunqa::sim::QuantumComputation> qc = std::make_unique<cunqa::sim::QuantumComputation>();

    cunqa::sim::DistributedCircuitSimulator dcs(std::move(qc));

    dcs.sayhello();

    return 0;
}