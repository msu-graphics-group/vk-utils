#ifndef VKUTILS_VK_ALLOC_SIMPLE_H
#define VKUTILS_VK_ALLOC_SIMPLE_H


#include "vk_alloc.h"
#include "external/vk_mem_alloc.h"
#include <vector>

namespace vk_utils
{
  struct MemoryAlloc_Simple : IMemoryAlloc
  {
    MemoryAlloc_Simple(VkDevice a_device, VkPhysicalDevice a_physicalDevice);

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
    VkPhysicalDeviceMemoryProperties m_physicalMemoryProps;

    std::vector<MemoryBlock> m_allocations;
  };
}

#endif// VKUTILS_VK_ALLOC_SIMPLE_H
