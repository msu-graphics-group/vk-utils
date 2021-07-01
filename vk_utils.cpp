#include "vk_utils.h"

#include <cstring>
#include <set>
#include <iostream>
#include <algorithm>
#include <cmath>

namespace vk_utils {

  static const char *g_debugReportExtName = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;

  std::string errorString(VkResult errorCode) {
    switch (errorCode) {
#define STR(r) case VK_##r: return #r
      STR(NOT_READY);
      STR(TIMEOUT);
      STR(EVENT_SET);
      STR(EVENT_RESET);
      STR(INCOMPLETE);
      STR(ERROR_OUT_OF_HOST_MEMORY);
      STR(ERROR_OUT_OF_DEVICE_MEMORY);
      STR(ERROR_INITIALIZATION_FAILED);
      STR(ERROR_DEVICE_LOST);
      STR(ERROR_MEMORY_MAP_FAILED);
      STR(ERROR_LAYER_NOT_PRESENT);
      STR(ERROR_EXTENSION_NOT_PRESENT);
      STR(ERROR_FEATURE_NOT_PRESENT);
      STR(ERROR_INCOMPATIBLE_DRIVER);
      STR(ERROR_TOO_MANY_OBJECTS);
      STR(ERROR_FORMAT_NOT_SUPPORTED);
      STR(ERROR_SURFACE_LOST_KHR);
      STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
      STR(SUBOPTIMAL_KHR);
      STR(ERROR_OUT_OF_DATE_KHR);
      STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
      STR(ERROR_VALIDATION_FAILED_EXT);
      STR(ERROR_INVALID_SHADER_NV);
#undef STR
      default:
        return "UNKNOWN_ERROR";
    }
  };

  static void RunTimeError(const char* msg)
  {
    printf("runtime error at %s, line %d : %s", __FILE__, __LINE__, msg);
    exit(99);
  }

  bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::vector<const char *> &requestedExtensions)
  {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(requestedExtensions.begin(), requestedExtensions.end());

    for (const auto &extension : availableExtensions) {
      requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
  }

  bool checkLayerSupport(std::vector<const char *> &requestedLayers, std::vector<std::string> &supportedLayers)
  {
    bool all_layers_supported = true;
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> layerProperties(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());
    std::vector<std::string> allPresentLayers;//(requestedLayers.begin(), requestedLayers.end());
    allPresentLayers.reserve(layerCount);
    for (const auto &layerProp : layerProperties)
    {
      allPresentLayers.emplace_back(layerProp.layerName);
    }

    for (const auto &layer : requestedLayers) {
      auto found = std::find(allPresentLayers.begin(), allPresentLayers.end(), layer);
      if (found != allPresentLayers.end()) {
        supportedLayers.emplace_back(layer);
      } else {
        std::cout << "Warning. Requested layer not found: " << layer << std::endl;
        all_layers_supported = false;
      }
    }

    return all_layers_supported;
  }

  VkInstance CreateInstance(bool &a_enableValidationLayers, std::vector<const char *> &a_requestedLayers, std::vector<const char *> &a_instanceExtensions, VkApplicationInfo *appInfo)
  {
    std::vector<const char *> enabledExtensions = a_instanceExtensions;
    std::vector<std::string> supportedLayers;

    auto all_layers_supported = checkLayerSupport(a_requestedLayers, supportedLayers);
    if (a_enableValidationLayers && !supportedLayers.empty())
    {
      uint32_t extensionCount;

      vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
      std::vector<VkExtensionProperties> extensionProperties(extensionCount);
      vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionProperties.data());

      bool foundExtension = false;
      for (VkExtensionProperties prop : extensionProperties)
      {
        if (strcmp(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, prop.extensionName) == 0)
        {
          foundExtension = true;
          break;
        }
      }

      if (!foundExtension)
        RunTimeError("Validation layers requested but extension VK_EXT_DEBUG_REPORT_EXTENSION_NAME not supported\n");

      enabledExtensions.push_back(g_debugReportExtName);
    }

    VkApplicationInfo applicationInfo = {};
    if (appInfo == nullptr)
    {
      applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
      applicationInfo.pApplicationName = "Default App Name";
      applicationInfo.applicationVersion = 0;
      applicationInfo.pEngineName = "DefaultEngine";
      applicationInfo.engineVersion = 0;
      applicationInfo.apiVersion = VK_API_VERSION_1_1;
    }
    else
    {
      applicationInfo.sType = appInfo->sType;
      applicationInfo.pApplicationName = appInfo->pApplicationName;
      applicationInfo.applicationVersion = appInfo->applicationVersion;
      applicationInfo.pEngineName = appInfo->pEngineName;
      applicationInfo.engineVersion = appInfo->engineVersion;
      applicationInfo.pNext = appInfo->pNext;
      applicationInfo.apiVersion = appInfo->apiVersion;
    }

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.flags = 0;
    createInfo.pApplicationInfo = &applicationInfo;

    std::vector<const char *> layer_names;
    if (a_enableValidationLayers && !supportedLayers.empty())
    {
      for (const auto &layer : supportedLayers)
        layer_names.push_back(layer.c_str());

      createInfo.enabledLayerCount = uint32_t(layer_names.size());
      createInfo.ppEnabledLayerNames = layer_names.data();

      VkValidationFeaturesEXT validationFeatures = {};
      validationFeatures.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
      validationFeatures.enabledValidationFeatureCount = 1;
      VkValidationFeatureEnableEXT enabledValidationFeatures[1] = { VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT };
      validationFeatures.pEnabledValidationFeatures = enabledValidationFeatures;

//      validationFeatures.pNext = createInfo.pNext;
      createInfo.pNext = &validationFeatures;
    }
    else
    {
      createInfo.enabledLayerCount = 0;
      createInfo.ppEnabledLayerNames = nullptr;
      a_enableValidationLayers = false;
    }

    createInfo.enabledExtensionCount = uint32_t(enabledExtensions.size());
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();

    VkInstance instance;
    VK_CHECK_RESULT(vkCreateInstance(&createInfo, nullptr, &instance));

    return instance;
  }


  void InitDebugReportCallback(VkInstance a_instance, DebugReportCallbackFuncType a_callback, VkDebugReportCallbackEXT *a_debugReportCallback)
  {
    VkDebugReportCallbackCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    createInfo.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
    createInfo.pfnCallback = a_callback;

    auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(a_instance, "vkCreateDebugReportCallbackEXT");
    if (vkCreateDebugReportCallbackEXT == nullptr)
      RunTimeError("Could not load vkCreateDebugReportCallbackEXT");

    // Create and register callback.
    VK_CHECK_RESULT(vkCreateDebugReportCallbackEXT(a_instance, &createInfo, nullptr, a_debugReportCallback));
  }

  VkPhysicalDevice FindPhysicalDevice(VkInstance a_instance, bool a_printInfo, unsigned a_preferredDeviceId, std::vector<const char *> a_deviceExt)
  {
    uint32_t deviceCount;
    vkEnumeratePhysicalDevices(a_instance, &deviceCount, nullptr);
    if (deviceCount == 0) {
      RunTimeError("vk_utils::FindPhysicalDevice, no Vulkan devices found");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(a_instance, &deviceCount, devices.data());

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    if (a_printInfo)
      std::cout << "FindPhysicalDevice: { " << std::endl;

    VkPhysicalDeviceProperties props;
    VkPhysicalDeviceFeatures features;

    for (int i = 0; i < devices.size(); i++)
    {
      vkGetPhysicalDeviceProperties(devices[i], &props);
      vkGetPhysicalDeviceFeatures(devices[i], &features);

      if (a_printInfo)
        std::cout << "  device " << i << ", name = " << props.deviceName;

      if (i == a_preferredDeviceId)
      {
        if (checkDeviceExtensionSupport(devices[i], a_deviceExt))
        {
          physicalDevice = devices[i];
          std::cout << " <-- (selected)" << std::endl;
        }
        else
        {
          std::cout << " <-- preferred device does not support requested extensions. Trying to find another device..." << std::endl;
        }
      }

      std::cout << std::endl;
    }

    if (a_printInfo)
      std::cout << "}" << std::endl;

    // try to select some device if preferred was not selected
    //
    if (physicalDevice == VK_NULL_HANDLE)
    {
      for (int i = 0; i < devices.size(); ++i)
      {
        if (checkDeviceExtensionSupport(devices[i], a_deviceExt))
        {
          physicalDevice = devices[i];
          break;
        }
      }
    }

    if (physicalDevice == VK_NULL_HANDLE)
      RunTimeError("vk_utils::FindPhysicalDevice, no Vulkan devices supporting requested extensions were found");

    return physicalDevice;
  }

  uint32_t GetQueueFamilyIndex(VkPhysicalDevice a_physicalDevice, int a_bits)
  {
    uint32_t queueFamilyCount;

    vkGetPhysicalDeviceQueueFamilyProperties(a_physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(a_physicalDevice, &queueFamilyCount, queueFamilies.data());

    uint32_t i = 0;
    for (; i < queueFamilies.size(); ++i)
    {
      VkQueueFamilyProperties props = queueFamilies[i];

      if (props.queueCount > 0 && (props.queueFlags & a_bits))
        break;
    }

    if (i == queueFamilies.size())
      RunTimeError(" vk_utils::GetComputeQueueFamilyIndex: could not find a queue family that supports operations");

    return i;
  }


  VkDevice CreateLogicalDevice(VkPhysicalDevice physicalDevice, const std::vector<const char *> &a_enabledLayers,
                               std::vector<const char *> a_extensions, VkPhysicalDeviceFeatures a_deviceFeatures,
                               QueueFID_T &a_queueIDXs, VkQueueFlags requestedQueueTypes, void* pNextFeatures)
  {
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
    const float defaultQueuePriority(0.0f);

    // Graphics queue
    if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
    {
      a_queueIDXs.graphics = GetQueueFamilyIndex(physicalDevice, VK_QUEUE_GRAPHICS_BIT);
      VkDeviceQueueCreateInfo queueInfo{};
      queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueInfo.queueFamilyIndex = a_queueIDXs.graphics;
      queueInfo.queueCount = 1;
      queueInfo.pQueuePriorities = &defaultQueuePriority;
      queueCreateInfos.push_back(queueInfo);
    }
    else
    {
      a_queueIDXs.graphics = VK_NULL_HANDLE;
    }

    // Dedicated compute queue
    if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
    {
      a_queueIDXs.compute = GetQueueFamilyIndex(physicalDevice, VK_QUEUE_COMPUTE_BIT);
      if (a_queueIDXs.compute != a_queueIDXs.graphics || queueCreateInfos.empty())
      {
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = a_queueIDXs.compute;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &defaultQueuePriority;
        queueCreateInfos.push_back(queueInfo);
      }
    }
    else
    {
      a_queueIDXs.compute = a_queueIDXs.graphics;
    }

    // Dedicated transfer queue
    if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
    {
      a_queueIDXs.transfer = GetQueueFamilyIndex(physicalDevice, VK_QUEUE_TRANSFER_BIT);
      if ((a_queueIDXs.transfer != a_queueIDXs.graphics) && (a_queueIDXs.transfer != a_queueIDXs.compute) || queueCreateInfos.empty())
      {
        VkDeviceQueueCreateInfo queueInfo{};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = a_queueIDXs.transfer;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &defaultQueuePriority;
        queueCreateInfos.push_back(queueInfo);
      }
    }
    else
    {
      a_queueIDXs.transfer = a_queueIDXs.graphics;
    }


    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.enabledLayerCount = uint32_t(a_enabledLayers.size());
    deviceCreateInfo.ppEnabledLayerNames = a_enabledLayers.data();
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pEnabledFeatures = &a_deviceFeatures;
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(a_extensions.size());
    deviceCreateInfo.ppEnabledExtensionNames = a_extensions.data();

    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
    if(pNextFeatures)
    {
      physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
      physicalDeviceFeatures2.features = a_deviceFeatures;
      physicalDeviceFeatures2.pNext = pNextFeatures;
      deviceCreateInfo.pEnabledFeatures = nullptr;
      deviceCreateInfo.pNext = &physicalDeviceFeatures2;
    }

    VkDevice device;
    VK_CHECK_RESULT(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device));

    return device;
  }


  uint32_t FindMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice)
  {
    VkPhysicalDeviceMemoryProperties memoryProperties;

    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);

    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
    {
      if ((memoryTypeBits & (1 << i)) && ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
        return i;
    }

    return -1;
  }

  std::vector<std::string> SubgroupOperationToString(VkSubgroupFeatureFlags flags)
  {
    std::vector<std::string> res;
    std::vector<std::pair<VkSubgroupFeatureFlagBits, std::string>> flagBits = {
        {VK_SUBGROUP_FEATURE_BASIC_BIT, "VK_SUBGROUP_FEATURE_BASIC_BIT"},
        {VK_SUBGROUP_FEATURE_VOTE_BIT, "VK_SUBGROUP_FEATURE_VOTE_BIT"},
        {VK_SUBGROUP_FEATURE_ARITHMETIC_BIT, "VK_SUBGROUP_FEATURE_ARITHMETIC_BIT"},
        {VK_SUBGROUP_FEATURE_BALLOT_BIT, "VK_SUBGROUP_FEATURE_BALLOT_BIT"},
        {VK_SUBGROUP_FEATURE_SHUFFLE_BIT, "VK_SUBGROUP_FEATURE_SHUFFLE_BIT"},
        {VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT, "VK_SUBGROUP_FEATURE_SHUFFLE_RELATIVE_BIT"},
        {VK_SUBGROUP_FEATURE_CLUSTERED_BIT, "VK_SUBGROUP_FEATURE_CLUSTERED_BIT"},
        {VK_SUBGROUP_FEATURE_QUAD_BIT, "VK_SUBGROUP_FEATURE_QUAD_BIT"},
        {VK_SUBGROUP_FEATURE_PARTITIONED_BIT_NV, "VK_SUBGROUP_FEATURE_PARTITIONED_BIT_NV"},
    };
    for(const auto& f : flagBits)
    {
      if(flags & f.first)
        res.emplace_back(f.second);
    }
    return res;
  }


  VkMemoryRequirements CreateBuffer(VkDevice a_dev, VkDeviceSize a_size, VkBufferUsageFlags a_usageFlags, VkBuffer &a_buf)
  {
    assert(a_dev != VK_NULL_HANDLE);

    if (a_buf != VK_NULL_HANDLE)
    {
      vkDestroyBuffer(a_dev, a_buf, VK_NULL_HANDLE);
    }

    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size  = a_size;
    bufferCreateInfo.usage = a_usageFlags;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK_RESULT(vkCreateBuffer(a_dev, &bufferCreateInfo, VK_NULL_HANDLE, &a_buf));

    VkMemoryRequirements result;
    vkGetBufferMemoryRequirements(a_dev, a_buf, &result);

    return result;
  }

  size_t Padding(size_t a_size, size_t a_alignment)
  {
    if (a_size % a_alignment == 0)
      return a_size;
    else
    {
      size_t sizeCut = a_size - (a_size % a_alignment);
      return sizeCut + a_alignment;
    }
  }

  std::vector<size_t> AssignMemOffsetsWithPadding(const std::vector<VkMemoryRequirements> &a_memInfos)
  {
    assert(!a_memInfos.empty());

    std::vector<VkDeviceSize> mem_offsets;
    size_t currOffset = 0;
    for (size_t i = 0; i < a_memInfos.size() - 1; i++)
    {
      mem_offsets.push_back(currOffset);
      currOffset += vk_utils::Padding(a_memInfos[i].size, a_memInfos[i + 1].alignment);
    }

    // put mem offset for last element of 'a_memInfos'
    //
    size_t last = a_memInfos.size() - 1;
    mem_offsets.push_back(currOffset);
    currOffset += vk_utils::Padding(a_memInfos[last].size, a_memInfos[last].alignment);

    // put total mem amount in last vector element
    //
    mem_offsets.push_back(currOffset);
    return mem_offsets;
  }

  VkDeviceMemory AllocateAndBindWithPadding(VkDevice a_dev, VkPhysicalDevice a_physDev, const std::vector<VkBuffer> &a_buffers, VkMemoryAllocateFlags flags)
  {
    if(a_buffers.empty())
    {
      std::cout << "[AllocateAndBindWithPadding]: error, zero input array" << std::endl;
      return VK_NULL_HANDLE;
    }

    std::vector<VkMemoryRequirements> memInfos(a_buffers.size());
    for(size_t i = 0; i < memInfos.size(); ++i)
    {
      if(a_buffers[i] != VK_NULL_HANDLE)
        vkGetBufferMemoryRequirements(a_dev, a_buffers[i], &memInfos[i]);
      else
      {
        memInfos[i] = memInfos[0];
        memInfos[i].size = 0;
      }
    }
    for(size_t i=1;i<memInfos.size();i++)
    {
      if(memInfos[i].memoryTypeBits != memInfos[0].memoryTypeBits)
      {
        std::cout << "[AllocateAndBindWithPadding]: error, input buffers has different 'memReq.memoryTypeBits'" << std::endl;
        return VK_NULL_HANDLE;
      }
    }

    auto offsets  = AssignMemOffsetsWithPadding(memInfos);
    auto memTotal = offsets[offsets.size() - 1];

    VkDeviceMemory res;
    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext           = nullptr;
    allocateInfo.allocationSize  = memTotal;
    allocateInfo.memoryTypeIndex = vk_utils::FindMemoryType(memInfos[0].memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, a_physDev);

    VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
    if(flags)
    {
      memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
      memoryAllocateFlagsInfo.flags = flags;

      allocateInfo.pNext = &memoryAllocateFlagsInfo;
    }

    VK_CHECK_RESULT(vkAllocateMemory(a_dev, &allocateInfo, NULL, &res));

    for (size_t i = 0; i < memInfos.size(); i++)
    {
      if(a_buffers[i] != VK_NULL_HANDLE)
        vkBindBufferMemory(a_dev, a_buffers[i], res, offsets[i]);
    }

    return res;
  }

  std::vector<uint32_t> ReadFile(const char *filename)
  {
    FILE *fp = fopen(filename, "rb");
    if (fp == nullptr)
    {
      std::string errorMsg = std::string("[vk_utils::ReadFile]: can't open file ") + std::string(filename);
      RunTimeError(errorMsg.c_str());
    }

    fseek(fp, 0, SEEK_END);
    long filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    long filesizepadded = long(std::ceil(filesize / 4.0)) * 4;

    std::vector<uint32_t> resData(filesizepadded / 4);

    char *str = (char *)resData.data();
    fread(str, filesize, sizeof(char), fp);
    fclose(fp);

    for (int i = filesize; i < filesizepadded; i++)
      str[i] = 0;

    return resData;
  }

  VkShaderModule CreateShaderModule(VkDevice a_device, const std::vector<uint32_t> &code)
  {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = code.data();

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(a_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
      RunTimeError("[CreateShaderModule]: failed to create shader module!");

    return shaderModule;
  }

  VkDescriptorSetLayout createDescriptorSetLayout(VkDevice a_device, const std::vector<VkDescriptorType> &a_descrTypes)
  {
    VkDescriptorSetLayout layout;

    std::vector<VkDescriptorSetLayoutBinding> bindings(a_descrTypes.size());
    for(size_t i = 0; i < a_descrTypes.size(); ++i)
    {
      VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
      descriptorSetLayoutBinding.binding = i;
      descriptorSetLayoutBinding.descriptorType = a_descrTypes[i];
      descriptorSetLayoutBinding.descriptorCount = 1;
      descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

      bindings[i] = descriptorSetLayoutBinding;
    }

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
    descriptorSetLayoutCreateInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorSetLayoutCreateInfo.bindingCount = bindings.size();
    descriptorSetLayoutCreateInfo.pBindings    = bindings.data();
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(a_device, &descriptorSetLayoutCreateInfo, nullptr, &layout));

    return layout;
  }

  VkDescriptorPool createDescriptorPool(VkDevice a_device, const std::vector<VkDescriptorType> &a_descrTypes,
                                        const std::vector<uint32_t> &a_descrTypeCounts, unsigned a_maxSets)
  {
    assert(a_descrTypes.size() == a_descrTypeCounts.size());

    VkDescriptorPool pool;

    std::vector<VkDescriptorPoolSize> poolSizes(a_descrTypes.size());
    for(size_t i = 0; i < a_descrTypes.size(); ++i)
    {
      VkDescriptorPoolSize descriptorPoolSize = {};
      descriptorPoolSize.type = a_descrTypes[i];
      descriptorPoolSize.descriptorCount = a_descrTypeCounts[i];

      poolSizes[i] = descriptorPoolSize;
    }

    VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
    descriptorPoolCreateInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolCreateInfo.maxSets       = a_maxSets;
    descriptorPoolCreateInfo.poolSizeCount = poolSizes.size();
    descriptorPoolCreateInfo.pPoolSizes    = poolSizes.data();

    VK_CHECK_RESULT(vkCreateDescriptorPool(a_device, &descriptorPoolCreateInfo, nullptr, &pool));

    return pool;
  }

  VkDescriptorSet createDescriptorSet(VkDevice a_device, VkDescriptorSetLayout a_pDSLayout, VkDescriptorPool a_pDSPool,
                                      const std::vector<VkDescriptorBufferInfo> &bufInfos)
  {
    VkDescriptorSet set;

    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
    descriptorSetAllocateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptorSetAllocateInfo.descriptorPool     = a_pDSPool;
    descriptorSetAllocateInfo.descriptorSetCount = 1;
    descriptorSetAllocateInfo.pSetLayouts        = &a_pDSLayout;

    VK_CHECK_RESULT(vkAllocateDescriptorSets(a_device, &descriptorSetAllocateInfo, &set));

    std::vector<VkWriteDescriptorSet> writeSets(bufInfos.size());
    for(size_t i = 0; i < bufInfos.size(); ++i)
    {
      VkWriteDescriptorSet writeDescriptorSet = {};
      writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      writeDescriptorSet.dstSet = set;
      writeDescriptorSet.dstBinding = i;
      writeDescriptorSet.descriptorCount = 1;
      writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      writeDescriptorSet.pBufferInfo = &bufInfos[i];

      writeSets[i] = writeDescriptorSet;
    }

    vkUpdateDescriptorSets(a_device, writeSets.size(), writeSets.data(), 0, nullptr);

    return set;
  }

  VkCommandBuffer createCommandBuffer(VkDevice a_device, VkCommandPool a_pool)
  {
    VkCommandBuffer cmd;
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
    commandBufferAllocateInfo.sType       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.commandPool = a_pool;
    commandBufferAllocateInfo.level       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1;
    VK_CHECK_RESULT(vkAllocateCommandBuffers(a_device, &commandBufferAllocateInfo, &cmd));

    return cmd;
  }

  void createStagingBuffer(VkDevice a_device, VkPhysicalDevice a_physDevice, const size_t a_bufferSize,
                           VkBuffer* a_pBuffer, VkDeviceMemory* a_pBufferMemory)
  {

    VkBufferCreateInfo bufferCreateInfo = {};
    bufferCreateInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size        = a_bufferSize;
    bufferCreateInfo.usage       = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK_RESULT(vkCreateBuffer(a_device, &bufferCreateInfo, nullptr, a_pBuffer));

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(a_device, (*a_pBuffer), &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo = {};
    allocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.allocationSize  = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = vk_utils::FindMemoryType(memoryRequirements.memoryTypeBits,
                                                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                                                            a_physDevice);

    VK_CHECK_RESULT(vkAllocateMemory(a_device, &allocateInfo, nullptr, a_pBufferMemory));

    VK_CHECK_RESULT(vkBindBufferMemory(a_device, (*a_pBuffer), (*a_pBufferMemory), 0));
  }

}

