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

#ifndef OCL_DEFORMER_CONTEXT_H
#define OCL_DEFORMER_CONTEXT_H

#define CL_HPP_TARGET_OPENCL_VERSION 120
#define CL_HPP_MINIMUM_OPENCL_VERSION 120

#include "KernelUtils.h"
#include "opencl.hpp"
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

namespace peasyocl {

/**
 * @brief A handle to set arguments in a kernel source
 *
 */

struct KernelHandle;

using SharedBuffer = std::shared_ptr<cl::Buffer>;
using KernelMap = std::map<cl::string, KernelHandle>;
using ArgumentMap = std::unordered_map<std::string, int>;
using BufferMap =
    std::unordered_map<std::string, std::pair<SharedBuffer, size_t>>;

struct KernelHandle {
    cl::Kernel kernel;
    ArgumentMap arguments;
    std::string key;
    cl::CommandQueue *queue;
    cl::Context *context;
    cl::Program program;
    std::string code;

    bool usesSharedMamory = false;

    bool built = false;
    bool dirty = true;

    int argCount = 0;

    const std::string GetArgumentName(const std::string &name) const;

    void ToggleSharedMemory(const bool val) { usesSharedMamory = val; }
    const bool UsesSharedMemory() const { return usesSharedMamory; }

    /**
     * @brief Add argument to kernel and create buffer
     *
     * @tparam T
     * @param flags Flags passed to the cl::Buffer object
     * @param name Name associated with this argument
     * @param size Size of elements
     * @param data Data of type T
     * @return int Success
     */
    template <typename T>
    int AddArgument(cl_mem_flags flags, const std::string &name,
                    const size_t &size, T *data);

    /**
     * @brief Add argument to kernel and create buffer
     *
     * @tparam T
     * @param flags Flags passed to the cl::Buffer object
     * @param name Name associated with this argument
     * @param size Size of elements
     * @param data Data of type const T&
     * @return int Success
     */
    template <typename T>
    int AddArgument(cl_mem_flags flags, const std::string &name,
                    const size_t &size, const T &data);

    /**
     * @brief Add argument to kernel with option to create buffer. Allows one to
     * create an empty buffer or argument with no buffer attached
     *
     * @tparam T Bool
     * @param flags Flags passed to the cl::Buffer object
     * @param name Name associated with this argument
     * @param size Size of elements
     * @param createBuffer Whether to create an empty buffer. Defaults to true
     * @return int Success
     */
    template <typename T>
    int AddArgument(cl_mem_flags flags, const std::string &name,
                    const size_t &size, const bool createBuffer = true);

    /**
     * @brief Set the Argument at index argIndex to data
     *
     * @tparam mem
     * @tparam T
     * @param argIndex Int in to the arguments map, i.e what argument to set
     * @param data Of type T* to set
     * @return int
     */
    template <typename mem, typename T>
    int SetArgument(const int argIndex, T *data);

    /**
     * @brief Set the Argument at index argIndex to data
     *
     * @tparam mem
     * @tparam T
     * @param argIndex Int in to the arguments map, i.e what argument to set
     * @param data Of type const T& to set
     * @return int
     */
    template <typename T> int SetArgument(const int argIndex, const T &data);

    /**
     * @brief Set the Argument with name to data
     *
     * @tparam mem
     * @tparam T
     * @param name The name of the argument to set
     * @param data Of type T* to set
     * @return int
     */
    template <typename mem, typename T>
    int SetArgument(const std::string &name, T *data);

    /**
     * @brief Set the Argument with name to data
     *
     * @tparam mem
     * @tparam T
     * @param name The name of the argument to set
     * @param data Of type const T& to set
     * @return int
     */
    template <typename T>
    int SetArgument(const std::string &name, const T &data);

    /**
     * @brief Read data from buffer
     *
     * @tparam T
     * @param data Result data to write to
     * @param buffer Bufer object to read from
     * @param size Size of elements to read
     * @return int
     */
    template <typename T>
    int ReadBufferData(T *data, SharedBuffer buffer, const size_t size);

    /**
     * @brief Set the data of buffer
     *
     * @tparam T
     * @param data Of type T*
     * @param buffer Buffer object to set
     * @param size Size of elements to read
     * @return int
     */
    template <typename T>
    int SetBufferData(T *data, SharedBuffer buffer, const size_t size);

    /**
     * @brief Read the data of buffer with name
     *
     * @tparam T
     * @param data
     * @param name Buffer to set. Name is associated with the name created
     * with AddArgument function
     * @return int
     */
    template <typename T> int ReadBufferData(T *data, const std::string &name);
    template <typename T>
    int ReadBufferData(T *data, const std::string &name, const size_t &size);

    /**
     * @brief Set the data of buffer with name
     *
     * @tparam T
     * @param data Of type T*
     * @param name Buffer to set. Name is associated with the name created
     * with AddArgument function
     * @return int
     */
    template <typename T> int SetBufferData(T *data, const std::string &name);
    template <typename T>
    int SetBufferData(T *data, const std::string &name, const size_t &size);
};

/**
 * @brief Singleton class to handle the opencl context
 *
 */
class Context {
  public:
    int Init();

    /**
     * @brief Get the Instance of the DeformerContext singleton
     *
     * @return DeformerContext*
     */
    static Context *GetInstance() {
        static Context *ctx = new Context();
        return ctx;
    }

    /**
     * @brief Check if the context is properly initialized
     *
     * @return true
     * @return false
     */
    bool IsValid() const { return initialized; }

    /**
     * @brief Enable or disable shared memory pools. Should be set before
     * creating kernels
     *
     * @param val Value to set
     */
    // void ToggleSharedMemory(const bool &val) { _shared_memory = val; }

    /**
     * @brief Return whether this context has enabled shared memory between
     * kernels
     *
     * @return true
     * @return false
     */
    // const bool UsesSharedMamory() const { return _shared_memory; }

    /**
     * @brief Add a .cl or .ocl file for executing
     *
     * @param fileName Name of opencl kernel file. Should be visible in
     * OCL_KERNEL_PATHS environment
     * @return int
     */
    // int AddSource(const utils::ClFile &clfile);
    // int AddSource(const std::string_view &fileName);

    /**
     * @brief Load a kernel from the sources added. Returns a KernelHandle which
     * is used to access kernel specific arguments.
     *
     * @param kernelName Looks for kernels inside any added sources
     * @param key Override internal key name for this kernel for storage inside
     * the _kernels map
     * @param err
     * @return KernelHandle*
     */
    KernelHandle *AddKernel(const std::string &code,
                            const std::vector<std::string> &includes,
                            const std::string &kernelName,
                            const std::string &key = "");

    void RemoveKernel(KernelHandle *kernel);
    // void RemoveKernel(const KernelHandle& kernel);

    /**
     * @brief Get a pointer to the KernelHandle with name
     *
     * @param name
     * @return KernelHandle*
     */
    KernelHandle *GetKernelHandle(const std::string &name);

    // int Build(const std::vector<std::string> &extraIncludes =
    //                        std::vector<std::string>());

    /**
     * @brief Check whether the specific kernel exists
     *
     * @param name Name of kernel
     * @return true If kernel exists
     * @return false If kernel does not exist
     */
    bool HasKernel(const std::string &name) {
        return _kernels.find(name) != _kernels.end();
    }

    /**
     * @brief Add a buffer to be stored on the Context. The buffer
     * will then be available for all clients to access. If the Context has
     * enabled shared memory the buffer will be stored with the hashed name
     * argument, while if the context does not enable shared memory, the hash
     * will be constructed using the key + name
     *
     * @param name the name of the buffer object to create
     * @param buffer SharedBuffer object pointing to a cl::Buffer
     * @param size the size of the buffer
     * @param key key to use for unshared memory access
     */

    void AddBuffer(const std::string &name, SharedBuffer buffer,
                   const size_t &size);

    /**
     * @brief Get the buffer object stored on the context at name, or at
     * name+key if shared memory is enabled
     *
     * @param name The name of the buffer object
     * @param key key to use for unshared memory access
     * @return SharedBuffer
     */
    SharedBuffer GetBuffer(const std::string &name);

    /**
     * @brief Get the memory size of a buffer bject
     *
     * @param name name of buffer
     * @param key key to use for unshared memory access
     * @return const size_t
     */
    const size_t GetBufferSize(const std::string &name);

    /**
     * @brief Execute a kernel with name kernelName
     *
     * @param global Global workgroup size
     * @param kernelName Name of kernel to execute
     * @return int
     */
    int Execute(const std::string &kernelName, const cl::NDRange &global,
                const cl::NDRange &local = cl::NullRange);
    int Execute(KernelHandle *kernelHandle, const cl::NDRange &global,
                const cl::NDRange &local = cl::NullRange);

    int ExecuteGraph(const std::string &kernelName, std::vector<int> sizes,
                     const std::string &wg_argument = "");
    int ExecuteGraph(KernelHandle *kernelHandle, std::vector<int> sizes,
                     const std::string &wg_argument = "");

    /**
     * @brief Flush and finish the queue
     *
     */
    void Finish();

  protected:
    std::map<std::string, bool> built;
    bool initialized = false;

  private:
    Context() = default;
    Context(const Context &) = delete;
    Context(Context &&) = delete;
    Context &operator=(const Context &) = delete;
    Context &operator=(Context &&) = delete;

    cl::string _LoadShader(const std::string_view &fileName, int *err);

    // bool _shared_memory = true;

    cl::Context _context;
    cl::CommandQueue _queue;
    cl::Program _program;
    BufferMap _buffers;
    ArgumentMap _arguments;
    KernelMap _kernels;
    cl::Device _device;
    int _buffer_count;
};

inline const std::string
KernelHandle::GetArgumentName(const std::string &name) const {
    // const bool use_shared = Context::GetInstance()->UsesSharedMamory();
    if (!UsesSharedMemory()) {
        return name + key;
    }
    return name;
}

template <typename T>
inline int KernelHandle::AddArgument(cl_mem_flags flags,
                                     const std::string &name,
                                     const size_t &size, T *data) {
    dirty = true;

    std::string argName = GetArgumentName(name);

    if (auto buff = Context::GetInstance()->GetBuffer(argName);
        buff == nullptr) {
        SharedBuffer d_data =
            std::make_shared<cl::Buffer>(*context, flags, size);
        Context::GetInstance()->AddBuffer(argName, d_data, size);
    }
    if (data != nullptr) {
        SetBufferData(data, Context::GetInstance()->GetBuffer(argName), size);
    }

    SetArgument<cl_mem, cl::Buffer>(
        argCount, Context::GetInstance()->GetBuffer(argName).get());
    arguments.insert({argName, argCount});
    argCount++;
    return 0;
}

template <typename T>
inline int KernelHandle::AddArgument(cl_mem_flags flags,
                                     const std::string &name,
                                     const size_t &size, const T &data) {

    return AddArgument(flags, name, size, &data);
}

template <typename T>
inline int
KernelHandle::AddArgument(cl_mem_flags flags, const std::string &name,
                          const size_t &size, const bool createBuffer) {
    std::string argName = GetArgumentName(name);
    if (createBuffer) {
        return AddArgument<T>(flags, argName, size, nullptr);
    }
    SetArgument<T>(argCount, (T *)nullptr);

    arguments.insert({argName, argCount});
    argCount++;
    return 0;
}

template <typename mem, typename T>
inline int KernelHandle::SetArgument(const int argIndex, T *data) {
    dirty = true;
    kernel.setArg(argIndex, sizeof(mem), data);
    return 0;
}

template <typename T>
inline int KernelHandle::SetArgument(const int argIndex, const T &data) {
    dirty = true;
    kernel.setArg<T>(argIndex, data);
    return 0;
}

template <typename mem, typename T>
inline int KernelHandle::SetArgument(const std::string &name, T *data) {
    SetArgument(arguments[GetArgumentName(name)], data);
    return 0;
}

template <typename T>
inline int KernelHandle::SetArgument(const std::string &name, const T &data) {
    SetArgument(arguments[GetArgumentName(name)], data);
    return 0;
}

template <typename T>
inline int KernelHandle::SetBufferData(T *data, SharedBuffer buffer,
                                       const size_t size) {
    cl_int err = queue->enqueueWriteBuffer(*buffer, CL_TRUE, 0, size, data);
    if (err != CL_SUCCESS) {
        printf("%i \n", err);
        printf("Error: Failed to write data to source array!\n");
        return 1;
    }
    dirty = true;
    return 0;
}

template <typename T>
inline int KernelHandle::SetBufferData(T *data, const std::string &name) {
    std::string argName = GetArgumentName(name);
    if (arguments.find(argName) == arguments.end()) {
        printf("Error: Buffer %s is not recognized!\n", name.c_str());
        return 1;
    }
    return SetBufferData(data, Context::GetInstance()->GetBuffer(argName),
                         Context::GetInstance()->GetBufferSize(argName));
}

template <typename T>
inline int KernelHandle::SetBufferData(T *data, const std::string &name,
                                       const size_t &size) {
    std::string argName = GetArgumentName(name);
    if (arguments.find(argName) == arguments.end()) {
        printf("Error: Buffer %s is not recognized!\n", name.c_str());
        return 1;
    }

    dirty = true;
    return SetBufferData(data, Context::GetInstance()->GetBuffer(argName),
                         size);
}

template <typename T>
inline int KernelHandle::ReadBufferData(T *data, SharedBuffer buffer,
                                        const size_t size) {
    cl_int err = queue->enqueueReadBuffer(*buffer, CL_TRUE, 0, size, data);
    if (err != CL_SUCCESS) {
        printf("Error: Failed to read output array! %d\n", err);
        return 1;
    }
    return 0;
}

template <typename T>
inline int KernelHandle::ReadBufferData(T *data, const std::string &name) {
    std::string argName = GetArgumentName(name);
    return ReadBufferData(data, Context::GetInstance()->GetBuffer(argName),
                          Context::GetInstance()->GetBufferSize(argName));
}

template <typename T>
inline int KernelHandle::ReadBufferData(T *data, const std::string &name,
                                        const size_t &size) {
    // return ReadBufferData(data, buffers[name].first, size);
    return ReadBufferData(
        data, Context::GetInstance()->GetBuffer(GetArgumentName(name)), size);
}

} // namespace peasyocl

#endif
