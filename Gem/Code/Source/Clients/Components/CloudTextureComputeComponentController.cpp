/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Component/TickBus.h>

#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/RPIUtils.h>

#include <Renderer/CloudTexturesDebugViewerFeatureProcessor.h>
#include "CloudTextureComputeComponentController.h"

namespace VolumetricClouds
{
    void CloudTextureComputeComponentConfig::Reflect(AZ::ReflectContext* context)
    {
        CloudTexturePresentationData::Reflect(context);
        CloudTextureComputeData::Reflect(context);
    
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CloudTextureComputeComponentConfig, AZ::ComponentConfig>()
                ->Version(1)
                ->Field("ComputeData", &CloudTextureComputeComponentConfig::m_computeData)
                ->Field("PresentationData", &CloudTextureComputeComponentConfig::m_presentationData)
                ;
    
            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<CloudTextureComputeComponentConfig>(
                    "CloudTextureComputeComponentConfig", "Configuration data for the Volumetric Clouds Component.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::Show)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &CloudTextureComputeComponentConfig::m_computeData, "Compute", "")
                    ->DataElement(AZ::Edit::UIHandlers::Default, &CloudTextureComputeComponentConfig::m_presentationData, "Presentation", "")
                    ;
            }
        }
    
    }
    
    void CloudTextureComputeComponentController::Reflect(AZ::ReflectContext* context)
    {
        CloudTextureComputeComponentConfig::Reflect(context);
    
        if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CloudTextureComputeComponentController>()
                ->Version(1)
                ->Field("Configuration", &CloudTextureComputeComponentController::m_configuration)
                ;
    
            if (auto editContext = serializeContext->GetEditContext())
            {
    
                editContext->Class<CloudTextureComputeComponentController>(
                    "CloudTextureComputeComponentController", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &CloudTextureComputeComponentController::m_configuration, "Configuration", "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::Show)
                    ;
            }
    
        }
    
    }
    
    void CloudTextureComputeComponentController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("CloudTextureProviderService"));
    }
    
    void CloudTextureComputeComponentController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("CloudTextureProviderService"));
    }
    
    CloudTextureComputeComponentController::CloudTextureComputeComponentController(const CloudTextureComputeComponentConfig& config)
        : m_configuration(config)
    {
        m_prevConfiguration = m_configuration;
    }
    
    void CloudTextureComputeComponentController::Activate(AZ::EntityId entityId)
    {
        m_entityId = entityId;
        CloudTextureProviderRequestBus::Handler::BusConnect(entityId);
    
        m_textureReadyEventHandler = CloudTexturesComputeFeatureProcessor::TextureReadyEvent::Handler(
            [this](AZ::Data::Instance<AZ::RPI::Image> image)
            {
                m_cloudTextureImage = image;
                // Enqueue on the TickBus a notification that this texture is ready.
                auto notifyTextureReadyFn = [entityId = m_entityId, image = m_cloudTextureImage]()
                {
                    CloudTextureProviderNotificationBus::Event(entityId, &CloudTextureProviderNotificationBus::Handler::OnCloudTextureImageReady, image);
                };
                AZ::TickBus::QueueFunction(AZStd::move(notifyTextureReadyFn));
            }
        );
    
        AZ::TransformNotificationBus::Handler::BusConnect(entityId);
    
        auto computeProcessor = GetComputeFeatureProcessor();
        computeProcessor->EnqueueComputeRequest(entityId, m_configuration.m_computeData, m_textureReadyEventHandler);
    
        if (!m_configuration.m_presentationData.IsHidden())
        {
            AZ::Transform transform = AZ::Transform::CreateIdentity();
            AZ::TransformBus::EventResult(transform, entityId, &AZ::TransformBus::Events::GetWorldTM);
            auto debugViewerProcessor = GetDebugViewerFeatureProcessor();
            debugViewerProcessor->AddCloudTextureInstance(entityId, m_cloudTextureImage, transform, m_configuration.m_presentationData);
        }
    
    }
    
    void CloudTextureComputeComponentController::Deactivate()
    {
        AZ::TransformNotificationBus::Handler::BusDisconnect();
    
        if (m_debugViewerFeatureProcessor)
        {
            m_debugViewerFeatureProcessor->RemoveCloudTextureInstance(m_entityId);
        }
        
        CloudTextureProviderRequestBus::Handler::BusDisconnect(m_entityId);
        m_entityId = AZ::EntityId(AZ::EntityId::InvalidEntityId);
    }
    
    
    void CloudTextureComputeComponentController::SetConfiguration(const CloudTextureComputeComponentConfig& config)
    {
        m_configuration = config;
        m_prevConfiguration = config;
    }
    
    const CloudTextureComputeComponentConfig& CloudTextureComputeComponentController::GetConfiguration() const
    {
        return m_configuration;
    }
    
    void CloudTextureComputeComponentController::OnConfigurationChanged()
    {
        if (m_prevConfiguration.m_computeData != m_configuration.m_computeData)
        {
            auto computeProcessor = GetComputeFeatureProcessor();
            computeProcessor->EnqueueComputeRequest(m_entityId, m_configuration.m_computeData, m_textureReadyEventHandler);
        }
    
        if (m_prevConfiguration.m_presentationData != m_configuration.m_presentationData)
        {
            if (m_configuration.m_presentationData.IsHidden())
            {
                if (m_debugViewerFeatureProcessor)
                {
                    m_debugViewerFeatureProcessor->RemoveCloudTextureInstance(m_entityId);
                }
            }
            else
            {
                if (m_debugViewerFeatureProcessor)
                {
                    m_debugViewerFeatureProcessor->UpdatePresentationData(m_entityId, m_configuration.m_presentationData);
                }
                else
                {
                    auto debugViewerFeatureProcessor = GetDebugViewerFeatureProcessor();
                    if (debugViewerFeatureProcessor)
                    {
                        AZ::Transform transform = AZ::Transform::CreateIdentity();
                        AZ::TransformBus::EventResult(transform, m_entityId, &AZ::TransformBus::Events::GetWorldTM);
                        debugViewerFeatureProcessor->AddCloudTextureInstance(m_entityId, m_cloudTextureImage, transform, m_configuration.m_presentationData);
                    }
                }
            }
        }
    
        m_prevConfiguration = m_configuration;
    }
    
    CloudTexturesComputeFeatureProcessor* CloudTextureComputeComponentController::GetComputeFeatureProcessor()
    {
        if (m_computeFeatureProcessor)
        {
            return m_computeFeatureProcessor;
        }
    
        auto scenePtr = AZ::RPI::Scene::GetSceneForEntityId(m_entityId);
        if (scenePtr)
        {
            m_computeFeatureProcessor = scenePtr->EnableFeatureProcessor<CloudTexturesComputeFeatureProcessor>();
        }
        AZ_Assert(!!m_computeFeatureProcessor, "CloudTextureComputeComponentController failed to enable CloudTexturesComputeFeatureProcessor!");
        return m_computeFeatureProcessor;
    }
    
    CloudTexturesDebugViewerFeatureProcessor* CloudTextureComputeComponentController::GetDebugViewerFeatureProcessor()
    {
        if (m_debugViewerFeatureProcessor)
        {
            return m_debugViewerFeatureProcessor;
        }
    
        auto scenePtr = AZ::RPI::Scene::GetSceneForEntityId(m_entityId);
        if (scenePtr)
        {
            m_debugViewerFeatureProcessor = scenePtr->EnableFeatureProcessor<CloudTexturesDebugViewerFeatureProcessor>();
        }
        AZ_Error(LogName, !!m_debugViewerFeatureProcessor, "CloudTextureComputeComponentController failed to enable CloudTexturesDebugViewerFeatureProcessor!");
        return m_debugViewerFeatureProcessor;
    }
    
    void CloudTextureComputeComponentController::ForceCloudTextureRegeneration(
        CloudTexturesComputeFeatureProcessor::ReadbackEvent::Handler* readbackHandler)
    {
        auto computeProcessor = GetComputeFeatureProcessor();
        computeProcessor->EnqueueComputeRequest(m_entityId, m_configuration.m_computeData
            , m_textureReadyEventHandler, readbackHandler);
    }
    
    
    //! AZ::TransformNotificationBus::Handler
    void CloudTextureComputeComponentController::OnTransformChanged(const AZ::Transform& /*local*/, const AZ::Transform& world)
    {
        if (m_debugViewerFeatureProcessor)
        {
            m_debugViewerFeatureProcessor->UpdateWorldTransform(m_entityId, world);
        }
    }
    
    /////////////////////////////////////////////////////////
    // CloudTextureProviderRequestBus::Handler overrides ....
    AZ::Data::Instance<AZ::RPI::Image> CloudTextureComputeComponentController::GetCloudTextureImage()
    {
        return m_cloudTextureImage;
    }
    /////////////////////////////////////////////////////////


} // namespace VolumetricClouds
