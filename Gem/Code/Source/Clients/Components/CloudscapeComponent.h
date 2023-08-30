/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <AzFramework/Components/ComponentAdapter.h>
#include <Clients/Components/CloudscapeComponentController.h>

namespace VolumetricClouds
{
    class CloudscapeComponent final
        : public AzFramework::Components::ComponentAdapter<CloudscapeComponentController, CloudscapeComponentConfig>
    {
    public:
    
        using BaseClass = AzFramework::Components::ComponentAdapter<CloudscapeComponentController, CloudscapeComponentConfig>;
        AZ_COMPONENT(CloudscapeComponent, CloudscapeComponentTypeId, BaseClass);
    
        CloudscapeComponent() = default;
        CloudscapeComponent(const CloudscapeComponentConfig& config);
    
        static void Reflect(AZ::ReflectContext* context);
    };
} // namespace VolumetricClouds
