/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

#include <VolumetricClouds/VolumetricCloudsTypeIds.h>
#include "CloudTextureComputeData.h"

namespace VolumetricClouds
{
    AZ_CLASS_ALLOCATOR_IMPL(CloudTextureComputeData, AZ::SystemAllocator);
    AZ_TYPE_INFO_WITH_NAME_IMPL(CloudTextureComputeData, "VolumetricClouds::CloudTextureComputeData", CloudTextureComputeDataTypeId);
    AZ_RTTI_NO_TYPE_INFO_IMPL(CloudTextureComputeData);

    void CloudTextureComputeData::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CloudTextureComputeData>()
                ->Version(1)
                ->Field("PixelSize", &CloudTextureComputeData::m_pixelSize)
                ->Field("Frequency", &CloudTextureComputeData::m_frequency)
                ->Field("PerlinOctaves",   &CloudTextureComputeData::m_perlinOctaves)
                ->Field("PerlinGain",      &CloudTextureComputeData::m_perlinGain)
                ->Field("PerlinAmplitude", &CloudTextureComputeData::m_perlinAmplitude)
                ->Field("WorleyOctaves",   &CloudTextureComputeData::m_worleyOctaves)
                ->Field("WorleyGain",      &CloudTextureComputeData::m_worleyGain)
                ->Field("WorleyAmplitude", &CloudTextureComputeData::m_worleyAmplitude)
                ;

            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<CloudTextureComputeData>(
                    "CloudTextureComputeData", "Configuration data for the Volumetric Clouds Component.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::Show)
                    ->DataElement(AZ::Edit::UIHandlers::ComboBox, &CloudTextureComputeData::m_pixelSize, "Pixel Size", "W,H,D dimensions in pixels for the Texture3D")
                        ->EnumAttribute(CloudTexturePixelSize::PixelSize16,  "16 pixels")
                        ->EnumAttribute(CloudTexturePixelSize::PixelSize32,  "32 pixels")
                        ->EnumAttribute(CloudTexturePixelSize::PixelSize64,  "64 pixels")
                        ->EnumAttribute(CloudTexturePixelSize::PixelSize128, "128 pixels")
                        ->EnumAttribute(CloudTexturePixelSize::PixelSize256, "256 pixels")
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudTextureComputeData::m_frequency, "Frequency", "The starting frequency for the noise FBM.")
                        ->Attribute(AZ::Edit::Attributes::Min, 1.0)
                        ->Attribute(AZ::Edit::Attributes::Max, 10.0)
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudTextureComputeData::m_perlinOctaves, "Perlin Octaves", "Number of octaves for the perlin noise FBM.")
                        ->Attribute(AZ::Edit::Attributes::Min, 1)
                        ->Attribute(AZ::Edit::Attributes::Max, 10)
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudTextureComputeData::m_perlinGain, "Perlin Gain", "For each perlin octave, in the perlin Fbm, the amplitude will be multiplied by this gain (aka persistence).")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.1)
                        ->Attribute(AZ::Edit::Attributes::Max, 2.0)
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudTextureComputeData::m_perlinAmplitude, "Perlin Amplitude", "Starting amplitude of the perlin Fbm.")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.1)
                        ->Attribute(AZ::Edit::Attributes::Max, 2.0)
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudTextureComputeData::m_worleyOctaves, "Worley Octaves", "Number of octaves for the worley noise FBM.")
                        ->Attribute(AZ::Edit::Attributes::Min, 1)
                        ->Attribute(AZ::Edit::Attributes::Max, 10)
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudTextureComputeData::m_worleyGain, "Worley Gain", "For each worley octave, in the worley Fbm, the amplitude will be multiplied by this gain (aka persistence).")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.1)
                        ->Attribute(AZ::Edit::Attributes::Max, 2.0)
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudTextureComputeData::m_worleyAmplitude, "Worley Amplitude", "Starting amplitude of the worley Fbm.")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.1)
                        ->Attribute(AZ::Edit::Attributes::Max, 2.0)
                    ;
            }
        }

    }

    bool CloudTextureComputeData::operator==(const CloudTextureComputeData& rhs) const
    {
        return (m_pixelSize == rhs.m_pixelSize) &&
            (m_frequency       == rhs.m_frequency) &&
            (m_perlinOctaves   == rhs.m_perlinOctaves) &&
            (m_perlinGain      == rhs.m_perlinGain) &&
            (m_perlinAmplitude == rhs.m_perlinAmplitude) &&
            (m_worleyOctaves   == rhs.m_worleyOctaves) &&
            (m_worleyGain      == rhs.m_worleyGain) &&
            (m_worleyAmplitude == rhs.m_worleyAmplitude)
            ;
    }

    bool CloudTextureComputeData::operator!=(const CloudTextureComputeData& rhs) const
    {
        return !(*this == rhs);
    }

} // namespace VolumetricClouds
