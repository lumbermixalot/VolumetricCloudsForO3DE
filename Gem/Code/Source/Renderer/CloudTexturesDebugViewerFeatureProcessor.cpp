/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <AzCore/Name/NameDictionary.h>

#include <Atom/RHI/DrawPacketBuilder.h>
#include <Atom/RHI.Reflect/InputStreamLayoutBuilder.h>

#include <Atom/RPI.Public/Image/AttachmentImagePool.h>
#include <Atom/RPI.Public/RenderPipeline.h>
#include <Atom/RPI.Public/RPIUtils.h>
#include <Atom/RPI.Public/Pass/PassFilter.h>
#include <Atom/RPI.Public/Shader/Shader.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/View.h>
#include <Atom/RPI.Public/ViewportContext.h>
#include <Atom/RPI.Public/ViewportContextBus.h>
#include <Atom/RPI.Public/Image/ImageSystemInterface.h>

#include "CloudTexturesDebugViewerFeatureProcessor.h"

namespace VolumetricClouds
{
    void CloudTexturesDebugViewerFeatureProcessor::CloudTextureInstance::UpdateShaderOptionGroup(const AZ::Name& visibleChannelOptionName)
    {
        switch (m_presentationData.m_visibleChannel)
        {
        case VisibleChannel::PerlinWorley:
            m_shaderOptionGroup.SetValue(visibleChannelOptionName, AZ::Name{"VisibleChannel::PerlinWorley"});
            break;
        case VisibleChannel::WorleyFbm1:
            m_shaderOptionGroup.SetValue(visibleChannelOptionName, AZ::Name{"VisibleChannel::WorleyFbm1"});
            break;
        case VisibleChannel::WorleyFbm2:
            m_shaderOptionGroup.SetValue(visibleChannelOptionName, AZ::Name{"VisibleChannel::WorleyFbm2"});
            break;
        case VisibleChannel::WorleyFbm4:
            m_shaderOptionGroup.SetValue(visibleChannelOptionName, AZ::Name{"VisibleChannel::WorleyFbm4"});
            break;
        default:
            m_shaderOptionGroup.SetValue(visibleChannelOptionName, AZ::Name{"VisibleChannel::AllChannels"});
            break;
        }
        m_needsUpdate = true;
    }

    void CloudTexturesDebugViewerFeatureProcessor::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext
                ->Class<CloudTexturesDebugViewerFeatureProcessor, AZ::RPI::FeatureProcessor>()
                ->Version(1);
        }
    }

    /////////////////////////////////////////////////////////////////////////////
    //! AZ::RPI::FeatureProcessor overrides START ...
    void CloudTexturesDebugViewerFeatureProcessor::Activate()
    {
        auto shaderAssetId = AZ::RPI::GetShaderAssetId(CloudTextureShaderPath);
        AZ_Assert(shaderAssetId.IsValid(), "Failed to find assetId for shader=%s", CloudTextureShaderPath);

        m_shaderAsset = AZ::Data::AssetManager::Instance().GetAsset<AZ::RPI::ShaderAsset>(shaderAssetId, AZ::Data::AssetLoadBehavior::PreLoad);
        AZ_Assert(!!m_shaderAsset, "Failed to find ShaderAsset with AssetId=%s and path=%s", shaderAssetId.ToFixedString().c_str(), CloudTextureShaderPath);

        AZ::Data::AssetBus::Handler::BusConnect(shaderAssetId);

        // Whether the shader asset is ready or not. OnAssetReady is always called. 
        // OnAssetReady will call ActivateInternal()
    }


    void CloudTexturesDebugViewerFeatureProcessor::Deactivate()
    {
        AZ::Data::AssetBus::Handler::BusDisconnect();

        DisableSceneNotification();

        m_shaderAsset.Reset();
        m_shader = nullptr;
        m_meshPipelineState = nullptr;
        m_viewportSize = { 0,0 };
        m_cloudTextureInstances.clear();

    }


    void CloudTexturesDebugViewerFeatureProcessor::Simulate([[maybe_unused]] const FeatureProcessor::SimulatePacket& packet)
    {
        AZ_PROFILE_SCOPE(RPI, "CloudTexturesDebugViewerFeatureProcessor: Simulate");

        UpdateShaderConstants();

    }

    void CloudTexturesDebugViewerFeatureProcessor::Render(const FeatureProcessor::RenderPacket& packet)
    {
        AZ_PROFILE_FUNCTION(AzRender);

        if (m_cloudTextureInstances.empty())
        {
            return;
        }

        constexpr float depth = 0.f;
        for (auto& view : packet.m_views)
        {
            if (!view->HasDrawListTag(m_drawListTag))
            {
                continue;
            }

            for (const auto& [entityId, cloudTextureInstance] : m_cloudTextureInstances)
            {
                if (
                    !cloudTextureInstance.m_drawPacket ||
                    (cloudTextureInstance.m_presentationData.m_visibleChannel == VisibleChannel::None)
                   )
                {
                    continue;
                }
                view->AddDrawPacket(cloudTextureInstance.m_drawPacket.get(), depth);
            }
        }

    }

    void CloudTexturesDebugViewerFeatureProcessor::AddRenderPasses([[maybe_unused]] AZ::RPI::RenderPipeline* renderPipeline)
    {
        AddPassRequestToRenderPipeline(renderPipeline, "Passes/CloudTextureRasterPassRequest.azasset", "AuxGeomPass", true);
    }


    void CloudTexturesDebugViewerFeatureProcessor::OnRenderPipelineChanged([[maybe_unused]] AZ::RPI::RenderPipeline* renderPipeline,
        AZ::RPI::SceneNotification::RenderPipelineChangeType changeType)
    {
        switch (changeType)
        {
        case AZ::RPI::SceneNotification::RenderPipelineChangeType::Added:
        case AZ::RPI::SceneNotification::RenderPipelineChangeType::PassChanged:
            if(!m_meshPipelineState)
            {
                m_meshPipelineState = aznew AZ::RPI::PipelineStateForDraw;
                m_meshPipelineState->Init(m_shader);


                AZ::RHI::InputStreamLayoutBuilder layoutBuilder;
                layoutBuilder.SetTopology(AZ::RHI::PrimitiveTopology::TriangleList);
                auto inputStreamLayout = layoutBuilder.End();

                m_meshPipelineState->SetInputStreamLayout(inputStreamLayout);
            }

            m_meshPipelineState->SetOutputFromScene(GetParentScene());
            const auto pipelineStatePtr = m_meshPipelineState->Finalize();

            if (pipelineStatePtr && pipelineStatePtr->IsInitialized())
            {
                UpdateDrawPackets();
            }
            else
            {
                ResetDrawPackets();
            }
            break;
        }
        
    }

    //! AZ::RPI::FeatureProcessor overrides END ...
    /////////////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////////
    //! Functions called by CloudscapeComponentController START
    bool CloudTexturesDebugViewerFeatureProcessor::AddCloudTextureInstance(const AZ::EntityId& entityId, AZ::Data::Instance<AZ::RPI::Image> image, const AZ::Transform& worldTM,
        const CloudTexturePresentationData& presentationData)
    {
        if (m_cloudTextureInstances.contains(entityId))
        {
            AZ_Error(LogName, false, "CloudTextureInstance with entityId=%s already exists!", entityId.ToString().c_str());
            return false;
        }

        CloudTextureInstance newInstance;

        CloudTextureProviderNotificationBus::MultiHandler::BusConnect(entityId);

        newInstance.m_cloudTextureImage = image;
        newInstance.m_worldMatrix = AZ::Matrix4x4::CreateFromTransform(worldTM);
        newInstance.m_presentationData = presentationData;
        if (m_shaderAsset.IsReady() && m_shader)
        {
            ReloadInstanceSrg(m_shaderAsset->GetDrawSrgLayout(m_shader->GetSupervariantIndex()), newInstance);
            newInstance.UpdateShaderOptionGroup(m_visibleChannelOptionName);
            UpdateDrawPacket(newInstance);
        }

        m_cloudTextureInstances.emplace(entityId, newInstance);


        return true;
    }

    bool CloudTexturesDebugViewerFeatureProcessor::RemoveCloudTextureInstance(const AZ::EntityId& entityId)
    {
        if (!m_cloudTextureInstances.contains(entityId))
        {
            AZ_Error(LogName, false, "CloudTextureInstance with entityId=%s doesn't exist!", entityId.ToString().c_str());
            return false;
        }

        CloudTextureProviderNotificationBus::MultiHandler::BusDisconnect(entityId);

        m_cloudTextureInstances.erase(entityId);
        return true;
    }

    bool CloudTexturesDebugViewerFeatureProcessor::UpdateWorldTransform(const AZ::EntityId& entityId, const AZ::Transform& worldTM)
    {
        if (!m_cloudTextureInstances.contains(entityId))
        {
            AZ_Warning(LogName, false, "ResizeTexture3D: entityId=%s doesn't exist", entityId.ToString().c_str());
            return false;
        }
        auto& cloudTextureInstance = m_cloudTextureInstances.at(entityId);

        if (!cloudTextureInstance.m_cloudTextureImage)
        {
            CloudTextureProviderRequestBus::EventResult(cloudTextureInstance.m_cloudTextureImage, entityId, &CloudTextureProviderRequestBus::Handler::GetCloudTextureImage);
        }
        cloudTextureInstance.m_worldMatrix = AZ::Matrix4x4::CreateFromTransform(worldTM);
        cloudTextureInstance.m_needsUpdate = true;
        return true;
    }

    bool CloudTexturesDebugViewerFeatureProcessor::UpdatePresentationData(const AZ::EntityId& entityId, const CloudTexturePresentationData& presentationData)
    {
        if (!m_cloudTextureInstances.contains(entityId))
        {
            AZ_Warning(LogName, false, "ResizeTexture3D: entityId=%s doesn't exist", entityId.ToString().c_str());
            return false;
        }
        auto& cloudTextureInstance = m_cloudTextureInstances.at(entityId);

        if (!cloudTextureInstance.m_cloudTextureImage)
        {
            CloudTextureProviderRequestBus::EventResult(cloudTextureInstance.m_cloudTextureImage, entityId, &CloudTextureProviderRequestBus::Handler::GetCloudTextureImage);
        }
        cloudTextureInstance.m_presentationData = presentationData;
        cloudTextureInstance.UpdateShaderOptionGroup(m_visibleChannelOptionName);
        return true;
    }

    //! Parameters for the compute shader that generates Texture3D END
    /////////////////////////////////////////////////////////////////

    //! Functions called by CloudscapeComponentController END
    /////////////////////////////////////////////////////////////////////


    void CloudTexturesDebugViewerFeatureProcessor::ActivateInternal()
    {
        AZ_Assert(m_shaderAsset.IsReady(), "CloudTexturesDebugViewerFeatureProcessor should be activated without a ready shader asset.");

        m_meshPipelineState = nullptr;
        m_shader = AZ::RPI::Shader::FindOrCreate(m_shaderAsset);
        AZ_Assert(m_shader, "CloudTexturesDebugViewerFeatureProcessor Failed to instantiate the runtime shader.");

        m_drawListTag = m_shader->GetDrawListTag();

        // Must be done before enabling scene notifications and creating the mesh pipeline state.
        ReloadInstancesSrg();

        DisableSceneNotification();
        EnableSceneNotification();


    }

    void CloudTexturesDebugViewerFeatureProcessor::ReloadInstanceSrg(const AZ::RHI::Ptr<AZ::RHI::ShaderResourceGroupLayout>& drawSrgLayout, CloudTextureInstance& cloudTextureInstance)
    {
        cloudTextureInstance.m_drawSrg = AZ::RPI::ShaderResourceGroup::Create(m_shaderAsset, m_shader->GetSupervariantIndex(), drawSrgLayout->GetName());
        cloudTextureInstance.m_shaderOptionGroup = m_shader->CreateShaderOptionGroup();
        cloudTextureInstance.m_shaderOptionGroup.SetAllToDefaultValues();
        cloudTextureInstance.UpdateShaderOptionGroup(m_visibleChannelOptionName);
        cloudTextureInstance.m_needsUpdate = true;
    }

    void CloudTexturesDebugViewerFeatureProcessor::ReloadInstancesSrg()
    {
        auto drawSrgLayout = m_shaderAsset->GetDrawSrgLayout(m_shader->GetSupervariantIndex());
        if (!drawSrgLayout)
        {
            AZ_Error("CloudTexturesDebugViewerFeatureProcessor", false, "Failed to get the draw shader resource group layout.");
            return;
        }

        for (auto& [entityId, cloudTextureInstance] : m_cloudTextureInstances)
        {
            ReloadInstanceSrg(drawSrgLayout, cloudTextureInstance);
        }

    }


    void CloudTexturesDebugViewerFeatureProcessor::UpdateShaderConstants()
    {
        for (auto& [entityId, cloudTextureInstance] : m_cloudTextureInstances)
        {
            if (!cloudTextureInstance.m_needsUpdate)
            {
                continue;
            }
            if (!cloudTextureInstance.m_drawSrg)
            {
                continue;
            }
            cloudTextureInstance.m_needsUpdate = false;
            auto& drawSrg = cloudTextureInstance.m_drawSrg;
            const AZ::RPI::ShaderOptionGroup& sog = const_cast< const AZ::RPI::ShaderOptionGroup&>(cloudTextureInstance.m_shaderOptionGroup);
            drawSrg->SetShaderVariantKeyFallbackValue(sog.GetShaderVariantKey());

            drawSrg->SetConstant(m_modelToWorldIndex, cloudTextureInstance.m_worldMatrix);
            drawSrg->SetImage(m_textureIndex, cloudTextureInstance.m_cloudTextureImage);
            drawSrg->SetConstant(m_alwaysFaceCameraIndex, cloudTextureInstance.m_presentationData.m_alwaysFaceCamera);
            drawSrg->SetConstant(m_texCoordZIndex, cloudTextureInstance.m_presentationData.m_texCoordZ);

            drawSrg->Compile();

        }

    }


    void CloudTexturesDebugViewerFeatureProcessor::UpdateDrawPackets()
    {
        for (auto& [entityId, cloudTextureInstance] : m_cloudTextureInstances)
        {
            UpdateDrawPacket(cloudTextureInstance);
        }
    }


    void CloudTexturesDebugViewerFeatureProcessor::UpdateDrawPacket(CloudTextureInstance& cloudTextureInstance)
    {
        if(m_meshPipelineState)
        {
            cloudTextureInstance.m_drawPacket = BuildDrawPacket(m_meshPipelineState, m_drawListTag, 6, cloudTextureInstance.m_drawSrg);
        }
    }

    void CloudTexturesDebugViewerFeatureProcessor::ResetDrawPackets()
    {
        for (auto& [entityId, cloudTextureInstance] : m_cloudTextureInstances)
        {
            cloudTextureInstance.m_drawPacket.reset();
        }
    }

    AZ::RHI::ConstPtr<AZ::RHI::DrawPacket> CloudTexturesDebugViewerFeatureProcessor::BuildDrawPacket(
                const AZ::RPI::Ptr<AZ::RPI::PipelineStateForDraw>& pipelineState,
                const AZ::RHI::DrawListTag& drawListTag,
                uint32_t vertexCount,
                const AZ::Data::Instance<AZ::RPI::ShaderResourceGroup>& drawSrg)
    {
        AZ::RHI::DrawLinear drawLinear;
        drawLinear.m_vertexCount = vertexCount;
        drawLinear.m_vertexOffset = 0;
        drawLinear.m_instanceCount = 1;
        drawLinear.m_instanceOffset = 0;

        AZ::RHI::DrawPacketBuilder drawPacketBuilder;
        drawPacketBuilder.Begin(nullptr);
        drawPacketBuilder.SetDrawArguments(drawLinear);
        drawPacketBuilder.AddShaderResourceGroup(drawSrg->GetRHIShaderResourceGroup());

        AZ::RHI::DrawPacketBuilder::DrawRequest drawRequest;
        drawRequest.m_listTag = drawListTag;
        drawRequest.m_pipelineState = pipelineState->GetRHIPipelineState();
        //drawRequest.m_streamBufferViews = streamBufferViews;
        drawPacketBuilder.AddDrawItem(drawRequest);

        return drawPacketBuilder.End();
    }


    void CloudTexturesDebugViewerFeatureProcessor::OnAssetStateChanged(AZ::Data::Asset<AZ::Data::AssetData> asset, [[maybe_unused]] bool isReload)
    {
        AZ_Info(LogName, "The shader asset is ready: %s", asset.GetHint().c_str());
        m_shaderAsset = asset;
        auto activateInternalFunc = [this]()
        {
            ActivateInternal();
        };
        AZ::TickBus::QueueFunction(AZStd::move(activateInternalFunc));
    }

    ////////////////////////////////////////////////////////////////////////
    //! Data::AssetBus START
    void CloudTexturesDebugViewerFeatureProcessor::OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
        const bool isReload = false;
        OnAssetStateChanged(asset, isReload);
    }

    void CloudTexturesDebugViewerFeatureProcessor::OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset)
    {
        const bool isReload = true;
        OnAssetStateChanged(asset, isReload);
    }
    //! Data::AssetBus END
    ////////////////////////////////////////////////////////////////////////


    ///////////////////////////////////////////////////////
    // CloudTextureProviderNotificationBus::MultiHandler overrides ...
    void CloudTexturesDebugViewerFeatureProcessor::OnCloudTextureImageReady(AZ::Data::Instance<AZ::RPI::Image> image)
    {
        auto entityId = *CloudTextureProviderNotificationBus::GetCurrentBusId();
        if (!m_cloudTextureInstances.contains(entityId))
        {
            AZ_Warning(LogName, false, "OnCloudTextureImageReady: entityId=%s doesn't exist", entityId.ToString().c_str());
            return;
        }
        auto& cloudTextureInstance = m_cloudTextureInstances.at(entityId);
        cloudTextureInstance.m_cloudTextureImage = image;
        cloudTextureInstance.m_needsUpdate = true;
    }
    ///////////////////////////////////////////////////////

} // namespace VolumetricClouds
