#pragma once

#include <QVulkanWindowRenderer>
#include <QMatrix4x4>
#include <vulkan/vulkan.h>
#include <QVulkanDeviceFunctions>
#include <vector>
#include <array>



class VulkanRenderer : public QVulkanWindowRenderer {
public:
    explicit VulkanRenderer(QVulkanWindow* w);

    void initResources() override;
    void initSwapChainResources() override;
    void releaseSwapChainResources() override;
    void releaseResources() override;
    void startNextFrame() override;
    void createVertexBuffer();
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
private:
    void createTriangle();
    VkShaderModule createShaderModule(const QString& filePath) const;

    QVulkanWindow* p_window;
    QVulkanDeviceFunctions* devFuncs = nullptr;

    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkShaderModule vertShaderModule = VK_NULL_HANDLE;
    VkShaderModule fragShaderModule = VK_NULL_HANDLE;

    float rotation = 0.0f;
};
