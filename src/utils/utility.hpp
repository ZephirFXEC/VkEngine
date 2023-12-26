//
// Created by Enzo Crema on 26/12/2023.
//
#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#define NDC_INLINE [[nodiscard]] __attribute__((always_inline)) inline

// constants for the application
#define USE_VMA true

#ifndef USE_VMA
using Alloc = VkDeviceMemory;
#else
using Alloc = VmaAllocation;
#endif