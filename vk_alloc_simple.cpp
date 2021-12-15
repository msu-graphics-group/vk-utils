#include "vk_alloc_simple.h"
#include "vk_utils.h"

namespace vk_utils
{
  std::shared_ptr<IMemoryAlloc> CreateMemoryAlloc_Simple(VkDevice a_device, VkPhysicalDevice a_physicalDevice)
  {
    return std::make_shared<MemoryAlloc_Simple>(a_device, a_physicalDevice);
  }


  MemoryAlloc_Simple::MemoryAlloc_Simple(VkDevice a_device, VkPhysicalDevice a_physicalDevice) :
    m_device(a_device), m_physicalDevice(a_physicalDevice)
  {
    vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_physicalMemoryProps);
  }

  uint32_t MemoryAlloc_Simple::Allocate(const MemAllocInfo& a_allocInfo)
  {
    VkMemoryAllocateInfo memAllocInfo {};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAllocInfo.allocationSize = a_allocInfo.memReq.size;
    memAllocInfo.memoryTypeIndex = vk_utils::findMemoryType(a_allocInfo.memReq.memoryTypeBits, a_allocInfo.memProps,
      m_physicalDevice);

    VkMemoryDedicatedAllocateInfo dedicatedInfo {};
    dedicatedInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
    if(a_allocInfo.dedicated_buffer || a_allocInfo.dedicated_image)
    {
      dedicatedInfo.pNext = memAllocInfo.pNext;
      memAllocInfo.pNext  = &dedicatedInfo;

      dedicatedInfo.buffer = a_allocInfo.dedicated_buffer;
      dedicatedInfo.image = a_allocInfo.dedicated_image;
    }

    VkMemoryAllocateFlagsInfo flagsInfo {};
    flagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    if(a_allocInfo.allocateFlags)
    {
      flagsInfo.pNext    = memAllocInfo.pNext;
      memAllocInfo.pNext = &flagsInfo;

      flagsInfo.flags = a_allocInfo.allocateFlags;
    }


    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkResult result = vkAllocateMemory(m_device, &memAllocInfo, nullptr, &memory);
    VK_CHECK_RESULT(result);

    MemoryBlock block {};
    block.memory = memory;
    block.size   = memAllocInfo.allocationSize;
    block.offset = 0;

    m_allocations.push_back(block);

    return m_allocations.size() - 1;
  }


  void MemoryAlloc_Simple::Free(uint32_t a_memBlockId)
  {
    if(a_memBlockId >= m_allocations.size())
      return;

    vkFreeMemory(m_device, m_allocations[a_memBlockId].memory, nullptr);
  }

  void MemoryAlloc_Simple::FreeAllMemory()
  {
    for(auto& alloc : m_allocations)
    {
      vkFreeMemory(m_device, alloc.memory, nullptr);
    }
  }

  MemoryBlock MemoryAlloc_Simple::GetMemoryBlock(uint32_t a_memBlockId) const
  {
    if(a_memBlockId >= m_allocations.size())
      return {};

    return m_allocations[a_memBlockId];
  }

  void* MemoryAlloc_Simple::Map(uint32_t a_memBlockId, VkDeviceSize a_offset, VkDeviceSize a_size)
  {
    if(a_memBlockId >= m_allocations.size())
      return nullptr;

    void* ptr = nullptr;
    VkResult result = vkMapMemory(m_device, m_allocations[a_memBlockId].memory, a_offset, a_size, 0, &ptr);
    VK_CHECK_RESULT(result);

    return ptr;
  }

  void MemoryAlloc_Simple::Unmap(uint32_t a_memBlockId)
  {
    if(a_memBlockId >= m_allocations.size())
      return;

    vkUnmapMemory(m_device, m_allocations[a_memBlockId].memory);
  }
}