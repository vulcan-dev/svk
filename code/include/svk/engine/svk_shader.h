#ifndef SVK_SHADER_H
#define SVK_SHADER_H

#include <vulkan/vulkan.h>

typedef enum SvkShaderType
{
    SVK_SHADERTYPE_UNKNOWN = -1,
    SVK_SHADERTYPE_VERTEX = 0,
    SVK_SHADERTYPE_FRAGMENT = 1
} SvkShaderType;

typedef struct svkShader
{
    VkShaderModule shader;
    VkPipelineShaderStageCreateInfo stageInfo;
} svkShader;

svkShader* svkShader_CreateFromFile(
    const VkDevice device,
    const char* filename,
    const VkPipelineShaderStageCreateFlags type);

#endif