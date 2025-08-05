#pragma once

#include <string>
#include <fstream>
#include <vector>

#include "argparse.hpp"
#include "args_qraise.hpp"

#include "utils/json.hpp"
#include "utils_qraise.hpp"
#include "logger.hpp"

using namespace cunqa;

namespace {
    
}

void write_sbatch_infrastructure_file(std::ofstream& sbatchFile, const CunqaArgs& args)
{

    //------------------ Variables block ----------------------------
    auto store = std::getenv("STORE");

    JSON infrastructure;
    std::ifstream f(args.infrastructure.value());
    infrastructure = JSON::parse(f);

    auto qpus = infrastructure.at("qpus").get<JSON>();
    auto classical_connectivity = infrastructure.at("classical_connectivity").get<std::vector<std::string>>();
    auto quantum_connectivity = infrastructure.at("quantum_connectivity").get<std::vector<std::vector<std::string>>>();
    int n_cc_qpus = 0;
    int n_qc_qpus = 0;

    std::vector<std::string> written_qpus;
    bool qpu_already_written;

    std::string simulator;
    std::string backend_path;
    std::vector<std::string> paths_to_backend;
    std::string qpu_properties_path;
    std::string family_name;
    int n_ports;

    //---------------------------------------------------------------


    //----------------- Sbatch header block --------------------------
    int n_tasks = qpus.size() + quantum_connectivity.size();
    int cores_per_task = 2; // 2 cores per QPU by default
    int mem_per_qpu = 2; // 2GB per QPU by default
    if (quantum_connectivity.size() > 0) {
        int max_qc_group_size = 0;
        for (auto& qc_group : quantum_connectivity) {
            if (qc_group.size() > max_qc_group_size) {
                max_qc_group_size = qc_group.size();
            }
        }
        cores_per_task = cores_per_task * max_qc_group_size;
        mem_per_qpu = mem_per_qpu * max_qc_group_size;
    }
    
    int total_mem = mem_per_qpu * qpus.size(); 
    int n_nodes = number_of_nodes(qpus.size(), cores_per_task, 1, 64);
    std::string time = "01:00:00"; 

    sbatchFile << "#!/bin/bash\n\n";
    sbatchFile << "#SBATCH --job-name=qraise \n";
    sbatchFile << "#SBATCH --ntasks=" << std::to_string(n_tasks) << "\n";
    sbatchFile << "#SBATCH -c " << std::to_string(cores_per_task) << "\n"; 
    sbatchFile << "#SBATCH --mem-per-cpu=" << std::to_string(mem_per_qpu) << "G\n";
    sbatchFile << "#SBATCH -N " << std::to_string(n_nodes) << "\n";
    sbatchFile << "#SBATCH --time=" << time << "\n";
    sbatchFile << "#SBATCH --output=qraise_%j\n";
    //sbatchFile << "#SBATCH --ntasks-per-node=" << 1 << "\n";
    //sbatchFile << "#SBATCH --mem=" << std::to_string(total_mem) << "G\n";
    //sbatchFile << "#SBATCH --nodelist=c7-3 \n";
    //--------------------------------------------------------


    // ------ Directory and enviroment variables block -------
    sbatchFile << "\n";
    sbatchFile << "if [ ! -d \"$STORE/.cunqa\" ]; then\n";
    sbatchFile << "mkdir $STORE/.cunqa\n";
    sbatchFile << "fi\n\n";

    sbatchFile << "EPILOG_PATH=" << store << "/.cunqa/epilog.sh\n";
    sbatchFile << "export INFO_PATH=" << store << "/.cunqa/qpus.json\n";
    sbatchFile << "export COMM_PATH=" << store << "/.cunqa/communications.json\n\n";
    //--------------------------------------------------------


    //--------- Quantum communications block -----------------
    bool first_qc_group = true;
    for (auto& qc_group : quantum_connectivity) {
        if (!first_qc_group) {
            sbatchFile << " &\n\n";
        }
        first_qc_group = false;

        for (auto& qc_qpu : qc_group) {
            n_ports = qc_group.size() * 3;
            simulator = infrastructure.at("qpus").at(qc_qpu).at("simulator").get<std::string>();
            paths_to_backend.push_back(infrastructure.at("qpus").at(qc_qpu).at("backend").get<std::string>());

            written_qpus.push_back(qc_qpu);
            n_qc_qpus++;
        }

        qpu_properties_path = R"({"qpu_properties":[)";
        bool first = true;
        for (auto& path : paths_to_backend) {
            if (!first) {
                qpu_properties_path += ", ";
            }
            first = false;
            qpu_properties_path += "\"" + path + "\"";
        } 
        qpu_properties_path += R"(]})";

        sbatchFile << "srun -n " + std::to_string(qc_group.size()) + " -c 1 --mem-per-cpu=1G --resv-ports=" + std::to_string(n_ports) + " --task-epilog=$EPILOG_PATH setup_qpus $INFO_PATH cloud qc default " + simulator + " \'" + qpu_properties_path + "\' &\n";

        sbatchFile << "sleep 1\n";

        sbatchFile << "srun -n 1 --resv-ports=" + std::to_string(qc_group.size()) + " setup_executor " + simulator;
    }
    //------------------------------------------------------


    //----------- Classical communications block -----------
    if ((classical_connectivity.size() > 0) && (quantum_connectivity.size() > 0)) {
        sbatchFile << " &\n\n";
    }

    bool first_cc_qpu = true;
    for (auto& cc_qpu : classical_connectivity) {
        qpu_already_written = std::find(written_qpus.begin(), written_qpus.end(), cc_qpu) != written_qpus.end();
        if (qpu_already_written) { continue; }
        if (!first_cc_qpu) {
            sbatchFile << " &\n\n";
        }
        first_cc_qpu = false;

        n_ports = 2;
        simulator = infrastructure.at("qpus").at(cc_qpu).at("simulator").get<std::string>();
        backend_path = infrastructure.at("qpus").at(cc_qpu).at("backend").get<std::string>();
        qpu_properties_path = R"({"qpu_properties":[")" + backend_path + R"("]})";

        sbatchFile << "srun -n 1 --resv-ports=2 --task-epilog=$EPILOG_PATH setup_qpus $INFO_PATH cloud cc " + cc_qpu + " " + simulator + " \'" + qpu_properties_path + "\'";

        written_qpus.push_back(cc_qpu);
        n_cc_qpus++;    
    }

    //-----------------------------------------------------


    //--------------- Simple QPUs block -------------------
    if (((classical_connectivity.size() > 0) || (quantum_connectivity.size() > 0)) && (qpus.size() > n_cc_qpus + n_qc_qpus)) {
        sbatchFile << " &\n\n";
    }

    bool first_simple_qpu = true;
    for (auto& [name, properties] : qpus.items()) {
        qpu_already_written = std::find(written_qpus.begin(), written_qpus.end(), name) != written_qpus.end();
        if (qpu_already_written) { continue; }
        if (!first_simple_qpu) {
            sbatchFile << " &\n\n"; 
        }
        first_simple_qpu = false;

        simulator = properties.at("simulator").get<std::string>();
        backend_path = properties.at("backend").get<std::string>();
        qpu_properties_path = R"({"qpu_properties":[")" + backend_path + R"("]})" ;
        sbatchFile << "srun -n 1 --task-epilog=$EPILOG_PATH setup_qpus $INFO_PATH cloud no_comm "  + name + " " + simulator + " \'" + qpu_properties_path + "\'"; 
        
    }
    //--------------------------------------------------
}