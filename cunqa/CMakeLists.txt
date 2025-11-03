
pybind11_add_module(qclient bindings.cpp)

#find_library(GNUTLS_LIB gnutls REQUIRED PATHS /mnt/netapp1/Optcesga_FT2_RHEL7/2022/gentoo/31032022/usr/lib64/)
#find_library(SODIUM_LIB sodium REQUIRED PATHS /mnt/netapp1/Optcesga_FT2_RHEL7/2022/gentoo/31032022/usr/lib64/)
target_link_libraries(qclient PRIVATE client) #${GNUTLS_LIB} ${SODIUM_LIB} bsd)


set(_PY_SITE "${Python_SITEARCH}")
if(NOT _PY_SITE)
  set(_PY_SITE "${Python_SITELIB}")
endif()
if(NOT _PY_SITE)
  message(WARNING "Could not determine Python site-packages dir; installing under CMAKE_INSTALL_PREFIX/cunqa")
  set(_PY_SITE "${CMAKE_INSTALL_PREFIX}")
endif()

install(TARGETS qclient DESTINATION "${_PY_SITE}/cunqa")

file(GLOB PYTHON_SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/*.py")
install(FILES ${PYTHON_SCRIPTS} DESTINATION "${_PY_SITE}/cunqa")
