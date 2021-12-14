#ifndef VKUTILS_VK_RESOURCE_ALLOC_H
#define VKUTILS_VK_RESOURCE_ALLOC_H

#include "vk_include.h"
#include <memory>

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

    virtual void Free(uint32_t a_memBlockId) = 0;

    virtual void FreeAllMemory() = 0;

    virtual MemoryBlock GetMemoryBlock(uint32_t a_memBlockId) const = 0;

    virtual void* Map(uint32_t a_memBlockId, VkDeviceSize a_offset = 0, VkDeviceSize a_size = VK_WHOLE_SIZE) = 0;

    virtual void Unmap(uint32_t a_memBlockId) = 0;

    virtual VkDevice GetDevice() const;

    virtual VkPhysicalDevice GetPhysicalDevice() const;

    virtual ~IMemoryAlloc() = default;
  };


  std::shared_ptr<IMemoryAlloc> CreateMemoryAlloc_VMA(VkInstance a_instance, VkDevice a_device, VkPhysicalDevice a_physicalDevice,
    VkFlags a_flags, uint32_t a_vkAPIVersion);
  std::shared_ptr<IMemoryAlloc> CreateMemoryAlloc_Simple(VkDevice a_device, VkPhysicalDevice a_physicalDevice);
}

#endif// VKUTILS_VK_RESOURCE_ALLOC_H
