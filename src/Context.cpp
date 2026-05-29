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

#include "Context.h"

#include "KernelUtils.h"

namespace peasyocl {

int Context::Init() {
    if (initialized) {
        return 0;
    }

    cl_int err;
    _device = cl::Device::getDefault(&err);
    if (err != CL_SUCCESS) {
        printf("Error: Failed to create a device group! %i \n", err);
        return 1;
    }

    _context = cl::Context(_device, nullptr, nullptr, nullptr, &err);
    if (err != CL_SUCCESS) {
        printf("Error: Failed to create a device group! %i \n", err);
        return 1;
    }

    _queue = cl::CommandQueue(_context, _device, 0, &err);
    if (err != CL_SUCCESS) {
        printf("Error: Failed to create a command commands! %i \n", err);
        return 1;
    }
    initialized = true;

    return 0;
}

void Context::AddBuffer(const std::string &name, SharedBuffer buffer,
                        const size_t &size) {
    _buffers.insert({name, {std::move(buffer), size}});
}

SharedBuffer Context::GetBuffer(const std::string &name) {
    if (_buffers.count(name) == 0) {
        return nullptr;
    }
    return _buffers.at(name).first;
}

const size_t Context::GetBufferSize(const std::string &name) {
    if (_buffers.count(name) == 0) {
        return {};
    }
    return _buffers.at(name).second;
}

KernelHandle *Context::AddKernel(const std::string &code,
                                 const std::vector<std::string> &includes,
                                 const std::string &kernelName,
                                 const std::string &key) {

    KernelHandle handle;

    if (!key.empty()) {
        handle.key = key;
    } else {
        handle.key = kernelName;
    }

    if (auto foundKernel = _kernels.find(handle.key);
        foundKernel != _kernels.end()) {
        if (foundKernel->second.built) {
            return &foundKernel->second;
        }
    }

    int err;
    handle.program = cl::Program(_context, code.c_str(), false, &err);

    if (err != CL_SUCCESS) {
        handle.built = false;
        printf("Error: Failed to create compute program! %i \n", err);
        return nullptr;
    }

    std::string flags = "-cl-std=CL2.0 ";
    for (utils::ClFile clFile : utils::ClFile::GetKernelPaths()) {
        if (!clFile.empty()) {
            flags.append(std::string("-I ").append(clFile.path));
        }
    }
    for (std::string path : includes) {
        if (!path.empty()) {
            flags.append(std::string("-I ").append(path));
        }
    }

    err = handle.program.build(_device, flags.c_str(), nullptr);

    if (err != CL_SUCCESS) {
        handle.built = false;
        printf("Error: Failed to build program %s: %i\n", kernelName.c_str(),
               err);
        std::string buildLog =
            handle.program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(_device);
        if (!buildLog.empty()) {
            printf("--- OpenCL build log for %s ---\n%s\n--- end build log ---\n",
                   kernelName.c_str(), buildLog.c_str());
        }
        return nullptr;
    }

    handle.kernel = cl::Kernel(handle.program, kernelName.c_str(), &err);

    if (err != CL_SUCCESS) {
        handle.built = false;
        printf("Error: Failed to create compute kernel with name %s\n",
               kernelName.c_str());
        return nullptr;
    }

    handle.built = true;
    handle.context = &_context;
    handle.queue = &_queue;
    _kernels[handle.key] = handle;
    return &_kernels[handle.key];
}

void Context::RemoveKernel(KernelHandle *kernel) {
    KernelMap::iterator it = _kernels.find(kernel->key);
    if (it == _kernels.end()) {
        return;
    }
    long idx = std::distance(_kernels.begin(), it);

    _kernels.erase(it);
    kernel->built = false;
}

KernelHandle *Context::GetKernelHandle(const std::string &name) {
    if (_kernels.find(name) == _kernels.end()) {
        return nullptr;
    }
    return &_kernels[name];
}

int Context::Execute(const std::string &kernelName, const cl::NDRange &global,
                     const cl::NDRange &local) {
    KernelHandle *kernel = GetKernelHandle(kernelName);
    if (!kernel) {
        return 1;
    }
    return Execute(&_kernels[kernelName], global, local);
}

int Context::Execute(KernelHandle *kernelHandle, const cl::NDRange &global,
                     const cl::NDRange &local) {
    if (!initialized) {
        return 1;
    }
    if (!kernelHandle->built) {
        return 1;
    }

    cl_int err;
    size_t l;

    cl::Event ev;
    err = _queue.enqueueNDRangeKernel(kernelHandle->kernel, cl::NullRange,
                                      global, local, NULL, &ev);

    ev.wait();

    if (err != CL_SUCCESS) {
        printf("Error: Failed to execute kernel %s!\n",
               kernelHandle->key.c_str());
        return 1;
    }

    kernelHandle->dirty = false;
    return 0;
}

int Context::ExecuteGraph(const std::string &kernelName,
                          const std::vector<int> sizes,
                          const std::string &wg_argument) {
    KernelHandle *kernel = GetKernelHandle(kernelName);
    if (!kernel) {
        return 1;
    }
    return ExecuteGraph(&_kernels[kernelName], sizes, wg_argument);
}

int Context::ExecuteGraph(KernelHandle *kernelHandle,
                          const std::vector<int> sizes,
                          const std::string &wg_argument) {
    if (!initialized) {
        return 1;
    }
    if (!kernelHandle->built) {
        return 1;
    }

    cl_int err;
    size_t l;

    cl::Event ev;
    int offset = 0;
    for (int i = 0; i < sizes.size(); i++) {
        if (!wg_argument.empty()) {
            kernelHandle->SetArgument(wg_argument, offset + sizes[i]);
        }
        err = _queue.enqueueNDRangeKernel(kernelHandle->kernel, offset,
                                          sizes[i], cl::NullRange, NULL, &ev);
        offset += sizes[i];
    }

    ev.wait();

    if (err != CL_SUCCESS) {
        printf("Error: Failed to execute kernel %s!\n",
               kernelHandle->key.c_str());
        return 1;
    }

    kernelHandle->dirty = false;
    return 0;
}

void Context::Finish() {
    _queue.finish();
    _queue.flush();
}

cl::string Context::_LoadShader(const std::string_view &fileName, int *err) {
    utils::ClFile kernelFile = utils::ClFile::GetClFileByName(fileName.data());
    if (kernelFile.empty()) {
        *err = 1;
        return "";
    }

    std::string kernelSource = kernelFile.LoadClKernelSource();
    if (kernelSource.empty()) {
        *err = 1;
        return "";
    }
    *err = 0;

    return kernelSource;
}

} // namespace peasyocl