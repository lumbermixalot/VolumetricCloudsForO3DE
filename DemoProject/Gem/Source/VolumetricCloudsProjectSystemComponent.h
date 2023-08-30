
#pragma once

#include <AzCore/Component/Component.h>

#include <VolumetricCloudsProject/VolumetricCloudsProjectBus.h>

namespace VolumetricCloudsProject
{
    class VolumetricCloudsProjectSystemComponent
        : public AZ::Component
        , protected VolumetricCloudsProjectRequestBus::Handler
    {
    public:
        AZ_COMPONENT_DECL(VolumetricCloudsProjectSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        VolumetricCloudsProjectSystemComponent();
        ~VolumetricCloudsProjectSystemComponent();

    protected:
        ////////////////////////////////////////////////////////////////////////
        // VolumetricCloudsProjectRequestBus interface implementation

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////
    };
}
