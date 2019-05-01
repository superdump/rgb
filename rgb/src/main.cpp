#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <unordered_set>

class HelloTriangleApplication
{
private:
  const int WIDTH = 800;
  const int HEIGHT = 600;

  GLFWwindow* window;

  VkInstance instance;

public:
  void run()
  {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
  }

private:
  void initWindow()
  {
    glfwInit();

    // Do not create an OpenGL context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // FIXME: Do not allow resizing
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
  }

  void initVulkan()
  {
    createInstance();
  }

  void createInstance()
  {
    // This is technically optional but provides useful information for the
    // driver to optimize for our specific application
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Required
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Get the extensions required by the window system
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    // FIXME: Global validation layers disabled
    createInfo.enabledLayerCount = 0;

    auto availableExtensions = getVkExtensions();
    printVkExtensions(availableExtensions);
    if (!checkVkExtensions(availableExtensions, glfwExtensions, glfwExtensionCount))
    {
      std::cerr << "ERROR: Missing extension" << std::endl;
      throw std::runtime_error("Missing extension");
    }
    std::cerr << "INFO: All required extensions present" << std::endl;

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS)
    {
      std::cerr << "ERROR: Failed to create a Vulkan instance: " << result << std::endl;
      throw std::runtime_error("Failed to create Vulkan instance!");
    }
  }

  std::vector<VkExtensionProperties> getVkExtensions()
  {
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);

    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    return extensions;
  }

  void printVkExtensions(const std::vector<VkExtensionProperties>& extensions)
  {
    std::cerr << "INFO: " << extensions.size() << " available extensions:" << std::endl;

    for (const auto& extension : extensions)
    {
      std::cerr << "\t" << extension.extensionName << std::endl;
    }
  }

  bool checkVkExtensions(const std::vector<VkExtensionProperties>& extensions, const char **requiredExtensions, const int requiredExtensionCount)
  {
    std::unordered_set<std::string> available;

    for (const auto& extension : extensions)
    {
      available.insert(&extension.extensionName[0
    ]);
    }

    bool extensionsPresent = true;
    for (int i = 0; i < requiredExtensionCount; ++i)
    {
      const char* required = requiredExtensions[i];
      if (available.find(required) == available.end())
      {
        std::cerr << "WARNING: Missing required extension: " << required << std::endl;
        extensionsPresent = false;
      }
    }

    return extensionsPresent;
  }

  void mainLoop()
  {
    while (!glfwWindowShouldClose(window))
    {
      glfwPollEvents();
    }
  }

  void cleanup()
  {
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);

    glfwTerminate();
  }
};

int main()
{
  HelloTriangleApplication app;

  try
  {
    app.run();
  }
  catch (const std::exception & e)
  {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}