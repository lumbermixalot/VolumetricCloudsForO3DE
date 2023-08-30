/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/
#pragma once

#include <AzCore/RTTI/TypeInfoSimple.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Math/Vector3.h>
#include <AzCore/Math/Color.h>

#include <Atom/RPI.Reflect/Image/Image.h>

#include <Renderer/CloudMaterialProperties.h>

namespace VolumetricClouds
{
    // Consolidates all the data for the shader constants needed
    // by the cloudscape shader.
    // See declaration of CloudscapeComponentConfig for details on each parameter.
    struct CloudscapeShaderConstantData
    {
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_TYPE_INFO_WITH_NAME_DECL(CloudscapeShaderConstantData);
        AZ_RTTI_NO_TYPE_INFO_DECL();

        virtual ~CloudscapeShaderConstantData() = default;

        static void Reflect(AZ::ReflectContext* reflection);

        // Only compares the reflected parameters.
        bool operator==(const CloudscapeShaderConstantData& rhs) const;
        bool operator!=(const CloudscapeShaderConstantData& rhs) const;

        // Used to scale world position XYZ when sampling
        // the Noise Textures during ray marching.
        float m_uvwScale = 0.25;
        // Maximum number of mip levels to consider when sampling
        // the low frequency texture. It will be clamped at runtime to the number
        // of mips available in the low frequency texture.
        // Also the clamping will make sure this value is never less than 1.
        uint32_t m_maxMipLevels = 10;
        // Clamped version of @m_maxMipLevels. Calculated at runtime.
        // A value between 1 and mip counts in low freq noise texture.
        uint32_t m_clampedMipLevels = 10; // DO NOT REFLECT

        // Ray Marching Steps, for performance/quality tradeoff.
        // These are the steps uses to ray march the clouds slab and calculate
        // the transmittance for each pixel.
        uint8_t m_minRayMarchingSteps = 32;
        uint8_t m_maxRayMarchingSteps = 64;

        float m_planetRadiusKm = 6371.0f; // TODO: Get this value from Sky Atmosphere Component.
        // Distance, above sea level, where the cloud slab begins.
        float m_cloudSlabDistanceAboveSeaLevelKm = 1.5f;
        // Clouds will be present only within the thickness
        // of the cloud slab.
        float m_cloudSlabThicknessKm = 3.5f;

        AZ::Vector3 m_sunColor = { 1.0f, 1.0f, 1.0f }; // DO NOT REFLECT (comes from an entity)
        float m_sunLightIntensity = 1.0f;
        AZ::Color m_ambientLightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        float m_ambientLightIntensity = 1.0f;
        // The shader expects a normalized vector.
        AZ::Vector3 m_directionTowardsTheSun = {0.0f, 0.0f, 1.0f}; // DO NOT REFLECT (comes from an entity)

        CloudMaterialProperties m_cloudMaterialProperties;

        //////////////////////////////////////////////////////////////
        // ******************* Weather Data Start
        // The weather map is a texture representing a squared area.
        // The length of each side of the square is measured in kilometers.
        float m_weatherMapSizeKm = 60.0;
        // This variable is the knob that controls cloud coverage between
        // The Red Channel (Low coverage) and the Green channel (High coverage).
        // When this parameter is less than 0.5 then the Red Channel determines
        // the coverage. Above 0.5, the Green Channel rapidly becomes the dominant
        // coverage.
        float m_globalCloudCoverage = 0.75;
        // This knob modulates the global density of the clouds.
        float m_globalCloudDensity = 1.0;
        float m_windSpeedKmPerSec = 0.0;
        AZ::Vector3 m_windDirection = { 1.0f, 1.0f, 0.0f };
        // Pushes the top of the clouds along the wind direction by this
        // distance. Useful for dramatic/artistic effects.
        float m_cloudTopOffsetKm = 0.0;
        // An RGBA Texture with weather map data. Info per channel:
        // R: Low coverage map.
        // G: High coverage map. Kicks in when @m_globalCloudCoverage > 0.5.
        // B: Peak height.
        // A: density
        AZ::Data::Instance<AZ::RPI::Image> m_weatherMap; // DO NOT REFLECT
        // ******************* Weather Data End
        //////////////////////////////////////////////////////////////


        // May come from a realtime texture generator (AttachmentImage)
        // or from a file (StreamingImage)
        AZ::Data::Instance<AZ::RPI::Image> m_lowFrequencyNoiseTexture; // DO NOT REFLECT (comes from an entity)

        // May come from a realtime texture generator (AttachmentImage)
        // or from a file (StreamingImage)

        AZ::Data::Instance<AZ::RPI::Image> m_highFrequencyNoiseTexture; // DO NOT REFLECT (comes from an entity)
    };

} // namespace VolumetricClouds
