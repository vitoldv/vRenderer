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
    if (!fs::exists(shader_src_dir) || !fs::is_directory(shader_src_dir))
    {
        throw std::runtime_error("Shader directory root does not exist or is not a directory: " + shader_src_dir.string());
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
        throw std::runtime_error("Required subfolder 'first_pass' does not exist or is not a directory within: " + shader_src_dir.string());
    }
    if (!fs::exists(second_pass_dir) || !fs::is_directory(second_pass_dir))
    {
        throw std::runtime_error("Required subfolder 'second_pass' does not exist or is not a directory within: " + shader_src_dir.string());
    }
}

/// <summary>
/// Collects shader modules information in shader folders.
/// </summary>
/// <returns></returns>
std::vector<VkShaderManager::ShaderModulePaths> VkShaderManager::collectShaderModules()
{
    std::map<std::string, ShaderModulePaths> modules;
    auto collectModules = [&, this](const fs::path& folder, RenderPass pass){
        for (const auto& entry : fs::directory_iterator(folder))
        {
            if (entry.is_regular_file())
            {
                const fs::path& p = entry.path();
                std::string filename = p.filename().string();
                std::string stem = p.stem().string();     // e.g., "shader" from "shader.vert"
                std::string extension = p.extension().string(); // e.g., ".vert"

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
                    modules[stem].vertSpvPath = shader_bin_dir / (p.stem().string() + ".vert" + ".spv");
                    getShaderDependencies(p, modules[stem].dependencies);
                }
                else if (extension == ".frag") {
                    modules[stem].fragSrcPath = p;
                    // Define expected SPIR-V path: e.g., shader.frag -> shader.frag.spv
                    modules[stem].fragSpvPath = shader_bin_dir / (p.stem().string() + ".frag" + ".spv");
                    getShaderDependencies(p, modules[stem].dependencies);
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
    command += " -o  " + shader_bin_dir.string();
    command += " -i  " + shader_include_dir.string();
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

void VkShaderManager::getShaderDependencies(fs::path shader, std::set<fs::path>& dependencies)
{
    // Parse #include-s
    std::ifstream stream;
    std::string line;
    stream.open(shader.string());
    if (!stream.is_open())
    {
        throw std::runtime_error("Could not read shader file.");
    }

    while (std::getline(stream, line))
    {
        size_t include_pos = line.find("#include");
        if (include_pos == std::string::npos) {
            continue; // No "#include" on this line
        }
        size_t first_quote_pos = line.find('"', include_pos);
        if (first_quote_pos == std::string::npos) {
            continue; // No opening quote found
        }
        size_t last_quote_pos = line.find('"', first_quote_pos + 1);
        if (last_quote_pos == std::string::npos) {
            continue; // No closing quote found
        }

        size_t path_start = first_quote_pos + 1;
        size_t path_length = last_quote_pos - path_start;
        std::string include_str = line.substr(path_start, path_length);
        fs::path include_path = shader_include_dir / include_str;
        if (fs::exists(include_path))
        {
            dependencies.insert(include_path);
        }
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
        bool old = false;
        auto vert_src_time = fs::last_write_time(module.vertSrcPath);
        auto frag_src_time = fs::last_write_time(module.fragSrcPath);
        auto vert_spv_time = fs::last_write_time(module.vertSpvPath);
        auto frag_spv_time = fs::last_write_time(module.fragSpvPath);
        for (const fs::path& dependency : module.dependencies)
        {
            auto time = fs::last_write_time(dependency);
            // Binary is old if any of dependency files was edited later
            old |= time > vert_spv_time;
            old |= time > frag_spv_time;
        }
        // Binaries are old if they are older than sources
        old |= vert_src_time > vert_spv_time;
        old |= frag_src_time > frag_spv_time;

        return !old;
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
