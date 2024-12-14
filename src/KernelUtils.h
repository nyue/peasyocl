// Copyright 2024 viktorlanner
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef OCL_KERNEL_UTILS
#define OCL_KERNEL_UTILS

#define OCL_KERNEL_PATHS_ENVIRONMENT "OCL_KERNEL_PATHS"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace peasyocl::utils {

struct ClFile {
    std::string path = "";

    bool empty() { return path.empty(); }
    /**
     * @brief Load data of file to an std::string
     *
     * @param path
     * @return std::string
     */
    inline std::string LoadClKernelSource() const {
        std::string content;
        std::ifstream filestream(path);

        if (!filestream.is_open()) {
            std::cout << "Error: Failed to load file: " << path << std::endl;
        }
        if (filestream.peek() == std::ifstream::traits_type::eof()) {
            return "";
        }
        std::string line = "";
        while (!filestream.eof()) {
            std::getline(filestream, line);
            content.append(line + "\n");
        }
        return content;
    }

    /**
     * @brief Find all paths in OCL_KERNEL_PATHS environment variable
     *
     * @return const std::vector<std::string>
     */
    inline static const std::vector<ClFile> GetKernelPaths() {
        std::vector<ClFile> result;
        const char *env;
        env = std::getenv(OCL_KERNEL_PATHS_ENVIRONMENT);

        if (!env) {
            return result;
        }

        std::string stringEnv(env);
        size_t prev = 0;
        size_t found = stringEnv.find(":");
        if (found == std::string::npos) {
            result.push_back(ClFile{stringEnv});
            return result;
        }

        while (found != std::string::npos) {
            std::string substr = stringEnv.substr(prev, found);
            result.push_back(ClFile{substr});
            prev = found;
            found = stringEnv.find(":", found + 1);
        }
        return result;
    }

    /**
     * @brief Get the Cl File By Name object
     *
     * @param name Name of cl file. The file should be visible in the
     * OCL_KERNEL_PATHS environment variable
     * @return const std::string
     */
    static const ClFile GetClFileByName(const std::string &name, const std::string &dir="") {
        std::vector<ClFile> paths = GetKernelPaths();
        if (!dir.empty()) {
            paths.push_back(ClFile{dir});
        }
        if (paths.size() == 0) {
            std::cout << "Failed: Could not find kernel paths" << std::endl;
            return ClFile();
        }

        for (auto &it : paths) {
            std::string path = it.path + "/" + name;
            if (name.rfind(".cl") == std::string::npos) {
                std::ifstream file(path + ".cl");
                if (file.good()) {
                    return ClFile{path};
                }
            }
            if (name.rfind(".ocl") == std::string::npos) {
                std::ifstream file(path + ".ocl");
                if (file.good()) {
                    return ClFile{path};
                }
            }
            std::ifstream file(path);
            if (file.good()) {
                return ClFile{path};
            }
        }
        return ClFile();
    }
};

} // namespace peasyocl::utils

#endif