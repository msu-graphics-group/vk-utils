#include "vk_context.h"
#include "vk_utils.h"
#include "vk_buffers.h"
#include "vk_images.h"

#include <cstring>
#include <cassert>
#include <iostream>

#include <cmath>
#include <cassert>
#include <string>
#include <set>

#include <algorithm>
#ifdef WIN32
#undef min
#undef max
#endif 

vk_utils::VulkanContext g_ctx;

std::set<std::string> get_supported_extensions(VkPhysicalDevice a_physDev) 
{
  uint32_t count;
  vkEnumerateDeviceExtensionProperties(a_physDev, nullptr, &count, nullptr); //get number of extensions
  std::vector<VkExtensionProperties> extensions(count);
  vkEnumerateDeviceExtensionProperties(a_physDev, nullptr, &count, extensions.data()); //populate buffer
  std::set<std::string> results;
  for (const auto& extension : extensions)
    results.insert(extension.extensionName);
  return results;
}

bool vk_utils::globalContextIsInitialized(const std::vector<const char*>& requiredExtensions)
{
  // todo: if initialized check that requiredExtensions were actually initialized
  // if context is initialized but device featurea are wrong, print error and exit
  return (g_ctx.instance != VK_NULL_HANDLE) && (g_ctx.physicalDevice != VK_NULL_HANDLE) && (g_ctx.device != VK_NULL_HANDLE);
}

vk_utils::VulkanContext vk_utils::globalContextGet(bool enableValidationLayers, unsigned int a_preferredDeviceId)
{
  if(globalContextIsInitialized())
    return g_ctx;
  else
    return globalContextInit(std::vector<const char*>(), enableValidationLayers, a_preferredDeviceId);
}

struct RTXDeviceFeatures
{
  VkPhysicalDeviceAccelerationStructureFeaturesKHR m_enabledAccelStructFeatures{};
  VkPhysicalDeviceBufferDeviceAddressFeatures      m_enabledDeviceAddressFeatures{};
  VkPhysicalDeviceRayQueryFeaturesKHR              m_enabledRayQueryFeatures;
};

static RTXDeviceFeatures SetupRTXFeatures(VkPhysicalDevice a_physDev)
{
  static RTXDeviceFeatures g_rtFeatures;

  g_rtFeatures.m_enabledRayQueryFeatures.sType    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
  g_rtFeatures.m_enabledRayQueryFeatures.rayQuery = VK_TRUE;

  g_rtFeatures.m_enabledDeviceAddressFeatures.sType               = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
  g_rtFeatures.m_enabledDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
  g_rtFeatures.m_enabledDeviceAddressFeatures.pNext               = &g_rtFeatures.m_enabledRayQueryFeatures;

  g_rtFeatures.m_enabledAccelStructFeatures.sType                 = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
  g_rtFeatures.m_enabledAccelStructFeatures.accelerationStructure = VK_TRUE;
  g_rtFeatures.m_enabledAccelStructFeatures.pNext                 = &g_rtFeatures.m_enabledDeviceAddressFeatures;

  return g_rtFeatures;
}

vk_utils::VulkanContext vk_utils::globalContextInit(const std::vector<const char*>& requiredExtensions, bool enableValidationLayers, unsigned int a_preferredDeviceId)
{
  if(globalContextIsInitialized(requiredExtensions))
    return g_ctx;

  std::vector<const char*> enabledLayers;
  std::vector<const char*> extensions;
  enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
  enabledLayers.push_back("VK_LAYER_LUNARG_standard_validation");
  VK_CHECK_RESULT(volkInitialize());
  g_ctx.instance = vk_utils::createInstance(enableValidationLayers, enabledLayers, extensions);
  volkLoadInstance(g_ctx.instance);

  g_ctx.physicalDevice = vk_utils::findPhysicalDevice(g_ctx.instance, true, a_preferredDeviceId);
  auto queueComputeFID = vk_utils::getQueueFamilyIndex(g_ctx.physicalDevice, VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT);

  const std::set<std::string> supportedExtensions = get_supported_extensions(g_ctx.physicalDevice);
  //std::cout << "extensions num = " << supportedExtensions.size() << std::endl;

  const bool supportRayQuery = (supportedExtensions.find("VK_KHR_acceleration_structure") != supportedExtensions.end()) && 
                               (supportedExtensions.find("VK_KHR_ray_query")              != supportedExtensions.end());

  const bool supportBindless = (supportedExtensions.find("VK_EXT_descriptor_indexing") != supportedExtensions.end());

  VkPhysicalDeviceVariablePointersFeatures varPointersQuestion = {};
  varPointersQuestion.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES;

  VkPhysicalDeviceFeatures2 deviceFeaturesQuestion = {};
  deviceFeaturesQuestion.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
  deviceFeaturesQuestion.pNext = &varPointersQuestion;
  vkGetPhysicalDeviceFeatures2(g_ctx.physicalDevice, &deviceFeaturesQuestion);
  
  // query for subgroup operations
  //
  {
    VkPhysicalDeviceSubgroupProperties subgroupProperties;
    subgroupProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;
    subgroupProperties.pNext = NULL;
    
    VkPhysicalDeviceProperties2 physicalDeviceProperties;
    physicalDeviceProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    physicalDeviceProperties.pNext = &subgroupProperties;
    
    vkGetPhysicalDeviceProperties2(g_ctx.physicalDevice, &physicalDeviceProperties);
    g_ctx.subgroupProps = subgroupProperties;
    //g_ctx.subgroupArithSupport = (subgroupProperties.supportedOperations & VK_SUBGROUP_FEATURE_ARITHMETIC_BIT);
    //g_ctx.subgroupSize         = subgroupProperties.subgroupSize;
  }

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  // query features for RTX
  //
  RTXDeviceFeatures rtxFeatures;
  if(supportRayQuery)
    rtxFeatures = SetupRTXFeatures(g_ctx.physicalDevice);

  VkPhysicalDeviceVariablePointersFeatures varPointers = {};
  varPointers.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VARIABLE_POINTERS_FEATURES;
  varPointers.pNext = supportRayQuery ? &rtxFeatures.m_enabledAccelStructFeatures : nullptr;
  varPointers.variablePointers              = varPointersQuestion.variablePointers;
  varPointers.variablePointersStorageBuffer = varPointersQuestion.variablePointersStorageBuffer;

  



  VkPhysicalDeviceDescriptorIndexingFeatures indexingFeatures{};
  indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
  indexingFeatures.pNext = &varPointers;
  indexingFeatures.shaderSampledImageArrayNonUniformIndexing = supportBindless ? VK_TRUE : VK_FALSE;
  indexingFeatures.runtimeDescriptorArray                    = supportBindless ? VK_TRUE : VK_FALSE;

  // query features for shaderInt8
  //
  VkPhysicalDeviceShaderFloat16Int8Features features = {};
  features.sType      = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES;
  features.shaderInt8 = deviceFeaturesQuestion.features.shaderInt16;
  features.pNext      = &indexingFeatures;

  std::vector<const char*> validationLayers, deviceExtensions;
  VkPhysicalDeviceFeatures enabledDeviceFeatures = {};
  enabledDeviceFeatures.shaderInt64   = deviceFeaturesQuestion.features.shaderInt64;
  enabledDeviceFeatures.shaderFloat64 = deviceFeaturesQuestion.features.shaderFloat64;
  
  vk_utils::QueueFID_T fIDs = {};
  
  if(requiredExtensions.size() == 0 && supportRayQuery) // TODO: remove this or work around
  {
    // Required by VK_KHR_RAY_QUERY
    deviceExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    deviceExtensions.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
    deviceExtensions.push_back("VK_KHR_spirv_1_4");
    deviceExtensions.push_back("VK_KHR_shader_float_controls");  
    // Required by VK_KHR_acceleration_structure
    deviceExtensions.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
    deviceExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    deviceExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
    // // Required by VK_KHR_ray_tracing_pipeline
    // m_deviceExtensions.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
    // // Required by VK_KHR_spirv_1_4
    // m_deviceExtensions.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
  }
  
  if(supportedExtensions.find("VK_KHR_shader_non_semantic_info") != supportedExtensions.end())
    deviceExtensions.push_back("VK_KHR_shader_non_semantic_info");
  if(supportedExtensions.find("VK_KHR_shader_float16_int8") != supportedExtensions.end())
    deviceExtensions.push_back("VK_KHR_shader_float16_int8");
  if(supportedExtensions.find("VK_KHR_variable_pointers") != supportedExtensions.end())  
    deviceExtensions.push_back("VK_KHR_variable_pointers");
  if(supportedExtensions.find("VK_EXT_descriptor_indexing") != supportedExtensions.end())
    deviceExtensions.push_back("VK_EXT_descriptor_indexing");
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  fIDs.compute = queueComputeFID;
  g_ctx.device = vk_utils::createLogicalDevice(g_ctx.physicalDevice, validationLayers, deviceExtensions, enabledDeviceFeatures,
                                               fIDs, VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT, &features);
  volkLoadDevice(g_ctx.device);                                            
  g_ctx.commandPool = vk_utils::createCommandPool(g_ctx.device, fIDs.compute, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

  // (2) initialize vulkan helpers
  //  
  {
    auto queueComputeFID = vk_utils::getQueueFamilyIndex(g_ctx.physicalDevice, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);
    vkGetDeviceQueue(g_ctx.device, queueComputeFID, 0, &g_ctx.computeQueue);
    vkGetDeviceQueue(g_ctx.device, queueComputeFID, 0, &g_ctx.transferQueue);
  }

  g_ctx.pCopyHelper       = std::make_shared<vk_utils::SimpleCopyHelper>(g_ctx.physicalDevice, g_ctx.device, g_ctx.transferQueue, queueComputeFID, 64*1024*1024); // TODO, select PinPong Helper by default!
  g_ctx.pAllocatorSpecial = vk_utils::CreateMemoryAlloc_Special(g_ctx.device, g_ctx.physicalDevice);
  {
    vk_utils::MemAllocInfo tempMemoryAllocInfo;
    tempMemoryAllocInfo.memUsage = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; // TODO, selecty depending on device and sample/application (???)
    VkBuffer tempBuffer = vk_utils::createBuffer(g_ctx.device, size_t(4*2048*2048)*sizeof(int), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT); // reserve 64 MB for temp buffers
    auto     tempImg    = vk_utils::createImg(g_ctx.device, 2048, 2048, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);   // reserve 64 MB for temp buffers
    g_ctx.pAllocatorSpecial->Allocate(tempMemoryAllocInfo, {tempBuffer});
    g_ctx.pAllocatorSpecial->Allocate(tempMemoryAllocInfo, {tempImg.image});
    vkDestroyBuffer(g_ctx.device, tempBuffer, nullptr);
    vkDestroyImageView(g_ctx.device, tempImg.view, nullptr);
    vkDestroyImage    (g_ctx.device, tempImg.image, nullptr);
  }
  return g_ctx;
}

void vk_utils::globalContextDestroy()
{
  if(g_ctx.device == VK_NULL_HANDLE)
    return;
 
  if(g_ctx.pAllocatorSpecial != nullptr)
    g_ctx.pAllocatorSpecial->FreeAllMemory();

  if(g_ctx.pAllocatorCommon != nullptr)
    g_ctx.pAllocatorCommon->FreeAllMemory();
  
  g_ctx.pAllocatorSpecial = nullptr;
  g_ctx.pAllocatorCommon  = nullptr;
  g_ctx.pCopyHelper       = nullptr;

  vkDestroyCommandPool(g_ctx.device, g_ctx.commandPool, nullptr);
  vkDestroyDevice(g_ctx.device, nullptr);
  vkDestroyInstance(g_ctx.instance, nullptr);
  g_ctx.device         = VK_NULL_HANDLE;
  g_ctx.physicalDevice = VK_NULL_HANDLE;
  g_ctx.instance       = VK_NULL_HANDLE;
}
