/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <Atom/RHI/FrameGraphAttachmentInterface.h>
#include <Atom/RHI/FrameGraphBuilder.h>
#include <Atom/RHI/CommandList.h>

#include <Atom/RPI.Public/Pass/PassUtils.h>
#include <Atom/RPI.Public/RenderPipeline.h>
#include <Atom/RPI.Public/RPIUtils.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/View.h>
#include <Atom/RPI.Reflect/Shader/ShaderAsset.h>

#include "CloudTextureComputePass.h"


namespace VolumetricClouds
{
    AZ::RPI::Ptr<CloudTextureComputePass> CloudTextureComputePass::Create(const AZ::RPI::PassDescriptor& descriptor)
    {
        AZ::RPI::Ptr<CloudTextureComputePass> pass = aznew CloudTextureComputePass(descriptor);
        return pass;
    }

    CloudTextureComputePass::CloudTextureComputePass(const AZ::RPI::PassDescriptor& descriptor)
        : AZ::RPI::ComputePass(descriptor)
    {
    }

    uint16_t CloudTextureComputePass::CalculateMipCount(uint32_t pixelSize)
    {
        return static_cast<uint16_t>(log2(pixelSize) - log2(MIN_PIXEL_SIZE)) + 1;
    }

    void CloudTextureComputePass::BuildInternal()
    {
        if (!m_texture3DAttachment)
        {
            // This is OK, because BuildInternal is always called when the pipeline is created,
            // and at this time there's no data given to this pass.
            // Later when ScenePtr->AddRenderPipeline is called, this function is called again,
            // but at this time m_texture3DAttachment was already setup and the attachment binding are properly
            // initialized.
            return;
        }

        const auto pixelSize = m_computeData.m_pixelSize;
        // Based on the pixelSize we can calculate all the mips.
        const uint16_t numMips = CalculateMipCount(pixelSize);
        uint16_t pixelSizeForMip = static_cast<uint16_t>(pixelSize);
        const auto shaderInputName = AZ::Name("m_cloudTextureMips");
        for (uint16_t mipIndex = 0; mipIndex < numMips; mipIndex++)
        {
            const AZStd::string slotNameStr = AZStd::string::format("OutputMip%u", mipIndex);
            const auto slotName = AZ::Name(slotNameStr);
            auto binding = FindAttachmentBinding(slotName);
            if (!binding)
            {
                AZ_Warning(LogName, false, "Failed to find binding for slot %s", slotName.GetCStr());
                return;
            }

            // By default, in the *.pass asset we have it as "NoBind" because during asset load time
            // we have no attachments. Attachments are actually created and defined at runtime by the CloudTextureFeatureProcessor.
            // Now that we know what the attachment should be, it is time to define the real shader constant name.
            // REMARK:
            // The real reason in the *.pass asset we start with "NoBind" is to avoid
            // harmless AZ::Errors related with the shaders m_cloudTextureMips[]
            // array binding. The bindings are actually known at runtime and not during
            // AZ::RPI::RenderPass::InitializeInternal().
            binding->m_shaderInputName = shaderInputName;

            // Make sure the imageView points to the correct mip level.
            AZ::RHI::ImageViewDescriptor viewDesc = AZ::RHI::ImageViewDescriptor::Create3D(m_texture3DAttachment->GetDescriptor().m_format,
                mipIndex, mipIndex, 0, pixelSizeForMip - 1);
            binding->m_unifiedScopeDesc.SetAsImage(viewDesc);

            AttachImageToSlot(slotName, m_texture3DAttachment);

            pixelSizeForMip = (pixelSizeForMip >> 1);
        }

        SetTargetThreadCounts(pixelSize, pixelSize, pixelSize);
    }

    void CloudTextureComputePass::FrameBeginInternal(FramePrepareParams params)
    {
        AZ::RPI::RenderPass::FrameBeginInternal(params);
    }

    void CloudTextureComputePass::FrameEndInternal()
    {
        if (!m_texture3DAttachment)
        {
            return;
        }

        m_isFinished = true;

        SetEnabled(false);
    }

    void CloudTextureComputePass::SetupFrameGraphDependencies(AZ::RHI::FrameGraphInterface frameGraph)
    {
        if (!m_texture3DAttachment)
        {
            AZ_Error(LogName, false, "Where is the texture3DAttachment?");
            return;
        }

        // Bread crumbs:
        // Typically when defining Image Attachments in *.pass json files, along with "Connections"
        // AND if the pass owns the persistent attachment the Import and Use are done automatically
        // by the base Pass class SetupFrameGraphDependencies.
        AZ::RHI::FrameGraphAttachmentInterface attachmentDatabase = frameGraph.GetAttachmentDatabase();
        attachmentDatabase.ImportImage(m_texture3DAttachment->GetAttachmentId(), m_texture3DAttachment->GetRHIImage());

        // REMARK:
        // Commented this block because it is redundant because AZ::RPI::ComputePass::SetupFrameGraphDependencies
        // already does the same.
        // 
        // AZ::RHI::ImageScopeAttachmentDescriptor desc;
        // desc.m_imageViewDescriptor = request.m_cloudTextureAttachment->GetImageView()->GetDescriptor();
        // desc.m_loadStoreAction.m_loadAction = AZ::RHI::AttachmentLoadAction::Load;
        // desc.m_attachmentId = request.m_cloudTextureAttachment->GetAttachmentId();
        // // From the point of view of this compute pass we only need Write access, but this attachment
        // // will be used later by CloudTexturesFeatureProcessor as a Read texture.
        // frameGraph.UseShaderAttachment(desc, AZ::RHI::ScopeAttachmentAccess::ReadWrite);

        AZ::RPI::ComputePass::SetupFrameGraphDependencies(frameGraph);
    }

    void CloudTextureComputePass::CompileResources(const AZ::RHI::FrameGraphCompileContext& context)
    {
        if (m_texture3DAttachment)
        {
            const auto& computeData = m_computeData;

            m_shaderResourceGroup->SetConstant(m_frequencyIndex, computeData.m_frequency);

            m_shaderResourceGroup->SetConstant(m_perlinOctavesIndex,   computeData.m_perlinOctaves);
            m_shaderResourceGroup->SetConstant(m_perlinGainIndex,      computeData.m_perlinGain);
            m_shaderResourceGroup->SetConstant(m_perlinAmplitudeIndex, computeData.m_perlinAmplitude);

            m_shaderResourceGroup->SetConstant(m_worleyOctavesIndex,   computeData.m_worleyOctaves);
            m_shaderResourceGroup->SetConstant(m_worleyGainIndex,      computeData.m_worleyGain);
            m_shaderResourceGroup->SetConstant(m_worleyAmplitudeIndex, computeData.m_worleyAmplitude);

            m_shaderResourceGroup->SetConstant(m_pixelSizeIndex, computeData.m_pixelSize);

        }
        AZ::RPI::ComputePass::CompileResources(context);
    }

    bool CloudTextureComputePass::IsEnabled() const
    {
        if (!AZ::RPI::Pass::IsEnabled())
        {
            return false;
        }

        return !m_isFinished && m_texture3DAttachment;
    }

    bool CloudTextureComputePass::SetRenderData(AZ::Data::Instance<AZ::RPI::AttachmentImage> texture3DAttachment,
        CloudTextureComputeData computeData)
    {
        if (m_isFinished)
        {
            AZ_Error(LogName, false, "This function can not be called after the pass is finished!");
            return false;
        }

        const auto pixelSize = computeData.m_pixelSize;
        if ( (pixelSize < MIN_PIXEL_SIZE) || (pixelSize > MAX_PIXEL_SIZE) )
        {
            AZ_Error(LogName, false, "This pass can not generate noise textures smaller than %u or larger than %u pixels. Got %u pixels.\n",
                MIN_PIXEL_SIZE, MAX_PIXEL_SIZE, pixelSize);
            return false;
        }

        // Make sure the Texture3D was memory allocated properly (with expected MipMap count, etc).
        const auto expectedMipsCount = CalculateMipCount(pixelSize);
        const auto &imageDesc = texture3DAttachment->GetDescriptor();
        if (imageDesc.m_mipLevels != expectedMipsCount)
        {
            AZ_Error(LogName, false, "For pixel size %u, Expected Mips count %u in attachment image don't match. Got %u\n",
                pixelSize, expectedMipsCount, imageDesc.m_mipLevels);
            return false;
        }

        m_texture3DAttachment = texture3DAttachment;
        m_computeData = computeData;

        SetEnabled(true);
        return true;
    }

} // namespace VolumetricClouds
