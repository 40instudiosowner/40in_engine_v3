#pragma once

#include <QVulkanWindowRenderer>
#include <QMatrix4x4>
#include <vulkan/vulkan.h>
#include <QVulkanDeviceFunctions>
#include <vector>

struct Vertex {
    float pos[2];
    float color[3];
};

class VulkanWindow;

class VulkanRenderer : public QVulkanWindowRenderer {
public:
    explicit VulkanRenderer(VulkanWindow* w);

    void initResources() override;
    void initSwapChainResources() override;
    void releaseSwapChainResources() override;
    void releaseResources() override;
    void startNextFrame() override;

private:
    void createTriangle();
    VkShaderModule createShaderModule(const QString& filePath) const;

    VulkanWindow* p_window;
    QVulkanDeviceFunctions* devFuncs = nullptr;

    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;

    float rotation = 0.0f;
};
