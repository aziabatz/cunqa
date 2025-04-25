#pragma once

#include "utils/json.hpp"
#include <iostream>

using namespace std::literals;

namespace config{

class RunConfig {
public:
    int shots;
    std::string method;
    int memory_slots;
    int seed;

    RunConfig();
    RunConfig(cunqa::JSON config);
};

void to_json(cunqa::JSON& j, const RunConfig& run_conf);
void from_json(const cunqa::JSON& j, RunConfig& run_conf);

RunConfig::RunConfig() :
    shots{1024},
    method{"statevector"},
    memory_slots{1},
    seed{23423}
{ }

RunConfig::RunConfig(cunqa::JSON config)
{ 
        from_json(config, *this);
}

void to_json(cunqa::JSON& j, const RunConfig& run_conf)
{
    j = {   
            {"shots", run_conf.shots}, 
            {"method", run_conf.method},
            {"memory_slots", run_conf.memory_slots},
            {"seed", run_conf.seed}
        };
}

void from_json(const cunqa::JSON& j, RunConfig& run_conf) 
{
    j.at("shots").get_to(run_conf.shots);
    j.at("method").get_to(run_conf.method);
    j.at("memory_slots").get_to(run_conf.memory_slots);
    j.at("seed").get_to(run_conf.seed);
}; 

};