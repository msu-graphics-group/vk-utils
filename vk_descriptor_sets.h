#ifndef VK_SORT_VK_UTILS_H
#define VK_SORT_VK_UTILS_H

#define USE_VOLK
#include "vk_include.h"

#include <string>
#include <vector>
#include <cassert>
#include <unordered_map>

namespace vk_utils
{

  VkDescriptorSetLayout createDescriptorSetLayout(VkDevice a_device, const std::vector<std::pair<VkDescriptorType, uint32_t>> &a_descrTypes,
                                                  VkShaderStageFlags a_stage = VK_SHADER_STAGE_COMPUTE_BIT);
  VkDescriptorPool createDescriptorPool(VkDevice a_device, const std::vector<std::pair<VkDescriptorType, uint32_t>> &a_descrTypes,
                                        unsigned a_maxSets);
  VkDescriptorSet createDescriptorSet(VkDevice a_device, VkDescriptorSetLayout a_pDSLayout, VkDescriptorPool a_pDSPool,
                                      const std::vector<VkDescriptorBufferInfo> &bufInfos);

  class DescriptorMaker
  {
  public:
    DescriptorMaker(VkDevice a_device, const std::vector<std::pair<VkDescriptorType, uint32_t>> &a_maxDescrTypes, uint32_t a_maxSets);
    ~DescriptorMaker();

    void BindBegin (VkShaderStageFlags a_shaderStage);
    void BindBuffer(uint32_t a_loc, VkBuffer a_buffer, VkBufferView a_buffView = VK_NULL_HANDLE, VkDescriptorType a_bindType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
    void BindAccelStruct(uint32_t a_loc, VkAccelerationStructureKHR a_accStruct, VkDescriptorType a_bindType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR);
    void BindImage (uint32_t a_loc, VkImageView  a_imageView, VkSampler a_sampler = VK_NULL_HANDLE, VkDescriptorType a_bindType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                            VkImageLayout a_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    void BindImageArray (uint32_t a_loc, const std::vector<VkImageView> &a_imageView, const std::vector<VkSampler> &a_sampler,
                                VkDescriptorType a_bindType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                VkImageLayout a_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    void BindEnd (VkDescriptorSet* a_pSet = nullptr, VkDescriptorSetLayout* a_pLayout = nullptr);

//    VkDescriptorSetLayout GetLayout(uint32_t a_setId) const {}
//    VkDescriptorSet       GetSet(uint32_t a_setId)    const {}

    VkDescriptorPool      GetPool() const { return m_pool; }

  private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkDescriptorPool m_pool = VK_NULL_HANDLE;

    struct Handles
    {
      VkBufferView buffView     = VK_NULL_HANDLE;
      VkBuffer     buffer       = VK_NULL_HANDLE;
      std::vector<VkImageView>  imageView;
      std::vector<VkSampler>    imageSampler;
      VkAccelerationStructureKHR accelStruct = VK_NULL_HANDLE;

      VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
      VkImageLayout imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    };

    std::unordered_map<uint32_t, Handles> m_bindings; // shader location to vk handle(s) array

    VkShaderStageFlags m_currentStageFlags = 0u;

//    struct DSData
//    {
//      DSData() { sets.reserve(4); }
//      DSData(VkDescriptorSetLayout a_layout) : layout(a_layout) { sets.reserve(4); }
//      VkDescriptorSetLayout        layout = {};
//      std::vector<VkDescriptorSet> sets;
//    };

//    std::unordered_map<PBKey, DSData> m_descriptors;

  };
}

#endif //VK_SORT_VK_UTILS_H
