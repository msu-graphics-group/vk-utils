#define VMA_IMPLEMENTATION
#define VK_NO_PROTOTYPES
#define VMA_STATIC_VULKAN_FUNCTIONS  0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include "external/vk_mem_alloc.h"

#include "vk_alloc_vma.h"
#include "vk_utils.h"
#include "vk_images.h"
#include "vk_buffers.h"

namespace vk_utils
{
  std::shared_ptr<IMemoryAlloc> CreateMemoryAlloc_VMA(VkInstance a_instance, VkDevice a_device, VkPhysicalDevice a_physicalDevice,
    VkFlags a_flags, uint32_t a_vkAPIVersion)
  {
    return std::make_shared<MemoryAlloc_VMA>(a_instance, a_device, a_physicalDevice, a_flags, a_vkAPIVersion);
  }

  std::shared_ptr<IMemoryAlloc> CreateMemoryAlloc_VMA(VkDevice a_device, VkPhysicalDevice a_physicalDevice, VmaAllocator a_allocator)
  {
    return std::make_shared<MemoryAlloc_VMA>(a_device, a_physicalDevice, a_allocator);
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
    if(a_vkAPIVersion >= VK_API_VERSION_1_1)
    {
      functions.vkGetBufferMemoryRequirements2KHR       = vkGetBufferMemoryRequirements2;
      functions.vkGetImageMemoryRequirements2KHR        = vkGetImageMemoryRequirements2;
      functions.vkBindBufferMemory2KHR                  = vkBindBufferMemory2;
      functions.vkBindImageMemory2KHR                   = vkBindImageMemory2;
      functions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;
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

  MemoryAlloc_VMA::MemoryAlloc_VMA(VkDevice a_device, VkPhysicalDevice a_physicalDevice, VmaAllocator a_allocator)
    : m_device(a_device), m_physicalDevice(a_physicalDevice), m_vma(a_allocator)
  {

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

    VkResult result = vmaAllocateMemory(m_vma, &a_allocInfo.memReq, &vmaAllocCreateInfo, &alloc, &vmaAllocInfo);

    VK_CHECK_RESULT(result);

    m_allocations.push_back(alloc);

    return m_allocations.size() - 1;
  }

  uint32_t MemoryAlloc_VMA::Allocate(const MemAllocInfo& a_allocInfoBuffers, const std::vector<VkBuffer> &a_buffers)
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
        logWarning("[MemoryAlloc_VMA::Allocate]: input buffers have different memReq.memoryTypeBits");
        return -1;
      }
    }

    auto bufOffsets  = assignMemOffsetsWithPadding(bufMemReqs);
    auto bufMemTotal = bufOffsets[bufOffsets.size() - 1];

    allocInfo.memReq      = bufMemReqs[0];
    allocInfo.memReq.size = bufMemTotal;

    //    vmaAllocateMemoryForBuffer(m_vma, a_buffers[i], vmaAllocCreateInfo, alloc, vmaAllocInfo);
    auto allocId = Allocate(allocInfo);

    for(size_t i = 0; i < bufMemReqs.size(); i++)
    {
      if(a_buffers[i] != VK_NULL_HANDLE)// unnecessary check ?
        vmaBindBufferMemory2(m_vma, m_allocations[allocId], bufOffsets[i], a_buffers[i], nullptr);
    }

    return allocId;
  }

  uint32_t MemoryAlloc_VMA::Allocate(const MemAllocInfo& a_allocInfoImages, const std::vector<VkImage> &a_images)
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
        logWarning("[MemoryAlloc_VMA::Allocate]: input images have different memReq.memoryTypeBits");
        return -1;
      }
    }

    auto imgOffsets  = assignMemOffsetsWithPadding(imgMemReqs);
    auto imgMemTotal = imgOffsets[imgOffsets.size() - 1];

    allocInfo.memReq      = imgMemReqs[0];
    allocInfo.memReq.size = imgMemTotal;

    auto allocId = Allocate(allocInfo);

    for(size_t i = 0; i < imgMemReqs.size(); i++)
    {
      if(a_images[i] != VK_NULL_HANDLE)// unnecessary check ?
        vmaBindImageMemory2(m_vma, m_allocations[allocId], imgOffsets[i], a_images[i], nullptr);
    }

    return allocId;
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

    MemoryBlock memInfo = {};
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
  
  //*********************************************************************************
  
  ResourceManager_VMA::ResourceManager_VMA(VkDevice a_device, VkPhysicalDevice a_physicalDevice, VmaAllocator a_allocator, ICopyEngine *a_pCopy) :
    m_device(a_device), m_physicalDevice(a_physicalDevice), m_vma(a_allocator), m_pCopy(a_pCopy)
  {
    m_samplerPool.init(m_device);
  }

  void ResourceManager_VMA::Cleanup()
  {
    for(auto& [buf, _] : m_bufAllocs)
    {
      VkBuffer tmp = buf;
      DestroyBuffer(tmp);
    }
    m_bufAllocs.clear();

    for(auto& [img, _] : m_imgAllocs)
    {
      VkImage tmp = img;
      DestroyImage(tmp);
    }
    m_imgAllocs.clear();

    m_samplerPool.deinit();
  }

  VkBuffer ResourceManager_VMA::CreateBuffer(VkDeviceSize a_size, VkBufferUsageFlags a_usage, VkMemoryPropertyFlags a_memProps,
    VkMemoryAllocateFlags)
  {
    VkBuffer buffer;

    VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
    bufferInfo.size  = a_size;
    bufferInfo.usage = a_usage;

    VmaAllocationCreateInfo allocInfo = {};
//    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    allocInfo.requiredFlags = a_memProps;

    VmaAllocation allocation;
    vmaCreateBuffer(m_vma, &bufferInfo, &allocInfo, &buffer,
      &allocation, nullptr);

    m_bufAllocs[buffer] = allocation;

    return buffer;
  }

  VkBuffer ResourceManager_VMA::CreateBuffer(const void* a_data, VkDeviceSize a_size, VkBufferUsageFlags a_usage,
    VkMemoryPropertyFlags a_memProps, VkMemoryAllocateFlags flags)
  {
    auto buf = CreateBuffer(a_size, a_usage, a_memProps, flags);

    m_pCopy->UpdateBuffer(buf, 0, a_data, a_size);

    return buf;
  }

  std::vector<VkBuffer> ResourceManager_VMA::CreateBuffers(const std::vector<void*> &a_dataPointers, const std::vector<VkDeviceSize> &a_sizes,
    const std::vector<VkBufferUsageFlags> &a_usages, VkMemoryPropertyFlags a_memProps, VkMemoryAllocateFlags flags)
  {
    std::vector<VkBuffer> buffers = CreateBuffers(a_sizes, a_usages, a_memProps, flags);

    for (size_t i = 0; i < buffers.size(); i++)
    {
      m_pCopy->UpdateBuffer(buffers[i], 0, a_dataPointers[i], a_sizes[i]);
    }

    return buffers;
  }

  std::vector<VkBuffer> ResourceManager_VMA::CreateBuffers(const std::vector<VkDeviceSize> &a_sizes, const std::vector<VkBufferUsageFlags> &a_usages,
    VkMemoryPropertyFlags a_memProps, VkMemoryAllocateFlags flags)
  {
    std::vector<VkBuffer> buffers(a_usages.size());
    for(size_t i = 0; i < buffers.size(); ++i)
    {
      VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
      bufferInfo.size  = a_sizes[i];
      bufferInfo.usage = a_usages[i];

      VmaAllocationCreateInfo allocInfo = {};
      allocInfo.requiredFlags = a_memProps;

      VmaAllocation allocation;
      vmaCreateBuffer(m_vma, &bufferInfo, &allocInfo, &buffers[i],
        &allocation, nullptr);

      m_bufAllocs[buffers[i]] = allocation;
    }

    return buffers;
  }

  VkImage ResourceManager_VMA::CreateImage(const VkImageCreateInfo& a_createInfo)
  {
    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
//    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
//    allocCreateInfo.pUserData = imageName.c_str();

    VkImage image;
    VmaAllocation allocation;
    vmaCreateImage(m_vma, &a_createInfo, &allocCreateInfo, &image, &allocation, nullptr);

    m_imgAllocs[image] = allocation;

    return image;
  }

  VkImage ResourceManager_VMA::CreateImage(uint32_t a_width, uint32_t a_height, VkFormat a_format, VkImageUsageFlags a_usage,
    uint32_t a_mipLvls)
  {
    VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageCreateInfo.usage         = a_usage;
    imageCreateInfo.format        = a_format;
    imageCreateInfo.extent.width  = a_width;
    imageCreateInfo.extent.height = a_height;
    imageCreateInfo.extent.depth  = 1;
    imageCreateInfo.mipLevels     = a_mipLvls;
    imageCreateInfo.arrayLayers   = 1;
    imageCreateInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;

    auto img = CreateImage(imageCreateInfo);

    return img;
  }

  VkImage ResourceManager_VMA::CreateImage(const void* a_data, uint32_t a_width, uint32_t a_height, VkFormat a_format,
    VkImageUsageFlags a_usage, VkImageLayout a_layout, uint32_t a_mipLvls)
  {
    auto img = CreateImage(a_width, a_height, a_format, a_usage, a_mipLvls);

    m_pCopy->UpdateImage(img, a_data, a_width, a_height, vk_utils::bppFromVkFormat(a_format), a_layout);

    return VK_NULL_HANDLE;
  }

  std::vector<VkImage> ResourceManager_VMA::CreateImages(const std::vector<VkImageCreateInfo>& a_createInfos)
  {
    std::vector<VkImage> res(a_createInfos.size());
    for(size_t i = 0; i < a_createInfos.size(); ++i) // no need to manually create single alloc in vma (?)
    {
      res[i] = CreateImage(a_createInfos[i]);
    }

    return res;
  }

  VulkanTexture ResourceManager_VMA::CreateTexture(const VkImageCreateInfo& a_createInfo, VkImageViewCreateInfo& a_imgViewCreateInfo)
  {
    VulkanTexture res{};
    res.image = CreateImage(a_createInfo);

    a_imgViewCreateInfo.image = res.image;
    VK_CHECK_RESULT(vkCreateImageView(m_device, &a_imgViewCreateInfo, nullptr, &res.descriptor.imageView));

    return res;
  }

  VulkanTexture ResourceManager_VMA::CreateTexture(const VkImageCreateInfo& a_createInfo, VkImageViewCreateInfo& a_imgViewCreateInfo,
    const VkSamplerCreateInfo& a_samplerCreateInfo)
  {
    VulkanTexture res{};
    res.image = CreateImage(a_createInfo);

    a_imgViewCreateInfo.image = res.image;
    VK_CHECK_RESULT(vkCreateImageView(m_device, &a_imgViewCreateInfo, nullptr, &res.descriptor.imageView));

    res.descriptor.sampler = m_samplerPool.acquireSampler(a_samplerCreateInfo);

    return res;
  }

  std::vector<VulkanTexture> ResourceManager_VMA::CreateTextures(const std::vector<VkImageCreateInfo>& a_createInfos,
                                                                 std::vector<VkImageViewCreateInfo>& a_imgViewCreateInfos)
  {
    std::vector<VulkanTexture> res(a_createInfos.size());
    for(size_t i = 0; i < a_createInfos.size(); ++i)
    {
      res[i] = CreateTexture(a_createInfos[i], a_imgViewCreateInfos[i]);
    }

    return res;
  }

  std::vector<VulkanTexture> ResourceManager_VMA::CreateTextures(const std::vector<VkImageCreateInfo>& a_createInfos,
                                                                 std::vector<VkImageViewCreateInfo>& a_imgViewCreateInfos,
                                                                 const std::vector<VkSamplerCreateInfo>& a_samplerCreateInfos)
  {
    std::vector<VulkanTexture> res(a_createInfos.size());
    for(size_t i = 0; i < a_createInfos.size(); ++i)
    {
      res[i] = CreateTexture(a_createInfos[i], a_imgViewCreateInfos[i], a_samplerCreateInfos[i]);
    }

    return res;
  }

  VkSampler ResourceManager_VMA::CreateSampler(const VkSamplerCreateInfo& a_samplerCreateInfo)
  {
    return m_samplerPool.acquireSampler(a_samplerCreateInfo);
  }

  void ResourceManager_VMA::DestroyBuffer(VkBuffer &a_buffer)
  {
    if(!m_bufAllocs.count(a_buffer))
    {
      logWarning("[ResourceManager_VMA::DestroyBuffer] trying to destroy unknown buffer");
      return;
    }

    vmaDestroyBuffer(m_vma, a_buffer, m_bufAllocs[a_buffer]);
    a_buffer = VK_NULL_HANDLE;
  }

  void ResourceManager_VMA::DestroyImage(VkImage &a_image)
  {
    if(!m_imgAllocs.count(a_image))
    {
      logWarning("[ResourceManager_VMA::DestroyImage] trying to destroy unknown image");
      return;
    }

    vmaDestroyImage(m_vma, a_image, m_imgAllocs[a_image]);
    a_image = VK_NULL_HANDLE;
  }

  void ResourceManager_VMA::DestroyTexture(VulkanTexture &a_texture)
  {
    DestroyImage(a_texture.image);

    if(a_texture.descriptor.imageView != VK_NULL_HANDLE)
    {
      vkDestroyImageView(m_device, a_texture.descriptor.imageView, nullptr);
      a_texture.descriptor.imageView = VK_NULL_HANDLE;
    }

    if(a_texture.descriptor.sampler != VK_NULL_HANDLE)
    {
      m_samplerPool.releaseSampler(a_texture.descriptor.sampler);
      a_texture.descriptor.sampler = VK_NULL_HANDLE;
    }
  }

  void ResourceManager_VMA::DestroySampler(VkSampler &a_sampler)
  {
    if(a_sampler != VK_NULL_HANDLE)
    {
      m_samplerPool.releaseSampler(a_sampler);
      a_sampler = VK_NULL_HANDLE;
    }
  }

}