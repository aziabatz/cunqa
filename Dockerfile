FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
  apt-get install -y --no-install-recommends \
  ca-certificates curl gnupg build-essential \
  cmake ninja-build git pkg-config openssh-client openssh-server \
  openmpi-bin libopenmpi-dev \
  libopenblas-dev liblapack-dev liblapacke-dev gfortran \
  libboost-all-dev nlohmann-json3-dev pybind11-dev libsodium-dev ccache \
  python3 python3-pip python3-venv python3-dev vim \
  slurmd slurm-client slurmctld munge slurm-wlm libslurm-dev

RUN set -eux; \
  mkdir -p /etc/munge /var/lib/munge /var/log/munge /run/munge; \
  # run munged as 'munge' user; all dirs owned by munge
  chown -R munge:munge /etc/munge /var/lib/munge /var/log/munge /run/munge; \
  chmod 0700 /etc/munge /var/lib/munge /var/log/munge; \
  chmod 0755 /run/munge; \
  # baked-in key so all nodes with this image share it
  if command -v create-munge-key >/dev/null 2>&1; then \
    create-munge-key; \
  else \
    dd if=/dev/urandom bs=1 count=1024 of=/etc/munge/munge.key; \
    chmod 0600 /etc/munge/munge.key; \
    chown munge:munge /etc/munge/munge.key; \
  fi

#  TODO uncomment before publishing
#  RUN apt-get clean && rm -rf /var/lib/apt/lists/*

RUN python3 -m pip install --no-cache-dir --upgrade pip wheel pybind11 \
    qiskit==1.2.4 jupyter jupyterlab

ENV PATH="/usr/local/bin:${PATH}"
ENV PYTHONPATH="/usr/local:${PYTHONPATH}"
ENV STORE="/usr/local"

WORKDIR /opt/cunqa
COPY CMakeLists.txt CMakePresets.json cmake_uninstall.cmake.in \
     .gitmodules .gitignore LICENSE.txt README.md VERSION \
     CITATION.cff configure.sh docker-compose.yml setup.py \
     /opt/cunqa/
COPY cmake/ /opt/cunqa/cmake/
COPY src/ /opt/cunqa/src/
COPY cunqa/ /opt/cunqa/cunqa/
COPY docs/ /opt/cunqa/docs/
COPY easybuild/ /opt/cunqa/easybuild/
COPY examples/ /opt/cunqa/examples/
COPY scripts/ /opt/cunqa/scripts/
COPY workspace/ /opt/cunqa/workspace/
COPY runtime/ /opt/cunqa/runtime/

ENV CCACHE_DIR=/ccache
RUN mkdir -p /ccache && chmod 777 /ccache
ENV PATH="/usr/lib/ccache:${PATH}"

RUN cmake -S /opt/cunqa -B /opt/build \
    -DBLA_VENDOR=OpenBLAS \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=OFF \
    -DCMAKE_C_COMPILER_LAUNCHER=ccache \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
RUN cmake --build /opt/build --parallel 4
RUN cmake --install /opt/build

RUN rm -rf /opt/cunqa/cunqa || true

RUN mkdir -p /workspace
WORKDIR /workspace

COPY docker/ /opt/cunqa/docker/

# Permitir login de root, con contraseña y con contraseña vacía
RUN sed -i 's/#PermitRootLogin prohibit-password/PermitRootLogin yes/' /etc/ssh/sshd_config && \
  sed -i 's/#PasswordAuthentication yes/PasswordAuthentication yes/' /etc/ssh/sshd_config && \
  echo "PermitEmptyPasswords yes" >> /etc/ssh/sshd_config && \
  # Eliminar la contraseña de root
  passwd -d root && \
  # Configurar el cliente SSH para no verificar las claves de host
  mkdir -p /root/.ssh && \
  echo "Host *\n  StrictHostKeyChecking no\n  UserKnownHostsFile=/dev/null" > /root/.ssh/config
