#include "vk_alloc_simple.h"
#include "vk_utils.h"
#include "vk_buffers.h"
#include "vk_images.h"

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

  uint32_t MemoryAlloc_Simple::Allocate(const MemAllocInfo& a_allocInfoBuffers, const std::vector<VkBuffer> &a_buffers)
  {
    MemAllocInfo allocInfo = a_allocInfoBuffers;
    std::vector<VkMemoryRequirements> bufMemReqs(a_buffers.size());
    for(size_t i = 0; i < a_buffers.size(); ++i)
    {
      if(a_buffers[i] != VK_NULL_HANDLE)
        vkGetBufferMemoryRequirements(m_device, a_buffers[i], &bufMemReqs[i]);
      else
      {
        bufMemReqs[i] = bufMemReqs[0];
        bufMemReqs[i].size = 0;
      }
    }
    for(size_t i = 1; i < bufMemReqs.size(); ++i)
    {
      if(bufMemReqs[i].memoryTypeBits != bufMemReqs[0].memoryTypeBits)
      {
        logWarning("[MemoryAlloc_Simple::Allocate]: input buffers have different memReq.memoryTypeBits");
        return -1;
      }
    }

    auto bufOffsets  = assignMemOffsetsWithPadding(bufMemReqs);
    auto bufMemTotal = bufOffsets[bufOffsets.size() - 1];

    allocInfo.memReq      = bufMemReqs[0];
    allocInfo.memReq.size = bufMemTotal;

    auto allocId = Allocate(allocInfo);

#if defined(VK_VERSION_1_1)
    std::vector<VkBindBufferMemoryInfo> bindInfos;
    bindInfos.reserve(bufMemReqs.size());
    for(size_t i = 0; i < bufMemReqs.size(); ++i)
    {
      if(a_buffers[i] != VK_NULL_HANDLE)
      {
        VkBindBufferMemoryInfo info {};
        info.sType        = VK_STRUCTURE_TYPE_BIND_BUFFER_MEMORY_INFO;
        info.buffer       = a_buffers[i];
        info.memory       = GetMemoryBlock(allocId).memory;
        info.memoryOffset = GetMemoryBlock(allocId).offset + bufOffsets[i];
        bindInfos.emplace_back(info);
      }
    }
    vkBindBufferMemory2(m_device, bindInfos.size(), bindInfos.data());
#else
    for(size_t i = 0; i < bufMemReqs.size(); i++)
    {
      if(a_buffers[i] != VK_NULL_HANDLE)
        vkBindBufferMemory(m_device, a_buffers[i], GetMemoryBlock(allocId).memory, GetMemoryBlock(allocId).offset + bufOffsets[i]);
    }
#endif

    return allocId;
  }

  uint32_t MemoryAlloc_Simple::Allocate(const MemAllocInfo& a_allocInfoImages, const std::vector<VkImage> &a_images)
  {
    MemAllocInfo allocInfo = a_allocInfoImages;
    std::vector<VkMemoryRequirements> imgMemReqs(a_images.size());
    for(size_t i = 0; i < a_images.size(); ++i)
    {
      if(a_images[i] != VK_NULL_HANDLE)
        vkGetImageMemoryRequirements(m_device, a_images[i], &imgMemReqs[i]);
      else
      {
        imgMemReqs[i] = imgMemReqs[0];
        imgMemReqs[i].size = 0;
      }
    }
    for(size_t i = 1; i < imgMemReqs.size(); ++i)
    {
      if(imgMemReqs[i].memoryTypeBits != imgMemReqs[0].memoryTypeBits)
      {
        logWarning("[MemoryAlloc_Simple::Allocate]: input images have different memReq.memoryTypeBits");
        return -1;
      }
    }

    auto imgOffsets  = assignMemOffsetsWithPadding(imgMemReqs);
    auto imgMemTotal = imgOffsets[imgOffsets.size() - 1];

    allocInfo.memReq      = imgMemReqs[0];
    allocInfo.memReq.size = imgMemTotal;

    auto allocId = Allocate(allocInfo);

#if defined(VK_VERSION_1_1)
    std::vector<VkBindImageMemoryInfo> bindInfos;
    bindInfos.reserve(imgMemReqs.size());
    for(size_t i = 0; i < imgMemReqs.size(); ++i)
    {
      if(a_images[i] != VK_NULL_HANDLE)
      {
        VkBindImageMemoryInfo info {};
        info.sType        = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
        info.image        = a_images[i];
        info.memory       = GetMemoryBlock(allocId).memory;
        info.memoryOffset = GetMemoryBlock(allocId).offset + imgOffsets[i];
        bindInfos.emplace_back(info);
      }
    }
    vkBindImageMemory2(m_device, bindInfos.size(), bindInfos.data());
#else
    for(size_t i = 0; i < imgMemReqs.size(); i++)
    {
      if(a_images[i] != VK_NULL_HANDLE)// unnecessary check ?
        vkBindBufferMemory(m_device, a_images[i], GetMemoryBlock(allocId).memory, GetMemoryBlock(allocId).offset + imgOffsets[i]);
    }
#endif

    return allocId;
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