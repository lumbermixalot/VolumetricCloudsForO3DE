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

#include <Atom/RPI.Public/Pass/FullscreenTrianglePass.h>
#include <Atom/RPI.Public/Shader/Shader.h>
#include <Atom/RPI.Public/Shader/ShaderResourceGroup.h>

#include <Renderer/CloudscapeShaderConstantData.h>

namespace VolumetricClouds
{
    /**
     *  This pass merges the cloud pixels into the main render target.
     */
    class CloudscapeRasterPass final
        : public AZ::RPI::FullscreenTrianglePass
    {
    public:
        AZ_RTTI(CloudscapeRasterPass, "{1DE6A41D-03CE-47D2-B8BC-75C863F14764}", AZ::RPI::FullscreenTrianglePass);
        AZ_CLASS_ALLOCATOR(CloudscapeRasterPass, AZ::SystemAllocator);
    
    
        virtual ~CloudscapeRasterPass() = default;
    
        //! Creates a LookModificationPass
        static AZ::RPI::Ptr<CloudscapeRasterPass> Create(const AZ::RPI::PassDescriptor& descriptor);

        void UpdateFrameCounter(uint32_t frameCounter);
    
    protected:
        CloudscapeRasterPass(const AZ::RPI::PassDescriptor& descriptor);
        
        //! Pass behavior overrides
        void InitializeInternal() override;
        void FrameBeginInternal(FramePrepareParams params) override;
    
    private:    
        // Scope producer functions...
        void SetupFrameGraphDependencies(AZ::RHI::FrameGraphInterface frameGraph) override;
        void CompileResources(const AZ::RHI::FrameGraphCompileContext& context) override;
        void BuildCommandListInternal(const AZ::RHI::FrameGraphExecuteContext& context) override;
    
        bool m_srgNeedsUpdate = true;

        AZ::RHI::ShaderInputNameIndex m_cloudscapeTextureIndexIndex = "m_cloudscapeTextureIndex";

        uint32_t m_cloudscapeTextureIndex = 0;
    };

}   // namespace VolumetricClouds
