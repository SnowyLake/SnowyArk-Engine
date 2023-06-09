#pragma once
#include "Engine\Source\Runtime\Core\Base\Common.h"
#include "Engine\Source\Runtime\Function\Rendering\Interface\Vulkan\VulkanRHI.h"
namespace Snowy::Ark
{
class Engine
{
public:

    Engine() = default;
    ~Engine() = default;

    void Init(ERHIType type);
    void Run();
    void Destroy();

private:
    std::shared_ptr<RHI> m_RHI;
};
}


