
#include <AzCore/Serialization/SerializeContext.h>

#include "VolumetricCloudsProjectSystemComponent.h"

#include <VolumetricCloudsProject/VolumetricCloudsProjectTypeIds.h>

namespace VolumetricCloudsProject
{
    AZ_COMPONENT_IMPL(VolumetricCloudsProjectSystemComponent, "VolumetricCloudsProjectSystemComponent",
        VolumetricCloudsProjectSystemComponentTypeId);

    void VolumetricCloudsProjectSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<VolumetricCloudsProjectSystemComponent, AZ::Component>()
                ->Version(0)
                ;
        }
    }

    void VolumetricCloudsProjectSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("VolumetricCloudsProjectService"));
    }

    void VolumetricCloudsProjectSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("VolumetricCloudsProjectService"));
    }

    void VolumetricCloudsProjectSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void VolumetricCloudsProjectSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    VolumetricCloudsProjectSystemComponent::VolumetricCloudsProjectSystemComponent()
    {
        if (VolumetricCloudsProjectInterface::Get() == nullptr)
        {
            VolumetricCloudsProjectInterface::Register(this);
        }
    }

    VolumetricCloudsProjectSystemComponent::~VolumetricCloudsProjectSystemComponent()
    {
        if (VolumetricCloudsProjectInterface::Get() == this)
        {
            VolumetricCloudsProjectInterface::Unregister(this);
        }
    }

    void VolumetricCloudsProjectSystemComponent::Init()
    {
    }

    void VolumetricCloudsProjectSystemComponent::Activate()
    {
        VolumetricCloudsProjectRequestBus::Handler::BusConnect();
    }

    void VolumetricCloudsProjectSystemComponent::Deactivate()
    {
        VolumetricCloudsProjectRequestBus::Handler::BusDisconnect();
    }
}
