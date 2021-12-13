#ifndef CHIMERA_VK_ALLOC_VMA_H
#define CHIMERA_VK_ALLOC_VMA_H

#include "vk_alloc.h"
#include "external/vk_mem_alloc.h"

namespace vk_utils
{
  struct MemoryAlloc_VMA : IMemoryAlloc
  {
    MemoryAlloc_VMA(VkDevice a_device, VkPhysicalDevice a_physicalDevice, VmaAllocator a_vma);

    uint32_t Allocate(const MemAllocInfo& a_allocInfo, VkResult* a_pResult = nullptr) override;

    void Free(uint32_t a_memBlockId) override;

    MemoryBlock GetMemoryBlockInfo(uint32_t a_memBlockId) const override;

    void* Map(uint32_t a_memBlockId, VkDeviceSize a_offset = 0, VkDeviceSize a_size = VK_WHOLE_SIZE,
      VkResult* a_pResult = nullptr) override;

    void Unmap(uint32_t a_memBlockId) override;

  private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physDevice = VK_NULL_HANDLE;

    VmaAllocator m_vma{0};
  };
}

#endif// CHIMERA_VK_ALLOC_VMA_H
