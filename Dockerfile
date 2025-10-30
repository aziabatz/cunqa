FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
  apt-get install -y --no-install-recommends \
  ca-certificates curl gnupg build-essential \
  cmake ninja-build git pkg-config \
  openmpi-bin libopenmpi-dev \
  libopenblas-dev liblapack-dev liblapacke-dev gfortran \
  libboost-all-dev nlohmann-json3-dev pybind11-dev libsodium-dev \
  python3 python3-pip python3-venv python3-dev vim

#  TODO uncomment before publishing
#  RUN apt-get clean && rm -rf /var/lib/apt/lists/*

RUN python3 -m pip install --no-cache-dir --upgrade pip wheel pybind11 qiskit

ENV PATH="/usr/local/bin:${PATH}"
ENV PYTHONPATH="/usr/local:${PYTHONPATH}"
ENV STORE="/usr/local"

WORKDIR /opt/cunqa
COPY . /opt/cunqa
RUN if [ -f .gitmodules ]; then git submodule update --init --recursive || true; fi
