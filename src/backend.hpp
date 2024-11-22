#include <iostream>
#include <fstream>
#include <memory>
#include "ip_config.hpp"
#include "../utils/constants.hpp"

using json = nlohmann::json;


<template SimType>
class Backend {
    std::unique_ptr<SimType> simulator;
    Config config;
public:

    Backend() :
        simulator{std::make_unique<SimType>()}
        config{};
    { } 

    Backend(const json& config) :
        simulator{std::make_unique<SimType>()}
        config{config};
    { }


}