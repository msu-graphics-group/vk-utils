#ifndef VKUTILS_VK_RESOURCE_ALLOC_H
#define VKUTILS_VK_RESOURCE_ALLOC_H

#include "vk_include.h"
#include "vk_copy.h"
#include <memory>
#include <unordered_map>

namespace vk_utils
{
  struct MemoryBlock
  {
    VkDeviceMemory memory;
    VkDeviceSize   offset;
    VkDeviceSize   size;
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

    virtual void* Map(uint32_t a_memBlockId, VkDeviceSize a_offset = 0, VkDeviceSize a_size = VK_WHOLE_SIZE) = 0;

    virtual void Unmap(uint32_t a_memBlockId) = 0;

    virtual VkDevice GetDevice() const = 0;

    virtual VkPhysicalDevice GetPhysicalDevice() const = 0;

    virtual ~IMemoryAlloc() = default;
  };


  std::shared_ptr<IMemoryAlloc> CreateMemoryAlloc_VMA(VkInstance a_instance, VkDevice a_device, VkPhysicalDevice a_physicalDevice,
    VkFlags a_flags = 0, uint32_t a_vkAPIVersion = VK_API_VERSION_1_1);
  std::shared_ptr<IMemoryAlloc> CreateMemoryAlloc_Simple(VkDevice a_device, VkPhysicalDevice a_physicalDevice);

  struct VulkanTexture
  {
    uint32_t resource_id = UINT32_MAX;
    VkImage image        = VK_NULL_HANDLE;
    VkDescriptorImageInfo descriptor{VK_NULL_HANDLE, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED};
  };

  struct ResourceManager
  {
    ResourceManager(VkDevice a_device, VkPhysicalDevice a_physicalDevice, IMemoryAlloc* a_pAlloc, ICopyEngine* a_pCopy);

    ResourceManager(ResourceManager const&) = delete;
    ResourceManager& operator=(ResourceManager const&) = delete;

    virtual ~ResourceManager();

    IMemoryAlloc* GetAllocator() { return m_pAlloc.get(); }
    ICopyEngine*  GetCopyEngine() {return m_pCopy.get(); }

    VkBuffer CreateBuffer(VkDeviceSize a_size, VkBufferUsageFlags a_usage, VkMemoryPropertyFlags a_memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VkMemoryAllocateFlags flags = {});
    VkBuffer CreateBuffer(const void* a_data, VkDeviceSize a_size, VkBufferUsageFlags a_usage, VkMemoryPropertyFlags a_memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VkMemoryAllocateFlags flags = {});
    std::vector<VkBuffer> CreateBuffers(const std::vector<void*> &a_data, const std::vector<VkDeviceSize> &a_sizes, const std::vector<VkBufferUsageFlags> &a_usages,
      VkMemoryPropertyFlags a_memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VkMemoryAllocateFlags flags = {});

    VkImage CreateImage(const VkImageCreateInfo& a_createInfo);
    VkImage CreateImage(uint32_t a_width, uint32_t a_height, VkFormat a_format, VkImageUsageFlags a_usage, uint32_t a_mipLvls = 1);
    VkImage CreateImage(const void* a_data, uint32_t a_width, uint32_t a_height, VkFormat a_format, VkImageUsageFlags a_usage, uint32_t a_mipLvls = 1);

    VulkanTexture CreateTexture(const VkImageCreateInfo& a_createInfo, const VkImageViewCreateInfo& a_imgViewCreateInfo);
    VulkanTexture CreateTexture(const VkImageCreateInfo& a_createInfo, const VkImageViewCreateInfo& a_imgViewCreateInfo,
      const VkSamplerCreateInfo& a_samplerCreateInfo);

    // create accel struct ...
    // map, unmap
    // destroy

  private:
    VkDevice         m_device         = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

    std::shared_ptr<IMemoryAlloc> m_pAlloc;
    std::shared_ptr<ICopyEngine>  m_pCopy;

    std::unordered_map<VkBuffer, uint32_t> m_bufAllocs;
    std::unordered_map<VkImage,  uint32_t> m_imgAllocs;
  };
}

#endif// VKUTILS_VK_RESOURCE_ALLOC_H
