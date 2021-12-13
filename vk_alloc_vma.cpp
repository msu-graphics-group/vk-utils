#define VMA_IMPLEMENTATION
#include "external/vk_mem_alloc.h"

#include "vk_alloc_vma.h"

namespace vk_utils
{
  static inline VmaMemoryUsage getVMAMemoryUsage(VkMemoryPropertyFlags flags)
  {
    if((flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
      return VMA_MEMORY_USAGE_GPU_ONLY;
    else if((flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
      return VMA_MEMORY_USAGE_CPU_ONLY;
    else if((flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
      return VMA_MEMORY_USAGE_CPU_TO_GPU;
    return VMA_MEMORY_USAGE_UNKNOWN;
  }

  MemoryAlloc_VMA::MemoryAlloc_VMA(VkDevice a_device, VkPhysicalDevice a_physicalDevice, VmaAllocator a_vma) :
    m_device()
  {

  }

  uint32_t MemoryAlloc_VMA::Allocate(const MemAllocInfo& a_allocInfo, VkResult* a_pResult)
  {

  }

  void MemoryAlloc_VMA::Free(uint32_t a_memBlockId)
  {

  }

  MemoryBlock MemoryAlloc_VMA::GetMemoryBlockInfo(uint32_t a_memBlockId) const
  {

  }

  void* MemoryAlloc_VMA::Map(uint32_t a_memBlockId, VkDeviceSize a_offset, VkDeviceSize a_size, VkResult* a_pResult)
  {

  }

  void MemoryAlloc_VMA::Unmap(uint32_t a_memBlockId)
  {

  }
}