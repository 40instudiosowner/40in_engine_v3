
#include "vulkan_window.h"
#include "vulkan_renderer.h"

QVulkanWindowRenderer* VulkanWindow::createRenderer()
{
    return new VulkanRenderer(this);
}