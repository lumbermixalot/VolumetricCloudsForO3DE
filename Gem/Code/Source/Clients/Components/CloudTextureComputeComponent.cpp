/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include "CloudTextureComputeComponent.h"

namespace VolumetricClouds
{
    CloudTextureComputeComponent::CloudTextureComputeComponent(const CloudTextureComputeComponentConfig& config)
        : BaseClass(config)
    {
    }
    
    void CloudTextureComputeComponent::Reflect(AZ::ReflectContext* context)
    {
        BaseClass::Reflect(context);
    
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CloudTextureComputeComponent, BaseClass>()
                ->Version(0)
                ;
        }

    }
} // namespace VolumetricClouds
