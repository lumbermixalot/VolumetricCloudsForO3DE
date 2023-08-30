/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <Atom/RPI.Public/RenderPipeline.h>
#include <Atom/RPI.Public/View.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/Pass/PassFilter.h>
#include <Atom/RPI.Public/Pass/PassSystem.h>
#include <Atom/RPI.Public/Image/AttachmentImagePool.h>
#include <Atom/RPI.Public/Image/ImageSystemInterface.h>

#include <Atom/RHI/FrameScheduler.h>
#include <Atom/RHI/PipelineState.h>

#include <Renderer/CloudscapeFeatureProcessor.h>
#include "CloudscapeComputePass.h"

namespace VolumetricClouds
{
    
    AZ::RPI::Ptr<CloudscapeComputePass> CloudscapeComputePass::Create(const AZ::RPI::PassDescriptor& descriptor)
    {
        AZ::RPI::Ptr<CloudscapeComputePass> pass = aznew CloudscapeComputePass(descriptor);
        return pass;
    }
    
    CloudscapeComputePass::CloudscapeComputePass(const AZ::RPI::PassDescriptor& descriptor)
        : AZ::RPI::ComputePass(descriptor)
    {
    }

    CloudscapeComputePass::~CloudscapeComputePass()
    {
        // Left this here, in case there are bugs that pop up related with destruction of the passes related with rendering clouds.
        //AZ_Printf("ERASEME", "%s\n", __FUNCTION__);
    }
    
    void CloudscapeComputePass::InitializeInternal()
    {
        AZ::RPI::ComputePass::InitializeInternal();

        m_srgNeedsUpdate = (m_shaderConstantData != nullptr);

        //InitializeShaderVariant();
    }

    void CloudscapeComputePass::SetImageAttachmentBinding(uint32_t attachmentIndex, AZ::Data::Instance<AZ::RPI::AttachmentImage> attachmentImage)
    {
        const AZStd::string slotNameStr = AZStd::string::format("Output%u", attachmentIndex);
        const auto slotName = AZ::Name(slotNameStr);
        auto binding = FindAttachmentBinding(slotName);
        AZ_Assert(!!binding, "Failed to find attachment binding for slot %s", slotName.GetCStr());

        // By default, in the *.pass asset we have it as "NoBind" because during asset load time
        // we have no attachments. Attachments are actually created and defined at runtime by the CloudscapeFeatureProcessor.
        // Now that we know what the attachment should be, it is time to define the real shader constant name.
        // REMARK:
        // The real reason in the *.pass asset we start with "NoBind" is to avoid
        // harmless AZ::Errors related with the shaders m_cloudscapeOut[]
        // array binding. The bindings are actually known at runtime and not during
        // AZ::RPI::RenderPass::InitializeInternal().
        binding->m_shaderInputName = AZ::Name("m_cloudscapeOut");

        AZ::RHI::ImageViewDescriptor viewDesc = AZ::RHI::ImageViewDescriptor::Create(attachmentImage->GetDescriptor().m_format,
            0, 0);
        binding->m_unifiedScopeDesc.SetAsImage(viewDesc);

        AttachImageToSlot(slotName, attachmentImage);
    }


    void CloudscapeComputePass::BuildInternal()
    {
        AZ::RPI::Scene* scene = m_pipeline->GetScene();
        auto* cloudscapeFeatureProcessor = scene->GetFeatureProcessor<CloudscapeFeatureProcessor>();
        if (!cloudscapeFeatureProcessor)
        {
            // This can happen when the feature processor is being destroyed.
            return;
        }

        const auto output0ImageAttachment = cloudscapeFeatureProcessor->GetOutput0ImageAttachment();
        // Bind the first attachment
        SetImageAttachmentBinding(0, output0ImageAttachment);
        SetImageAttachmentBinding(1, cloudscapeFeatureProcessor->GetOutput1ImageAttachment());

        const auto attachmentSize = output0ImageAttachment->GetDescriptor().m_size;

        // Each Thread is invoked to write to 1 out of 16 pixels (0..15)
        // in 4x4 block.
        // Which means the total thread counts in X = ceil(imageWidth/4)
        // and for Y = ceil(imageHeight/4);
        // REMARK: Each frame, the feature processor will call UpdateFrameCounter(), which will
        // define the pixel index in the range 0..15.
        
        //const auto totalThreadsX = attachmentSize.m_width;
        //const auto totalThreadsY = attachmentSize.m_height;

        const auto totalThreadsX = static_cast<uint32_t>(AZStd::ceil(static_cast<float>(attachmentSize.m_width) / 4.0f));
        const auto totalThreadsY = static_cast<uint32_t>(AZStd::ceil(static_cast<float>(attachmentSize.m_height) / 4.0f));

        SetTargetThreadCounts(totalThreadsX, totalThreadsY, 1);
    }

    // void CloudscapeComputePass::FrameBeginInternal(FramePrepareParams params)
    // {
    //     AZ::RPI::ComputePass::FrameBeginInternal(params);
    // }


    // void CloudscapeComputePass::SetupFrameGraphDependencies(AZ::RHI::FrameGraphInterface frameGraph)
    // {
    //     AZ::RPI::ComputePass::SetupFrameGraphDependencies(frameGraph);
    // }

    
    void CloudscapeComputePass::CompileResources(const AZ::RHI::FrameGraphCompileContext& context)
    {
       AZ_Assert(m_shaderResourceGroup != nullptr, "CloudscapeComputePass %s has a null shader resource group when calling Compile.", GetPathName().GetCStr());

       m_shaderResourceGroup->SetConstant(m_pixelIndex4x4Index, m_pixelIndex4x4);

       if (m_srgNeedsUpdate && m_shaderConstantData)
       {
           m_shaderResourceGroup->SetConstant(m_uvwScaleIndex, m_shaderConstantData->m_uvwScale);
           m_shaderResourceGroup->SetConstant(m_maxMipLevelsIndex, m_shaderConstantData->m_clampedMipLevels);

           const auto minSteps = AZStd::min<uint32_t>(m_shaderConstantData->m_minRayMarchingSteps, m_shaderConstantData->m_maxRayMarchingSteps);
           const auto maxSteps = AZStd::max<uint32_t>(m_shaderConstantData->m_minRayMarchingSteps, m_shaderConstantData->m_maxRayMarchingSteps);
           m_shaderResourceGroup->SetConstant(m_minRayMarchingStepsIndex, minSteps);
           m_shaderResourceGroup->SetConstant(m_maxRayMarchingStepsIndex, maxSteps);


           m_shaderResourceGroup->SetConstant(m_planetRadiusKmIndex, static_cast<float>(m_shaderConstantData->m_planetRadiusKm));
           m_shaderResourceGroup->SetConstant(m_cloudSlabDistanceAboveSeaLevelKmIndex,  m_shaderConstantData->m_cloudSlabDistanceAboveSeaLevelKm);
           m_shaderResourceGroup->SetConstant(m_cloudSlabThicknessKmIndex, m_shaderConstantData->m_cloudSlabThicknessKm);

           AZ::Color sunColorAndIntensity = AZ::Color::CreateFromVector3AndFloat(m_shaderConstantData->m_sunColor, m_shaderConstantData->m_sunLightIntensity);
           m_shaderResourceGroup->SetConstant(m_sunColorAndIntensityIndex, sunColorAndIntensity);

           AZ::Color ambientLightColorAndIntensity = m_shaderConstantData->m_ambientLightColor;
           ambientLightColorAndIntensity.SetA(m_shaderConstantData->m_ambientLightIntensity);
           m_shaderResourceGroup->SetConstant(m_ambientLightColorAndIntensityIndex, ambientLightColorAndIntensity);
           
           m_shaderResourceGroup->SetConstant(m_directionTowardsTheSunIndex, m_shaderConstantData->m_directionTowardsTheSun);

           // The user inputs the data in [m-1], but the shader assumes all the data is computed in Km.
           const float absorptionCoefficient = m_shaderConstantData->m_cloudMaterialProperties.m_absorptionCoefficient * (1000.0f); //* 10.0f);
           const float scatteringCoefficient = m_shaderConstantData->m_cloudMaterialProperties.m_scatteringCoefficient * (1000.0f);// *10.0f);
           m_shaderResourceGroup->SetConstant(m_aCoefIndex, absorptionCoefficient);
           m_shaderResourceGroup->SetConstant(m_sCoefIndex, scatteringCoefficient);
           m_shaderResourceGroup->SetConstant(m_henyeyGreensteinGIndex, m_shaderConstantData->m_cloudMaterialProperties.m_henyeyGreensteinG);
           AZ::Vector3 abc(m_shaderConstantData->m_cloudMaterialProperties.m_multiScatteringA,
               m_shaderConstantData->m_cloudMaterialProperties.m_multiScatteringB,
               m_shaderConstantData->m_cloudMaterialProperties.m_multiScatteringC);
           m_shaderResourceGroup->SetConstant(m_multipleScatteringABCIndex,  abc);

           m_shaderResourceGroup->SetConstant(m_weatherMapSizeKmIndex, m_shaderConstantData->m_weatherMapSizeKm);
           m_shaderResourceGroup->SetConstant(m_globalCloudCoverageIndex, m_shaderConstantData->m_globalCloudCoverage);
           m_shaderResourceGroup->SetConstant(m_globalCloudDensityIndex, m_shaderConstantData->m_globalCloudDensity);

           AZ::Vector3 windDirection = m_shaderConstantData->m_windDirection;
           const float windDirectionLength = windDirection.GetLength();
           windDirection = AZ::IsClose(windDirectionLength, 0.0f, 0.01f)
               ? AZ::Vector3::CreateZero()
               : (windDirection / windDirectionLength);
           m_shaderResourceGroup->SetConstant(m_windSpeedKmPerSecIndex, m_shaderConstantData->m_windSpeedKmPerSec);
           m_shaderResourceGroup->SetConstant(m_windDirectionIndex, windDirection);
           m_shaderResourceGroup->SetConstant(m_cloudTopOffsetKmIndex, m_shaderConstantData->m_cloudTopOffsetKm);

           m_shaderResourceGroup->SetImage(m_lowFreqNoiseTextureImageIndex, m_shaderConstantData->m_lowFrequencyNoiseTexture);
           m_shaderResourceGroup->SetImage(m_highFreqNoiseTextureImageIndex, m_shaderConstantData->m_highFrequencyNoiseTexture);
           m_shaderResourceGroup->SetImage(m_weatherMapImageIndex, m_shaderConstantData->m_weatherMap);

           m_srgNeedsUpdate = false;
       }

       AZ::RPI::ComputePass::CompileResources(context);
    }
    
    
    // void CloudscapeComputePass::BuildCommandListInternal(const AZ::RHI::FrameGraphExecuteContext& context)
    // {
    //     AZ::RPI::ComputePass::BuildCommandListInternal(context);
    // }


    void CloudscapeComputePass::UpdateShaderConstantData(const CloudscapeShaderConstantData& shaderData)
    {
        // If any of the textures is nullptr we disable this pass.
        if (!shaderData.m_lowFrequencyNoiseTexture ||
            !shaderData.m_highFrequencyNoiseTexture ||
            !shaderData.m_weatherMap)
        {
            m_shaderConstantData = nullptr;
            SetEnabled(false);
        }
        else
        {
            m_shaderConstantData = &shaderData;
            m_srgNeedsUpdate = true;
            if (!IsEnabled())
            {
                SetEnabled(true);
            }
        }
    }


    void CloudscapeComputePass::UpdateFrameCounter(uint32_t frameCounter)
    {
        m_pixelIndex4x4 = frameCounter % 16;
    }

    // ComputePass overrides...
    void CloudscapeComputePass::OnShaderReloadedInternal()
    {
        m_srgNeedsUpdate = true;
    }

}   // VolumetricClouds AZ
