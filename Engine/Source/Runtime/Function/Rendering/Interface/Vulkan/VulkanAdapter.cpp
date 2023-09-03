#include "VulkanAdapter.h"
namespace Snowy::Ark
{
void VulkanAdapter::QueryProperties()
{
    m_Native.getProperties2(&m_Properties);
}
void VulkanAdapter::QueryQueueFamilyIndices()
{
}
void VulkanAdapter::QuerySwapChainSupport()
{
}
}