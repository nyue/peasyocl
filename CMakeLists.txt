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

cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(peasyocl VERSION 0.1.0 LANGUAGES CXX)

# Set the C++ requirements
set(CMAKE_CXX_STANDARD 17) # c++ 17 used for some convenience functions
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Target name
set(OCLMODULE_NAME peasyocl)

# Some RPATH stuff
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)

add_subdirectory(src)

include(CMakePackageConfigHelpers)

configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/${OCLMODULE_NAME}Config.cmake"
  INSTALL_DESTINATION cmake
)

install(FILES 
    "${CMAKE_CURRENT_BINARY_DIR}/${OCLMODULE_NAME}Config.cmake"
    DESTINATION cmake
)