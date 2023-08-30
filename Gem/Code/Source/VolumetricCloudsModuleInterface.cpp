/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include "VolumetricCloudsModuleInterface.h"
#include <AzCore/Memory/Memory.h>

#include <VolumetricClouds/VolumetricCloudsTypeIds.h>

#include <Clients/VolumetricCloudsSystemComponent.h>
#include <Clients/Components/CloudTextureComputeComponent.h>
#include <Clients/Components/CloudTextureAssetComponent.h>
#include <Clients/Components/CloudscapeComponent.h>

namespace VolumetricClouds
{
    AZ_TYPE_INFO_WITH_NAME_IMPL(VolumetricCloudsModuleInterface,
        "VolumetricCloudsModuleInterface", VolumetricCloudsModuleInterfaceTypeId);
    AZ_RTTI_NO_TYPE_INFO_IMPL(VolumetricCloudsModuleInterface, AZ::Module);
    AZ_CLASS_ALLOCATOR_IMPL(VolumetricCloudsModuleInterface, AZ::SystemAllocator);

    VolumetricCloudsModuleInterface::VolumetricCloudsModuleInterface()
    {
        // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
        // Add ALL components descriptors associated with this gem to m_descriptors.
        // This will associate the AzTypeInfo information for the components with the the SerializeContext, BehaviorContext and EditContext.
        // This happens through the [MyComponent]::Reflect() function.
        m_descriptors.insert(m_descriptors.end(), {
            VolumetricCloudsSystemComponent::CreateDescriptor(),
            CloudTextureComputeComponent::CreateDescriptor(),
            CloudTextureAssetComponent::CreateDescriptor(),
            CloudscapeComponent::CreateDescriptor(),
            });
    }

    AZ::ComponentTypeList VolumetricCloudsModuleInterface::GetRequiredSystemComponents() const
    {
        return AZ::ComponentTypeList{
            azrtti_typeid<VolumetricCloudsSystemComponent>(),
        };
    }
} // namespace VolumetricClouds
