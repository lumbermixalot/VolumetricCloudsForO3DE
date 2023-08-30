/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <AzCore/std/containers/deque.h>
#include <Atom/RPI.Public/FeatureProcessor.h>

#include "CloudTextureComputePipeline.h"

namespace VolumetricClouds
{
    class AZ::RPI::Scene;

    class CloudTexturesComputeFeatureProcessor final
        : public AZ::RPI::FeatureProcessor
    {
    public:
        AZ_CLASS_ALLOCATOR(CloudTexturesComputeFeatureProcessor, AZ::SystemAllocator)
        AZ_RTTI(CloudTexturesComputeFeatureProcessor, "{D980DB16-3AC5-4DB5-BE95-9C06E19AB555}", AZ::RPI::FeatureProcessor);

        static void Reflect(AZ::ReflectContext* context);

        CloudTexturesComputeFeatureProcessor() = default;
        virtual ~CloudTexturesComputeFeatureProcessor() = default;

        // With this we notify the caller the noise texture has been generated.
        using TextureReadyEvent = AZ::Event<AZ::Data::Instance<AZ::RPI::Image> /*image*/>;

        // This event will be signaled each time a cpu data buffer has been readback
        // for a particular mip level.
        // @param image This is a Texture3D (along with all mip levels) that is ready to be used
        //        for example as a readonly SRV in shader etc. 
        // @param mipDataBuffer Array that contains all the pixel data for a given Texture3D subresource.
        //        Can be used to store the data to disk, etc.
        // @param mipSlice The mipmap index of the subresource.
        // @param mipSize Subresource size in pixels.
        using ReadbackEvent = AZ::Event<AZ::Data::Instance<AZ::RPI::Image> /*image*/,
                                        AZStd::shared_ptr<AZStd::vector<uint8_t>> /*mipDataBuffer*/,
                                        uint16_t /*mipSlice*/ ,
                                        const AZ::RHI::Size& /*mipSize*/>;

        // @param readbackHandler If different than null, then attachment readback will be added to the compute pass
        //        for all mip levels. 
        bool EnqueueComputeRequest(const AZ::EntityId& entityId, const CloudTextureComputeData& computeData,
                                   TextureReadyEvent::Handler& readyHandler,
                                   ReadbackEvent::Handler* readbackHandler = nullptr);

        struct CloudTextureComputeRequest
        {
            // When true, the dispatched compute pipeline will also
            // enqueue attachment readback operations to read the whole
            // volume texture and all its mipmaps.
            // CloudTextureNoticationBus::OnCloudTextureMipmapReadbackReady  will be disptached
            // for each mip level.
            bool m_withAttachmentReadback = false;

            // Every time the @m_computeData or @m_cloudTextureAttachment
            // change, we instantiate a CloudTextureRender, which in turn
            // creates a short live render pipeline that generates a new
            // noise texture. We record here the task id to know when
            // the texture is ready to be used by the presentation shader
            // or to be saved to Disk, etc.
            CloudTextureComputePipeline::RenderTaskId m_textureComputeTaskId = 0;
            AZ::Data::Instance<AZ::RPI::AttachmentImage> m_cloudTextureAttachment;
            CloudTextureComputeData m_computeData;
            TextureReadyEvent m_readyEvent;
            ReadbackEvent m_readbackEvent;
        };

    private:

        CloudTexturesComputeFeatureProcessor(const CloudTexturesComputeFeatureProcessor&) = delete;

        static constexpr char LogName[] = "CloudTexturesComputeFeatureProcessor";

        AZ::Data::Instance<AZ::RPI::AttachmentImage> CreateTexture3DAttachmentImage(uint32_t pixelSize);
        AZStd::shared_ptr<CloudTextureComputePipeline> CreateTextureComputeTask(CloudTextureComputeRequest& cloudTextureInstance);

        //////////////////////////////////////////////////////////////////
        //! AZ::RPI::FeatureProcessor overrides START...
        void Activate() override;
        void Deactivate() override;
        void OnRenderEnd() override;
        ///////////////////////////////////////////////////////////////////

        void ActivateComputeScene();
        void DeactivateComputeScene();

        static constexpr const char* FeatureProcessorName = "CloudTexturesComputeFeatureProcessor";

        AZStd::unordered_map<AZ::EntityId, CloudTextureComputeRequest> m_computeRequests;

        // When a new CloudTextureComputeRequest is created, we add the entityid in this queue.
        // Periodically we check if there's an entity at the front of the queue. If there's
        // an entity, we use the  CloudTextureComputeRequest compute data to spawn a new CloudTextureComputePipeline.
        // This feature processor only runs one CloudTextureComputePipeline at a time (even though we could,
        // in principle, run all in parallel) because there are crashes related with AttachmentReadbacks when running in parallel.
        // This is why we use a queue instead of a vector.
        AZStd::deque<AZ::EntityId> m_cloudTextureComputeTasks;

        // This compute task relates with the CloudTextureComputeRequest at the front of @m_cloudTextureComputeTasks
        AZStd::shared_ptr<CloudTextureComputePipeline> m_currentCloudTextureComputeTask;
        // We need to create a scene that will be used by CloudTextureComputePipeline(s)
        // to instantiate their render pipeline that runs the compute pass
        // the generates the 3D Noise Textures. The idea is that if we use the default main scene
        // each time a CloudTextureComputePipeline all other feature processors in the main scene would be notified
        // and it spawns a mess of notifications that are not relevant to the other feature processors.
        AZ::RPI::ScenePtr m_computeScene;

    };
} // namespace VolumetricClouds
