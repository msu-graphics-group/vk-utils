#ifndef CHIMERA_VK_RESOURCE_ALLOC_H
#define CHIMERA_VK_RESOURCE_ALLOC_H

#include "vk_include.h"

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
    VkMemoryRequirements  memReq{0, 0, 0};
    VkMemoryPropertyFlags memProps{0};
    VkMemoryAllocateFlags allocateFlags{0};
  };

  struct IMemoryAlloc
  {
    virtual uint32_t Allocate(const MemAllocInfo& a_allocInfo, VkResult* a_pResult = nullptr) = 0;

    virtual void Free(uint32_t a_memBlockId) = 0;

    virtual MemoryBlock GetMemoryBlockInfo(uint32_t a_memBlockId) const = 0;

    virtual void* Map(uint32_t a_memBlockId, VkDeviceSize a_offset = 0, VkDeviceSize a_size = VK_WHOLE_SIZE,
      VkResult* a_pResult = nullptr) = 0;

    virtual void Unmap(uint32_t a_memBlockId) = 0;

    virtual ~IMemoryAlloc() = default;
  };


}

#endif// CHIMERA_VK_RESOURCE_ALLOC_H
