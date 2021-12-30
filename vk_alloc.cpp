#include "vk_alloc.h"
#include "vk_utils.h"
#include "vk_buffers.h"
#include "vk_images.h"

namespace vk_utils
{

  ResourceManager::ResourceManager(VkDevice a_device, VkPhysicalDevice a_physicalDevice, IMemoryAlloc *a_pAlloc, ICopyEngine *a_pCopy) :
    m_device(a_device), m_physicalDevice(a_physicalDevice), m_pAlloc(a_pAlloc), m_pCopy(a_pCopy)
  {

  }

  ResourceManager::~ResourceManager()
  {
    // @TODO
  }

  VkBuffer ResourceManager::CreateBuffer(VkDeviceSize a_size, VkBufferUsageFlags a_usage, VkMemoryPropertyFlags a_memProps,
    VkMemoryAllocateFlags flags)
  {
      VkMemoryRequirements memreq = {};
      auto buf = vk_utils::createBuffer(m_device, a_size, a_usage, &memreq);

      MemAllocInfo allocInfo = {};
      allocInfo.allocateFlags = flags;
      allocInfo.memProps = a_memProps;
      allocInfo.memReq = memreq;

      uint32_t allocId = m_pAlloc->Allocate(allocInfo);
      vkBindBufferMemory(m_device, buf, m_pAlloc->GetMemoryBlock(allocId).memory, m_pAlloc->GetMemoryBlock(allocId).offset);

      return buf;
  }

  VkBuffer ResourceManager::CreateBuffer(const void* a_data, VkDeviceSize a_size, VkBufferUsageFlags a_usage,
    VkMemoryPropertyFlags a_memProps, VkMemoryAllocateFlags flags)
  {
    auto buf = CreateBuffer(a_size, a_usage, a_memProps, flags);

    m_pCopy->UpdateBuffer(buf, 0, a_data, a_size);

    return buf;
  }

  std::vector<VkBuffer> ResourceManager::CreateBuffers(const std::vector<void*> &a_dataPointers, const std::vector<VkDeviceSize> &a_sizes,
    const std::vector<VkBufferUsageFlags> &a_usages, VkMemoryPropertyFlags a_memProps, VkMemoryAllocateFlags flags)
  {
    std::vector<VkBuffer> buffers(a_dataPointers.size());
    for(size_t i = 0; i < buffers.size(); ++i)
    {
      VkMemoryRequirements memreq = {};
      auto buf = vk_utils::createBuffer(m_device, a_sizes[i], a_usages[i], &memreq);

      buffers[i]  = buf;
    }

    MemAllocInfo allocInfo = {};
    allocInfo.allocateFlags    = flags;
    allocInfo.memProps         = a_memProps;

    uint32_t allocId = m_pAlloc->Allocate(allocInfo, buffers);

    for (size_t i = 0; i < buffers.size(); i++)
    {
      m_pCopy->UpdateBuffer(buffers[i], 0, a_dataPointers[i], a_sizes[i]);
      m_bufAllocs[buffers[i]] = allocId;
    }

    return buffers;
  }

  VkImage ResourceManager::CreateImage(const VkImageCreateInfo& a_createInfo)
  {

    return VK_NULL_HANDLE;
  }

  VkImage ResourceManager::CreateImage(uint32_t a_width, uint32_t a_height, VkFormat a_format, VkImageUsageFlags a_usage,
    uint32_t a_mipLvls)
  {
    return VK_NULL_HANDLE;
  }

  VkImage ResourceManager::CreateImage(const void* a_data, uint32_t a_width, uint32_t a_height, VkFormat a_format,
    VkImageUsageFlags a_usage, uint32_t a_mipLvls)
  {
    return VK_NULL_HANDLE;
  }

  VulkanTexture ResourceManager::CreateTexture(const VkImageCreateInfo& a_createInfo, const VkImageViewCreateInfo& imageViewCreateInfo)
  {
    return {};
  }

  VulkanTexture ResourceManager::CreateTexture(const VkImageCreateInfo& a_createInfo, const VkImageViewCreateInfo& imageViewCreateInfo,
    const VkSamplerCreateInfo& samplerCreateInfo)
  {
    return {};
  }


}
