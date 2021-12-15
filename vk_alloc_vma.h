#ifndef VKUTILS_VK_ALLOC_VMA_H
#define VKUTILS_VK_ALLOC_VMA_H

#include "vk_alloc.h"
#include "external/vk_mem_alloc.h"
#include <vector>

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
    void Free(uint32_t a_memBlockId) override;
    void FreeAllMemory() override;
    MemoryBlock GetMemoryBlock(uint32_t a_memBlockId) const override;
    void* Map(uint32_t a_memBlockId, VkDeviceSize a_offset = 0, VkDeviceSize a_size = VK_WHOLE_SIZE) override;
    void Unmap(uint32_t a_memBlockId) override;

    VkDevice GetDevice() const override { return m_device; }
    VkPhysicalDevice GetPhysicalDevice() const override { return m_physicalDevice; }

  private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;

    VmaAllocator m_vma = VK_NULL_HANDLE;

    std::vector<VmaAllocation> m_allocations;
  };
}

#endif// VKUTILS_VK_ALLOC_VMA_H
