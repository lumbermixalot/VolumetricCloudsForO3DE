/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TransformBus.h>

#include <Atom/RPI.Public/ViewportContextBus.h>
#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>
#include <AtomLyIntegration/CommonFeatures/CoreLights/DirectionalLightBus.h>

#include <VolumetricClouds/VolumetricCloudsBus.h>
#include <VolumetricClouds/CloudTextureProviderBus.h>
#include <Renderer/CloudscapeShaderConstantData.h>

namespace AZ::RPI {
    class Scene;
}

namespace VolumetricClouds
{
    class CloudscapeFeatureProcessor;

    //! Common configuration for CloudscapeComponent that can be used to create
    //! CloudscapeComponents dynamically and can be shared with EditorCloudscapeComponent.
    class CloudscapeComponentConfig final
        : public AZ::ComponentConfig
    {
    public:
        AZ_RTTI(CloudscapeComponentConfig, "{6E894FEA-BAE8-4089-B76D-B082C4A5B394}", AZ::ComponentConfig);
        AZ_CLASS_ALLOCATOR(CloudscapeComponentConfig, AZ::SystemAllocator);
    
        static void Reflect(AZ::ReflectContext* context);
        
        // Typically a 128x128x128 texture3D
        AZ::EntityId m_lowFreqTextureEntity;
        // Typically a 32x32x32 texture3D
        AZ::EntityId m_highFreqTextureEntity;

        // An entity that has a directional light component which will be used
        // to define the color of the sun light and direction-towards-the-sun.
        AZ::EntityId m_sunEntity;

        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> m_weatherMap;

        CloudscapeShaderConstantData m_shaderConstantData;
    };
    

    class CloudscapeComponentController final
        : private CloudTextureProviderNotificationBus::MultiHandler
        , private AZ::Data::AssetBus::Handler
        , private AZ::TransformNotificationBus::Handler // To detect changes in Sun direction.
        , private AZ::RPI::ViewportContextIdNotificationBus::Handler
        , public VolumetricCloudsRequestBus::Handler
    {
    public:
        friend class EditorCloudscapeComponent;
    
        AZ_CLASS_ALLOCATOR(CloudscapeComponentController, AZ::SystemAllocator);
        AZ_RTTI(CloudscapeComponentController, "{2B0304E8-15F1-4A46-A4FA-0B2AC5197ED1}");
    
        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
    
        CloudscapeComponentController();
        CloudscapeComponentController(const CloudscapeComponentConfig& config);
    
        void Activate(AZ::EntityId entityId);
        void Deactivate();
        void SetConfiguration(const CloudscapeComponentConfig& config);
        const CloudscapeComponentConfig& GetConfiguration() const;

        /////////////////////////////////////////////////////////
        // VolumetricCloudsRequestBus::Handler overrides START
        void BeginCallBatch() override;
        bool IsCallBatching() override;

        float GetUVWScale() override;
        void SetUVWScale(float uvwScale) override;
        uint32_t GetMaxMipLevels() override;
        void SetMaxMipLevels(uint32_t maxMipLevels) override;
        AZStd::tuple<uint8_t, uint8_t> GetRayMarchingSteps() override;
        void SetRayMarchingSteps(uint8_t min, uint8_t max) override;
        // Planetary Data
        float GetPlanetRadiusKm() override;
        void SetPlanetRadiusKm(float radiusKm) override;
        float GetDistanceToCloudSlabKm() override;
        void SetDistanceToCloudSlabKm(float distanceKm) override;
        float GetCloudSlabThicknessKm() override;
        void SetCloudSlabThicknessKm(float thicknessKm) override;
        AZ::Color GetSunLightColorAndIntensity() override;
        void SetSunLightColorAndIntensity(const AZ::Color& rgbColorAlphaIntensity) override;
        AZ::Color GetAmbientLightColorAndIntensity() override;
        void SetAmbientLightColorAndIntensity(const AZ::Color& rgbColorAlphaIntensity) override;
        // Weather Data
        float GetWeatherMapSizeKm() override;
        void SetWeatherMapSizeKm(float mapSizeKm) override;
        float GetCloudCoverage() override;
        void SetCloudCoverage(float coverage) override;
        float GetCloudDensity() override;
        void SetCloudDensity(float density) override;
        AZ::Vector3 GetWindVelocity() override;
        void SetWindVelocity(const AZ::Vector3& velocity) override;
        float GetCloudTopShiftKm() override;
        void SetCloudTopShiftKm(float topShiftKm) override;
        // Cloud Material Properties
        const CloudMaterialProperties& GetCloudMaterialProperties() override;
        void SetCloudMaterialProperties(const CloudMaterialProperties& cmp) override;

        void EndCallBatch() override;
        // VolumetricCloudsRequestBus::Handler overrides END
        /////////////////////////////////////////////////////////

    private:
        AZ_DISABLE_COPY(CloudscapeComponentController);
        static constexpr char LogName[] = "CloudscapeComponentController";

        void OnConfigurationChanged();
        void EnableFeatureProcessor();

        void FetchAllSunLightData();
        void NotifySunLightDataChanged();

        // A helper function that makes sure the shader constant data
        // make sense and are clamped within good boundaries before being sent to the
        // feature processor.
        void SubmitShaderConstantData();

        //////////////////////////////////////////////////////////////////
        //! CloudTextureProviderNotificationBus overrides START...
        void OnCloudTextureImageReady(AZ::Data::Instance<AZ::RPI::Image> image) override;
        //! CloudTextureProviderNotificationBus overrides END ...
        ///////////////////////////////////////////////////////////////////

        //! Data::AssetBus
        void OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        //! Common functionality of AssetBus events.
        void OnAssetStateChanged(AZ::Data::Asset<AZ::Data::AssetData> asset, bool isReload);

        ////////////////////////////////////////////////////////////////////
        //! AZ::TransformNotificationBus::Handler
        void OnTransformChanged(const AZ::Transform& /*local*/, const AZ::Transform& /*world*/) override;
        ////////////////////////////////////////////////////////////////////

        //! RPI::ViewportContextIdNotificationBus
        void OnViewportSizeChanged(AzFramework::WindowSize size) override;

        // This boolean was added so only one Volumetric Cloudscape component is active per level.
        bool m_isActive = false;
        
        // If true, EBUS apis won't submit changes to the renderer, AND only  EndCallBatch()
        // will have such effect.
        // If false, EBUS apis will immediately submit changes to the renderer.
        bool m_isBatchingShaderConstantChanges = false;
    
        AZ::EntityId m_entityId;
        CloudscapeComponentConfig m_configuration;
        CloudscapeComponentConfig m_prevConfiguration;

        AZ::RPI::Scene* m_scene; //Cache a reference to the scene where @m_entityId exists.
        CloudscapeFeatureProcessor* m_cloudscapeFeatureProcessor = nullptr;

        AZ::Render::DirectionalLightConfigurationChangedEvent::Handler m_directionalLightConfigChangedEventHandler;
    };

} // namespace VolumetricClouds
