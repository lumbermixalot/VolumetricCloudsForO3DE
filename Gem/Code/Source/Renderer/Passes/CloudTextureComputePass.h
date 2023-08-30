/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/
#pragma once

#include <AzCore/std/containers/queue.h>
#include <AzCore/Memory/SystemAllocator.h>

#include <Atom/RPI.Public/Pass/ComputePass.h>
#include <Atom/RPI.Public/Pass/ParentPass.h>
#include <Atom/RPI.Public/Pass/Pass.h>
#include <Atom/RPI.Public/Shader/ShaderReloadNotificationBus.h>
#include <Atom/RPI.Reflect/Pass/ComputePassData.h>
#include <Atom/RPI.Reflect/Pass/PassDescriptor.h>

#include <Atom/RPI.Reflect/Pass/RenderPassData.h>

#include "CloudTextureComputeData.h"

namespace VolumetricClouds
{
    //! The compute pass to generate an RGBA Texture3D what contains
    //! the noise data to render clouds.
    class CloudTextureComputePass
        : public AZ::RPI::ComputePass
    {
        AZ_RPI_PASS(CloudTextureComputePass);

    public:
        AZ_RTTI(CloudTextureComputePass, "{B712A155-4508-4BD2-9CD7-E74F11D31F52}", AZ::RPI::ComputePass);
        AZ_CLASS_ALLOCATOR(CloudTextureComputePass, AZ::SystemAllocator);
        virtual ~CloudTextureComputePass() = default;

        static constexpr uint32_t MAX_PIXEL_SIZE = 512;
        static constexpr uint32_t MIN_PIXEL_SIZE = 4;
        static uint16_t CalculateMipCount(uint32_t pixelSize);

        static AZ::RPI::Ptr<CloudTextureComputePass> Create(const AZ::RPI::PassDescriptor& descriptor);

        // Must be called before the pipeline that owns this pass runs.
        // Returns true (success) if the size of the texture3DAttachment is within the limits, etc.
        bool SetRenderData(AZ::Data::Instance<AZ::RPI::AttachmentImage> texture3DAttachment,
                           CloudTextureComputeData computeData);
        bool IsFinished() { return m_isFinished; }

        //! Besides the standard enable flag,
        //! The pass is disabled .
        bool IsEnabled() const override;

    private:
        CloudTextureComputePass(const AZ::RPI::PassDescriptor& descriptor);

        static constexpr char LogName[] = "CloudTextureComputePass";

        // Pass overrides
        void BuildInternal() override;

        // ScopeProducer overrides
        void SetupFrameGraphDependencies(AZ::RHI::FrameGraphInterface frameGraph) override;
        void CompileResources(const AZ::RHI::FrameGraphCompileContext& context) override;

        // RenderPass overrides
        void FrameBeginInternal(FramePrepareParams params) override;
        void FrameEndInternal() override;

        //! Index and resource of the Texture3D that will be filled with
        //! Perlin-Worley noise data by this compute shader.
        AZ::RHI::ShaderInputNameIndex m_frequencyIndex = "m_frequency";
        AZ::RHI::ShaderInputNameIndex m_perlinOctavesIndex = "m_perlinOctaves";
        AZ::RHI::ShaderInputNameIndex m_perlinGainIndex = "m_perlinGain";
        AZ::RHI::ShaderInputNameIndex m_perlinAmplitudeIndex = "m_perlinAmplitude";
        AZ::RHI::ShaderInputNameIndex m_worleyOctavesIndex = "m_worleyOctaves";
        AZ::RHI::ShaderInputNameIndex m_worleyGainIndex = "m_worleyGain";
        AZ::RHI::ShaderInputNameIndex m_worleyAmplitudeIndex = "m_worleyAmplitude";
        AZ::RHI::ShaderInputNameIndex m_pixelSizeIndex = "m_pixelSize";


        // This pass runs in one frame, and when done this becomes true.
        bool m_isFinished = false;

        AZ::Data::Instance<AZ::RPI::AttachmentImage> m_texture3DAttachment;
        CloudTextureComputeData m_computeData;
    };

} // namespace VolumetricClouds
