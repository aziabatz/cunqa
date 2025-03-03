# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/works/cunqa/src/third-party/libzmq"
  "/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/works/cunqa/build_ft3/src/third-party/libzmq/src/libzmq-build"
  "/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/works/cunqa/build_ft3/src/third-party/libzmq"
  "/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/works/cunqa/build_ft3/src/third-party/libzmq/tmp"
  "/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/works/cunqa/build_ft3/src/third-party/libzmq/src/libzmq-stamp"
  "/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/works/cunqa/build_ft3/src/third-party/libzmq/src"
  "/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/works/cunqa/build_ft3/src/third-party/libzmq/src/libzmq-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/works/cunqa/build_ft3/src/third-party/libzmq/src/libzmq-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/mnt/netapp1/Store_CESGA/home/cesga/jvazquez/works/cunqa/build_ft3/src/third-party/libzmq/src/libzmq-stamp${cfgdir}") # cfgdir has leading slash
endif()
