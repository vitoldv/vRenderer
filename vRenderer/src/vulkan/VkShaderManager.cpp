#include "VkShaderManager.h"

VkShaderManager::VkShaderManager(VkContext context)
{
    this->context = context;

    validate();
    shaderModules = collectShaderModules();
    verifyCompleteModules(shaderModules);

    auto shaders = getShadersForCompilation(shaderModules);
    if (!shaders.empty())
    {
        compile(shaders);
    }
}

/// <summary>
/// Returns VkPipelineShaderStageCreateInfo ready to be used in pipeline creation by provided render pass.
/// P.S. It a dummy implementatin that will be revisioned in near future.
/// TODO: Think of smart shader selection either by name from material metadata or map.
/// </summary>
/// <param name="pass"></param>
/// <returns></returns>
std::array<VkPipelineShaderStageCreateInfo, 2> VkShaderManager::getShaderStage(RenderPass pass, std::string* str)
{
    ShaderModulePaths* shaderModule;
    if (str == nullptr)
    {
        auto it = std::find_if(shaderModules.begin(), shaderModules.end(), [pass](ShaderModulePaths m) {return m.pass == pass;});
        if (it == shaderModules.end())
        {
            throw std::runtime_error("There is no any loaded shader modules for required render pass.");
        }
        shaderModule = &(*it);
    }
    else
    {
        auto it = std::find_if(shaderModules.begin(), shaderModules.end(), [pass, str](ShaderModulePaths m) {                    
            return m.pass == pass && m.name.find(*str) != std::string::npos;
        });
        if (it == shaderModules.end())
        {
            throw std::runtime_error("There is no any loaded shader modules for required render pass.");
        }
        shaderModule = &(*it);
    }

    // build shader modules to link to graphics pipeline
    VkShaderModule vertexShaderModule = VkUtils::createShaderModule(VkUtils::readFile(shaderModule->vertSpvPath.string()), context);
    VkShaderModule fragmentShaderModule = VkUtils::createShaderModule(VkUtils::readFile(shaderModule->fragSpvPath.string()), context);
    vkShaderModules.push_back(vertexShaderModule);
    vkShaderModules.push_back(fragmentShaderModule);

    // VERTEX STAGE CREATION
    VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo = {};
    vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;			// shader stage name
    vertexShaderStageCreateInfo.module = vertexShaderModule;				// shader module to be used
    vertexShaderStageCreateInfo.pName = "main";								// shader enter function

    // FRAGMENT STAGE CREATION
    VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo = {};
    fragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;		// shader stage name
    fragmentShaderStageCreateInfo.module = fragmentShaderModule;				// shader module to be used
    fragmentShaderStageCreateInfo.pName = "main";								// shader enter function
    
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages = { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };

    return shaderStages;
}

void VkShaderManager::cleanup()
{
    for (int i = 0; i < vkShaderModules.size(); i++)
    {
        vkDestroyShaderModule(context.logicalDevice, vkShaderModules[i], nullptr);
    }
}

VkShaderManager::~VkShaderManager()
{
}

/// <summary>
/// Validates const paths to shader folders and shader compilation script
/// </summary>
void VkShaderManager::validate()
{
    // Validate root shader directory and compilation script paths
    if (!fs::exists(shader_dir_root_) || !fs::is_directory(shader_dir_root_))
    {
        throw std::runtime_error("Shader directory root does not exist or is not a directory: " + shader_dir_root_.string());
    }

    if (!fs::exists(compile_script_path_))
    {
        // Basic check for existence. Further checks (e.g., executability) are platform-dependent
        // and system() call will handle some of it.
        throw std::runtime_error("Shader compilation script does not exist: " + compile_script_path_.string());
    }

    // Ensure subfolders exist
    if (!fs::exists(first_pass_dir) || !fs::is_directory(first_pass_dir))
    {
        throw std::runtime_error("Required subfolder 'first_pass' does not exist or is not a directory within: " + shader_dir_root_.string());
    }
    if (!fs::exists(second_pass_dir) || !fs::is_directory(second_pass_dir))
    {
        throw std::runtime_error("Required subfolder 'second_pass' does not exist or is not a directory within: " + shader_dir_root_.string());
    }
}

/// <summary>
/// Collects shader modules information in shader folders.
/// </summary>
/// <returns></returns>
std::vector<VkShaderManager::ShaderModulePaths> VkShaderManager::collectShaderModules()
{
    std::map<std::string, ShaderModulePaths> modules;
    auto collectModules = [&](const fs::path& folder, RenderPass pass){
        for (const auto& entry : fs::directory_iterator(folder))
        {
            if (entry.is_regular_file())
            {
                const fs::path& p = entry.path();
                std::string filename = p.filename().string();
                std::string stem = p.stem().string();     // e.g., "shader" from "shader.vert"
                std::string extension = p.extension().string(); // e.g., ".vert"

                // Handle cases like "shader.vert.spv" vs "shader.vert" for stem
                // We are interested in the base name before .vert or .frag
                if (extension == ".spv") // Skip pre-compiled files in this discovery pass for sources
                { 
                    continue;
                }

                // If stem itself ends with .vert or .frag (e.g. from a filename like shader.vert.glsl)
                // we might need a more robust way to get the true base name.
                // For simplicity, assuming names like "myShader.vert", "myShader.frag"
                // For filenames like "shader.type.ext", stem() gives "shader.type". We need "shader".
                // A simple approach is to remove known shader type extensions from the stem.
                std::string base_name = stem;
                if (base_name.length() > 5 && (base_name.rfind(".vert") == base_name.length() - 5 || base_name.rfind(".frag") == base_name.length() - 5)) {
                    // This case is tricky if shader names are like "render.pass.vert"
                    // For now, assuming simple "name.vert", "name.frag"
                }

                modules[stem].pass = pass;
                if (extension == ".vert")
                {
                    modules[stem].vertSrcPath = p;
                    // Define expected SPIR-V path: e.g., shader.vert -> shader.vert.spv
                    modules[stem].vertSpvPath = fs::path(p.string() + ".spv");
                }
                else if (extension == ".frag") {
                    modules[stem].fragSrcPath = p;
                    // Define expected SPIR-V path: e.g., shader.frag -> shader.frag.spv
                    modules[stem].fragSpvPath = fs::path(p.string() + ".spv");
                }
            }
        }
    };

    collectModules(first_pass_dir, RenderPass::FIRST);
    collectModules(second_pass_dir, RenderPass::SECOND);

    std::vector<ShaderModulePaths> m;
    for (auto& pair : modules)
    {
        pair.second.name = pair.first;
        m.push_back(pair.second);
    }

    return m;
}

/// <summary>
///  Returns paths of shaders that require compilation
/// </summary>
/// <param name="modules"></param>
/// <returns></returns>
std::vector<std::string> VkShaderManager::getShadersForCompilation(std::vector<ShaderModulePaths>& modules)
{
    std::vector<std::string> shadersToCompile;

    for (const auto& module : modules)
    {
        if (!areBinariesUpToDate(module))
        {
            std::cout << "Module " << module.name << " needs recompilation (binaries missing or outdated)." << std::endl;
            shadersToCompile.push_back(module.vertSrcPath.string());
            shadersToCompile.push_back(module.fragSrcPath.string());
        }
    }

    return shadersToCompile;
}

/// <summary>
/// Invokes the shader compilation batch script for provided shaders.
/// </summary>
/// <param name="shaderSources"></param>
void VkShaderManager::compile(const std::vector<std::string>& shaderSources)
{
    // system() is platform-dependent and has security considerations if the path is not trusted.
    // For a hard-coded path, it's generally simpler.
    // The script is assumed to compile all shaders in the known shader_dir_root_.
    std::string command = compile_script_path_.string();
    for (auto& shader : shaderSources)
    {
        command += " " + shader;
    }
    int result = system(command.c_str());

    if (result != 0) {
        // The problem doesn't explicitly state to throw on script failure,
        // but it's good practice to report it. The subsequent verification step
        // will catch if the script didn't produce the necessary files.
        std::cerr << "Warning: Shader compilation script execution may have failed (exit code: " << result << ")." << std::endl;
    }
}

/// <summary>
/// Checks if existing SPIR-V binaries for a shader module are up-to-date.
/// </summary>
/// <param name="module"></param>
/// <returns></returns>
bool VkShaderManager::areBinariesUpToDate(const ShaderModulePaths& module) const
{
    if (!fs::exists(module.vertSpvPath) || !fs::exists(module.fragSpvPath))
    {
        return false; // Binaries don't exist
    }
    try
    {
        auto vert_src_time = fs::last_write_time(module.vertSrcPath);
        auto frag_src_time = fs::last_write_time(module.fragSrcPath);
        auto vert_spv_time = fs::last_write_time(module.vertSpvPath);
        auto frag_spv_time = fs::last_write_time(module.fragSpvPath);
        // Binaries are up-to-date if they are newer or same time as sources
        return (vert_spv_time >= vert_src_time) && (frag_spv_time >= frag_src_time);
    }
    catch (const fs::filesystem_error& e)
    {
        std::cerr << "Filesystem error while checking timestamps: " << e.what() << std::endl;
        return false; // Error accessing files, assume not up-to-date
    }
}

/// <summary>
/// Verifies that provided shader modules are complete (have both vertex and fragment shader source files).
/// </summary>
/// <param name="modules"></param>
void VkShaderManager::verifyCompleteModules(const std::vector<ShaderModulePaths>& modules)
{
    std::vector<bool> passModule(2, false);
    for (const auto& module : modules)
    {
        if (!module.vertSrcPath.empty() && !module.fragSrcPath.empty())
        {
            // Check again if binaries are present and up-to-date *after* potential compilation
            if (fs::exists(module.vertSrcPath) && fs::exists(module.fragSrcPath)) 
            { // This check implicitly handles timestamp
                passModule[static_cast<int>(module.pass)] = true;
            }
        }
    }
    for (int i = 0; i < passModule.size(); i++)
    {
        if (!passModule[i])
        {
            throw std::runtime_error("There is no any complete shader module for pass " + std::to_string(i));
        }
    }
}
