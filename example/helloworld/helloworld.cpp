#include <gegege/vulkan_engine/vulkan_engine.hpp>

int main()
{
    gegege::VulkanEngine vulkan_engine{};
    vulkan_engine.startup();
    vulkan_engine.run();
    vulkan_engine.shutdown();
    return 0;
}
