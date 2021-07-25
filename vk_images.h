#ifndef VK_SORT_VK_TEXTURES_H
#define VK_SORT_VK_TEXTURES_H

#define USE_VOLK
#include "vk_include.h"

#include <string>
#include <vector>
#include <cassert>

namespace vk_utils
{
  struct VulkanImageMem
  {
    VkFormat format;
    VkImageAspectFlags aspectMask;
    VkImage image;
    VkImageView view;
    VkDeviceMemory mem;
    VkDeviceSize mem_offset;
  };

  VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat> &depthFormats, VkFormat *depthFormat);
  bool isDepthFormat(VkFormat a_format);
  bool isStencilFormat(VkFormat a_format);
  bool isDepthOrStencil(VkFormat a_format);

  void createDepthTexture(VkDevice a_device, VkPhysicalDevice a_physDevice, const uint32_t a_width, const uint32_t a_height,
                          VulkanImageMem *a_depthImg);
  VkMemoryRequirements createImg(VkDevice a_device,
                                 VkFormat a_format,
                                 VkImageUsageFlags a_usage,
                                 VulkanImageMem *a_pImgMem,
                                 uint32_t a_width,
                                 uint32_t a_height,
                                 VkImageCreateInfo *a_pImageCreateInfo = nullptr);

  void createImageViewAndBindMem(VkDevice a_device, VulkanImageMem *a_pImgMem,
                                 VkImageViewCreateInfo *a_pViewCreateInfo = nullptr);

  void createImgAllocAndBind(VkDevice a_device,
                             VkPhysicalDevice a_physicalDevice,
                             VkFormat a_format,
                             VkImageUsageFlags a_usage,
                             VulkanImageMem *a_pImgMem,
                             uint32_t a_width,
                             uint32_t a_height,
                             VkImageCreateInfo *a_pImageCreateInfo = nullptr,
                             VkImageViewCreateInfo *a_pViewCreateInfo = nullptr);


  // *** layout transitions and image barriers ***
  // taken from https://github.com/SaschaWillems/Vulkan
  void setImageLayout(
      VkCommandBuffer cmdBuffer,
      VkImage image,
      VkImageLayout oldImageLayout,
      VkImageLayout newImageLayout,
      VkImageSubresourceRange subresourceRange,
      VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
      VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

  void setImageLayout(
      VkCommandBuffer cmdBuffer,
      VkImage image,
      VkImageAspectFlags aspectMask,
      VkImageLayout oldImageLayout,
      VkImageLayout newImageLayout,
      VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
      VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

  void insertImageMemoryBarrier(
      VkCommandBuffer cmdBuffer,
      VkImage image,
      VkAccessFlags srcAccessMask,
      VkAccessFlags dstAccessMask,
      VkImageLayout oldImageLayout,
      VkImageLayout newImageLayout,
      VkPipelineStageFlags srcStageMask,
      VkPipelineStageFlags dstStageMask,
      VkImageSubresourceRange subresourceRange);
  // ****************
}


#endif //VK_SORT_VK_TEXTURES_H
