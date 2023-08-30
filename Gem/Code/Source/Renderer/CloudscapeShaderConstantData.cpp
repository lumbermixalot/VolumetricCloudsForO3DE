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
#include "CloudscapeShaderConstantData.h"

namespace VolumetricClouds
{
    AZ_CLASS_ALLOCATOR_IMPL(CloudscapeShaderConstantData, AZ::SystemAllocator);
    AZ_TYPE_INFO_WITH_NAME_IMPL(CloudscapeShaderConstantData, "VolumetricClouds::CloudscapeShaderConstantData", CloudscapeShaderConstantDataTypeId);
    AZ_RTTI_NO_TYPE_INFO_IMPL(CloudscapeShaderConstantData);

    void CloudscapeShaderConstantData::Reflect(AZ::ReflectContext* context)
    {
        CloudMaterialProperties::Reflect(context);

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CloudscapeShaderConstantData>()
                ->Version(1)
                ->Field("UVWScale", &CloudscapeShaderConstantData::m_uvwScale)
                ->Field("MaxMipLevels", &CloudscapeShaderConstantData::m_maxMipLevels)
                ->Field("MinRayMarchingSteps", &CloudscapeShaderConstantData::m_minRayMarchingSteps)
                ->Field("MaxRayMarchingSteps", &CloudscapeShaderConstantData::m_maxRayMarchingSteps)
                ->Field("PlanetRadiusKm", &CloudscapeShaderConstantData::m_planetRadiusKm)
                ->Field("CloudSlabDistanceAboveSeaLevelKm", &CloudscapeShaderConstantData::m_cloudSlabDistanceAboveSeaLevelKm)
                ->Field("CloudSlabThicknessKm", &CloudscapeShaderConstantData::m_cloudSlabThicknessKm)
                ->Field("SunLightIntensity", &CloudscapeShaderConstantData::m_sunLightIntensity)
                ->Field("AmbientLightColor", &CloudscapeShaderConstantData::m_ambientLightColor)
                ->Field("AmbientLightIntensity", &CloudscapeShaderConstantData::m_ambientLightIntensity)
                ->Field("WeatherMapSizeKm", &CloudscapeShaderConstantData::m_weatherMapSizeKm)
                ->Field("GlobalCloudCoverage", &CloudscapeShaderConstantData::m_globalCloudCoverage)
                ->Field("GlobalCloudDensity", &CloudscapeShaderConstantData::m_globalCloudDensity)
                ->Field("WindSpeedKmPerSec", &CloudscapeShaderConstantData::m_windSpeedKmPerSec)
                ->Field("WindDirection", &CloudscapeShaderConstantData::m_windDirection)
                ->Field("CloudTopOffsetKm", &CloudscapeShaderConstantData::m_cloudTopOffsetKm)
                ->Field("CloudMaterialProperties", &CloudscapeShaderConstantData::m_cloudMaterialProperties)
                ;

            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<CloudscapeShaderConstantData>(
                    "Shader Constants", "Data for the Cloudscape shader.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::Show)
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudscapeShaderConstantData::m_uvwScale, "UVW Scale", "Used to scale world position XYZ when sampling the Noise Textures during ray marching.")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.0001)
                        ->Attribute(AZ::Edit::Attributes::Max, 1.0)
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudscapeShaderConstantData::m_maxMipLevels, "Max Mip Levels", "Maximum number of mip levels used during noise texture sampling.")
                        ->Attribute(AZ::Edit::Attributes::Min, 1)
                        ->Attribute(AZ::Edit::Attributes::Max, 10)
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Ray Marching Steps")
                        ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudscapeShaderConstantData::m_minRayMarchingSteps, "Min", "The shorter the ray marching distance, the ray marching steps will be closer to this minimum count.")
                            ->Attribute(AZ::Edit::Attributes::Min, 1)
                            ->Attribute(AZ::Edit::Attributes::Max, 128)
                        ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudscapeShaderConstantData::m_maxRayMarchingSteps, "Max", "The longest the ray marching distance, the ray marching steps will be closer to this maximum count.")
                            ->Attribute(AZ::Edit::Attributes::Min, 1)
                            ->Attribute(AZ::Edit::Attributes::Max, 128)
                    ->EndGroup()
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Planetary Data")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &CloudscapeShaderConstantData::m_planetRadiusKm, "Planet Radius", "Defaults to Earth Radius = 6371[Km]")
                            ->Attribute(AZ::Edit::Attributes::Suffix, " Km")
                        ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudscapeShaderConstantData::m_cloudSlabDistanceAboveSeaLevelKm, "Cloud Slab Distance", "The distance, above sea level, where the cloud slab starts.")
                            ->Attribute(AZ::Edit::Attributes::Suffix, " Km")
                            ->Attribute(AZ::Edit::Attributes::Min, 0.5)
                            ->Attribute(AZ::Edit::Attributes::Max, 10.0)
                        ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudscapeShaderConstantData::m_cloudSlabThicknessKm, "Cloud Slab Thickness", "The thickness, also known as depth, of the cloud slab.")
                            ->Attribute(AZ::Edit::Attributes::Suffix, " Km")
                            ->Attribute(AZ::Edit::Attributes::Min, 0.5)
                            ->Attribute(AZ::Edit::Attributes::Max, 10.0)
                        ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudscapeShaderConstantData::m_sunLightIntensity, "Sun Light Intensity", "A scaling factor to the color of the sun light.")
                            ->Attribute(AZ::Edit::Attributes::Min, 0.0)
                            ->Attribute(AZ::Edit::Attributes::Max, 100.0)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &CloudscapeShaderConstantData::m_ambientLightColor, "Ambient Light Color", "The ambient light color, which will be modulated by the Sun Light Color.")
                        ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudscapeShaderConstantData::m_ambientLightIntensity, "Ambient Light Intensity", "A scaling factor to the color of the ambient light. It will be modulated by the Sun Light Intensity.")
                            ->Attribute(AZ::Edit::Attributes::Min, 0.0)
                            ->Attribute(AZ::Edit::Attributes::Max, 10.0)
                    ->EndGroup()
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Weather Data")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudscapeShaderConstantData::m_weatherMapSizeKm, "Weather Map Size", "Length, in world dimensions, of the weather map.")
                            ->Attribute(AZ::Edit::Attributes::Suffix, " Km")
                            ->Attribute(AZ::Edit::Attributes::Min, 1.0)
                            ->Attribute(AZ::Edit::Attributes::Max, 100.0)
                        ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudscapeShaderConstantData::m_globalCloudCoverage, "Cloud Coverage", "Global interpolation factor between Red channel (low coverage) and Green channel (high coverage).")
                            ->Attribute(AZ::Edit::Attributes::Min, 0.0)
                            ->Attribute(AZ::Edit::Attributes::Max, 1.0)
                        ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudscapeShaderConstantData::m_globalCloudDensity, "Cloud Density", "Global cloud density modulator.")
                            ->Attribute(AZ::Edit::Attributes::Min, 0.0)
                            ->Attribute(AZ::Edit::Attributes::Max, 1.0)
                        ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudscapeShaderConstantData::m_windSpeedKmPerSec, "Wind Speed", "Clouds will appear to move at the speed of the wind.")
                            ->Attribute(AZ::Edit::Attributes::Suffix, " Km/s")
                            ->Attribute(AZ::Edit::Attributes::Min, 0.0)
                            ->Attribute(AZ::Edit::Attributes::Max, 10.0)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &CloudscapeShaderConstantData::m_windDirection, "Wind Direction", "")
                        ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudscapeShaderConstantData::m_cloudTopOffsetKm, "Cloud Top Shift Offset", "Proportionally pushes the top of the clouds along the wind direction by this distance.")
                            ->Attribute(AZ::Edit::Attributes::Suffix, " Km")
                            ->Attribute(AZ::Edit::Attributes::Min, -5.0)
                            ->Attribute(AZ::Edit::Attributes::Max, 5.0)
                    ->EndGroup()
                    ->DataElement(AZ::Edit::UIHandlers::Default, &CloudscapeShaderConstantData::m_cloudMaterialProperties, "Cloud Material Properties", "")
                    ;
            }
        }

    }

    bool CloudscapeShaderConstantData::operator==(const CloudscapeShaderConstantData& rhs) const
    {
        return AZ::IsClose(m_uvwScale, rhs.m_uvwScale, 0.00001f) &&
               (m_maxMipLevels == rhs.m_maxMipLevels) &&
               (m_minRayMarchingSteps == rhs.m_minRayMarchingSteps) &&
               (m_maxRayMarchingSteps == rhs.m_maxRayMarchingSteps) &&
               (m_planetRadiusKm ==  rhs.m_planetRadiusKm) &&
               AZ::IsClose(m_cloudSlabDistanceAboveSeaLevelKm, rhs.m_cloudSlabDistanceAboveSeaLevelKm) &&
               AZ::IsClose(m_cloudSlabThicknessKm, rhs.m_cloudSlabThicknessKm) &&
               AZ::IsClose(m_weatherMapSizeKm, rhs.m_weatherMapSizeKm) &&
               AZ::IsClose(m_sunLightIntensity, rhs.m_sunLightIntensity) &&
               m_ambientLightColor.IsClose(rhs.m_ambientLightColor) &&
               AZ::IsClose(m_ambientLightIntensity, rhs.m_ambientLightIntensity) &&
               AZ::IsClose(m_globalCloudCoverage, rhs.m_globalCloudCoverage) &&
               AZ::IsClose(m_globalCloudDensity, rhs.m_globalCloudDensity) &&
               AZ::IsClose(m_windSpeedKmPerSec, rhs.m_windSpeedKmPerSec) &&
               m_windDirection.IsClose(rhs.m_windDirection) &&
               AZ::IsClose(m_cloudTopOffsetKm, rhs.m_cloudTopOffsetKm) &&
               (m_cloudMaterialProperties == rhs.m_cloudMaterialProperties)
               ;
    }

    bool CloudscapeShaderConstantData::operator!=(const CloudscapeShaderConstantData& rhs) const
    {
        return !(*this == rhs);
    }

} // namespace VolumetricClouds
