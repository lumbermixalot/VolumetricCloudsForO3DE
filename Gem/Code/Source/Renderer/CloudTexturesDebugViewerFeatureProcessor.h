/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <Atom/RPI.Public/Buffer/Buffer.h>
#include <Atom/RPI.Public/PipelineState.h>
#include <Atom/RPI.Public/Shader/ShaderResourceGroup.h>
#include <Atom/RPI.Public/FeatureProcessor.h>

#include <VolumetricClouds/CloudTextureProviderBus.h>
#include <Renderer/CloudTexturePresentationData.h>

namespace VolumetricClouds
{
    class AZ::RPI::Scene;
    class AZ::RPI::Shader;

    class CloudTextureComputePass;

    class CloudTexturesDebugViewerFeatureProcessor final
        : public AZ::RPI::FeatureProcessor
        , private AZ::Data::AssetBus::Handler
        , private CloudTextureProviderNotificationBus::MultiHandler
    {
    public:
        AZ_CLASS_ALLOCATOR(CloudTexturesDebugViewerFeatureProcessor, AZ::SystemAllocator)
        AZ_RTTI(CloudTexturesDebugViewerFeatureProcessor, "{C83F4C25-33BE-4E77-9413-9B35F61483B3}", AZ::RPI::FeatureProcessor);

        static void Reflect(AZ::ReflectContext* context);

        CloudTexturesDebugViewerFeatureProcessor() = default;
        virtual ~CloudTexturesDebugViewerFeatureProcessor() = default;

        // There should be a TextureProviderRequestBus connected to @entityId so we can get the texture.
        bool AddCloudTextureInstance(const AZ::EntityId& entityId, AZ::Data::Instance<AZ::RPI::Image> image, const AZ::Transform& worldTM, const CloudTexturePresentationData& presentationData);
        bool RemoveCloudTextureInstance(const AZ::EntityId& entityId);

        bool UpdateWorldTransform(const AZ::EntityId& entityId, const AZ::Transform& worldTM);
        bool UpdatePresentationData(const AZ::EntityId& entityId, const CloudTexturePresentationData& presentationData);

    private:
        CloudTexturesDebugViewerFeatureProcessor(const CloudTexturesDebugViewerFeatureProcessor&) = delete;

        static constexpr char LogName[] = "CloudTexturesDebugViewerFeatureProcessor";

        ///////////////////////////////////////////////////////
        // CloudTextureProviderNotificationBus::MultiHandler overrides ...
        void OnCloudTextureImageReady(AZ::Data::Instance<AZ::RPI::Image> image) override;
        ///////////////////////////////////////////////////////
        
        
        struct CloudTextureInstance
        {
            // When true, the instance is ready to render.
            bool m_needsUpdate = true;

            AZ::Data::Instance<AZ::RPI::ShaderResourceGroup> m_drawSrg;
            AZ::RPI::ShaderOptionGroup m_shaderOptionGroup;
            AZ::RHI::ConstPtr<AZ::RHI::DrawPacket> m_drawPacket;

            AZ::Matrix4x4 m_worldMatrix = AZ::Matrix4x4::CreateIdentity();
            AZ::Data::Instance<AZ::RPI::Image> m_cloudTextureImage;
            CloudTexturePresentationData m_presentationData;

            void UpdateShaderOptionGroup(const AZ::Name& visibleChannelOptionName);
        };

        void ActivateInternal();
        void ReloadInstancesSrg();
        void ReloadInstanceSrg(const AZ::RHI::Ptr<AZ::RHI::ShaderResourceGroupLayout>& drawSrgLayout, CloudTextureInstance& cloudTextureInstance);

        /////////////////////////////////////////////////////////////////
        //! Data::AssetBus
        void OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        void OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset) override;
        //! Common functionality of AssetBus events.
        void OnAssetStateChanged(AZ::Data::Asset<AZ::Data::AssetData> asset, bool isReload);
        /////////////////////////////////////////////////////////////////

        //////////////////////////////////////////////////////////////////
        //! AZ::RPI::FeatureProcessor overrides START...
        void Activate() override;
        void Deactivate() override;
        void Simulate(const FeatureProcessor::SimulatePacket& packet) override;
        void Render(const FeatureProcessor::RenderPacket& packet) override;
        void AddRenderPasses(AZ::RPI::RenderPipeline* renderPipeline) override;
        // From RPI::SceneNotificationBus, but coming from AZ::RPI::FeatureProcessor
        void OnRenderPipelineChanged(AZ::RPI::RenderPipeline* pipeline, AZ::RPI::SceneNotification::RenderPipelineChangeType changeType) override;
        //! AZ::RPI::FeatureProcessor overrides END ...
        ///////////////////////////////////////////////////////////////////


        void UpdateShaderConstants();
        void UpdateDrawPacket(CloudTextureInstance& cloudTextureInstance);
        void UpdateDrawPackets();
        void ResetDrawPackets();


        //! build a draw packet to draw the clouds
        AZ::RHI::ConstPtr<AZ::RHI::DrawPacket> BuildDrawPacket(
            const AZ::RPI::Ptr<AZ::RPI::PipelineStateForDraw>& pipelineState,
            const AZ::RHI::DrawListTag& drawListTag,
            uint32_t vertexCount,
            const AZ::Data::Instance<AZ::RPI::ShaderResourceGroup>& srg);

        static constexpr const char* FeatureProcessorName = "CloudTexturesDebugViewerFeatureProcessor";

        // Set by CloudscapeComponentController.
        static constexpr char CloudTextureShaderPath[] = "Shaders/CloudTexture/CloudTextureViewer.azshader";
        AZ::Data::Asset<AZ::RPI::ShaderAsset> m_shaderAsset;
        AZ::Data::Instance<AZ::RPI::Shader> m_shader = nullptr;
        // The pipeline state can only be created during OnRenderPipelineChanged (RenderPipelineChangeType::Added).
        // It also can be updated during OnRenderPipelineChanged (RenderPipelineChangeType::PassChanged).
        AZ::RPI::Ptr<AZ::RPI::PipelineStateForDraw> m_meshPipelineState;

        AZ::RHI::DrawListTag m_drawListTag;

        AZStd::unordered_map<AZ::EntityId, CloudTextureInstance> m_cloudTextureInstances;

        // SRG Constants Indices and the constant
        const AZ::Name m_visibleChannelOptionName{"o_visible_channel"};
        AZ::RHI::ShaderInputNameIndex m_modelToWorldIndex = "m_modelToWorld";
        AZ::RHI::ShaderInputNameIndex m_alwaysFaceCameraIndex = "m_alwaysFaceCamera";
        AZ::RHI::ShaderInputNameIndex m_textureIndex = "m_texture";
        AZ::RHI::ShaderInputNameIndex m_texCoordZIndex = "m_texCoordZ";

        AzFramework::WindowSize m_viewportSize{0,0};
    };
} // namespace VolumetricClouds
