#ifndef VKUTILS_VK_ALLOC_VMA_H
#define VKUTILS_VK_ALLOC_VMA_H

#include "vk_alloc.h"
#include "external/vk_mem_alloc.h"
#include <vector>
#include <unordered_map>

namespace vk_utils
{
  VmaAllocator initVMA(VkInstance a_instance, VkDevice a_device, VkPhysicalDevice a_physicalDevice,
    VkFlags a_flags, uint32_t a_vkAPIVersion);

  struct MemoryAlloc_VMA : IMemoryAlloc
  {
    MemoryAlloc_VMA(VkDevice a_device, VkPhysicalDevice a_physicalDevice, VmaAllocator a_allocator);
    MemoryAlloc_VMA(VkInstance a_instance, VkDevice a_device, VkPhysicalDevice a_physicalDevice,
      VkFlags a_flags = 0, uint32_t a_vkAPIVersion = VK_API_VERSION_1_1);


    uint32_t Allocate(const MemAllocInfo& a_allocInfo) override;
    std::pair<uint32_t, uint32_t> Allocate(const MemAllocInfo& a_allocInfo, const std::vector<VkBuffer> &a_buffers, const std::vector<VkImage> &a_textures) override;

    void Free(uint32_t a_memBlockId) override;
    void FreeAllMemory() override;
    MemoryBlock GetMemoryBlock(uint32_t a_memBlockId) const override;
    void* Map(uint32_t a_memBlockId, VkDeviceSize a_offset = 0, VkDeviceSize a_size = VK_WHOLE_SIZE) override;
    void Unmap(uint32_t a_memBlockId) override;

    VkDevice GetDevice() const override { return m_device; }
    VkPhysicalDevice GetPhysicalDevice() const override { return m_physicalDevice; }

    VmaAllocator GetVMA() {return m_vma; }

  private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

    VmaAllocator m_vma = VK_NULL_HANDLE;

    std::vector<VmaAllocation> m_allocations;
  };

  struct ResourceManager_VMA
  {
    ResourceManager_VMA(VkDevice a_device, VkPhysicalDevice a_physicalDevice, VmaAllocator a_allocator, ICopyEngine* a_pCopy);

    ResourceManager_VMA(ResourceManager_VMA const&) = delete;
    ResourceManager_VMA& operator=(ResourceManager_VMA const&) = delete;

    virtual ~ResourceManager_VMA();

    ICopyEngine*  GetCopyEngine() {return m_pCopy.get(); }

    VkBuffer CreateBuffer(VkDeviceSize a_size, VkBufferUsageFlags a_usage, VkMemoryPropertyFlags a_memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VkMemoryAllocateFlags flags = {});
    VkBuffer CreateBuffer(const void* a_data, VkDeviceSize a_size, VkBufferUsageFlags a_usage, VkMemoryPropertyFlags a_memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VkMemoryAllocateFlags flags = {});
    std::vector<VkBuffer> CreateBuffers(const std::vector<void*> &a_data, const std::vector<VkDeviceSize> &a_sizes, const std::vector<VkBufferUsageFlags> &a_usages,
      VkMemoryPropertyFlags a_memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VkMemoryAllocateFlags flags = {});

    VkImage CreateImage(const VkImageCreateInfo& a_createInfo);
    VkImage CreateImage(uint32_t a_width, uint32_t a_height, VkFormat a_format, VkImageUsageFlags a_usage, uint32_t a_mipLvls = 1);
    VkImage CreateImage(const void* a_data, uint32_t a_width, uint32_t a_height, VkFormat a_format, VkImageUsageFlags a_usage,
      VkImageLayout a_finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, uint32_t a_mipLvls = 1);

    VulkanTexture CreateTexture(const VkImageCreateInfo& a_createInfo, const VkImageViewCreateInfo& a_imgViewCreateInfo);
    VulkanTexture CreateTexture(const VkImageCreateInfo& a_createInfo, const VkImageViewCreateInfo& a_imgViewCreateInfo,
      const VkSamplerCreateInfo& a_samplerCreateInfo);

    // create accel struct ...
    // map, unmap

    void DestroyBuffer(VkBuffer &a_buffer);
    void DestroyImage(VkImage &a_image);
    void DestroyTexture(VulkanTexture &a_texture);

  private:
    VkDevice         m_device         = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

    VmaAllocator m_vma = VK_NULL_HANDLE;
    std::shared_ptr<ICopyEngine>  m_pCopy;

    std::unordered_map<VkBuffer, VmaAllocation> m_bufAllocs;
    std::unordered_map<VkImage,  VmaAllocation> m_imgAllocs;
  };
}

#endif// VKUTILS_VK_ALLOC_VMA_H
