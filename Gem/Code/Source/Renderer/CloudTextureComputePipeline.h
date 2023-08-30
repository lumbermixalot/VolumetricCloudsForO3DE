/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <Atom/RPI.Public/Base.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/Pass/AttachmentReadback.h>

#include <Renderer/Passes/CloudTextureComputeData.h>

namespace VolumetricClouds
{
    class CloudTextureComputePass;
    
    // This class generates a 3D noise texture used for clouds. The Texture3D
    // is generated along with all of its mipmap levels, all in a single pass.
    // This class instantiates a minimal render pipeline, which in turn instantiates
    // the CloudTextureComputePass to generate the Texture3D, optionally you can
    // enable an AttachmentReadback pass to read the Texture3D into CPU memory.
    class CloudTextureComputePipeline final
    {
    public:
        CloudTextureComputePipeline() = default;
        ~CloudTextureComputePipeline() = default;

        using RenderTaskId = uint32_t;
        struct CloudTextureSubresourceReadback
        {
            AZStd::shared_ptr<AZStd::vector<uint8_t>> m_dataBuffer;
            uint16_t m_mipSlice = 0;
            AZ::RHI::Size m_mipSize = {};
        };
        // readbackResults will be empty if there was no request to readback the Texture3D mip subresoruces from GPU to CPU.
        using CloudTextureRenderCallback = AZStd::function<void(RenderTaskId renderTaskId, const AZStd::vector<CloudTextureSubresourceReadback>& readbackResults)>;

        // Instantiates a short lived render pipeline that spawns a compute pass that generates
        // a Texture3D. Returns a unique RenderTaskId ( greater than 0, if successful) for this job that will be used in the callback.
        RenderTaskId StartTextureCompute(AZ::RPI::Scene* scene, AZ::Data::Instance<AZ::RPI::AttachmentImage> texture3DAttachment
                                       , const CloudTextureComputeData& computeData, CloudTextureRenderCallback callback, bool withAttachmentReadback = false);
    
        // removes the render pipeline from the scene if rendering is complete
        // Note: must be called outside of the feature processor Simulate/Render phases
        void CheckAndRemovePipeline();
    
        bool IsRenderingNoiseTexture() const { return m_isRendering; }
    
    private:
        AZ_DISABLE_COPY_MOVE(CloudTextureComputePipeline);

        void SetupAttachmentReadback(uint32_t pixelSize);
        // resultNoMips will be cast to  AZ::RPI::AttachmentsReadbackGroup::ReadbackResultWithMips
        void AttachmentReadbackCallback(const AZ::RPI::AttachmentReadback::ReadbackResult& resultNoMips);

        static constexpr char PipelineDescriptorAssetPath[] = "Passes/CloudTexturePipelineDescriptor.azasset";
        static constexpr char LogName[] = "CloudTextureComputePipeline";
        static RenderTaskId m_renderTaskCounter;
    
        AZ::RPI::Scene* m_scene = nullptr;
    
        CloudTextureComputePass* m_textureComputePass = nullptr;
        AZ::RPI::RenderPipelineId m_renderPipelineId;
        uint32_t m_renderTaskId = 0;
        CloudTextureRenderCallback m_callback;
        bool m_isRendering = false;
        AZStd::shared_ptr<AZ::RPI::AttachmentReadback> m_attachmentsReadback;
        // This vector will be as long as the number of expected mip maps.
        AZStd::vector<CloudTextureSubresourceReadback> m_attachmentsReadbackData;
        bool m_isReadbackComplete = false;
    };
} // namespace VolumetricClouds
