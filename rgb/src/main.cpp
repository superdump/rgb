#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <functional>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <unordered_set>

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
  const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
  const VkAllocationCallbacks* pAllocator,
  VkDebugUtilsMessengerEXT* pDebugMessenger)
{
  auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
  if (func != nullptr)
  {
    return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
  }
  else
  {
    return VK_ERROR_EXTENSION_NOT_PRESENT;
  }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
  VkDebugUtilsMessengerEXT debugMessenger,
  const VkAllocationCallbacks* pAllocator)
{
  auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
  if (func != nullptr)
  {
    func(instance, debugMessenger, pAllocator);
  }
}

struct QueueFamilyIndices
{
  std::optional<uint32_t> graphicsFamily;

  bool isComplete()
  {
    return graphicsFamily.has_value();
  }
};

class HelloTriangleApplication
{
private:
  const int WIDTH = 800;
  const int HEIGHT = 600;

  const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
  };

#ifdef NDEBUG
  const bool enableValidationLayers = false;
#else
  const bool enableValidationLayers = true;
#endif

  GLFWwindow* window;

  VkInstance instance;
  VkDebugUtilsMessengerEXT debugMessenger;

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
    setupDebugMessenger();
    choosePhysicalDevice();
  }

  QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
  {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
      if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
      {
        indices.graphicsFamily = i;
      }

      if (indices.isComplete())
      {
        break;
      }

      ++i;
    }
    return indices;
  }

  void choosePhysicalDevice()
  {
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
      std::cerr << "ERROR: No Vulkan physical devices found!" << std::endl;
      throw std::runtime_error("No Vulkan physical devices found!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices)
    {
      if (isDeviceSuitable(device))
      {
        physicalDevice = device;
        break;
      }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
      std::cerr << "ERROR: No suitable Vulkan physical device found!" << std::endl;
      throw std::runtime_error("No suitable Vulkan physical device found!");
    }
  }

  bool isDeviceSuitable(const VkPhysicalDevice device)
  {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;

    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    QueueFamilyIndices indices = findQueueFamilies(device);

    return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
      && indices.isComplete();
  }

  void setupDebugMessenger()
  {
    if (!enableValidationLayers)
    {
      return;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
      VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; // Optional

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
    {
      std::cerr << "ERROR: Failed to set up the debug messenger!" << std::endl;
      throw std::runtime_error("Failed to set up the debug messenger!");
    }
  }

  void createInstance()
  {
    if (enableValidationLayers && !checkValidationLayerSupport())
    {
      std::cerr << "ERROR: Missing validation layers!" << std::endl;
      throw std::runtime_error("Missing validation layers!");
    }

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

    auto requiredExtensions = getRequiredExtensions();

    auto availableExtensions = getVkExtensions();
    printVkExtensions(availableExtensions);
    if (!checkVkExtensions(availableExtensions, requiredExtensions))
    {
      std::cerr << "ERROR: Missing extension" << std::endl;
      throw std::runtime_error("Missing extension");
    }
    std::cerr << "INFO: All " << requiredExtensions.size() << " required extensions present" << std::endl;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    // Enable validation layers
    if (enableValidationLayers)
    {
      std::cerr << "INFO: Enabling " << validationLayers.size()
        << " validation layer" << (validationLayers.size() <= 1 ? "" : "s")
        << std::endl;
      createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
      createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
      createInfo.enabledLayerCount = 0;
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS)
    {
      std::cerr << "ERROR: Failed to create a Vulkan instance: " << result << std::endl;
      throw std::runtime_error("Failed to create Vulkan instance!");
    }
  }

  bool checkValidationLayerSupport()
  {
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

    std::unordered_set<std::string> availableLayers;
    for (const auto& layer : layers)
    {
      availableLayers.insert(layer.layerName);
    }

    bool layersPresent = true;
    for (const auto validationLayer : validationLayers)
    {
      if (availableLayers.find(validationLayer) == availableLayers.end())
      {
        std::cerr << "WARNING: " << validationLayer << " is missing" << std::endl;
        layersPresent = false;
      }
    }

    return layersPresent;
  }

  static std::string getSeverityFromFlag(const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity)
  {
    static const std::unordered_map<VkDebugUtilsMessageSeverityFlagBitsEXT, std::string> severityToString = {
      {VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,   "ERROR"},
      {VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT, "WARNING"},
      {VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,    "INFO"},
      {VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT, "VERBOSE"},
    };

    const auto value = severityToString.find(messageSeverity);
    if (value == severityToString.end())
    {
      return "UNKNOWN";
    }
    else
    {
      return value->second;
    }
  }

  static std::string getTypeFromFlag(const VkDebugUtilsMessageTypeFlagsEXT messageType)
  {
    static const std::unordered_map<VkDebugUtilsMessageTypeFlagsEXT, std::string> typeToString = {
      {VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT,     "GENERAL"},
      {VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,  "VALIDATION"},
      {VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT, "PERFORMANCE"},
    };

    const auto value = typeToString.find(messageType);
    if (value == typeToString.end())
    {
      return "UNKNOWN";
    }
    else
    {
      return value->second;
    }
  }

  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
  {
    std::cerr << "Validation layer: " << getSeverityFromFlag(messageSeverity)
      << " : " << getTypeFromFlag(messageType) << " : "
      << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
  }

  std::vector<const char*> getRequiredExtensions()
  {
    // Get the extensions required by the window system
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers)
    {
      extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
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

  bool checkVkExtensions(const std::vector<VkExtensionProperties>& extensions,
    const std::vector<const char *>& requiredExtensions)
  {
    std::unordered_set<std::string> available;

    for (const auto& extension : extensions)
    {
      available.insert(&extension.extensionName[0
    ]);
    }

    bool extensionsPresent = true;
    for (const auto requiredExtension : requiredExtensions)
    {
      if (available.find(requiredExtension) == available.end())
      {
        std::cerr << "WARNING: Missing required extension: "
          << requiredExtension << std::endl;
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
    if (enableValidationLayers)
    {
      DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

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