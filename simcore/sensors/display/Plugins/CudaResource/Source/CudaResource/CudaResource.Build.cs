// Copyright 2024 Tencent Inc. All rights reserved.
//

using System;
using System.IO;
using UnrealBuildTool;

// cuda resource module
public class CudaResource : ModuleRules
{
    // third party path
    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "../../ThirdParty/")); }
    }

    // cuda resource module target
    public CudaResource(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        // add module dependencies
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "RHI",
                "RenderCore",
                "Projects"
            }
        );

        // add module dependencies
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "RHI",
                "RenderCore",
                "BuildSettings",
                "VulkanRHI"
                // ... add private dependencies that you statically link with here ...    
            }
        );
        // get engine path
        string EnginePath = Path.GetFullPath(Target.RelativeEnginePath);
        //Include RHI Headers for texture access in GPU memory
        PrivateIncludePaths.AddRange(
            new string[]
            {
                // ... add other private include paths required here ...
                EnginePath + "Source/Runtime/VulkanRHI/Private/",
                EnginePath + "Source/ThirdParty/Vulkan/Include/vulkan"
            }
        );
        // Include RHI Headers for texture access in GPU memory
        PublicIncludePaths.AddRange(new string[] { EnginePath + "Source/Runtime/VulkanRHI/Public/" });

        // windows target
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            // d3d11 and d3d12
            PrivateDependencyModuleNames.AddRange(new string[] { "D3D11RHI", "D3D12RHI" });
            PublicDependencyModuleNames.AddRange(new string[] { "D3D11RHI", "D3D12RHI" });
            //DynamicallyLoadedModuleNames.AddRange(new string[] { "D3D11RHI" , "D3D12RHI" });
            //Include RHI Headers for texture access in GPU memory
            PrivateIncludePaths.AddRange(
                new string[]
                {
                    EnginePath + "Source/Runtime/Windows/D3D11RHI/Private/",
                    EnginePath + "Source/Runtime/Windows/D3D11RHI/Private/Windows/",
                    EnginePath + "Source/Runtime/D3D12RHI/Private/",
                    EnginePath + "Source/Runtime/D3D12RHI/Private/Windows/",
                    EnginePath + "Source/ThirdParty/Windows/DirectX/Include/",
                    EnginePath + "Source/ThirdParty/Windows/D3DX12/Include/",
                    EnginePath + "Source/Runtime/VulkanRHI/Private/Windows/",
                    Path.Combine(ThirdPartyPath, "TxExt", "include"),
                }
            );

            PublicIncludePaths.AddRange(
                new string[]
                {
                    EnginePath + "Source/Runtime/Windows/D3D11RHI/Public/",
                    EnginePath + "Source/Runtime/D3D12RHI/Public/"
                }
            );

            //check CUDA
            if (System.Environment.GetEnvironmentVariable("CUDA_PATH_11_4", EnvironmentVariableTarget.Machine) != null)
            {
                // CUDA 11.4
                string cuda_path =
                    System.Environment.GetEnvironmentVariable("CUDA_PATH_11_4", EnvironmentVariableTarget.Machine);
                if (Directory.Exists(cuda_path))
                {
                    // find cuda
                    PublicDefinitions.Add("FIND_CUDA");

                    // cuda include
                    string cuda_include = "include";
                    string cuda_lib = "lib/x64";

                    PublicIncludePaths.Add(Path.Combine(cuda_path, cuda_include));

                    //PublicAdditionalLibraries.Add(Path.Combine(cuda_path, cuda_lib, "cudart.lib"));
                    PublicAdditionalLibraries.AddRange(
                        new string[]
                        {
                            // cudart_static.lib
                            // cuda.lib
                            // nppial.lib
                            // nppif.lib
                            // nppist.lib
                            // nppidei.lib
                            // npps.lib
                            // nppisu.lib
                            // nppicc.lib
                            // nppig.lib
                            // nppc.lib
                            // nvjpeg.lib
                            // display_cuda_ext.lib
                            Path.Combine(cuda_path, cuda_lib, "cudart_static.lib"),
                            Path.Combine(cuda_path, cuda_lib, "cuda.lib"),
                            Path.Combine(cuda_path, cuda_lib, "nppial.lib"),
                            Path.Combine(cuda_path, cuda_lib, "nppif.lib"),
                            Path.Combine(cuda_path, cuda_lib, "nppist.lib"),
                            Path.Combine(cuda_path, cuda_lib, "nppidei.lib"),
                            Path.Combine(cuda_path, cuda_lib, "npps.lib"),
                            Path.Combine(cuda_path, cuda_lib, "nppisu.lib"),
                            Path.Combine(cuda_path, cuda_lib, "nppicc.lib"),
                            Path.Combine(cuda_path, cuda_lib, "nppig.lib"),
                            Path.Combine(cuda_path, cuda_lib, "nppc.lib"),
                            Path.Combine(cuda_path, cuda_lib, "nvjpeg.lib"),
                            Path.Combine(ThirdPartyPath, "TxExt", "lib/windows", "display_cuda_ext.lib")
                        }
                    );
                    //nppisu64_11.dll
                    RuntimeDependencies.Add("$(TargetOutputDir)/nppisu64_11.dll",
                        Path.Combine(cuda_path, "bin", "nppisu64_11.dll"));
                    // nppicc64_11.dll
                    RuntimeDependencies.Add("$(TargetOutputDir)/nppc64_11.dll",
                        Path.Combine(cuda_path, "bin", "nppc64_11.dll"));
                    // nvjpeg64_11.dll
                    RuntimeDependencies.Add("$(TargetOutputDir)/nvjpeg64_11.dll",
                        Path.Combine(cuda_path, "bin", "nvjpeg64_11.dll"));
                    // nppidei64_11.dll
                    RuntimeDependencies.Add("$(TargetOutputDir)/nppidei64_11.dll",
                        Path.Combine(cuda_path, "bin", "nppidei64_11.dll"));
                    // nppig64_11.dll
                    RuntimeDependencies.Add("$(TargetOutputDir)/nppig64_11.dll",
                        Path.Combine(cuda_path, "bin", "nppig64_11.dll"));
                    // cublas64_11.dll
                    RuntimeDependencies.Add("$(TargetOutputDir)/cublas64_11.dll",
                        Path.Combine(cuda_path, "bin", "cublas64_11.dll"));
                    // cudnn64_8.dll
                    RuntimeDependencies.Add("$(TargetOutputDir)/cudnn64_8.dll",
                        Path.Combine(cuda_path, "bin", "cudnn64_8.dll"));
                    // cublasLt64_11.dll
                    RuntimeDependencies.Add("$(TargetOutputDir)/cublasLt64_11.dll",
                        Path.Combine(cuda_path, "bin", "cublasLt64_11.dll"));
                    // nvrtc64_112_0.dll
                    RuntimeDependencies.Add("$(TargetOutputDir)/nvrtc64_112_0.dll",
                        Path.Combine(cuda_path, "bin", "nvrtc64_112_0.dll"));

                    // cudnn_ops_infer64_8
                    RuntimeDependencies.Add("$(TargetOutputDir)/cudnn_ops_infer64_8.dll",
                        Path.Combine(ThirdPartyPath, "TxExt", "lib/windows", "cudnn_ops_infer64_8.dll"));
                    // nvrtc-builtins64_115
                    RuntimeDependencies.Add("$(TargetOutputDir)/nvrtc-builtins64_115.dll",
                        Path.Combine(ThirdPartyPath, "TxExt", "lib/windows", "nvrtc-builtins64_115.dll"));
                    // cudnn_ops_train64_8.dll
                    RuntimeDependencies.Add("$(TargetOutputDir)/cudnn_ops_train64_8.dll",
                        Path.Combine(ThirdPartyPath, "TxExt", "lib/windows", "cudnn_ops_train64_8.dll"));
                    // nvinfer.dll
                    RuntimeDependencies.Add("$(TargetOutputDir)/nvinfer.dll",
                        Path.Combine(ThirdPartyPath, "TxExt", "lib/windows", "nvinfer.dll"));
                    // nvinfer_plugin.dll
                    RuntimeDependencies.Add("$(TargetOutputDir)/nvinfer_plugin.dll",
                        Path.Combine(ThirdPartyPath, "TxExt", "lib/windows", "nvinfer_plugin.dll"));
                    // nvonnxparser.dll
                    RuntimeDependencies.Add("$(TargetOutputDir)/nvonnxparser.dll",
                        Path.Combine(ThirdPartyPath, "TxExt", "lib/windows", "nvonnxparser.dll"));
                    // nvinfer_builder_resource.dll
                    RuntimeDependencies.Add("$(TargetOutputDir)/nvinfer_builder_resource.dll",
                        Path.Combine(ThirdPartyPath, "TxExt", "lib/windows", "nvinfer_builder_resource.dll"));
                    // display_cuda_ext.dll
                    RuntimeDependencies.Add("$(TargetOutputDir)/display_cuda_ext.dll",
                        Path.Combine(ThirdPartyPath, "TxExt", "lib/windows", "display_cuda_ext.dll"));
                    // zlibwapi.dll
                    RuntimeDependencies.Add("$(TargetOutputDir)/zlibwapi.dll",
                        Path.Combine(ThirdPartyPath, "TxExt", "lib/windows", "zlibwapi.dll"));
                }
            }
        }
        // Linux
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            //Include RHI Headers for texture access in GPU memory
            PrivateIncludePaths.AddRange(
                new string[]
                {
                    // ... add other private include paths required here ...
                    EnginePath + "Source/Runtime/VulkanRHI/Private/Linux/",
                    Path.Combine(ThirdPartyPath, "TxExt", "include"),
                }
            );

            // add module VulkanRHI
            PrivateDependencyModuleNames.AddRange(new string[] { "VulkanRHI" });

            //Include CUDA
            string cuda_path = "/usr/local/cuda";
            if (Directory.Exists(cuda_path))
            {
                // find cuda
                PublicDefinitions.Add("FIND_CUDA");

                // add cuda include and lib
                string cuda_include = "include";
                string cuda_lib = "lib64";
                string cuda_lib_stubs = "lib64/stubs";

                PublicIncludePaths.Add(Path.Combine(cuda_path, cuda_include));

                // libcudart_static.a
                PublicAdditionalLibraries.Add(Path.Combine(cuda_path, cuda_lib, "libcudart_static.a"));
                // libcuda.so
                PublicAdditionalLibraries.Add(Path.Combine(cuda_path, cuda_lib_stubs, "libcuda.so"));
                // libnppicc.so
                PublicAdditionalLibraries.Add(Path.Combine(cuda_path, cuda_lib, "libnppicc.so"));
                // libnppig.so
                PublicAdditionalLibraries.Add(Path.Combine(cuda_path, cuda_lib, "libnppig.so"));
                // libnppial.so
                PublicAdditionalLibraries.Add(Path.Combine(cuda_path, cuda_lib, "libnppial.so"));
                // libnppif.so
                PublicAdditionalLibraries.Add(Path.Combine(cuda_path, cuda_lib, "libnppif.so"));
                // libnppist.so
                PublicAdditionalLibraries.Add(Path.Combine(cuda_path, cuda_lib, "libnppist.so"));
                // libnppidei.so
                PublicAdditionalLibraries.Add(Path.Combine(cuda_path, cuda_lib, "libnppidei.so"));
                // libnpps.so
                PublicAdditionalLibraries.Add(Path.Combine(cuda_path, cuda_lib, "libnpps.so"));
                // libnppisu.so
                PublicAdditionalLibraries.Add(Path.Combine(cuda_path, cuda_lib, "libnppisu.so"));
                // libnppc.so
                PublicAdditionalLibraries.Add(Path.Combine(cuda_path, cuda_lib, "libnppc.so"));
                // libnvjpeg.so
                PublicAdditionalLibraries.Add(Path.Combine(cuda_path, cuda_lib, "libnvjpeg.so"));
                // libcublas.so
                PublicAdditionalLibraries.Add(Path.Combine(cuda_path, cuda_lib, "libcublas.so"));
                // libdisplay_cuda_ext.so
                PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "TxExt", "lib/ubuntu",
                    "libdisplay_cuda_ext.so"));


                // libcudart_static.a
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libcudart_static.a",
                    Path.Combine(cuda_path, cuda_lib, "libcudart_static.a"));
                // libcuda.so
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libcuda.so.1",
                   Path.Combine(cuda_path, cuda_lib_stubs, "libcuda.so"));
                // libnppicc.so
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libnppicc.so.11",
                    Path.Combine(cuda_path, cuda_lib, "libnppicc.so"));
                // libnppial.so
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libnppig.so.11",
                    Path.Combine(cuda_path, cuda_lib, "libnppig.so"));
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libnppial.so.11",
                    Path.Combine(cuda_path, cuda_lib, "libnppial.so"));
                // libnppial.so
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libnppif.so.11",
                    Path.Combine(cuda_path, cuda_lib, "libnppif.so"));
                // libnppist.so
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libnppist.so.11",
                    Path.Combine(cuda_path, cuda_lib, "libnppist.so"));
                // libnppidei.so
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libnppidei.so.11",
                    Path.Combine(cuda_path, cuda_lib, "libnppidei.so"));
                // libnpps.so
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libnpps.so.11",
                    Path.Combine(cuda_path, cuda_lib, "libnpps.so"));
                // libnppisu.so
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libnppisu.so.11",
                    Path.Combine(cuda_path, cuda_lib, "libnppisu.so"));
                // libnppc.so
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libnppc.so.11",
                    Path.Combine(cuda_path, cuda_lib, "libnppc.so"));
                // libnvjpeg.so
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libnvjpeg.so.11",
                    Path.Combine(cuda_path, cuda_lib, "libnvjpeg.so"));
                // libcublas.so
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libcublas.so.11",
                    Path.Combine(cuda_path, cuda_lib, "libcublas.so"));
                // libcublasLt.so
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libcublasLt.so.11",
                    Path.Combine(cuda_path, cuda_lib, "libcublasLt.so"));

                // libcudnn_ops_infer.so
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libcudnn_ops_infer.so.8",
                    Path.Combine(ThirdPartyPath, "TxExt", "lib/ubuntu", "libcudnn_ops_infer.so.8"));
                // libcudnn_cnn_infer.so
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libcudnn_cnn_infer.so.8",
                    Path.Combine(ThirdPartyPath, "TxExt", "lib/ubuntu", "libcudnn_cnn_infer.so.8"));
                // libcudnn_ops_train.so
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libcudnn_ops_train.so.8",
                    Path.Combine(ThirdPartyPath, "TxExt", "lib/ubuntu", "libcudnn_ops_train.so.8"));
                // libdisplay_cuda_ext
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libdisplay_cuda_ext.so",
                    Path.Combine(ThirdPartyPath, "TxExt", "lib/ubuntu", "libdisplay_cuda_ext.so"));
                // libcudnn.so
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libcudnn.so.8",
                    Path.Combine(ThirdPartyPath, "TxExt", "lib/ubuntu", "libcudnn.so.8"));
                // libnvinfer.so
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libnvinfer.so.8",
                    Path.Combine(ThirdPartyPath, "TxExt", "lib/ubuntu", "libnvinfer.so.8"));
                // libnvinfer_builder_resource.so
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libnvinfer_builder_resource.so.8.2.1",
                    Path.Combine(ThirdPartyPath, "TxExt", "lib/ubuntu", "libnvinfer_builder_resource.so.8.2.1"));
                // libnvinfer_plugin.so
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libnvinfer_plugin.so.8",
                    Path.Combine(ThirdPartyPath, "TxExt", "lib/ubuntu", "libnvinfer_plugin.so.8"));
                // libnvonnxparser.so
                RuntimeDependencies.Add("$(TargetOutputDir)/ubuntu18_20/libnvonnxparser.so.8",
                    Path.Combine(ThirdPartyPath, "TxExt", "lib/ubuntu", "libnvonnxparser.so.8"));
            }
        }
    }
}