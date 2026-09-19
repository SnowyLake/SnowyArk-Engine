#include <Runtime/GAL/GraphicsDevice.h>

#include <memory>

#include <Runtime/Core/Log.h>
#include <Runtime/GAL/Vulkan/VulkanDevice.h>

namespace SnowyArk
{

std::unique_ptr<GraphicsDevice> GraphicsDevice::Create(const GraphicsBackend backend)
{
    switch (backend)
    {
    case GraphicsBackend::Vulkan:
        return VulkanDevice::Create();
    }

    Log::Error("Unsupported graphics backend.");
    return nullptr;
}

}
