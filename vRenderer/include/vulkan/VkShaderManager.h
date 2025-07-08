#pragma once

#include <string>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <filesystem> 
#include <fstream>
#include <stdexcept>  
#include <cstdlib>  
#include <iostream>
#include <algorithm>

#include "Singleton.h"
#include "VulkanUtils.h"

using namespace VkUtils;
namespace fs = std::filesystem;

class VkShaderManager : public Singleton<VkShaderManager>
{
    friend class Singleton<VkShaderManager>;

public:

    enum class RenderPass
    {
        FIRST,
        SECOND
    };

    enum class Shader
    {
        DEFAULT_FIRST_PASS,
        DEFAULT_SECOND_PASS
    };

    std::array<VkPipelineShaderStageCreateInfo, 2> getShaderStage(RenderPass pass, std::string* str = nullptr);
    void cleanup();

protected:

    // Constructor
    // shader_dir_root: Path to the main directory containing "first_pass" and "second_pass" subfolders.
    // compile_script_path: Path to the batch script for compiling shaders.
    VkShaderManager(VkContext context);

    ~VkShaderManager();

private:

    const fs::path compile_script_path_ = "vRenderer\\scripts\\shader_compiler.bat";
    const fs::path shader_src_dir = "vRenderer\\shaders\\vulkan\\src";
    const fs::path shader_bin_dir = "vRenderer\\shaders\\vulkan\\bin";
    const fs::path shader_include_dir = shader_src_dir / "include";
    const fs::path first_pass_dir = shader_src_dir / "first_pass";
    const fs::path second_pass_dir = shader_src_dir / "second_pass";

    // Represents the set of files for a single shader module
    struct ShaderModulePaths
    {
        RenderPass pass;
        std::string name;
        fs::path vertSrcPath;
        fs::path fragSrcPath;
        fs::path vertSpvPath; // Expected path for compiled SPIR-V vertex shader
        fs::path fragSpvPath; // Expected path for compiled SPIR-V fragment shader
        std::set<fs::path> dependencies;
    };    

    std::vector<ShaderModulePaths> shaderModules;
    std::vector<VkShaderModule> vkShaderModules;

    void validate();
    std::vector<ShaderModulePaths> collectShaderModules();
    void verifyCompleteModules(const std::vector<ShaderModulePaths>& modules);

    std::vector<std::string> getShadersForCompilation(std::vector<ShaderModulePaths>& modules);
    bool areBinariesUpToDate(const ShaderModulePaths& module) const;
    void compile(const std::vector<std::string>& shaderSources);

    void getShaderDependencies(fs::path shader, std::set<fs::path>& dependencies);

    VkContext context;
};