#pragma once

#include <cstdlib>
#include <string>
#include <unistd.h>
#include <limits.h>
#include <sstream>

namespace cunqa {
    namespace runtime_env {

        inline std::string get_env_string(const char* key)
        {
            if (const char* value = std::getenv(key)) {
                return value;
            }
            return {};
        }

        inline std::string job_id()
        {
            if (auto job = std::getenv("SLURM_JOB_ID")) return job;
            if (auto job = std::getenv("CUNQA_JOB_ID")) return job;
            if (auto job = std::getenv("OMPI_COMM_WORLD_JOBID")) return job;
            std::ostringstream oss;
            oss << "local-" << ::getpid();
            return oss.str();
        }

        inline std::string proc_id()
        {
            if (auto pid = std::getenv("SLURM_PROCID")) return pid;
            if (auto pid = std::getenv("OMPI_COMM_WORLD_RANK")) return pid;
            if (auto pid = std::getenv("PMI_RANK")) return pid;
            return "0";
        }

        inline std::string world_size()
        {
            if (auto size = std::getenv("SLURM_NTASKS")) return size;
            if (auto size = std::getenv("OMPI_COMM_WORLD_SIZE")) return size;
            if (auto size = std::getenv("PMI_SIZE")) return size;
            return {};
        }

        inline std::string task_pid()
        {
            if (auto pid = std::getenv("SLURM_TASK_PID")) return pid;
            return std::to_string(::getpid());
        }

        inline std::string node_name()
        {
            if (auto nodename = std::getenv("SLURMD_NODENAME")) return nodename;
            char hostname[HOST_NAME_MAX];
            if (::gethostname(hostname, sizeof(hostname)) == 0) {
                return hostname;
            }
            return "localhost";
        }

        inline std::string port_range()
        {
            if (auto ports = std::getenv("SLURM_STEP_RESV_PORTS")) return ports;
            if (auto ports = std::getenv("CUNQA_PORT_RANGE")) return ports;
            return {};
        }

    } 
}
