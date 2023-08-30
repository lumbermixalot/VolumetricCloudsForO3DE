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
    enum class CloudTexturePixelSize : uint32_t
    {
        PixelSize16 = 16,
        PixelSize32 = 32,
        PixelSize64 = 64,
        PixelSize128 = 128,
        PixelSize256 = 256,
    };

    // Has all the data the compute shader needs to generate a Texture3D
    // with PerlinWorley noise.
    struct CloudTextureComputeData
    {
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_TYPE_INFO_WITH_NAME_DECL(CloudTextureComputeData);
        AZ_RTTI_NO_TYPE_INFO_DECL();

        static void Reflect(AZ::ReflectContext* reflection);

        bool operator==(const CloudTextureComputeData& rhs) const;
        bool operator!=(const CloudTextureComputeData& rhs) const;

        //! W, H, D dimensions a 3D Texture.
        //! (m_pixelSize x m_pixelSize x m_pixelSize).
        uint32_t m_pixelSize = 128;

        // The starting frequency for the noiseFBM.
        // We expect a value between 1 and 10.
        float m_frequency = 4.0;

        int m_perlinOctaves = 7;
        float m_perlinGain = 0.5504;
        // The starting amplitude for the perlin FBM.
        float m_perlinAmplitude = 1.0;

        int m_worleyOctaves = 3;
        float m_worleyGain = 0.45;
        // The starting amplitude for the worley FBM.
        float m_worleyAmplitude = 0.625;
    };

} // namespace VolumetricClouds
