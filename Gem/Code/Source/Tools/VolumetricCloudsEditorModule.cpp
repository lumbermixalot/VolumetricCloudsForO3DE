/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <VolumetricClouds/VolumetricCloudsTypeIds.h>
#include <VolumetricCloudsModuleInterface.h>
#include "VolumetricCloudsEditorSystemComponent.h"
#include <Tools/Components/EditorCloudTextureComputeComponent.h>
#include <Tools/Components/EditorCloudTextureAssetComponent.h>
#include <Tools/Components/EditorCloudscapeComponent.h>

namespace VolumetricClouds
{
    class VolumetricCloudsEditorModule
        : public VolumetricCloudsModuleInterface
    {
    public:
        AZ_RTTI(VolumetricCloudsEditorModule, VolumetricCloudsEditorModuleTypeId, VolumetricCloudsModuleInterface);
        AZ_CLASS_ALLOCATOR(VolumetricCloudsEditorModule, AZ::SystemAllocator);

        VolumetricCloudsEditorModule()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            // Add ALL components descriptors associated with this gem to m_descriptors.
            // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
            // This happens through the [MyComponent]::Reflect() function.
            m_descriptors.insert(m_descriptors.end(), {
                VolumetricCloudsEditorSystemComponent::CreateDescriptor(),
                EditorCloudTextureComputeComponent::CreateDescriptor(),
                EditorCloudTextureAssetComponent::CreateDescriptor(),
                EditorCloudscapeComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         * Non-SystemComponents should not be added here
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList {
                azrtti_typeid<VolumetricCloudsEditorSystemComponent>(),
            };
        }
    };
}// namespace VolumetricClouds

AZ_DECLARE_MODULE_CLASS(Gem_VolumetricClouds, VolumetricClouds::VolumetricCloudsEditorModule)
