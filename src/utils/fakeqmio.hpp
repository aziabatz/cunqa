#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>
using json = nlohmann::json;


enum Error {
  Q1Gates,
  Q2Gates,
  Q2GatesRB,
  Qubits
};

std::unordered_map<std::string, int> gate_calibrations =
    {
        {"Q1Gates", Error::Q1Gates},
        {"Q2Gates", Error::Q2Gates},
        {"Q2Gates(RB)", Error::Q2GatesRB},
        {"Qubits", Error::Qubits}
    };

json from_calibrations_to_json(std::string path_to_calibrations){

    std::vector<json> errors;
    json calibrations;

    json qerror_json = {
        {"type", "qerror"},
        {"operations", {}},
        {"gate_qubits", {}},
        {"noise_qubits", {}},
        {"probabilities", {}}
        //{"instructions", {}} //fundamental
    };

    json roerror_json = {
        {"type", "roerror"},
        {"operations", {"measure"}},
        {"probabilities", {}},
        {"gate_qubits", {}}
    };

    std::ifstream calibration_file(path_to_calibrations);
    if (!calibration_file.is_open()){
        std::cerr << "Calibration file does not exist " << path_to_calibrations << "\n";
        return 1; // Salir con error
    }

    try {
        calibration_file >> calibrations;
        calibration_file.close();
    } catch (const std::exception& e) {
        std::cerr << "Json file cannot be read. " << e.what() << "\n";
        return 1; // Salir con error
    }


    for (auto gates = calibrations.begin(); gates != calibrations.end(); gates++){ 

        for (auto error = gates.value().begin(); error != gates.value().end(); error++){

            switch(gate_calibrations[gates.key()]){
                case Error::Q1Gates:
                    qerror_json["operations"] = {"sx"};
                    qerror_json["gate_qubits"] = {{static_cast<int>(error.key()[2])}};
                    qerror_json["noise_qubits"] = {{static_cast<int>(error.key()[2])}};
                    qerror_json["probabilities"] = {{error.value()["SX"]["Fidelity(RB)"]}};
                    errors.push_back(qerror_json);
                    break;

                case Error::Q2Gates:
                    break;

                case Error::Q2GatesRB:
                    qerror_json["operations"] = {"ecr"};
                    qerror_json["gate_qubits"] = {{error.value()["ECR"]["Control"], error.value()["ECR"]["Target"]}};
                    qerror_json["noise_qubits"] = {{error.value()["ECR"]["Control"], error.value()["ECR"]["Target"]}};
                    qerror_json["probabilities"] = {{error.value()["ECR"]["Fidelity(RB)"]}};
                    errors.push_back(qerror_json);
                    break;

                case Error::Qubits:
                    roerror_json["probabilities"] = {{error.value()["Readout fidelity(RB)"]}};
                    roerror_json["gate_qubits"] = {{static_cast<int>(error.key()[2])}};
                    roerror_json["noise_qubits"] = {{static_cast<int>(error.key()[2])}};
                    errors.push_back(roerror_json);
                    break; 

            }
        }
    }

/*     json noise_model = {
        {"error_model", {
            "errors", {errors}
            }
        }
    };  */  

    json noise_model = {
        {
            "errors", errors
        }
    };    


    return noise_model;
}



json fakeqmio_backend(std::string path_to_calibrations){

    json calibration_json = from_calibrations_to_json(path_to_calibrations);

    json fakeqmio_backend = {
        {"backend", {
            {"basis_gates", ""},
            {"conditional", true},
            {"custom_instructions", ""},
            {"description", "FakeQmio"},
            {"is_simulator", true},
            {"max_shots", 1000000},
            {"memory", true},
            {"n_qubits", 32},
            {"coupling_map", {}},
            {"gates", {}},
            {"name", "FakeQmio"},
            {"simulator", "AerSimulator"},
            {"url", ""},
            {"version", ""},
            calibration_json
            }
        }
        

    };

    return fakeqmio_backend;
}


