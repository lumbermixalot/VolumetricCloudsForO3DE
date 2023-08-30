/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <Atom/RPI.Public/Image/AttachmentImagePool.h>
#include <Atom/RPI.Public/RenderPipeline.h>
#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/Image/ImageSystemInterface.h>

#include <Renderer/Passes/CloudTextureComputePass.h>
#include "CloudTexturesComputeFeatureProcessor.h"

namespace VolumetricClouds
{
    void CloudTexturesComputeFeatureProcessor::Reflect(AZ::ReflectContext* context)
    {
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext
                ->Class<CloudTexturesComputeFeatureProcessor, AZ::RPI::FeatureProcessor>()
                ->Version(1);
        }
    }

    /////////////////////////////////////////////////////////////////////////////
    //! AZ::RPI::FeatureProcessor overrides START ...
    void CloudTexturesComputeFeatureProcessor::Activate()
    {
        ActivateComputeScene();
    }

    void CloudTexturesComputeFeatureProcessor::ActivateComputeScene()
    {
        AZ_Assert(!m_computeScene, "Compute Scene was already activated!");

        AZ::RPI::SceneDescriptor sceneDesc;
        m_computeScene = AZ::RPI::Scene::CreateScene(sceneDesc);

        // Currently the scene has to be activated after render pipeline was added so some feature processors (i.e. imgui) can be initialized properly 
        // with pipeline's pass information. 
        m_computeScene->Activate();

        AZ::RPI::RPISystemInterface::Get()->RegisterScene(m_computeScene);
    }

    void CloudTexturesComputeFeatureProcessor::Deactivate()
    {
        m_computeRequests.clear();

        DeactivateComputeScene();
    }

    void CloudTexturesComputeFeatureProcessor::DeactivateComputeScene()
    {
        decltype(m_cloudTextureComputeTasks) tmpQueue;
        AZStd::swap(tmpQueue, m_cloudTextureComputeTasks);

        if (m_computeScene)
        {
            AZ::RPI::RPISystemInterface::Get()->UnregisterScene(m_computeScene);
            m_computeScene = nullptr;
        }

    }


    void CloudTexturesComputeFeatureProcessor::OnRenderEnd()
    {
        // // The C++ 20 new way to do remove erase if.
        // AZStd::erase_if(m_cloudTextureComputeTasks, [](AZStd::shared_ptr<CloudTextureComputePipeline>& renderer) {
        //     renderer->CheckAndRemovePipeline();
        //     return !renderer->IsRenderingNoiseTexture();
        // });
        if (m_currentCloudTextureComputeTask)
        {
            m_currentCloudTextureComputeTask->CheckAndRemovePipeline();
            if (m_currentCloudTextureComputeTask->IsRenderingNoiseTexture())
            {
                return;
            }
            m_currentCloudTextureComputeTask.reset();
        }

        while (!m_cloudTextureComputeTasks.empty())
        {
            AZ::EntityId entityId = m_cloudTextureComputeTasks.front();
            m_cloudTextureComputeTasks.pop_front();
            if (!m_computeRequests.contains(entityId))
            {
                AZ_Info(LogName, "CloudTextureComputeRequest with entityId=%s already gone.\n", entityId.ToString().c_str());
                continue;
            }
            auto& CloudTextureComputeRequest = m_computeRequests.at(entityId);
            m_currentCloudTextureComputeTask = CreateTextureComputeTask(CloudTextureComputeRequest);
            AZ_Info(LogName, "Created new compute task id=%u for entityId=%s.\n",
                CloudTextureComputeRequest.m_textureComputeTaskId, entityId.ToString().c_str());
            break;
        }

    }

    //! AZ::RPI::FeatureProcessor overrides END ...
    /////////////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////////
    //! Functions called by CloudTextureComputeComponentController START
    bool CloudTexturesComputeFeatureProcessor::EnqueueComputeRequest(const AZ::EntityId& entityId
        , const CloudTextureComputeData& computeData
        , CloudTexturesComputeFeatureProcessor::TextureReadyEvent::Handler& readyHandler
        , CloudTexturesComputeFeatureProcessor::ReadbackEvent::Handler* readbackHandler)
    {
        CloudTextureComputeRequest newRequest;
        newRequest.m_cloudTextureAttachment = CreateTexture3DAttachmentImage(computeData.m_pixelSize);
        newRequest.m_computeData = computeData;
        newRequest.m_withAttachmentReadback = (readbackHandler != nullptr);
        newRequest.m_textureComputeTaskId = 0; // A valid value will be assigned when the compute pipeline is created for this request.
        m_computeRequests.emplace(entityId, AZStd::move(newRequest));

        // The handlers must be set AFTER emplace because AZ::Events resets the connected
        // handlers during AZStd::move.
        if (readyHandler.IsConnected())
        {
            readyHandler.Disconnect();
        }
        readyHandler.Connect(m_computeRequests.at(entityId).m_readyEvent);
        if (readbackHandler)
        {
            if (readbackHandler->IsConnected())
            {
                readbackHandler->Disconnect();
            }
            readbackHandler->Connect(m_computeRequests.at(entityId).m_readbackEvent);
        }
        
        m_cloudTextureComputeTasks.push_back(entityId);
        return true;
    }
    //! Functions called by CloudscapeComponentController END
    /////////////////////////////////////////////////////////////////////

    AZ::Data::Instance<AZ::RPI::AttachmentImage> CloudTexturesComputeFeatureProcessor::CreateTexture3DAttachmentImage(uint32_t pixelSize)
    {
        AZ::RHI::ImageDescriptor imageDesc = AZ::RHI::ImageDescriptor::Create3D(
            AZ::RHI::ImageBindFlags::ShaderReadWrite, pixelSize, pixelSize, pixelSize, AZ::RHI::Format::R8G8B8A8_UNORM);
        imageDesc.m_mipLevels = CloudTextureComputePass::CalculateMipCount(pixelSize);
        AZ::RHI::ClearValue clearValue = AZ::RHI::ClearValue::CreateVector4Float(0, 0, 0, 0);
        AZ::Data::Instance<AZ::RPI::AttachmentImagePool> pool = AZ::RPI::ImageSystemInterface::Get()->GetSystemAttachmentPool();
        return AZ::RPI::AttachmentImage::Create(*pool.get(), imageDesc, AZ::Name("CloudTextureAttachmentImage"), &clearValue, nullptr);
    }


    AZStd::shared_ptr<CloudTextureComputePipeline> CloudTexturesComputeFeatureProcessor::CreateTextureComputeTask(CloudTextureComputeRequest& CloudTextureComputeRequest)
    {
        auto texture3DReadyCB = [&](CloudTextureComputePipeline::RenderTaskId textureComputeTaskId,
                                    const AZStd::vector<CloudTextureComputePipeline::CloudTextureSubresourceReadback>& readbackResults) {
            AZ::EntityId foundEntityId;
            for (auto& [entityId, CloudTextureComputeRequest] : m_computeRequests)
            {
                if (CloudTextureComputeRequest.m_textureComputeTaskId != textureComputeTaskId)
                {
                    continue;
                }

                foundEntityId = entityId;

                CloudTextureComputeRequest.m_readyEvent.Signal(CloudTextureComputeRequest.m_cloudTextureAttachment);
                for (const auto & subresourceReadback : readbackResults)
                {
                    CloudTextureComputeRequest.m_readbackEvent.Signal(CloudTextureComputeRequest.m_cloudTextureAttachment,
                        subresourceReadback.m_dataBuffer, subresourceReadback.m_mipSlice, subresourceReadback.m_mipSize);
                }

                CloudTextureComputeRequest.m_textureComputeTaskId = 0;
                CloudTextureComputeRequest.m_withAttachmentReadback = false;

                break;
            }
            if (foundEntityId.IsValid())
            {
                // We will erase this entity from the map if it is not in the queue.
                bool foundInQueue = false;
                for (const auto& entityId : m_cloudTextureComputeTasks)
                {
                    if (entityId == foundEntityId)
                    {
                        foundInQueue = true;
                        break;
                    }
                }
                if (!foundInQueue)
                {
                    m_computeRequests.erase(foundEntityId);
                }
            }
        };

        auto textureComputeTask = AZStd::make_shared<CloudTextureComputePipeline>();
        CloudTextureComputeRequest.m_textureComputeTaskId = textureComputeTask->StartTextureCompute(
            m_computeScene.get(),
            CloudTextureComputeRequest.m_cloudTextureAttachment,
            CloudTextureComputeRequest.m_computeData,
            texture3DReadyCB,
            CloudTextureComputeRequest.m_withAttachmentReadback);
        return textureComputeTask;
    }

} // namespace VolumetricClouds
