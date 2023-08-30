/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <Atom/RPI.Public/Pass/Pass.h>
#include <Atom/RPI.Public/Pass/PassSystemInterface.h>
#include <Atom/RPI.Public/Pass/PassFilter.h>
#include <Atom/RPI.Public/RenderPipeline.h>
#include <Atom/RPI.Public/View.h>
#include <Atom/RPI.Reflect/Asset/AssetUtils.h>
#include <Atom/RPI.Reflect/System/AnyAsset.h>

#include <Renderer/Passes/CloudTextureComputePass.h>
#include "CloudTextureComputePipeline.h"

namespace VolumetricClouds
{
    CloudTextureComputePipeline::RenderTaskId CloudTextureComputePipeline::m_renderTaskCounter = 0;

    CloudTextureComputePipeline::RenderTaskId CloudTextureComputePipeline::StartTextureCompute(AZ::RPI::Scene* scene, AZ::Data::Instance<AZ::RPI::AttachmentImage> texture3DAttachment,
        const CloudTextureComputeData& computeData, CloudTextureRenderCallback callback, bool withAttachmentReadback)
    {
        m_scene = scene;
        AZ_Assert(m_isRendering == false, "CloudTextureComputePipeline::StartRender called while a noise Texture render was already in progress");
        if (m_isRendering)
        {
            return 0;
        }
    
        m_isRendering = true;
        m_callback = callback;

        AZ::Data::Asset<AZ::RPI::AnyAsset> pipelineAsset = AZ::RPI::AssetUtils::LoadAssetByProductPath<AZ::RPI::AnyAsset>(PipelineDescriptorAssetPath, AZ::RPI::AssetUtils::TraceLevel::Error);
        if (!pipelineAsset.IsReady())
        {
            AZ_Assert(false, "Failed to load pipeline asset at %s", PipelineDescriptorAssetPath);
            AZ_Error(LogName, false, "Failed to load pipeline asset at %s", PipelineDescriptorAssetPath);
            return 0;
        }

        const AZ::RPI::RenderPipelineDescriptor* tmpRenderPipelineDescriptor = AZ::RPI::GetDataFromAnyAsset<AZ::RPI::RenderPipelineDescriptor>(pipelineAsset);
        AZ_Assert(!!tmpRenderPipelineDescriptor, "Couldn't read asset %s as RenderPipelineDescriptor", PipelineDescriptorAssetPath);
        if (!tmpRenderPipelineDescriptor)
        {
            return 0;
        }
        AZ::RPI::RenderPipelineDescriptor renderPipelineDescriptor = *tmpRenderPipelineDescriptor;
        // Define a unique name for the pipeline within the scene.
        m_renderTaskCounter++;
        m_renderTaskId = m_renderTaskCounter;
        renderPipelineDescriptor.m_name = AZStd::string::format("CloudTexturePipeline_%u", m_renderTaskId);

        AZ::RPI::RenderPipelinePtr renderPipeline = AZ::RPI::RenderPipeline::CreateRenderPipeline(renderPipelineDescriptor);
        m_renderPipelineId = renderPipeline->GetId();

        // Hold a reference to the compute pass
        const auto passName = AZ::Name("CloudTextureComputePass");
        AZ::RPI::PassFilter passFilter = AZ::RPI::PassFilter::CreateWithPassName(passName, renderPipeline.get());
        AZ::RPI::Pass* existingPass = AZ::RPI::PassSystemInterface::Get()->FindFirstPass(passFilter);
        m_textureComputePass = azrtti_cast<CloudTextureComputePass*>(existingPass);
        if (!m_textureComputePass)
        {
            AZ_Error(LogName, false, "%s Failed to find pass: %s", __FUNCTION__, passName.GetCStr());
            return 0;
        }
        m_textureComputePass->SetEnabled(false);
        // If the data is correct, SetRenderData() will enable the Pass.
        if (!m_textureComputePass->SetRenderData(texture3DAttachment, computeData))
        {
            AZ_Assert(false, "Failed to set render data for CloudTexturePipeline with name %s", renderPipelineDescriptor.m_name.c_str());
            AZ_Error(LogName, false, "Failed to set render data for CloudTexturePipeline with name %s", renderPipelineDescriptor.m_name.c_str());
            return 0;
        }
    
        // Add the pipeline to the scene
        m_scene->AddRenderPipeline(renderPipeline);

        if (withAttachmentReadback)
        {
            // Setup the attachment readback if the user needs to read the Texture3D from GPU to CPU memory.
            SetupAttachmentReadback(computeData.m_pixelSize);
        }

        return m_renderTaskId;
    }
    
    void CloudTextureComputePipeline::CheckAndRemovePipeline()
    {
        if (m_textureComputePass && m_textureComputePass->IsFinished())
        {
            if (m_attachmentsReadback && !m_isReadbackComplete)
            {
                return;
            }

            m_callback(m_renderTaskId, m_attachmentsReadbackData);

            m_isRendering = false;
    
            // remove the cubemap pipeline
            // Note: this must not be called in the scope of a feature processor Simulate or Render to avoid a race condition with other feature processors
            m_scene->RemoveRenderPipeline(m_renderPipelineId);
            m_attachmentsReadback.reset();
            m_textureComputePass = nullptr;
        }
    }

    void CloudTextureComputePipeline::AttachmentReadbackCallback(const AZ::RPI::AttachmentReadback::ReadbackResult& result)
    {
        AZ_Assert(result.m_userIdentifier == m_renderTaskId, "Got unexpected user identifier <%u>. Was expecting <%u>.", result.m_userIdentifier, m_renderTaskId);

        for (const auto& mipDataBuffer : result.m_mipDataBuffers)
        {
            const auto mipIdx = mipDataBuffer.m_mipInfo.m_slice;
            m_attachmentsReadbackData[mipIdx].m_dataBuffer = mipDataBuffer.m_mipBuffer;
            m_attachmentsReadbackData[mipIdx].m_mipSlice = mipIdx;
            m_attachmentsReadbackData[mipIdx].m_mipSize = mipDataBuffer.m_mipInfo.m_size;
        }

        m_isReadbackComplete = true;
    }

    void CloudTextureComputePipeline::SetupAttachmentReadback(uint32_t pixelSize)
    {
        AZStd::fixed_string<128> scope_name = AZStd::fixed_string<128>::format("Texture3DCapture_%u", m_renderTaskId);
        m_attachmentsReadback = AZStd::make_shared<AZ::RPI::AttachmentReadback>(AZ::RHI::ScopeId{ scope_name });
        m_attachmentsReadback->SetCallback(AZStd::bind(&CloudTextureComputePipeline::AttachmentReadbackCallback, this, AZStd::placeholders::_1));
        m_attachmentsReadback->SetUserIdentifier(m_renderTaskId);

        m_isReadbackComplete = false;
        AZ::Name slotName("OutputMip0");

        const auto mipsCount = CloudTextureComputePass::CalculateMipCount(pixelSize);
        m_attachmentsReadbackData.reserve(mipsCount);
        for (uint16_t i = 0; i < mipsCount; i++)
        {
            m_attachmentsReadbackData.push_back({});
        }
        const uint16_t mipSliceMax = mipsCount - 1;
        AZ::RHI::ImageSubresourceRange mipsRange(0 /*mipSliceMin*/, mipSliceMax, 0, 0);
        const bool result = m_textureComputePass->ReadbackAttachment(m_attachmentsReadback, m_renderTaskId,
            slotName, AZ::RPI::PassAttachmentReadbackOption::Output, &mipsRange);
        AZ_Error(LogName, result, "%s Failed to initialize ReadbackAttachment\n", __FUNCTION__);
    }
    
} // namespace VolumetricClouds
