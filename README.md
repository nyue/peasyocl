
# peasyocl
peasyocl is a very easy, plug-and-play library for opencl. It allows for very basic setup of devices and contexts, adding of source files from a single environment variable, and simple argument handling for kernels.

## Requirements
- opencl 1.2

The opencl.hpp header is included.

## Build

The following packages was also required for a successful build

### Ubuntu
```sh
apt install ocl-icd-opencl-dev opencl-headers
```

### RH/Rocky/Alma
```
dnf install ocl-icd-devel.x86_64
```

## Install
Simply build and install using cmake. The package will be accessible with 
`find_package(peasyocl)` for your integration. Use with `peasyocl_INCLUDE_DIR` for headers and simply `peasyocl` for the lib.

## Usage
### Create the context
```
peasyocl::Context* oclContext = peasyocl::Context::getInstance();
oclContext->Init();
oclContext->AddSource("kernelSource.cl");
oclContext->Build();

peasyocl::KernelHandle* kernel = oclContext->AddKernel("kernelName", "thisKernel");
```

### Add Arguments
```
err = kernel->AddArgument<int>(CL_MEM_READ_ONLY, "vectorArg", vector.size(), vector.data());
err |= kernel->AddArgument<int>(CL_MEM_READ_ONLY, "intArg", 1, false);
err |= kernel->AddArgument<float>(CL_MEM_WRITE_ONLY, "result", globalSize);
```

Note that even if the arguments and buffers are associated with a name, we still need to create them in the correct order since they are indexed in the background. 

### Setting Data
This allows to set data of the buffers or arguments at runtime
```
kernel->SetBufferData<float>(vector.data(), "vectorArg", vector.size());
kernel->SetArgument<int>("intArg", 1);
```

### Execute and Read
```
err = context->Execute(globalSize, "thisKernel");
if (err != CL_SUCCESS) {
    std::cout<<"Failed: Could not invoke OCL" << std::endl;
    return;
}

std::vector<float> result;
result.resize(globalSize, 0);
tgtHandle->ReadBufferData(result.data(), "result", globalSize);

oclContext->Finish();
```