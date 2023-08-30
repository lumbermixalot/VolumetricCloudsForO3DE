/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Serialization/SerializeContext.h>

#include <Atom/RPI.Public/Scene.h>
#include <Atom/RPI.Public/RPIUtils.h>

#include <Renderer/CloudTexturesDebugViewerFeatureProcessor.h>
#include "CloudTextureAssetComponentController.h"

namespace VolumetricClouds
{
        void CloudTextureAssetComponentConfig::Reflect(AZ::ReflectContext* context)
        {
            if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
            {
                serializeContext->Class<CloudTextureAssetComponentConfig, AZ::ComponentConfig>()
                    ->Version(1)
                    ->Field("CloudTextureAsset", &CloudTextureAssetComponentConfig::m_cloudTextureAsset)
                    ->Field("PresentationData", &CloudTextureAssetComponentConfig::m_presentationData)
                    ;

                if (auto editContext = serializeContext->GetEditContext())
                {
                    editContext->Class<CloudTextureAssetComponentConfig>(
                        "CloudTextureAssetComponentConfig", "Configuration data for the Volumetric Clouds Component.")
                        ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::Show)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &CloudTextureAssetComponentConfig::m_cloudTextureAsset, "3D Texture Asset", "")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &CloudTextureAssetComponentConfig::m_presentationData, "Presentation", "")
                        ;
                }
            }

        }

        void CloudTextureAssetComponentController::Reflect(AZ::ReflectContext* context)
        {
            CloudTextureAssetComponentConfig::Reflect(context);

            if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
            {
                serializeContext->Class<CloudTextureAssetComponentController>()
                    ->Version(1)
                    ->Field("Configuration", &CloudTextureAssetComponentController::m_configuration)
                    ;

                if (auto editContext = serializeContext->GetEditContext())
                {

                    editContext->Class<CloudTextureAssetComponentController>(
                        "CloudTextureAssetComponentController", "")
                        ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &CloudTextureAssetComponentController::m_configuration, "Configuration", "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::Show)
                        ;
                }

            }

        }

        void CloudTextureAssetComponentController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
        {
            provided.push_back(AZ_CRC_CE("CloudTextureProviderService"));
        }

        void CloudTextureAssetComponentController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
        {
            incompatible.push_back(AZ_CRC_CE("CloudTextureProviderService"));
        }

        CloudTextureAssetComponentController::CloudTextureAssetComponentController(const CloudTextureAssetComponentConfig& config)
            : m_configuration(config)
        {
            m_prevConfiguration = m_configuration;
        }

        void CloudTextureAssetComponentController::Activate(AZ::EntityId entityId)
        {
            m_entityId = entityId;
            AZ::TransformNotificationBus::Handler::BusConnect(entityId);
            CloudTextureProviderRequestBus::Handler::BusConnect(entityId);

            auto textureAssetId = m_configuration.m_cloudTextureAsset.GetId();
            if (textureAssetId.IsValid())
            {
                AZ::Data::AssetBus::Handler::BusConnect(textureAssetId);
                m_configuration.m_cloudTextureAsset.QueueLoad();
            }
        }

        void CloudTextureAssetComponentController::Deactivate()
        {
            AZ::Data::AssetBus::Handler::BusDisconnect();
            CloudTextureProviderRequestBus::Handler::BusDisconnect();
            AZ::TransformNotificationBus::Handler::BusDisconnect();

            if (m_debugViewerFeatureProcessor)
            {
                m_debugViewerFeatureProcessor->RemoveCloudTextureInstance(m_entityId);
            }
            
            m_entityId = AZ::EntityId(AZ::EntityId::InvalidEntityId);
        }

        void CloudTextureAssetComponentController::SetConfiguration(const CloudTextureAssetComponentConfig& config)
        {
            m_configuration = config;
            m_prevConfiguration = config;
        }

        const CloudTextureAssetComponentConfig& CloudTextureAssetComponentController::GetConfiguration() const
        {
            return m_configuration;
        }

        void CloudTextureAssetComponentController::OnConfigurationChanged()
        {

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

        CloudTexturesDebugViewerFeatureProcessor* CloudTextureAssetComponentController::GetDebugViewerFeatureProcessor()
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

        void CloudTextureAssetComponentController::OnAssetStateChanged(AZ::Data::Asset<AZ::Data::AssetData> asset, [[maybe_unused]] bool isReload)
        {
            if (m_configuration.m_cloudTextureAsset.GetId() == asset.GetId())
            {
                AZ_Info(LogName, "The cloud texture asset is ready: %s", asset.GetHint().c_str());
                m_configuration.m_cloudTextureAsset = asset;
                auto updateTexture = [this]()
                {
                    m_cloudTextureImage = AZ::RPI::StreamingImage::FindOrCreate(m_configuration.m_cloudTextureAsset);

                    CloudTextureProviderNotificationBus::Event(m_entityId, &CloudTextureProviderNotificationBus::Handler::OnCloudTextureImageReady, m_cloudTextureImage);

                    if (!m_configuration.m_presentationData.IsHidden() && !m_debugViewerFeatureProcessor)
                    {
                        AZ::Transform transform = AZ::Transform::CreateIdentity();
                        AZ::TransformBus::EventResult(transform, m_entityId, &AZ::TransformBus::Events::GetWorldTM);
                        auto debugViewerProcessor = GetDebugViewerFeatureProcessor();
                        debugViewerProcessor->AddCloudTextureInstance(m_entityId, m_cloudTextureImage, transform, m_configuration.m_presentationData);
                    }

                };
                AZ::TickBus::QueueFunction(AZStd::move(updateTexture));
            } 
        }

        ////////////////////////////////////////////////////////////////////////
        //! Data::AssetBus START
        void CloudTextureAssetComponentController::OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset)
        {
            const bool isReload = false;
            OnAssetStateChanged(asset, isReload);
        }

        void CloudTextureAssetComponentController::OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset)
        {
            const bool isReload = true;
            OnAssetStateChanged(asset, isReload);
        }
        //! Data::AssetBus END
        ////////////////////////////////////////////////////////////////////////


        //! AZ::TransformNotificationBus::Handler
        void CloudTextureAssetComponentController::OnTransformChanged(const AZ::Transform& /*local*/, const AZ::Transform& world)
        {
            if (m_debugViewerFeatureProcessor)
            {
                m_debugViewerFeatureProcessor->UpdateWorldTransform(m_entityId, world);
            }
        }

        /////////////////////////////////////////////////////////
        // CloudTextureProviderRequestBus::Handler overrides ....
        AZ::Data::Instance<AZ::RPI::Image> CloudTextureAssetComponentController::GetCloudTextureImage()
        {
            return m_cloudTextureImage;
        }
        /////////////////////////////////////////////////////////

} // namespace VolumetricClouds
