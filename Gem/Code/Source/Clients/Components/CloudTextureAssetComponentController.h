/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <AzCore/Component/Component.h>
#include <AzCore/Component/TransformBus.h>

#include <Atom/RPI.Reflect/Image/StreamingImageAsset.h>
#include <Atom/RPI.Public/Image/StreamingImage.h>

#include <Renderer/CloudTexturePresentationData.h>
#include <VolumetricClouds/CloudTextureProviderBus.h>

namespace AZ::RPI {
    class Scene;
}

namespace VolumetricClouds
{
    class CloudTexturesDebugViewerFeatureProcessor;

    //! Common configuration for CloudTextureAssetComponent that can be used to create
    //! CloudTextureAssetComponents dynamically and can be shared with EditorCloudTextureAssetComponent.
    class CloudTextureAssetComponentConfig final
        : public AZ::ComponentConfig
    {
    public:
        AZ_RTTI(CloudTextureAssetComponentConfig, "{219D8BD9-1A74-4011-AEAE-C1D489A3CEDC}", AZ::ComponentConfig);
        AZ_CLASS_ALLOCATOR(CloudTextureAssetComponentConfig, AZ::SystemAllocator);
    
        static void Reflect(AZ::ReflectContext* context);

        AZ::Data::Asset<AZ::RPI::StreamingImageAsset> m_cloudTextureAsset;

        // How to Debug render the Texture3D in the scene.
        CloudTexturePresentationData m_presentationData;

    };
    
    class CloudTextureAssetComponentController final
        : private AZ::TransformNotificationBus::Handler
        , public CloudTextureProviderRequestBus::Handler
        , private AZ::Data::AssetBus::Handler
    {
    public:
        friend class EditorCloudTextureAssetComponent;
    
        AZ_CLASS_ALLOCATOR(CloudTextureAssetComponentController, AZ::SystemAllocator);
        AZ_RTTI(CloudTextureAssetComponentController, "{4420CD6B-FEAC-4F0A-ADC5-EE936180DEF2}");

        static void Reflect(AZ::ReflectContext* context);
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
    
        CloudTextureAssetComponentController() = default;
        CloudTextureAssetComponentController(const CloudTextureAssetComponentConfig& config);
    
        void Activate(AZ::EntityId entityId);
        void Deactivate();
        void SetConfiguration(const CloudTextureAssetComponentConfig& config);
        const CloudTextureAssetComponentConfig& GetConfiguration() const;

        /////////////////////////////////////////////////////////
        // CloudTextureProviderRequestBus::Handler overrides ....
        AZ::Data::Instance<AZ::RPI::Image> GetCloudTextureImage() override;
        /////////////////////////////////////////////////////////

    private:
        AZ_DISABLE_COPY(CloudTextureAssetComponentController);
        static constexpr char LogName[] = "CloudTextureAssetComponentController";

        //! AZ::TransformNotificationBus::Handler
        void OnTransformChanged(const AZ::Transform& /*local*/, const AZ::Transform& /*world*/) override;

        //! Data::AssetBus
        void OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        //! Common functionality of AssetBus events.
        void OnAssetStateChanged(AZ::Data::Asset<AZ::Data::AssetData> asset, bool isReload);

        void OnConfigurationChanged();

        CloudTexturesDebugViewerFeatureProcessor* GetDebugViewerFeatureProcessor();
    
        AZ::EntityId m_entityId;
        CloudTextureAssetComponentConfig m_configuration;
        CloudTextureAssetComponentConfig m_prevConfiguration;

        // We don't own this. We just cache these references.
        CloudTexturesDebugViewerFeatureProcessor* m_debugViewerFeatureProcessor = nullptr;

        AZ::Data::Instance<AZ::RPI::StreamingImage> m_cloudTextureImage;
    };

} // namespace VolumetricClouds
