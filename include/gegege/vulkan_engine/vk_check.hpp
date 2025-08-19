#pragma once

#include <SDL3/SDL_log.h>
#include <vulkan/vk_enum_string_helper.h>

#define VK_CHECK(x)                                                                              \
    do                                                                                           \
    {                                                                                            \
        vk::Result err = x;                                                                      \
        if (err != vk::Result::eSuccess)                                                         \
        {                                                                                        \
            SDL_Log("Vulkan Engine: Detected Vulkan error: %s", string_VkResult((VkResult)err)); \
            abort();                                                                             \
        }                                                                                        \
    } while (0)
