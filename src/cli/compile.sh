module load cesga/2022 gcc/system gcccore/system openmpi/4.1.4 scalapack/2.2.0 openblas/system
 
AER_INCL_DIR=/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/works/api-simulator/installation/include/aer-cpp/src
PYBIND_INCL_DIR=/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/conda/envs/qiskit-aer/include/pybind11
PYTHON_INCL_DIR=/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/conda/envs/qiskit-aer/include/python3.9
PYTHON_LIB=/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/conda/envs/qiskit-aer/lib
API_INCLUDES=/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/works/api-simulator/installation/include
CUSTOM_JSON_LIB=/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/works/api-simulator/installation/lib

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PYTHON_LIB:$CUSTOM_JSON_LIB

g++ -std=c++17 qraise.cpp -o qraise -I$AER_INCL_DIR -I$PYBIND_INCL_DIR -I$PYTHON_INCL_DIR -I$API_INCLUDES -L$PYTHON_LIB -L$CUSTOM_JSON_LIB -fopenmp -llapack -lblas -fPIC -lpython3.9 -lmpi -lcustom-json
#g++ -std=c++17 setup_qpus.cpp -o setup_qpus -I$AER_INCL_DIR -I$PYBIND_INCL_DIR -I$PYTHON_INCL_DIR -I$API_INCLUDES -L$PYTHON_LIB -L$CUSTOM_JSON_LIB -fopenmp -llapack -lblas -fPIC -lpython3.9 -lmpi -lcustom-json

#g++ -shared -std=c++17 -fPIC $(python -m pybind11 --includes) pybind_test.cpp -o client$(python-config --extension-suffix) -I$API_INCLUDES -lpython3.9