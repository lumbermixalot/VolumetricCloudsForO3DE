/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include "CloudscapeComponent.h"

namespace VolumetricClouds
{
    CloudscapeComponent::CloudscapeComponent(const CloudscapeComponentConfig& config)
        : BaseClass(config)
    {
    }
    
    void CloudscapeComponent::Reflect(AZ::ReflectContext* context)
    {
        BaseClass::Reflect(context);
    
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CloudscapeComponent, BaseClass>()
                ->Version(0)
                ;
        }

    }
} // namespace VolumetricClouds
