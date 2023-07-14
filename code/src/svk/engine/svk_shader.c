#include "svk/engine/svk_shader.h"
#include "svk/util/svk_filesystem.h"
#include "svk/svk_common.h"

typedef struct svkShaderCreateResult
{
    VkShaderModule module;
    VkResult result;
} svkShaderCreateResult;

// Private Functions
//------------------------------------------------------------------------
internal svkShaderCreateResult CreateShaderModule(VkDevice device, svkString* shaderCode)
{
    VkShaderModuleCreateInfo createInfo = SVK_ZMSTRUCT2(VkShaderModuleCreateInfo);
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = shaderCode->len;
    createInfo.pCode = (uint32_t*)shaderCode->data;

    svkShaderCreateResult shaderResult = SVK_ZMSTRUCT2(svkShaderCreateResult);
    shaderResult.result = vkCreateShaderModule(device, &createInfo, NULL, &shaderResult.module);

    return shaderResult;
}

// Public Functions
//------------------------------------------------------------------------
svkShader* svkShader_CreateFromFile(
    const VkDevice device,
    const char* filename,
    const VkPipelineShaderStageCreateFlags type)
{
    svkFilesystemResult result;
    svkString* shaderCode = svkFS_FileToString(filename, &result);
    if (result != FS_SUCCESS)
    {
        fprintf(stderr, "Failed reading shader \"%s\", error code: %d\n", filename, result);
        SVK_FREE(shaderCode);
        return NULL;
    }

    svkShader* shader = SVK_ALLOCSTRUCT2(svkShader);
    svkShaderCreateResult shaderResult = CreateShaderModule(device, shaderCode);
    if (shaderResult.result != VK_SUCCESS)
    {
        printf("Failed creating shader module for \"%s\", error code: %d\n", filename, shaderResult.result);
        SVK_FREE(shaderCode);
        return NULL;
    }

    shader->shader = shaderResult.module;
    SVK_ZM(shader->stageInfo, sizeof(VkPipelineShaderStageCreateInfo));
    shader->stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader->stageInfo.stage = type;
    shader->stageInfo.module = shader->shader;
    shader->stageInfo.pName = "main";

    printf("Loaded shader: \"%s\"\n", filename);

    return shader;
}