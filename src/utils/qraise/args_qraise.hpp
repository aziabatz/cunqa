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
    std::optional<std::string>& backend  = kwarg("b,backend", "Path to the backend config file.");
    std::string& simulator               = kwarg("sim,simulator", "Simulator reponsible of running the simulations.").set_default("Aer");
    std::string& mem_per_qpu             = kwarg("mem-per-qpu", "Memory given to each QPU.").set_default("7G");
    std::optional<std::string>& fakeqmio = kwarg("fq,fakeqmio", "Raise FakeQmio backend from calibration file", /*implicit*/"last_calibrations");
    std::optional<std::string>& comm = kwarg("comm", "Raise QPUs with MPI communications").set_default("no_comm");

    void welcome() {
        std::cout << "Welcome to qraise command, a command responsible for turn on the required QPUs.\n" << std::endl;
    }
};