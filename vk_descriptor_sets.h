#ifndef VK_SORT_VK_UTILS_H
#define VK_SORT_VK_UTILS_H

#define USE_VOLK
#include "vk_include.h"

#include <string>
#include <vector>
#include <cassert>

namespace vk_utils
{

  VkDescriptorSetLayout createDescriptorSetLayout(VkDevice a_device, const std::vector<std::pair<VkDescriptorType, uint32_t>> &a_descrTypes);
  VkDescriptorPool createDescriptorPool(VkDevice a_device, const std::vector<VkDescriptorType> &a_descrTypes,
                                        const std::vector<uint32_t> &a_descrTypeCounts, unsigned a_maxSets);
  VkDescriptorSet createDescriptorSet(VkDevice a_device, VkDescriptorSetLayout a_pDSLayout, VkDescriptorPool a_pDSPool,
                                      const std::vector<VkDescriptorBufferInfo> &bufInfos);
}

#endif //VK_SORT_VK_UTILS_H
