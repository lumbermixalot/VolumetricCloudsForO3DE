/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <AzFramework/Components/ComponentAdapter.h>
#include <Clients/Components/CloudTextureComputeComponentController.h>

namespace VolumetricClouds
{
    class CloudTextureComputeComponent final
        : public AzFramework::Components::ComponentAdapter<CloudTextureComputeComponentController, CloudTextureComputeComponentConfig>
    {
    public:
    
        using BaseClass = AzFramework::Components::ComponentAdapter<CloudTextureComputeComponentController, CloudTextureComputeComponentConfig>;
        AZ_COMPONENT(CloudTextureComputeComponent, CloudTextureComputeComponentTypeId, BaseClass);
    
        CloudTextureComputeComponent() = default;
        CloudTextureComputeComponent(const CloudTextureComputeComponentConfig& config);
    
        static void Reflect(AZ::ReflectContext* context);
    };
} // namespace VolumetricClouds
