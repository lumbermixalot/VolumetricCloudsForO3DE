/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/
#pragma once

#include <AzCore/Memory/SystemAllocator.h>

#include <Atom/RHI/CommandList.h>
#include <Atom/RHI/DrawItem.h>
#include <Atom/RHI/ScopeProducer.h>

#include <Atom/RPI.Public/Pass/ComputePass.h>
#include <Atom/RPI.Public/Shader/Shader.h>
#include <Atom/RPI.Public/Shader/ShaderResourceGroup.h>
#include <Atom/RPI.Public/Image/StreamingImage.h>

#include <Renderer/CloudscapeShaderConstantData.h>

namespace VolumetricClouds
{
    /**
     *  This compute pass does all the heavy work to paint clouds. This is the pass that runs the expensive
     *  raymarching required to draw volumetric clouds.
     *  Two imported RW attachments are bound at all times. But for each frame
     *  we only render to one of them in an alternated (aka ping pong) fashion.
     *  The idea is that one of the attachments carries the pixel from the previous frame.
     *  while the other is the one we are going to write to in the current frame.
     *  When we are rendering to the current frame we only render to 1 of 16 pixels
     *  in a 4x4 block.
     */
    class CloudscapeComputePass final
        : public AZ::RPI::ComputePass
    {
        AZ_RPI_PASS(CloudscapeComputePass);

    public:
        AZ_RTTI(CloudscapeComputePass, "{ECB96676-7F1B-41E5-9C5F-89EA02FD116E}", AZ::RPI::ComputePass);
        AZ_CLASS_ALLOCATOR(CloudscapeComputePass, AZ::SystemAllocator);
    
    
        virtual ~CloudscapeComputePass();// = default;
    
        static AZ::RPI::Ptr<CloudscapeComputePass> Create(const AZ::RPI::PassDescriptor& descriptor);

        void UpdateShaderConstantData(const CloudscapeShaderConstantData& shaderData);

        void UpdateFrameCounter(uint32_t frameCounter);
    
    private:
        CloudscapeComputePass(const AZ::RPI::PassDescriptor& descriptor);

        //! Pass behavior overrides
        void InitializeInternal() override;
        void BuildInternal() override;
        // void FrameBeginInternal(FramePrepareParams params) override;

        // Scope producer functions...
        // void SetupFrameGraphDependencies(AZ::RHI::FrameGraphInterface frameGraph) override;
        void CompileResources(const AZ::RHI::FrameGraphCompileContext& context) override;
        // void BuildCommandListInternal(const AZ::RHI::FrameGraphExecuteContext& context) override;

        // ComputePass overrides...
        void OnShaderReloadedInternal() override;

        // A helper function
        void SetImageAttachmentBinding(uint32_t attachmentIndex, AZ::Data::Instance<AZ::RPI::AttachmentImage> attachmentImage);
    
        bool m_srgNeedsUpdate = true;
        const CloudscapeShaderConstantData* m_shaderConstantData = nullptr;
        uint32_t m_pixelIndex4x4 = 0; // Frame Counter % 16.

        AZ::RHI::ShaderInputNameIndex m_pixelIndex4x4Index = "m_pixelIndex4x4";

        AZ::RHI::ShaderInputNameIndex m_uvwScaleIndex = "m_uvwScale";
        AZ::RHI::ShaderInputNameIndex m_maxMipLevelsIndex = "m_maxMipLevels";
        AZ::RHI::ShaderInputNameIndex m_minRayMarchingStepsIndex = "m_minRayMarchingSteps";
        AZ::RHI::ShaderInputNameIndex m_maxRayMarchingStepsIndex = "m_maxRayMarchingSteps";

        AZ::RHI::ShaderInputNameIndex m_planetRadiusKmIndex = "m_planetRadiusKm";
        AZ::RHI::ShaderInputNameIndex m_cloudSlabDistanceAboveSeaLevelKmIndex = "m_cloudSlabDistanceAboveSeaLevelKm";
        AZ::RHI::ShaderInputNameIndex m_cloudSlabThicknessKmIndex = "m_cloudSlabThicknessKm";

        AZ::RHI::ShaderInputNameIndex m_sunColorAndIntensityIndex = "m_sunColorAndIntensity";
        AZ::RHI::ShaderInputNameIndex m_ambientLightColorAndIntensityIndex = "m_ambientLightColorAndIntensity";
        AZ::RHI::ShaderInputNameIndex m_directionTowardsTheSunIndex = "m_directionTowardsTheSun";

        AZ::RHI::ShaderInputNameIndex m_weatherMapSizeKmIndex = "m_weatherMapSizeKm";
        AZ::RHI::ShaderInputNameIndex m_globalCloudCoverageIndex = "m_globalCloudCoverage";
        AZ::RHI::ShaderInputNameIndex m_globalCloudDensityIndex = "m_globalCloudDensity";
        AZ::RHI::ShaderInputNameIndex m_windSpeedKmPerSecIndex = "m_windSpeedKmPerSec";
        AZ::RHI::ShaderInputNameIndex m_windDirectionIndex = "m_windDirection";
        AZ::RHI::ShaderInputNameIndex m_cloudTopOffsetKmIndex = "m_cloudTopOffsetKm";

        AZ::RHI::ShaderInputNameIndex m_aCoefIndex = "m_aCoef";
        AZ::RHI::ShaderInputNameIndex m_sCoefIndex = "m_sCoef";
        AZ::RHI::ShaderInputNameIndex m_henyeyGreensteinGIndex = "m_henyeyGreensteinG";
        AZ::RHI::ShaderInputNameIndex m_multipleScatteringABCIndex = "m_multipleScatteringABC";

        AZ::RHI::ShaderInputNameIndex m_lowFreqNoiseTextureImageIndex = "m_lowFreqNoiseTexture";
        AZ::RHI::ShaderInputNameIndex m_highFreqNoiseTextureImageIndex = "m_highFreqNoiseTexture";
        AZ::RHI::ShaderInputNameIndex m_weatherMapImageIndex = "m_weatherMap";

    };

}   // namespace VolumetricClouds
