#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#include "external/vk_mem_alloc.h"

#include "vk_alloc_vma.h"
#include "vk_utils.h"

namespace vk_utils
{
  std::shared_ptr<IMemoryAlloc> CreateMemoryAlloc_VMA(VkInstance a_instance, VkDevice a_device, VkPhysicalDevice a_physicalDevice,
    VkFlags a_flags, uint32_t a_vkAPIVersion)
  {
    return std::make_shared<MemoryAlloc_VMA>(a_instance, a_device, a_physicalDevice, a_flags, a_vkAPIVersion);
  }

  static inline VmaMemoryUsage getVMAMemoryUsage(VkMemoryPropertyFlags flags)
  {
    if((flags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)       == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
      return VMA_MEMORY_USAGE_GPU_ONLY;
    else if((flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
      return VMA_MEMORY_USAGE_CPU_ONLY;
    else if((flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)  == VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
      return VMA_MEMORY_USAGE_CPU_TO_GPU;

    return VMA_MEMORY_USAGE_UNKNOWN;
  }

  VmaAllocator initVMA(VkInstance a_instance, VkDevice a_device, VkPhysicalDevice a_physicalDevice,
    VkFlags a_flags, uint32_t a_vkAPIVersion)
  {
    VmaVulkanFunctions functions = {};
    functions.vkAllocateMemory                    = vkAllocateMemory;
    functions.vkGetInstanceProcAddr               = vkGetInstanceProcAddr;
    functions.vkGetDeviceProcAddr                 = vkGetDeviceProcAddr;
    functions.vkGetPhysicalDeviceProperties       = vkGetPhysicalDeviceProperties;
    functions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    functions.vkFreeMemory                        = vkFreeMemory;
    functions.vkMapMemory                         = vkMapMemory;
    functions.vkUnmapMemory                       = vkUnmapMemory;
    functions.vkFlushMappedMemoryRanges           = vkFlushMappedMemoryRanges;
    functions.vkInvalidateMappedMemoryRanges      = vkInvalidateMappedMemoryRanges;
    functions.vkBindBufferMemory                  = vkBindBufferMemory;
    functions.vkBindImageMemory                   = vkBindImageMemory;
    functions.vkGetBufferMemoryRequirements       = vkGetBufferMemoryRequirements;
    functions.vkGetImageMemoryRequirements        = vkGetImageMemoryRequirements;
    functions.vkCreateBuffer                      = vkCreateBuffer;
    functions.vkDestroyBuffer                     = vkDestroyBuffer;
    functions.vkCreateImage                       = vkCreateImage;
    functions.vkDestroyImage                      = vkDestroyImage;
    functions.vkCmdCopyBuffer                     = vkCmdCopyBuffer;
    if(a_vkAPIVersion >= 1001000)
    {
      functions.vkGetBufferMemoryRequirements2KHR       = vkGetBufferMemoryRequirements2KHR;
      functions.vkGetImageMemoryRequirements2KHR        = vkGetImageMemoryRequirements2KHR;
      functions.vkBindBufferMemory2KHR                  = vkBindBufferMemory2KHR;
      functions.vkBindImageMemory2KHR                   = vkBindImageMemory2KHR;
      functions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR;
    }

    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.vulkanApiVersion = a_vkAPIVersion;
    allocatorInfo.instance         = a_instance;
    allocatorInfo.physicalDevice   = a_physicalDevice;
    allocatorInfo.device           = a_device;
    allocatorInfo.flags            = a_flags;  // VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    allocatorInfo.pVulkanFunctions = &functions;

    VmaAllocator allocator;
    vmaCreateAllocator(&allocatorInfo, &allocator);

    return allocator;
  }

  MemoryAlloc_VMA::MemoryAlloc_VMA(VkInstance a_instance, VkDevice a_device, VkPhysicalDevice a_physicalDevice,
    VkFlags a_flags, uint32_t a_vkAPIVersion) : m_device(a_device), m_physicalDevice(a_physicalDevice)
  {
    m_vma = initVMA(a_instance, a_device, a_physicalDevice, a_flags, a_vkAPIVersion);
  }

  uint32_t MemoryAlloc_VMA::Allocate(const MemAllocInfo& a_allocInfo)
  {
    VmaAllocationCreateInfo vmaAllocCreateInfo = {};
    vmaAllocCreateInfo.usage = getVMAMemoryUsage(a_allocInfo.memProps);
    if(a_allocInfo.dedicated_image || a_allocInfo.dedicated_buffer)
    {
      vmaAllocCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
    }

    VmaAllocationInfo vmaAllocInfo;
    VmaAllocation     alloc = nullptr;

    VkResult result = vmaAllocateMemory(m_vma, &a_allocInfo.memReq, &vmaAllocCreateInfo,
      &alloc, &vmaAllocInfo);

    VK_CHECK_RESULT(result);

    m_allocations.push_back(alloc);

    return m_allocations.size() - 1;
  }

  void MemoryAlloc_VMA::Free(uint32_t a_memBlockId)
  {
    if(a_memBlockId >= m_allocations.size())
      return;

    vmaFreeMemory(m_vma, m_allocations[a_memBlockId]);
  }

  void MemoryAlloc_VMA::FreeAllMemory()
  {
    for(auto& alloc : m_allocations)
    {
      vmaFreeMemory(m_vma, alloc);
    }
  }

  MemoryBlock MemoryAlloc_VMA::GetMemoryBlock(uint32_t a_memBlockId) const
  {
    if(a_memBlockId >= m_allocations.size())
      return {};

    VmaAllocationInfo allocInfo;
    vmaGetAllocationInfo(m_vma, m_allocations[a_memBlockId], &allocInfo);

    MemoryBlock memInfo;
    memInfo.memory = allocInfo.deviceMemory;
    memInfo.offset = allocInfo.offset;
    memInfo.size   = allocInfo.size;

    return memInfo;
  }

  void* MemoryAlloc_VMA::Map(uint32_t a_memBlockId, VkDeviceSize a_offset, VkDeviceSize a_size)
  {
    if(a_memBlockId >= m_allocations.size())
      return nullptr;

    (void*)a_offset;
    (void*)a_size;

    void* ptr;
    VkResult result = vmaMapMemory(m_vma, m_allocations[a_memBlockId], &ptr);
    VK_CHECK_RESULT(result);

    return ptr;
  }

  void MemoryAlloc_VMA::Unmap(uint32_t a_memBlockId)
  {
    if(a_memBlockId >= m_allocations.size())
      return;

    vmaUnmapMemory(m_vma, m_allocations[a_memBlockId]);
  }
}