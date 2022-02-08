#ifndef VKUTILS_VK_RESOURCE_ALLOC_H
#define VKUTILS_VK_RESOURCE_ALLOC_H

#include "vk_include.h"
#include "vk_copy.h"
#include "external/samplers_vk.h"
#include <memory>
#include <unordered_map>

namespace vk_utils
{
  struct MemoryBlock
  {
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkDeviceSize   offset = 0u;
    VkDeviceSize   size   = 0u;
  };

  struct MemAllocInfo
  {
    VkMemoryRequirements  memReq {0, 0, 0};
    VkMemoryPropertyFlags memProps {0};
    VkMemoryAllocateFlags allocateFlags {0};
    VkImage               dedicated_image  {VK_NULL_HANDLE};
    VkBuffer              dedicated_buffer {VK_NULL_HANDLE};
  };

  struct IMemoryAlloc
  {
    virtual uint32_t Allocate(const MemAllocInfo& a_allocInfo) = 0;
    virtual uint32_t Allocate(const MemAllocInfo& a_allocInfoBuffers, const std::vector<VkBuffer> &a_buffers) = 0;
    virtual uint32_t Allocate(const MemAllocInfo& a_allocInfoImages, const std::vector<VkImage> &a_images) = 0;

    virtual void Free(uint32_t a_memBlockId) = 0;

    virtual void FreeAllMemory() = 0;

    virtual MemoryBlock GetMemoryBlock(uint32_t a_memBlockId) const = 0;

    virtual void* Map(uint32_t a_memBlockId, VkDeviceSize a_offset, VkDeviceSize a_size) = 0;

    virtual void Unmap(uint32_t a_memBlockId) = 0;

    virtual VkDevice GetDevice() const = 0;

    virtual VkPhysicalDevice GetPhysicalDevice() const = 0;

    virtual ~IMemoryAlloc() = default;
  };


  std::shared_ptr<IMemoryAlloc> CreateMemoryAlloc_VMA(VkInstance a_instance, VkDevice a_device, VkPhysicalDevice a_physicalDevice,
    VkFlags a_flags = 0, uint32_t a_vkAPIVersion = VK_API_VERSION_1_1);
  std::shared_ptr<IMemoryAlloc> CreateMemoryAlloc_Simple(VkDevice a_device, VkPhysicalDevice a_physicalDevice);
  std::shared_ptr<IMemoryAlloc> CreateMemoryAlloc_Special(VkDevice a_device, VkPhysicalDevice a_physicalDevice);

  struct VulkanTexture
  {
    uint32_t resource_id = UINT32_MAX;
    VkImage image        = VK_NULL_HANDLE;
    VkDescriptorImageInfo descriptor{VK_NULL_HANDLE, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED};
  };

  struct IResourceManager
  {
    virtual VkBuffer CreateBuffer(VkDeviceSize a_size, VkBufferUsageFlags a_usage,
      VkMemoryPropertyFlags a_memProps, VkMemoryAllocateFlags flags) = 0;
    virtual VkBuffer CreateBuffer(const void* a_data, VkDeviceSize a_size, VkBufferUsageFlags a_usage,
      VkMemoryPropertyFlags a_memProps, VkMemoryAllocateFlags flags) = 0;
    virtual std::vector<VkBuffer> CreateBuffers(const std::vector<VkDeviceSize> &a_sizes, const std::vector<VkBufferUsageFlags> &a_usages,
      VkMemoryPropertyFlags a_memProps, VkMemoryAllocateFlags flags) = 0;
    virtual std::vector<VkBuffer> CreateBuffers(const std::vector<void*> &a_data, const std::vector<VkDeviceSize> &a_sizes,
      const std::vector<VkBufferUsageFlags> &a_usages, VkMemoryPropertyFlags a_memProps, VkMemoryAllocateFlags flags)  = 0;

    virtual VkImage CreateImage(const VkImageCreateInfo& a_createInfo) = 0;
    virtual VkImage CreateImage(uint32_t a_width, uint32_t a_height, VkFormat a_format, VkImageUsageFlags a_usage, uint32_t a_mipLvls) = 0;
    virtual VkImage CreateImage(const void* a_data, uint32_t a_width, uint32_t a_height, VkFormat a_format, VkImageUsageFlags a_usage,
      VkImageLayout a_finalLayout, uint32_t a_mipLvls) = 0;

    virtual std::vector<VkImage> CreateImages(const std::vector<VkImageCreateInfo>& a_createInfos) = 0;

    virtual VulkanTexture CreateTexture(const VkImageCreateInfo& a_createInfo, VkImageViewCreateInfo& a_imgViewCreateInfo) = 0;
    virtual VulkanTexture CreateTexture(const VkImageCreateInfo& a_createInfo, VkImageViewCreateInfo& a_imgViewCreateInfo,
      const VkSamplerCreateInfo& a_samplerCreateInfo) = 0;

    virtual std::vector<VulkanTexture> CreateTextures(const std::vector<VkImageCreateInfo>& a_createInfos,
                                                      std::vector<VkImageViewCreateInfo>& a_imgViewCreateInfos) = 0;
    virtual std::vector<VulkanTexture> CreateTextures(const std::vector<VkImageCreateInfo>& a_createInfos,
                                                      std::vector<VkImageViewCreateInfo>& a_imgViewCreateInfos,
                                                      const std::vector<VkSamplerCreateInfo>& a_samplerCreateInfos) = 0;

    virtual VkSampler CreateSampler(const VkSamplerCreateInfo& a_samplerCreateInfo) = 0;

    virtual void DestroyBuffer(VkBuffer &a_buffer) = 0;
    virtual void DestroyImage(VkImage &a_image) = 0;
    virtual void DestroyTexture(VulkanTexture &a_texture) = 0;
    virtual void DestroySampler(VkSampler &a_sampler) = 0;

    // create accel struct ?
    // map, unmap
  };

  struct ResourceManager : IResourceManager
  {
    ResourceManager(VkDevice a_device, VkPhysicalDevice a_physicalDevice, IMemoryAlloc* a_pAlloc, ICopyEngine* a_pCopy);

    ResourceManager(ResourceManager const&) = delete;
    ResourceManager& operator=(ResourceManager const&) = delete;

    virtual ~ResourceManager() { Cleanup(); };

    void Cleanup();

    IMemoryAlloc* GetAllocator() { return m_pAlloc.get(); }
    ICopyEngine*  GetCopyEngine() {return m_pCopy.get(); }

    VkBuffer CreateBuffer(VkDeviceSize a_size, VkBufferUsageFlags a_usage,  VkMemoryPropertyFlags a_memProps, VkMemoryAllocateFlags flags) override;
    VkBuffer CreateBuffer(const void* a_data, VkDeviceSize a_size, VkBufferUsageFlags a_usage,
      VkMemoryPropertyFlags a_memProps, VkMemoryAllocateFlags flags) override;
    std::vector<VkBuffer> CreateBuffers(const std::vector<VkDeviceSize> &a_sizes, const std::vector<VkBufferUsageFlags> &a_usages,
      VkMemoryPropertyFlags a_memProps, VkMemoryAllocateFlags flags) override;
    std::vector<VkBuffer> CreateBuffers(const std::vector<void*> &a_data, const std::vector<VkDeviceSize> &a_sizes,
      const std::vector<VkBufferUsageFlags> &a_usages, VkMemoryPropertyFlags a_memProps, VkMemoryAllocateFlags flags) override;

    VkImage CreateImage(const VkImageCreateInfo& a_createInfo) override;
    VkImage CreateImage(uint32_t a_width, uint32_t a_height, VkFormat a_format, VkImageUsageFlags a_usage, uint32_t a_mipLvls) override;
    VkImage CreateImage(const void* a_data, uint32_t a_width, uint32_t a_height, VkFormat a_format, VkImageUsageFlags a_usage,
      VkImageLayout a_finalLayout, uint32_t a_mipLvls) override;

    std::vector<VkImage> CreateImages(const std::vector<VkImageCreateInfo>& a_createInfos) override;

    VulkanTexture CreateTexture(const VkImageCreateInfo& a_createInfo, VkImageViewCreateInfo& a_imgViewCreateInfo) override;
    VulkanTexture CreateTexture(const VkImageCreateInfo& a_createInfo, VkImageViewCreateInfo& a_imgViewCreateInfo,
      const VkSamplerCreateInfo& a_samplerCreateInfo) override;

    std::vector<VulkanTexture> CreateTextures(const std::vector<VkImageCreateInfo>& a_createInfos,
                                              std::vector<VkImageViewCreateInfo>& a_imgViewCreateInfos) override;
    std::vector<VulkanTexture> CreateTextures(const std::vector<VkImageCreateInfo>& a_createInfos,
                                              std::vector<VkImageViewCreateInfo>& a_imgViewCreateInfos,
                                              const std::vector<VkSamplerCreateInfo>& a_samplerCreateInfos) override;

    VkSampler CreateSampler(const VkSamplerCreateInfo& a_samplerCreateInfo) override;

    void DestroyBuffer(VkBuffer &a_buffer) override;
    void DestroyImage(VkImage &a_image) override;
    void DestroyTexture(VulkanTexture &a_texture) override;
    void DestroySampler(VkSampler &a_sampler) override;

    // create accel struct ?
    // map, unmap

  private:
    VkDevice         m_device         = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

    std::shared_ptr<IMemoryAlloc> m_pAlloc;
    std::shared_ptr<ICopyEngine>  m_pCopy;
    vk_utils::SamplerPool m_samplerPool;

    std::unordered_map<VkBuffer, uint32_t> m_bufAllocs;
    std::unordered_map<VkImage,  uint32_t> m_imgAllocs;
  };
}

#endif// VKUTILS_VK_RESOURCE_ALLOC_H
