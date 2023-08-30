/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include "CloudTextureAssetComponent.h"

namespace VolumetricClouds
{
    CloudTextureAssetComponent::CloudTextureAssetComponent(const CloudTextureAssetComponentConfig& config)
        : BaseClass(config)
    {
    }
    
    void CloudTextureAssetComponent::Reflect(AZ::ReflectContext* context)
    {
        BaseClass::Reflect(context);
    
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CloudTextureAssetComponent, BaseClass>()
                ->Version(0)
                ;
        }

    }
} // namespace VolumetricClouds
