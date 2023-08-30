
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>

#include "VolumetricCloudsProjectSystemComponent.h"

#include <VolumetricCloudsProject/VolumetricCloudsProjectTypeIds.h>

namespace VolumetricCloudsProject
{
    class VolumetricCloudsProjectModule
        : public AZ::Module
    {
    public:
        AZ_RTTI(VolumetricCloudsProjectModule, VolumetricCloudsProjectModuleTypeId, AZ::Module);
        AZ_CLASS_ALLOCATOR(VolumetricCloudsProjectModule, AZ::SystemAllocator);

        VolumetricCloudsProjectModule()
            : AZ::Module()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            m_descriptors.insert(m_descriptors.end(), {
                VolumetricCloudsProjectSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<VolumetricCloudsProjectSystemComponent>(),
            };
        }
    };
}// namespace VolumetricCloudsProject

AZ_DECLARE_MODULE_CLASS(Gem_VolumetricCloudsProject, VolumetricCloudsProject::VolumetricCloudsProjectModule)
