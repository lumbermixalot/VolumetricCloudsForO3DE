/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <AzFramework/Components/ComponentAdapter.h>
#include <Clients/Components/CloudTextureAssetComponentController.h>

namespace VolumetricClouds
{
    class CloudTextureAssetComponent final
        : public AzFramework::Components::ComponentAdapter<CloudTextureAssetComponentController, CloudTextureAssetComponentConfig>
    {
    public:
    
        using BaseClass = AzFramework::Components::ComponentAdapter<CloudTextureAssetComponentController, CloudTextureAssetComponentConfig>;
        AZ_COMPONENT(CloudTextureAssetComponent, CloudTextureAssetComponentTypeId, BaseClass);
    
        CloudTextureAssetComponent() = default;
        CloudTextureAssetComponent(const CloudTextureAssetComponentConfig& config);
    
        static void Reflect(AZ::ReflectContext* context);
    };
} // namespace VolumetricClouds
