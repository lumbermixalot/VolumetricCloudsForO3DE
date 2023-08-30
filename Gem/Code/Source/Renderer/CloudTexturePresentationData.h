/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <AzCore/RTTI/TypeInfoSimple.h>
#include <AzCore/Memory/SystemAllocator.h>

namespace VolumetricClouds
{

    enum class VisibleChannel : AZ::u8
    {
        PerlinWorley,
        WorleyFbm1,
        WorleyFbm2,
        WorleyFbm4, 
        AllChannels,
        None // Do not render this texture (as a billboard) in the scene.
    };

    // Has all the data the shader needs to draw a slice of a 3D Noise Texture.
    struct CloudTexturePresentationData
    {
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_TYPE_INFO_WITH_NAME_DECL(CloudTexturePresentationData);
        AZ_RTTI_NO_TYPE_INFO_DECL();

        static void Reflect(AZ::ReflectContext* reflection);

        bool operator==(const CloudTexturePresentationData& rhs) const;
        bool operator!=(const CloudTexturePresentationData& rhs) const;

        bool IsHidden() const;

        bool m_alwaysFaceCamera = false;

        // Controls the Z coordinate of the Texture3D that should be displayed.
        // This is a value from 0.0 to 1.0 (both inclusive).
        float m_texCoordZ = 0.0;

        // What channel to see when visualizing the Texture3D with the Debug Viewer Feature Processor.
        VisibleChannel m_visibleChannel = VisibleChannel::None;

    };

} // namespace VolumetricClouds
