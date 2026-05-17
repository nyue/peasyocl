# Copyright 2024 viktorlanner
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     https://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

if (WIN32)
      if (DEFINED ENV{HFS})
      	    set(OpenCL_INCLUDE_DIR "$ENV{HFS}/toolkit/include")
	    set(OpenCL_LIBRARY "$ENV{HFS}/custom/houdini/dsolib/OpenCL.lib")
	    include_directories("${OpenCL_INCLUDE_DIR}")
      else()
	message(ERROR "Must use OpenCL from Houdini")
      endif()
endif()

find_package(OpenCL REQUIRED)

set(OPENCL_CLHPP_HEADERS_DIR .)

set(SOURCES Context.cpp)
set(HEADERS Context.h KernelUtils.h opencl.hpp)

add_library(${OCLMODULE_NAME}
    SHARED
        ${HEADERS}
        ${SOURCES}
)

target_link_libraries(${OCLMODULE_NAME}
    PUBLIC
        ${OpenCL_LIBRARIES}
)

set_target_properties(${OCLMODULE_NAME} PROPERTIES PREFIX "")

target_compile_features(${OCLMODULE_NAME}
    PRIVATE
        cxx_auto_type
)

install(
    FILES
        ${HEADERS}
    DESTINATION
        include/peasyocl
)

install(
    TARGETS ${OCLMODULE_NAME}
    EXPORT ${OCLMODULE_NAME}Targets
    LIBRARY DESTINATION lib
    INCLUDES DESTINATION include/peasyocl
)
install(
    EXPORT ${OCLMODULE_NAME}Targets
    DESTINATION cmake
)
