/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/
#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

#include <VolumetricClouds/VolumetricCloudsTypeIds.h>

namespace VolumetricClouds
{
    class CloudTexturesFeatureProcessor;
    struct CloudMaterialProperties;

    class VolumetricCloudsRequests
    {
    public:
        AZ_RTTI(VolumetricCloudsRequests, VolumetricCloudsRequestsTypeId);
        virtual ~VolumetricCloudsRequests() = default;

        // Put your public methods here

        // By default when calling the APIs they would trigger immediate modification
        // of the cloud rendering system. For cases when several parameters should be modified
        // before commiting to the renderer, call BeginCallBatch(), then call all the Set..() functions
        // and in the end call EndCallBatch() which will submit all the parameters to the renderer.
        virtual void BeginCallBatch() = 0;
        virtual bool IsCallBatching() = 0;

        virtual float GetUVWScale() = 0;
        virtual void SetUVWScale(float uvwScale) = 0;
        virtual uint32_t GetMaxMipLevels() = 0;
        virtual void SetMaxMipLevels(uint32_t maxMipLevels) = 0;
        virtual AZStd::tuple<uint8_t, uint8_t> GetRayMarchingSteps() = 0;
        virtual void SetRayMarchingSteps(uint8_t min, uint8_t max) = 0;
        // Planetary Data
        virtual float GetPlanetRadiusKm() = 0;
        virtual void SetPlanetRadiusKm(float radiusKm) = 0;
        virtual float GetDistanceToCloudSlabKm() = 0;
        virtual void SetDistanceToCloudSlabKm(float distanceKm) = 0;
        virtual float GetCloudSlabThicknessKm() = 0;
        virtual void SetCloudSlabThicknessKm(float thicknessKm) = 0;
        virtual AZ::Color GetSunLightColorAndIntensity() = 0; // Alpha is the intensity
        virtual void SetSunLightColorAndIntensity(const AZ::Color& rgbColorAlphaIntensity) = 0;
        virtual AZ::Color GetAmbientLightColorAndIntensity() = 0; // Alpha is the intensity
        virtual void SetAmbientLightColorAndIntensity(const AZ::Color& rgbColorAlphaIntensity) = 0;
        // Weather Data
        virtual float GetWeatherMapSizeKm() = 0;
        virtual void SetWeatherMapSizeKm(float mapSizeKm) = 0;
        virtual float GetCloudCoverage() = 0;
        virtual void SetCloudCoverage(float coverage) = 0;
        virtual float GetCloudDensity() = 0;
        virtual void SetCloudDensity(float density) = 0;
        virtual AZ::Vector3 GetWindVelocity() = 0;
        virtual void SetWindVelocity(const AZ::Vector3& velocity) = 0;
        virtual float GetCloudTopShiftKm() = 0;
        virtual void SetCloudTopShiftKm(float topShiftKm) = 0;
        // Cloud Material Properties 
        virtual const CloudMaterialProperties& GetCloudMaterialProperties() = 0;
        virtual void SetCloudMaterialProperties(const CloudMaterialProperties& cmp) = 0;

        // Submits the current state of the parameters to the renderer.
        virtual void EndCallBatch() = 0;

    };

    class VolumetricCloudsBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using VolumetricCloudsRequestBus = AZ::EBus<VolumetricCloudsRequests, VolumetricCloudsBusTraits>;
    using VolumetricCloudsInterface = AZ::Interface<VolumetricCloudsRequests>;

} // namespace VolumetricClouds
