#pragma once

#include "simulators/circuit_executor.hpp"
#include "framework/json.hpp"
#include "framework/config.hpp"
#include "noise/noise_model.hpp"
#include "framework/circuit.hpp"
#include "controllers/controller_execute.hpp"
#include "framework/results/result.hpp"
#include "controllers/aer_controller.hpp"
#include <string>
#include "config/run_config.hpp"
#include <nlohmann/json.hpp>

//#include <utils/fakeqmio.hpp>


using json = nlohmann::json;
using namespace std::literals;
using namespace AER;
using namespace config;




class AerSimulator {
private:
    Noise::NoiseModel noise_model;
public:

    AerSimulator(){
        Noise::NoiseModel noise_default;
        noise_model = noise_default;
    }

    AerSimulator(const std::string& backend_path){
        json backend_json;
        std::ifstream backend_file(backend_path);

        if (backend_file.is_open()) {

            backend_file >> backend_json;
            backend_file.close();

        } else {
            std::cerr << "Error: File cannot be open: " << backend_path << std::endl;
        }

        if (backend_json.contains("noise")){
            Noise::NoiseModel noise_from_file(backend_json["noise"]);
            noise_model = noise_from_file; 
        } else {
            Noise::NoiseModel noise_model;
        }

        
    }



    // TODO: Añadir el modelo de ruido

    
    json execute(json circuit_json, const config::RunConfig& run_config) {
        
        Circuit circuit(circuit_json);

        json run_config_json(run_config);

        Config aer_default(run_config_json);
        //Config aer_default;
        //AER::from_json(run_config_json, aer_default); // Si const config::RunConfig& run_config --> const json& run_config_json

        std::vector<std::shared_ptr<Circuit>> circuits;

        circuits.push_back(std::make_shared<Circuit>(circuit));

        Result result = controller_execute<Controller>(circuits, this->noise_model, aer_default);

        return result.to_json();
    }

};

class AerNoise{
private:
    Noise::NoiseModel noise_model;
public:

    AerNoise(){
        std::string path_to_noise = "/mnt/netapp1/Store_CESGA/home/cesga/acarballido/repos/api-simulator/src/examples/fakeqmio-noise/fakeqmio_noise.json";
        json noise_model_json;
        std::ifstream noise_file(path_to_noise);

        if (noise_file.is_open()) {

            noise_file >> noise_model_json;
            noise_file.close();

        } else {
            std::cerr << "Error: File cannot be open: " << path_to_noise << std::endl;
        }

        Noise::NoiseModel noise_from_file(noise_model_json);
        noise_model = noise_from_file;
    }



    // TODO: Añadir el modelo de ruido

    
    json execute(json circuit_json, const config::RunConfig& run_config) {
        
        Circuit circuit(circuit_json);

        json run_config_json(run_config);

        Config aer_default(run_config_json);
        //Config aer_default;
        //AER::from_json(run_config_json, aer_default); // Si const config::RunConfig& run_config --> const json& run_config_json

        std::vector<std::shared_ptr<Circuit>> circuits;

        circuits.push_back(std::make_shared<Circuit>(circuit));

        Result result = controller_execute<Controller>(circuits, this->noise_model, aer_default);

        return result.to_json();
    }

};