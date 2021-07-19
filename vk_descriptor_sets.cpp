#include "vk_descriptor_sets.h"
#include "vk_utils.h"

namespace vk_utils
{
  VkDescriptorSetLayout createDescriptorSetLayout(VkDevice a_device, const std::vector<std::pair<VkDescriptorType, uint32_t>> &a_descrTypes)
  {
    VkDescriptorSetLayout layout;

    std::vector<VkDescriptorSetLayoutBinding> bindings(a_descrTypes.size());
    for(size_t i = 0; i < a_descrTypes.size(); ++i)
    {
      VkDescriptorSetLayoutBinding descriptorSetLayoutBinding = {};
      descriptorSetLayoutBinding.binding = i;
      descriptorSetLayoutBinding.descriptorType  = a_descrTypes[i].first;
      descriptorSetLayoutBinding.descriptorCount = a_descrTypes[i].second;
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
}
