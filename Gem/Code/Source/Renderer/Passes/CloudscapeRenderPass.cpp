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
#include "CloudscapeRenderPass.h"

namespace VolumetricClouds
{
    
    AZ::RPI::Ptr<CloudscapeRenderPass> CloudscapeRenderPass::Create(const AZ::RPI::PassDescriptor& descriptor)
    {
        AZ::RPI::Ptr<CloudscapeRenderPass> pass = aznew CloudscapeRenderPass(descriptor);
        return AZStd::move(pass);
    }
    
    CloudscapeRenderPass::CloudscapeRenderPass(const AZ::RPI::PassDescriptor& descriptor)
        : AZ::RPI::FullscreenTrianglePass(descriptor)
    {
    }
    
    void CloudscapeRenderPass::InitializeInternal()
    {
        AZ::RPI::FullscreenTrianglePass::InitializeInternal();

        m_cloudscapeTextureIndex = 0;
        m_srgNeedsUpdate = true;

        //InitializeShaderVariant();
    }


    void CloudscapeRenderPass::FrameBeginInternal(FramePrepareParams params)
    {
        AZ::RPI::FullscreenTrianglePass::FrameBeginInternal(params);
    }


    void CloudscapeRenderPass::SetupFrameGraphDependencies(AZ::RHI::FrameGraphInterface frameGraph)
    {
        AZ::RPI::FullscreenTrianglePass::SetupFrameGraphDependencies(frameGraph);
    }

    
    void CloudscapeRenderPass::CompileResources(const AZ::RHI::FrameGraphCompileContext& context)
    {
       AZ_Assert(m_shaderResourceGroup != nullptr, "CloudscapeRenderPass %s has a null shader resource group when calling Compile.", GetPathName().GetCStr());

       if (m_srgNeedsUpdate)
       {
           m_shaderResourceGroup->SetConstant(m_cloudscapeTextureIndexIndex, m_cloudscapeTextureIndex);
           m_srgNeedsUpdate = false;
       }

       AZ::RPI::FullscreenTrianglePass::CompileResources(context);
    }
    
    
    void CloudscapeRenderPass::BuildCommandListInternal(const AZ::RHI::FrameGraphExecuteContext& context)
    {
        //AZ_Assert(m_shaderResourceGroup != nullptr, "CloudscapeRenderPass %s has a null shader resource group when calling Execute.", GetPathName().GetCStr());
        //
        //AZ::RHI::CommandList* commandList = context.GetCommandList();
        //
        //commandList->SetViewport(m_viewportState);
        //commandList->SetScissor(m_scissorState);
        //
        //SetSrgsForDraw(commandList);
        //
        //m_item.m_pipelineState = GetPipelineStateFromShaderVariant();
        //
        //commandList->Submit(m_item);
        AZ::RPI::FullscreenTrianglePass::BuildCommandListInternal(context);
    }


    void CloudscapeRenderPass::UpdateFrameCounter(uint32_t frameCounter)
    {
        m_cloudscapeTextureIndex = frameCounter % 2;
        m_srgNeedsUpdate = true;
    }

}   // VolumetricClouds AZ
