#include "vulkan_renderer.h"
#include "vulkan_window.h"
#include <QVulkanFunctions>
#include <QFile>
#include <QDebug>
#include <cmath>

VulkanRenderer::VulkanRenderer(VulkanWindow* w)
    : p_window(w)
{
}

void VulkanRenderer::initResources()
{
    devFuncs = p_window->vulkanInstance()->deviceFunctions(p_window->device());
    assert(devFuncs);

    createTriangle();
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

void VulkanRenderer::createTriangle()
{
    std::vector<Vertex> vertices = {
          {{ 0.0f, -0.5f }, {1.0f, 0.0f, 0.0f}},
          {{ 0.5f,  0.5f }, {0.0f, 1.0f, 0.0f}},
          {{-0.5f,  0.5f }, {0.0f, 0.0f, 1.0f}},
    };

    VkDevice device = p_window->device();
    VkPhysicalDevice physDev = p_window->physicalDevice();

    VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

    // Создание vertex buffer
    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    devFuncs->vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer);

    VkMemoryRequirements memReq;
    devFuncs->vkGetBufferMemoryRequirements(device, vertexBuffer, &memReq);

    VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = 0; // Упростим — найдем ниже

    VkPhysicalDeviceMemoryProperties memProps;
    p_window->vulkanInstance()->functions()->vkGetPhysicalDeviceMemoryProperties(physDev, &memProps);
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) 
    {
        if ((memReq.memoryTypeBits & (1 << i)) &&
            (memProps.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) 
        {
            allocInfo.memoryTypeIndex = i;
            break;
        }
    }

    devFuncs->vkAllocateMemory(device, &allocInfo, nullptr, &vertexBufferMemory);
    void* data;
    devFuncs->vkMapMemory(device, vertexBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    devFuncs->vkUnmapMemory(device, vertexBufferMemory);
    devFuncs->vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);

    //  Создание pipeline
    VkShaderModule vertShader = createShaderModule("shaders/triangle.vert.spv");
    VkShaderModule fragShader = createShaderModule("shaders/triangle.frag.spv");

    VkPipelineShaderStageCreateInfo vertStage = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    vertStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertStage.module = vertShader;
    vertStage.pName = "main";

    VkPipelineShaderStageCreateInfo fragStage = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
    fragStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragStage.module = fragShader;
    fragStage.pName = "main";

    VkPipelineShaderStageCreateInfo stages[] = { vertStage, fragStage };

    // ---- Pipeline layout (push constants) ----
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(QMatrix4x4);

    VkPipelineLayoutCreateInfo layoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    layoutInfo.pushConstantRangeCount = 1;
    layoutInfo.pPushConstantRanges = &pushConstantRange;
    devFuncs->vkCreatePipelineLayout(device, &layoutInfo, nullptr, &pipelineLayout);

    // ---- Остальная графическая конфигурация ----
    VkPipelineVertexInputStateCreateInfo vertexInput = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport viewport{};
    viewport.width = (float)p_window->swapChainImageSize().width();
    viewport.height = (float)p_window->swapChainImageSize().height();
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{ {0, 0}, p_window->swapChainImageSize().width(), p_window->swapChainImageSize().height()};

    VkPipelineViewportStateCreateInfo viewportState = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisampling = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = 0xF;

    VkPipelineColorBlendStateCreateInfo colorBlending = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    VkGraphicsPipelineCreateInfo pipelineInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = stages;
    pipelineInfo.pVertexInputState = &vertexInput;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = p_window->defaultRenderPass();
    pipelineInfo.subpass = 0;

    devFuncs->vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);

    devFuncs->vkDestroyShaderModule(device, vertShader, nullptr);
	devFuncs->vkDestroyShaderModule(device, fragShader, nullptr);
}

void VulkanRenderer::startNextFrame()
{
    rotation += 1.0f;
    if (rotation > 360.0f)
        rotation = 0.0f;

    QMatrix4x4 model;
    model.rotate(rotation, 0.0f, 0.0f, 1.0f);

    VkCommandBuffer cmdBuf = p_window->currentCommandBuffer();
    devFuncs->vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    devFuncs->vkCmdPushConstants(
        cmdBuf,
        pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(QMatrix4x4),
        model.constData()
    );

    VkBuffer vertexBuffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    devFuncs->vkCmdBindVertexBuffers(cmdBuf, 0, 1, vertexBuffers, offsets);
    devFuncs->vkCmdDraw(cmdBuf, 3, 1, 0, 0);

    p_window->frameReady();
    p_window->requestUpdate();
}

void VulkanRenderer::initSwapChainResources() {}
void VulkanRenderer::releaseSwapChainResources() {}
void VulkanRenderer::releaseResources()
{
    VkDevice device = p_window->device();
    devFuncs->vkDestroyPipeline(device, pipeline, nullptr);
    devFuncs->vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    devFuncs->vkDestroyBuffer(device, vertexBuffer, nullptr);
    devFuncs->vkFreeMemory(device, vertexBufferMemory, nullptr);
}
