#include "vulkan_renderer.h"
#include "vulkan_window.h"
#include <QVulkanFunctions>
#include <QFile>
#include <QDebug>
#include <cmath>


struct Vertex {
    QVector2D pos;
    QVector3D color;
};

std::array<Vertex, 3> vertices = { {
    {{ 0.0f, -0.5f }, {1.0f, 0.0f, 0.0f}},
    {{ 0.5f,  0.5f }, {0.0f, 1.0f, 0.0f}},
    {{-0.5f,  0.5f }, {0.0f, 0.0f, 1.0f}}
} };



VulkanRenderer::VulkanRenderer(QVulkanWindow* w)
    : p_window(w)
{
}

void VulkanRenderer::initResources()
{
    // Получаем функции устройства
    devFuncs = p_window->vulkanInstance()->deviceFunctions(p_window->device());

    // Создаём pipeline и вершинный буфер
    createTriangle();
    createVertexBuffer();
  
}

VkShaderModule VulkanRenderer::createShaderModule(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qFatal("Failed to open shader: %s", qPrintable(filePath));
    }
    QByteArray code = file.readAll();

    VkShaderModuleCreateInfo info = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    info.codeSize = code.size();
    info.pCode = reinterpret_cast<const uint32_t*>(code.constData());

    VkShaderModule module;
    devFuncs->vkCreateShaderModule(p_window->device(), &info, nullptr, &module);
    return module;
}

// ---------------------- Создание треугольника ----------------------
static VkShaderModule loadShaderModule(QVulkanDeviceFunctions* devFuncs, VkDevice device, const QString& path) {
    QFile f(path);
    if (!f.open(QFile::ReadOnly)) {
        qFatal("Failed to open shader: %s", qUtf8Printable(path));
    }
    QByteArray code = f.readAll();

    VkShaderModuleCreateInfo moduleCreateInfo{};
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.codeSize = code.size();
    moduleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.constData());

    VkShaderModule shaderModule;
    if (devFuncs->vkCreateShaderModule(device, &moduleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        qFatal("Failed to create shader module for %s", qUtf8Printable(path));
    }
    return shaderModule;
}

void VulkanRenderer::createTriangle()
{
    VkDevice device = p_window->device();

    // Загружаем SPIR-V шейдеры
    vertShaderModule = loadShaderModule(devFuncs, device, "shaders/triangle.vert.spv");
    fragShaderModule = loadShaderModule(devFuncs, device, "shaders/triangle.frag.spv");

    // Pipeline layout (без дескрипторов для простого треугольника)
    VkPipelineLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    if (devFuncs->vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        qFatal("Failed to create pipeline layout");
    }

    // Shader stages
    VkPipelineShaderStageCreateInfo vertStage{};
    vertStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStage.module = vertShaderModule;
    vertStage.pName = "main";

    VkPipelineShaderStageCreateInfo fragStage{};
    fragStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStage.module = fragShaderModule;
    fragStage.pName = "main";

    VkPipelineShaderStageCreateInfo stages[] = { vertStage, fragStage };

    VkVertexInputBindingDescription binding{};
    binding.binding = 0;
    binding.stride = sizeof(Vertex);
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::array<VkVertexInputAttributeDescription, 2> attributes{};
    attributes[0].binding = 0;
    attributes[0].location = 0; // matches shader 'layout(location = 0) in vec3 inPos;'
    attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributes[0].offset = offsetof(Vertex, pos);

    attributes[1].binding = 0;
    attributes[1].location = 1; // matches shader 'layout(location = 1) in vec2 inUv;'
    attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributes[1].offset = offsetof(Vertex, color);

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &binding;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributes.data();


    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewport + scissor
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)p_window->width();
    viewport.height = (float)p_window->height();
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = { (uint32_t)p_window->width(), (uint32_t)p_window->height() };

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo raster{};
    raster.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    raster.depthClampEnable = VK_FALSE;
    raster.rasterizerDiscardEnable = VK_FALSE;
    raster.polygonMode = VK_POLYGON_MODE_FILL;
    raster.lineWidth = 1.0f;
    raster.cullMode = VK_CULL_MODE_BACK_BIT;
    raster.frontFace = VK_FRONT_FACE_CLOCKWISE;
    raster.depthBiasEnable = VK_FALSE;

    // Multisampling
    VkPipelineMultisampleStateCreateInfo multisample{};
    multisample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample.sampleShadingEnable = VK_FALSE;
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Color blending
    VkPipelineColorBlendAttachmentState colorBlendAttach{};
    colorBlendAttach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttach.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlend{};
    colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlend.logicOpEnable = VK_FALSE;
    colorBlend.attachmentCount = 1;
    colorBlend.pAttachments = &colorBlendAttach;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_FALSE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    // Pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = stages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &raster;
    pipelineInfo.pMultisampleState = &multisample;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlend;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = p_window->defaultRenderPass();
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;



    VkResult res;
    if (res = devFuncs->vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline)) {
        qFatal("Failed to create graphics pipeline");
    }
        qDebug() << "pipeline result:" << res << " handle=" << pipeline;
}

uint32_t VulkanRenderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDevice physDevice = p_window->physicalDevice();

    VkPhysicalDeviceMemoryProperties memProps;
    p_window->vulkanInstance()->functions()->vkGetPhysicalDeviceMemoryProperties(
        physDevice,
        &memProps
    );

    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    qFatal("Failed to find suitable memory type!");
    return 0;
}



void VulkanRenderer::createVertexBuffer()
{
    VkDevice device = p_window->device();

    VkDeviceSize bufferSize = sizeof(vertices);

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (devFuncs->vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS)
        qFatal("Failed to create vertex buffer");

    VkMemoryRequirements memReq;
    devFuncs->vkGetBufferMemoryRequirements(device, vertexBuffer, &memReq);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        memReq.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    if (devFuncs->vkAllocateMemory(device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS)
        qFatal("Failed to allocate vertex buffer memory");

    devFuncs->vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);

    // запись данных
    void* data;
    devFuncs->vkMapMemory(device, vertexBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), bufferSize);
    devFuncs->vkUnmapMemory(device, vertexBufferMemory);
}


void VulkanRenderer::startNextFrame()
{
    VkCommandBuffer cmdBuf = p_window->currentCommandBuffer();

    VkDeviceSize offsets[] = { 0 };

    VkRenderPassBeginInfo rpBegin{};
    rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBegin.renderPass = p_window->defaultRenderPass();
    rpBegin.framebuffer = p_window->currentFramebuffer();
    rpBegin.renderArea.offset = { 0, 0 };
    rpBegin.renderArea.extent = { (uint32_t)p_window->width(), (uint32_t)p_window->height() };

    VkClearValue clearValues[2];
    clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };
    rpBegin.clearValueCount = 2;
    rpBegin.pClearValues = clearValues;

    devFuncs->vkCmdBeginRenderPass(cmdBuf, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

    devFuncs->vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    devFuncs->vkCmdBindVertexBuffers(cmdBuf, 0, 1, &vertexBuffer, offsets);
    devFuncs->vkCmdDraw(cmdBuf, 3, 1, 0, 0);

    devFuncs->vkCmdEndRenderPass(cmdBuf);

    p_window->frameReady();
}



void VulkanRenderer::initSwapChainResources()
{


}
void VulkanRenderer::releaseSwapChainResources() {}
void VulkanRenderer::releaseResources()
{
    VkDevice device = p_window->device();
    devFuncs->vkDestroyPipeline(device, pipeline, nullptr);
    devFuncs->vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    devFuncs->vkDestroyBuffer(device, vertexBuffer, nullptr);
    devFuncs->vkFreeMemory(device, vertexBufferMemory, nullptr);
}
