#ifndef SVK_ENGINE_H
#define SVK_ENGINE_H

#include <vulkan/vulkan.h>
#include <stdalign.h>

#include "svk/engine/svk_shader.h"
#include "svk/util/svk_vector.h"
#include "svk/svk.h"

#include "cglm/cglm.h"

#define MAX_FRAMES_IN_FLIGHT 2

/* Notes:
 * Functions beginning with `_svkEngine_` are meant for internal use
 * Structures prefixed with an underscore are meant for internal use
*/

// Enums
//------------------------------------------------------------------------
typedef enum SvkResult
{
    SVK_SUCCESS = 0x0000000000,
    SVK_ERROR_CREATE_WINDOW = 0x0000000001,

    SVK_ERROR_GET_INSTANCE_EXTENSIONS = 0x0000000002,
    SVK_ERROR_VALIDATE_LAYER_SUPPORT = 0x0000000003,
    SVK_ERROR_CREATE_INSTANCE = 0x0000000004,
    SVK_ERROR_NO_VULKAN_SUPPORT = 0x0000000005,
    SVK_ERROR_NO_SUITABLE_DEVICE = 0x0000000006
} SvkResult;

extern SvkResult lastErrorCode;
SvkResult SVK_GetLastError();

// "Private" Structures
//------------------------------------------------------------------------
struct _svkEngineInfo
{
    u32 version;
    const char* appName;
};

struct _svkEngineCoreQueue
{
    VkQueue graphics;
    VkQueue present;
};

struct _svkEngineSwapChain
{
    VkSwapchainKHR swapChain;
    SVKVECTOR_TYPE(VkImage) images;
    SVKVECTOR_TYPE(VkImageView) imageViews;
    SVKVECTOR_TYPE(VkFramebuffer) frameBuffers;
    VkFormat imageFormat;
    VkExtent2D extent;
};

struct _svkEngineScene
{
    SVKVECTOR_TYPE(svkDrawable) drawables;
    struct svkCamera* camera;
};

struct _svkEngineRenderer
{
    SVKARRAY_TYPE(VkSemaphore) imageAvailableSemaphores;
    SVKARRAY_TYPE(VkSemaphore) renderFinishedSemaphores;
    SVKARRAY_TYPE(VkFence) inFlightFences;
    VkClearValue clearColor;
};

struct _svkEngineDebug
{
    float gpuTime;
};

struct _svkEngineDepth
{
    VkImage image;
    VkDeviceMemory imageMemory;
    VkImageView imageView;
};

struct _svkEngineCore
{
    VkInstance instance;

    struct _svkEngineCoreQueue queues;
    struct _svkEngineRenderer renderer;
    struct _svkEngineDepth depth;

    SVKVECTOR_TYPE(svkShader) shaders; // TODO: Move into renderer

    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VkSurfaceKHR surface;
    VkPipeline graphicsPipeline;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkRenderPass renderPass;

    uint32_t currentFrame;

    VkCommandPool commandPool;
    VkQueryPool timeQueryPool;
    VkDescriptorPool descriptorPool;

    SVKARRAY_TYPE(VkCommandBuffer) commandBuffers;
};

typedef struct _svkEngineDrawableBuffers
{
    VkBuffer vertexBuffer;
    VkBuffer indexBuffer;
    SVKARRAY_TYPE(VkBuffer) uniformBuffers;
    VkDeviceMemory vertexMemory;
    VkDeviceMemory indexMemory;
    SVKARRAY_TYPE(VkDeviceMemory) uniformBuffersMemory;

    SVKARRAY_TYPE(void*) mappedBuffers;
} _svkEngineDrawableBuffers;

// Public Structures
//------------------------------------------------------------------------
typedef struct svkWindow svkWindow;
typedef struct SDL_Window SDL_Window;

typedef struct svkUniformBufferObj
{
    alignas(16) mat4 model;
    alignas(16) mat4 view;
    alignas(16) mat4 proj;
} svkUniformBufferObj;

typedef struct svkQueueFamilyIndices
{
    int64_t graphicsFamily;
    int64_t presentFamily;
} svkQueueFamilyIndices;

typedef struct svkSwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    SVKVECTOR_TYPE(VkSurfaceFormatKHR) formats;
    SVKVECTOR_TYPE(VkPresentModeKHR) presentModes;
} svkSwapChainSupportDetails;

typedef struct svkDrawable
{
    SVKVECTOR_TYPE(svkVertex) vertices;
    SVKVECTOR_TYPE(uint16_t) indices;
    _svkEngineDrawableBuffers* buffers;
    SVKARRAY_TYPE(VkDescriptorSet) descriptorSets;
} svkDrawable;

typedef struct svkCamera
{
    vec3 pos;

    float yaw;
    float pitch;
    float speed;
    float mouseSensitivity;

    float lastMouseX;
    float lastMouseY;
    bool firstMouse;
    bool mouseLocked;

    float nearClip;
    float farClip;

    float aspectRatio;

    alignas(16) mat4 view;
    alignas(16) mat4 projection;
} svkCamera;

typedef struct svkEngine
{
    struct _svkEngineCore core;
    struct _svkEngineInfo info;
    struct _svkEngineScene* scene;
    struct _svkEngineSwapChain* swapChain;
    struct _svkEngineDebug debug;

    VkDebugUtilsMessengerEXT debugMessenger;
} svkEngine;

// Functions
//------------------------------------------------------------------------
svkEngine* svkEngine_Create(
    const char* appName,
    const u32   appVersion);

VkFormat _svkEngine_FindDepthFormat(const VkPhysicalDevice physicalDevice);

void svkEngine_Destroy(svkEngine* svke);
VkResult svkEngine_RecreateSwapChain(svkEngine* engine, SDL_Window* window);

svkDrawable* svkDrawable_Create(
    const svkVertex* vertices,
    const uint16_t vertexCount,
    const uint16_t* indices,
    const uint16_t indexCount);

// Internal Functions
//------------------------------------------------------------------------
svkQueueFamilyIndices _svkEngine_FindQueueFamilies(
    const VkPhysicalDevice physicalDevice,
    const VkSurfaceKHR surface);

VkResult _svkEngine_Initialize(
    svkEngine*  svke,
    SDL_Window* window);

#endif