#include "main_window.h"
#include "ui/ui_mainwindow.h"  // генерируетс€ автоматически из .ui
#include "vulkan_window.h"
#include <QVulkanInstance>
#include <QWidget>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    auto inst = new QVulkanInstance;
    if (!inst->create())
        qFatal("Failed to create Vulkan instance");

    vulkanWindow = new VulkanWindow;
    vulkanWindow->setVulkanInstance(inst);

    QWidget* container = QWidget::createWindowContainer(vulkanWindow);
    container->setMinimumSize(400, 400);
    container->setFocusPolicy(Qt::StrongFocus);

    // ¬ставл€ем Vulkan-окно в graphicsView
    ui->graphicsView->setViewport(container);
}

MainWindow::~MainWindow()
{
    delete ui;
}
