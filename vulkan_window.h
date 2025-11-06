
#pragma once

#include <QVulkanWindow>

class VulkanRenderer;

class VulkanWindow : public QVulkanWindow {
public:
    QVulkanWindowRenderer* createRenderer() override;
};
