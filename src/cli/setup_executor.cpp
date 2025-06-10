
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>

#include "qpu.hpp"
#include "backends/simulators/Munich/munich_executor.hpp"

#include "utils/json.hpp"
#include "utils/helpers/murmur_hash.hpp"
#include "logger.hpp"

using namespace std::string_literals;

using namespace cunqa;
using namespace cunqa::sim;

void publish_endpoint(const std::string& endpoint, const std::string& tmp_fifo)
{
    auto fd = open(tmp_fifo, O_WRONLY);
    if (fd < 0) {
        LOGGER_ERROR("Error happened creating the FIFO to publish the executor endpoint: {}", strerror(errno));
        return 1;
    }

    ssize_t bytes = write(fd, endpoint.c_str(), endpoint.size());
    if (bytes < 0) {
        LOGGER_ERROR("Error writing the FIFO to publish the executor endpoint: {}", strerror(errno));
        close(fd);
        return 1;
    }

    LOGGER_DEBUG("Publish executor endpoint: {}", endpoing);

    close(fd);
}

int main(int argc, char *argv[])
{
    std::string sim_arg(argv[1]);
    std::string family_name(argv[2]);

    switch(murmur::hash(sim_arg)) {
        case murmur::hash("Aer"): 
            LOGGER_ERROR("Aer does not support quantum communications.");
            
        case murmur::hash("Munich"):
            LOGGER_DEBUG("Raising executor with Munich.");
            MunichExecutor executor();
            executor.connect(family_name);
            executor.run();
        default:
            LOGGER_ERROR("No {} communication method available.", communications);
            return EXIT_FAILURE;
    }     
    return EXIT_SUCCESS;
}