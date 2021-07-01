#ifndef VK_SORT_VK_UTILS_H
#define VK_SORT_VK_UTILS_H

#include "volk.h"
#include <string>
#include <vector>
#include <cassert>

namespace vk_utils
{
  constexpr uint64_t FENCE_TIMEOUT = 100000000000l;

  struct QueueFID_T
  {
    uint32_t graphics;
    uint32_t compute;
    uint32_t transfer;
  };

  VkInstance createInstance(bool &a_enableValidationLayers, std::vector<const char *> &a_requestedLayers,
                            std::vector<const char *> &a_instanceExtensions, VkApplicationInfo* appInfo = nullptr);

  VkPhysicalDevice findPhysicalDevice(VkInstance a_instance, bool a_printInfo, unsigned a_preferredDeviceId, std::vector<const char *> a_deviceExt = {});

  VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, const std::vector<const char *>& a_enabledLayers,
                               std::vector<const char *> a_extensions, VkPhysicalDeviceFeatures a_deviceFeatures,
                               QueueFID_T &a_queueIDXs, VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT,
                               void* pNextFeatures = nullptr);
  uint32_t findMemoryType(uint32_t memoryTypeBits, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice);

  std::vector<std::string> subgroupOperationToString(VkSubgroupFeatureFlags flags);

  size_t getPaddedSize(size_t a_size, size_t a_alignment);
  uint32_t getSBTAlignedSize(uint32_t value, uint32_t alignment);

  std::vector<uint32_t> readSPVFile(const char* filename);
  VkShaderModule createShaderModule(VkDevice a_device, const std::vector<uint32_t>& code);
  VkPipelineShaderStageCreateInfo loadShader(VkDevice a_device, const std::string& fileName, VkShaderStageFlagBits stage,
                                             std::vector<VkShaderModule> &modules);

  VkDescriptorSetLayout createDescriptorSetLayout(VkDevice a_device, const std::vector<VkDescriptorType> &a_descrTypes);
  VkDescriptorPool createDescriptorPool(VkDevice a_device, const std::vector<VkDescriptorType> &a_descrTypes,
                                        const std::vector<uint32_t> &a_descrTypeCounts, unsigned a_maxSets);
  VkDescriptorSet createDescriptorSet(VkDevice a_device, VkDescriptorSetLayout a_pDSLayout, VkDescriptorPool a_pDSPool,
                                      const std::vector<VkDescriptorBufferInfo> &bufInfos);

  VkCommandBuffer createCommandBuffer(VkDevice a_device, VkCommandPool a_pool);

  void createStagingBuffer(VkDevice a_device, VkPhysicalDevice a_physDevice, const size_t a_bufferSize,
                           VkBuffer* a_pBuffer, VkDeviceMemory* a_pBufferMemory);

  VkMemoryRequirements createBuffer(VkDevice a_dev, VkDeviceSize a_size, VkBufferUsageFlags a_usageFlags, VkBuffer &a_buf);
  VkDeviceMemory allocateAndBindWithPadding(VkDevice a_dev, VkPhysicalDevice a_physDev, const std::vector<VkBuffer> &a_buffers,
                                            VkMemoryAllocateFlags flags = {});

  std::string errorString(VkResult errorCode);

  typedef VkBool32 (VKAPI_PTR *DebugReportCallbackFuncType)(VkDebugReportFlagsEXT      flags,
                                                            VkDebugReportObjectTypeEXT objectType,
                                                            uint64_t                   object,
                                                            size_t                     location,
                                                            int32_t                    messageCode,
                                                            const char*                pLayerPrefix,
                                                            const char*                pMessage,
                                                            void*                      pUserData);

  void initDebugReportCallback(VkInstance a_instance, DebugReportCallbackFuncType a_callback, VkDebugReportCallbackEXT* a_debugReportCallback);
}

#define VK_CHECK_RESULT(f) 													           \
{																										           \
    VkResult __vk_check_result = (f);													 \
    if (__vk_check_result != VK_SUCCESS)											 \
    {																								           \
        printf("Fatal : VkResult is %s in %s at line %d\n",    \
               vk_utils::errorString(__vk_check_result).c_str(),  __FILE__, __LINE__); \
        assert(__vk_check_result == VK_SUCCESS);							 \
    }																								           \
}

#endif //VK_SORT_VK_UTILS_H
