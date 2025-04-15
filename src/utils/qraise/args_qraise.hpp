#pragma once

#include <optional>

#include "argparse.hpp"
#include "logger/logger.hpp"

using namespace std::literals;

struct MyArgs : public argparse::Args 
{
    //int& node = kwarg("node", "Specific node to raise the qpus."); //Alvaro 
    int& n_qpus                          = kwarg("n,num_qpus", "Number of QPUs to be raised.");
    std::string& time                    = kwarg("t,time", "Time for the QPUs to be raised.");
    int& cores_per_qpu                   = kwarg("c,cores", "Number of cores per QPU.").set_default(2);
    int& mem_per_qpu                     = kwarg("mem,mem-per-qpu", "Memory given to each QPU in GB.").set_default(24);
    std::optional<int>& number_of_nodes  = kwarg("N,n_nodes", "Number of nodes.").set_default(1);
    std::optional<std::vector<std::string>>& node_list = kwarg("node_list", "List of nodes where the QPUs will be deployed.").multi_argument(); 
    std::optional<int>& qpus_per_node    = kwarg("qpuN,qpus_per_node", "Number of qpus in each node.");
    std::optional<std::string>& backend  = kwarg("b,backend", "Path to the backend config file.");
    std::string& simulator               = kwarg("sim,simulator", "Simulator reponsible of running the simulations.").set_default("Aer");
    std::optional<std::string>& fakeqmio = kwarg("fq,fakeqmio", "Raise FakeQmio backend from calibration file", /*implicit*/"last_calibrations");
    std::string& family_name             = kwarg("fam,family_name", "Name that identifies which QPUs were raised together").set_default("default");
    std::string& mode                    = kwarg("mode", "Infraestructure mode: HPC or CLOUD").set_default("hpc");
    std::optional<std::string>& comm     = kwarg("comm", "Raise QPUs with MPI communications").set_default("no_comm");

    void welcome() {
        std::cout << "Welcome to qraise command, a command responsible for turn on the required QPUs.\n" << std::endl;
    }
};