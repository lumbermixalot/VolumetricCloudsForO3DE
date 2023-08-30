/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <VolumetricClouds/VolumetricCloudsTypeIds.h>
#include <VolumetricCloudsModuleInterface.h>
#include "VolumetricCloudsSystemComponent.h"

namespace VolumetricClouds
{
    class VolumetricCloudsModule
        : public VolumetricCloudsModuleInterface
    {
    public:
        AZ_RTTI(VolumetricCloudsModule, VolumetricCloudsModuleTypeId, VolumetricCloudsModuleInterface);
        AZ_CLASS_ALLOCATOR(VolumetricCloudsModule, AZ::SystemAllocator);
    };
}// namespace VolumetricClouds

AZ_DECLARE_MODULE_CLASS(Gem_VolumetricClouds, VolumetricClouds::VolumetricCloudsModule)
