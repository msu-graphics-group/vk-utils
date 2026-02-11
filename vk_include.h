#ifndef VK_UTILS_VULKAN_INCLUDE_H
#define VK_UTILS_VULKAN_INCLUDE_H

#ifdef __APPLE__
#define VK_ENABLE_BETA_EXTENSIONS // Для VK_KHR_portability_subset
#define VK_NO_PROTOTYPES
#endif

#if defined(USE_VOLK)
#include "volk.h"
#elif defined(USE_ETNA)
#include <etna/Vulkan.hpp>
#else
#include <vulkan/vulkan.h>
#endif

#endif // VK_UTILS_VULKAN_INCLUDE_H
