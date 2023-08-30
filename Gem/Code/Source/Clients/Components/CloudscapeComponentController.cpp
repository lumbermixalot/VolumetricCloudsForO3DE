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
#include <Atom/RPI.Public/View.h>
#include <Atom/RPI.Public/ViewportContext.h>

#include <Renderer/CloudscapeFeatureProcessor.h>
#include "CloudscapeComponentController.h"

namespace VolumetricClouds
{
        void CloudscapeComponentConfig::Reflect(AZ::ReflectContext* context)
        {
            CloudscapeShaderConstantData::Reflect(context);

            if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
            {
                serializeContext->Class<CloudscapeComponentConfig, AZ::ComponentConfig>()
                    ->Version(1)
                    ->Field("LowFreqTextureEntity", &CloudscapeComponentConfig::m_lowFreqTextureEntity)
                    ->Field("HighFreqTextureEntity", &CloudscapeComponentConfig::m_highFreqTextureEntity)
                    ->Field("SunEntity", &CloudscapeComponentConfig::m_sunEntity)
                    ->Field("WeatherMap", &CloudscapeComponentConfig::m_weatherMap)
                    ->Field("ShaderConstantData", &CloudscapeComponentConfig::m_shaderConstantData)
                    ;

                if (auto editContext = serializeContext->GetEditContext())
                {
                    editContext->Class<CloudscapeComponentConfig>(
                        "CloudscapeComponentConfig", "Configuration data for the Volumetric Clouds Component.")
                        ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::Show)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &CloudscapeComponentConfig::m_lowFreqTextureEntity, "Low Frequency Texture Entity", "An entity that provides a, typically 128x128x128, Texture3D for sampling low frequency cloud-like data.")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &CloudscapeComponentConfig::m_highFreqTextureEntity, "High Frequency Texture Entity", "An entity that provides a, typically 32x32x32, Texture3D for sampling high frequency cloud-like data.")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &CloudscapeComponentConfig::m_weatherMap, "Weather Map", "4-channels weather map data.")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &CloudscapeComponentConfig::m_sunEntity, "Sun Entity", "An entity with a Directional Light Component, representing the Sun. Defines sun light direction and color.")
                        ->DataElement(AZ::Edit::UIHandlers::Default, &CloudscapeComponentConfig::m_shaderConstantData, "Shader Constants", "")
                        ;
                }
            }
        }

        void CloudscapeComponentController::Reflect(AZ::ReflectContext* context)
        {
            CloudscapeComponentConfig::Reflect(context);

            if (auto* serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
            {
                // Register all the tuple types used below so that they marshal to/from python and Lua correctly.
                serializeContext->RegisterGenericType<AZStd::tuple<uint8_t, uint8_t>>();

                serializeContext->Class<CloudscapeComponentController>()
                    ->Version(1)
                    ->Field("Configuration", &CloudscapeComponentController::m_configuration)
                    ;

                if (auto editContext = serializeContext->GetEditContext())
                {

                    editContext->Class<CloudscapeComponentController>(
                        "CloudscapeComponentController", "")
                        ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &CloudscapeComponentController::m_configuration, "Configuration", "")
                        ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                        ;
                }
            }

            AZ::BehaviorContext* behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context);
            if (behaviorContext)
            {
                behaviorContext->EBus<VolumetricCloudsRequestBus>("VolumetricCloudsRequestBus")
                    ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)
                    ->Attribute(AZ::Script::Attributes::Module, ScriptingModuleName)
                    ->Event("BeginCallBatch", &VolumetricCloudsRequestBus::Events::BeginCallBatch)
                    ->Event("IsCallBatching", &VolumetricCloudsRequestBus::Events::IsCallBatching)
                    ->Event("GetUVWScale", &VolumetricCloudsRequestBus::Events::GetUVWScale)
                    ->Event("SetUVWScale", &VolumetricCloudsRequestBus::Events::SetUVWScale)
                    ->Event("GetMaxMipLevels", &VolumetricCloudsRequestBus::Events::GetMaxMipLevels)
                    ->Event("SetMaxMipLevels", &VolumetricCloudsRequestBus::Events::SetMaxMipLevels)
                    // From  Lua
                    // local tuple = VolumetricCloudsRequestBus.Broadcast.GetRayMarchingSteps()
                    // Debug.Log("min=" .. tostring(tuple:Get0()) .. ", max=" .. tostring(tuple:Get1()))
                    // From Python
                    // stepsMin, stepsMax = volumetric_clouds.VolumetricCloudsRequestBus(bus.Broadcast, 'GetRayMarchingSteps')
                    // For more details see around line 676 of C:\GIT\o3de\Code\Framework\AzCore\AzCore\RTTI\AzStdOnDemandReflection.inl
                    ->Event("GetRayMarchingSteps", &VolumetricCloudsRequestBus::Events::GetRayMarchingSteps)
                    ->Event("SetRayMarchingSteps", &VolumetricCloudsRequestBus::Events::SetRayMarchingSteps)
                    ->Event("EndCallBatch", &VolumetricCloudsRequestBus::Events::EndCallBatch)
                    // Planetary data
                    ->Event("GetPlanetRadiusKm", &VolumetricCloudsRequestBus::Events::GetPlanetRadiusKm)
                    ->Event("SetPlanetRadiusKm", &VolumetricCloudsRequestBus::Events::SetPlanetRadiusKm)
                    ->Event("GetDistanceToCloudSlabKm", &VolumetricCloudsRequestBus::Events::GetDistanceToCloudSlabKm)
                    ->Event("SetDistanceToCloudSlabKm", &VolumetricCloudsRequestBus::Events::SetDistanceToCloudSlabKm)
                    ->Event("GetCloudSlabThicknessKm", &VolumetricCloudsRequestBus::Events::GetCloudSlabThicknessKm)
                    ->Event("SetCloudSlabThicknessKm", &VolumetricCloudsRequestBus::Events::SetCloudSlabThicknessKm)
                    ->Event("GetSunLightColorAndIntensity", &VolumetricCloudsRequestBus::Events::GetSunLightColorAndIntensity)
                    ->Event("SetSunLightColorAndIntensity", &VolumetricCloudsRequestBus::Events::SetSunLightColorAndIntensity)
                    ->Event("GetAmbientLightColorAndIntensity", &VolumetricCloudsRequestBus::Events::GetAmbientLightColorAndIntensity)
                    ->Event("SetAmbientLightColorAndIntensity", &VolumetricCloudsRequestBus::Events::SetAmbientLightColorAndIntensity)
                    // Weather data
                    ->Event("GetWeatherMapSizeKm", &VolumetricCloudsRequestBus::Events::GetWeatherMapSizeKm)
                    ->Event("SetWeatherMapSizeKm", &VolumetricCloudsRequestBus::Events::SetWeatherMapSizeKm)
                    ->Event("GetCloudCoverage", &VolumetricCloudsRequestBus::Events::GetCloudCoverage)
                    ->Event("SetCloudCoverage", &VolumetricCloudsRequestBus::Events::SetCloudCoverage)
                    ->Event("GetCloudDensity", &VolumetricCloudsRequestBus::Events::GetCloudDensity)
                    ->Event("SetCloudDensity", &VolumetricCloudsRequestBus::Events::SetCloudDensity)
                    ->Event("GetWindVelocity", &VolumetricCloudsRequestBus::Events::GetWindVelocity)
                    ->Event("SetWindVelocity", &VolumetricCloudsRequestBus::Events::SetWindVelocity)
                    ->Event("GetCloudTopShiftKm", &VolumetricCloudsRequestBus::Events::GetCloudTopShiftKm)
                    ->Event("SetCloudTopShiftKm", &VolumetricCloudsRequestBus::Events::SetCloudTopShiftKm)
                    // Cloud Material Properties
                    ->Event("GetCloudMaterialProperties", &VolumetricCloudsRequestBus::Events::GetCloudMaterialProperties)
                    ->Event("SetCloudMaterialProperties", &VolumetricCloudsRequestBus::Events::SetCloudMaterialProperties)
                    ;
            }
        }

        void CloudscapeComponentController::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
        {
            provided.push_back(AZ_CRC_CE("VolumetricCloudscapeService"));
        }

        void CloudscapeComponentController::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
        {
            incompatible.push_back(AZ_CRC_CE("VolumetricCloudscapeService"));
        }

        CloudscapeComponentController::CloudscapeComponentController() : m_directionalLightConfigChangedEventHandler([this]() { NotifySunLightDataChanged(); })
        {

        }

        CloudscapeComponentController::CloudscapeComponentController(const CloudscapeComponentConfig& config)
            : m_configuration(config), m_directionalLightConfigChangedEventHandler([this]() { NotifySunLightDataChanged(); })
        {
            m_prevConfiguration = m_configuration;
        }

        void CloudscapeComponentController::Activate(AZ::EntityId entityId)
        {
            if (VolumetricCloudsRequestBus::HasHandlers())
            {
                m_isActive = false;
                return;
            }
            m_isActive = true;
            VolumetricCloudsRequestBus::Handler::BusConnect();

            m_entityId = entityId;
            m_scene = AZ::RPI::Scene::GetSceneForEntityId(m_entityId);


            if (m_configuration.m_lowFreqTextureEntity.IsValid())
            {
                CloudTextureProviderNotificationBus::MultiHandler::BusConnect(m_configuration.m_lowFreqTextureEntity);
                AZ::Data::Instance<AZ::RPI::Image> image;
                CloudTextureProviderRequestBus::EventResult(image, m_configuration.m_lowFreqTextureEntity, &CloudTextureProviderRequestBus::Handler::GetCloudTextureImage);
                m_configuration.m_shaderConstantData.m_lowFrequencyNoiseTexture = image;
            }

            if (m_configuration.m_highFreqTextureEntity.IsValid())
            {
                CloudTextureProviderNotificationBus::MultiHandler::BusConnect(m_configuration.m_highFreqTextureEntity);
                AZ::Data::Instance<AZ::RPI::Image> image;
                CloudTextureProviderRequestBus::EventResult(image, m_configuration.m_highFreqTextureEntity, &CloudTextureProviderRequestBus::Handler::GetCloudTextureImage);
                m_configuration.m_shaderConstantData.m_highFrequencyNoiseTexture = image;
            }

            if (m_configuration.m_sunEntity.IsValid())
            {
                //FIXME. Validate the entity is actually activated.
                FetchAllSunLightData();
                AZ::TransformNotificationBus::Handler::BusConnect(m_configuration.m_sunEntity);
                AZ::Render::DirectionalLightRequestBus::Event(m_configuration.m_sunEntity, &AZ::Render::DirectionalLightRequests::BindConfigurationChangedEventHandler, m_directionalLightConfigChangedEventHandler);
            }

            auto textureAssetId = m_configuration.m_weatherMap.GetId();
            if (textureAssetId.IsValid())
            {
                AZ::Data::AssetBus::Handler::BusConnect(textureAssetId);
                m_configuration.m_weatherMap.QueueLoad();
            }

            m_prevConfiguration = m_configuration;
            EnableFeatureProcessor();

            auto viewportContextInterface = AZ::Interface<AZ::RPI::ViewportContextRequestsInterface>::Get();
            auto viewportContext = viewportContextInterface->GetViewportContextByScene(m_scene);
            AZ::RPI::ViewportContextIdNotificationBus::Handler::BusConnect(viewportContext->GetId());

        }

        void CloudscapeComponentController::Deactivate()
        {
            if (!m_isActive)
            {
                return;
            }

            VolumetricCloudsRequestBus::Handler::BusDisconnect();
            AZ::RPI::ViewportContextIdNotificationBus::Handler::BusDisconnect();
            m_directionalLightConfigChangedEventHandler.Disconnect();
            AZ::TransformNotificationBus::Handler::BusDisconnect();
            AZ::Data::AssetBus::Handler::BusDisconnect();
            CloudTextureProviderNotificationBus::MultiHandler::BusDisconnect();
            m_entityId = AZ::EntityId(AZ::EntityId::InvalidEntityId);
            if (m_cloudscapeFeatureProcessor)
            {
                m_scene->DisableFeatureProcessor<CloudscapeFeatureProcessor>();
                m_cloudscapeFeatureProcessor = nullptr;
                m_scene = nullptr;
            }
            m_isActive = false;
        }

        void CloudscapeComponentController::SetConfiguration(const CloudscapeComponentConfig& config)
        {
            m_configuration = config;
            m_prevConfiguration = config;
        }

        const CloudscapeComponentConfig& CloudscapeComponentController::GetConfiguration() const
        {
            return m_configuration;
        }

        void CloudscapeComponentController::OnConfigurationChanged()
        {
            if (!m_isActive)
            {
                m_prevConfiguration = m_configuration;
                return;
            }

            bool doUpdate = false;

            auto weatherMapAssetId = m_configuration.m_weatherMap.GetId();
            auto prevWeatherMapAssetId = m_prevConfiguration.m_weatherMap.GetId();
            if (weatherMapAssetId.IsValid() && (weatherMapAssetId != prevWeatherMapAssetId))
            {
                AZ::Data::AssetBus::Handler::BusDisconnect();

                // Let's disable whatever weatherMap we have at the moment.
                doUpdate = true;
                if (m_configuration.m_shaderConstantData.m_weatherMap)
                {
                    m_configuration.m_shaderConstantData.m_weatherMap.reset();
                }
                AZ::Data::AssetBus::Handler::BusConnect(weatherMapAssetId);
                m_configuration.m_weatherMap.QueueLoad();
            }

            if (m_cloudscapeFeatureProcessor)
            {
                if (m_prevConfiguration.m_lowFreqTextureEntity != m_configuration.m_lowFreqTextureEntity)
                {
                    doUpdate = true;
                    if (m_prevConfiguration.m_lowFreqTextureEntity.IsValid())
                    {
                        CloudTextureProviderNotificationBus::MultiHandler::BusDisconnect(m_prevConfiguration.m_lowFreqTextureEntity);
                        m_configuration.m_shaderConstantData.m_lowFrequencyNoiseTexture.reset();
                    }

                    if (m_configuration.m_lowFreqTextureEntity.IsValid())
                    {
                        CloudTextureProviderNotificationBus::MultiHandler::BusConnect(m_configuration.m_lowFreqTextureEntity);
                        AZ::Data::Instance<AZ::RPI::Image> image;
                        CloudTextureProviderRequestBus::EventResult(image, m_configuration.m_lowFreqTextureEntity, &CloudTextureProviderRequestBus::Handler::GetCloudTextureImage);
                        m_configuration.m_shaderConstantData.m_lowFrequencyNoiseTexture = image;
                    }
                }

                if (m_prevConfiguration.m_highFreqTextureEntity != m_configuration.m_highFreqTextureEntity)
                {
                    doUpdate = true;
                    if (m_prevConfiguration.m_highFreqTextureEntity.IsValid())
                    {
                        CloudTextureProviderNotificationBus::MultiHandler::BusDisconnect(m_prevConfiguration.m_highFreqTextureEntity);
                        m_configuration.m_shaderConstantData.m_highFrequencyNoiseTexture.reset();
                    }

                    if (m_configuration.m_highFreqTextureEntity.IsValid())
                    {
                        CloudTextureProviderNotificationBus::MultiHandler::BusConnect(m_configuration.m_highFreqTextureEntity);

                        AZ::Data::Instance<AZ::RPI::Image> image;
                        CloudTextureProviderRequestBus::EventResult(image, m_configuration.m_highFreqTextureEntity, &CloudTextureProviderRequestBus::Handler::GetCloudTextureImage);
                        m_configuration.m_shaderConstantData.m_highFrequencyNoiseTexture = image;

                    }
                }

                if (m_prevConfiguration.m_sunEntity != m_configuration.m_sunEntity)
                {
                    doUpdate = true;
                    if (m_prevConfiguration.m_sunEntity.IsValid())
                    {
                        AZ::TransformNotificationBus::Handler::BusDisconnect(m_prevConfiguration.m_sunEntity);
                        m_directionalLightConfigChangedEventHandler.Disconnect();
                    }

                    if (m_configuration.m_sunEntity.IsValid())
                    {
                        FetchAllSunLightData();
                        AZ::TransformNotificationBus::Handler::BusConnect(m_configuration.m_sunEntity);
                        AZ::Render::DirectionalLightRequestBus::Event(m_configuration.m_sunEntity, &AZ::Render::DirectionalLightRequests::BindConfigurationChangedEventHandler, m_directionalLightConfigChangedEventHandler);
                    }
                }
                
                if (m_prevConfiguration.m_shaderConstantData != m_configuration.m_shaderConstantData)
                {
                    doUpdate = true;
                }

                if (doUpdate)
                {
                    SubmitShaderConstantData();
                }

            }

            m_prevConfiguration = m_configuration;
        }

        void CloudscapeComponentController::EnableFeatureProcessor()
        {
            if (m_scene)
            {
                m_cloudscapeFeatureProcessor = m_scene->EnableFeatureProcessor<CloudscapeFeatureProcessor>();
            }

            AZ_Assert(!!m_cloudscapeFeatureProcessor, "Failed to enable CloudscapeFeatureProcessor");
            //m_cloudscapeFeatureProcessor->UpdateColor(m_configuration.m_color);
        }

        void CloudscapeComponentController::SubmitShaderConstantData()
        {
            if (m_isBatchingShaderConstantChanges)
            {
                return;
            }
            if (m_cloudscapeFeatureProcessor)
            {
                const uint32_t imageMipLevels = m_configuration.m_shaderConstantData.m_lowFrequencyNoiseTexture
                    ? m_configuration.m_shaderConstantData.m_lowFrequencyNoiseTexture->GetDescriptor().m_mipLevels
                    : 1;
                uint32_t maxMipLevels = AZStd::min(imageMipLevels, m_configuration.m_shaderConstantData.m_maxMipLevels);
                m_configuration.m_shaderConstantData.m_clampedMipLevels = AZStd::max(1u, maxMipLevels);
                m_cloudscapeFeatureProcessor->UpdateShaderConstantData(m_configuration.m_shaderConstantData);
            }
        }

        //////////////////////////////////////////////////////////////////
        //! CloudTextureProviderNotificationBus overrides START...
        void CloudscapeComponentController::OnCloudTextureImageReady(AZ::Data::Instance<AZ::RPI::Image> image)
        {
            auto entityId = *CloudTextureProviderNotificationBus::GetCurrentBusId();
            if (entityId == m_configuration.m_lowFreqTextureEntity)
            {
                m_configuration.m_shaderConstantData.m_lowFrequencyNoiseTexture = image;
            }
            else if (entityId == m_configuration.m_highFreqTextureEntity)
            {
                m_configuration.m_shaderConstantData.m_highFrequencyNoiseTexture = image;
            }

            SubmitShaderConstantData();

        }

        //! CloudTextureProviderNotificationBus overrides END ...
        //////////////////////////////////////////////////////////////////

        void CloudscapeComponentController::OnAssetStateChanged(AZ::Data::Asset<AZ::Data::AssetData> asset, [[maybe_unused]] bool isReload)
        {
            if (m_configuration.m_weatherMap.GetId() == asset.GetId())
            {
                AZ_Info(LogName, "The weather map texture asset is ready: %s", asset.GetHint().c_str());
                m_configuration.m_weatherMap = asset;
                auto updateTexture = [this]()
                {
                    if (m_cloudscapeFeatureProcessor)
                    {
                        m_configuration.m_shaderConstantData.m_weatherMap = AZ::RPI::StreamingImage::FindOrCreate(m_configuration.m_weatherMap);
                        m_cloudscapeFeatureProcessor->UpdateShaderConstantData(m_configuration.m_shaderConstantData);
                    }
                };
                AZ::TickBus::QueueFunction(AZStd::move(updateTexture));
            } 
            //else if (m_shaderAsset.GetId() == asset.GetId())
            //{
            //    AZ_Info(LogName, "The shader asset is ready: %s", asset.GetHint().c_str());
            //    m_shaderAsset = asset;
            //    auto enableFeatureProcessor = [this]()
            //    {
            //        EnableFeatureProcessor();
            //    };
            //    AZ::TickBus::QueueFunction(AZStd::move(enableFeatureProcessor));
            //}
        }

        ////////////////////////////////////////////////////////////////////////
        //! Data::AssetBus START
        void CloudscapeComponentController::OnAssetReady(AZ::Data::Asset<AZ::Data::AssetData> asset)
        {
            const bool isReload = false;
            OnAssetStateChanged(asset, isReload);
        }

        void CloudscapeComponentController::OnAssetReloaded(AZ::Data::Asset<AZ::Data::AssetData> asset)
        {
            const bool isReload = true;
            OnAssetStateChanged(asset, isReload);
        }
        //! Data::AssetBus END
        ////////////////////////////////////////////////////////////////////////


        ////////////////////////////////////////////////////////////////////
        //! AZ::TransformNotificationBus::Handler
        void CloudscapeComponentController::OnTransformChanged(const AZ::Transform& /*local*/, const AZ::Transform& worldTM)
        {
            m_configuration.m_shaderConstantData.m_directionTowardsTheSun = -worldTM.GetBasisY();
            if (m_cloudscapeFeatureProcessor)
            {
                m_cloudscapeFeatureProcessor->UpdateShaderConstantData(m_configuration.m_shaderConstantData);
            }
        }
        ////////////////////////////////////////////////////////////////////

        void CloudscapeComponentController::FetchAllSunLightData()
        {
            AZ::Transform worldTM = AZ::Transform::CreateIdentity();
            AZ::TransformBus::EventResult(worldTM, m_configuration.m_sunEntity, &AZ::TransformBus::Events::GetWorldTM);
            m_configuration.m_shaderConstantData.m_directionTowardsTheSun = -worldTM.GetBasisY();

            AZ::Color sunColor = AZ::Color::CreateOne();
            AZ::Render::DirectionalLightRequestBus::EventResult(sunColor, m_configuration.m_sunEntity, &AZ::Render::DirectionalLightRequests::GetColor);
            m_configuration.m_shaderConstantData.m_sunColor = sunColor.GetAsVector3();
        }

        void CloudscapeComponentController::NotifySunLightDataChanged()
        {
            AZ::Color sunColor = AZ::Color::CreateOne();
            AZ::Render::DirectionalLightRequestBus::EventResult(sunColor, m_configuration.m_sunEntity, &AZ::Render::DirectionalLightRequests::GetColor);
            const AZ::Vector3 newColor = sunColor.GetAsVector3();
            if (newColor.IsClose(m_configuration.m_shaderConstantData.m_sunColor))
            {
                return;
            }

            m_configuration.m_shaderConstantData.m_sunColor = newColor;
            if (m_cloudscapeFeatureProcessor)
            {
                m_cloudscapeFeatureProcessor->UpdateShaderConstantData(m_configuration.m_shaderConstantData);
            }
        }

        //! RPI::ViewportContextIdNotificationBus
        void CloudscapeComponentController::OnViewportSizeChanged([[maybe_unused]] AzFramework::WindowSize size)
        {
            // The feature processor owns two image attachments that are used
            // by the passes it owns. Their sizes must be of the size of the viewport.
            // Once attachments are defined for a pass they can not be modified (making those changes
            // is not supported by the APIs at the moment). So we have to reschedule the destruction of this feature processor
            // and recreate this feature processor which in turns will recreate and add the passes
            // to the render pipeline.
            m_scene->DisableFeatureProcessor<CloudscapeFeatureProcessor>();
            m_cloudscapeFeatureProcessor = nullptr;

            // Recreate the feature processor on the next tick.
            auto recreateFeatureProcessorFunc = [this]()
            {
                EnableFeatureProcessor();
                SubmitShaderConstantData();
            };
            AZ::TickBus::QueueFunction(AZStd::move(recreateFeatureProcessorFunc));

        }

        /////////////////////////////////////////////////////////
        // VolumetricCloudsRequestBus::Handler overrides START
        void CloudscapeComponentController::BeginCallBatch()
        {
            m_isBatchingShaderConstantChanges = true;
        }

        bool CloudscapeComponentController::IsCallBatching()
        {
            return m_isBatchingShaderConstantChanges;
        }

        float CloudscapeComponentController::GetUVWScale()
        {
            return m_configuration.m_shaderConstantData.m_uvwScale;
        }

        void CloudscapeComponentController::SetUVWScale(float uvwScale)
        {
            m_configuration.m_shaderConstantData.m_uvwScale = uvwScale;
            SubmitShaderConstantData();
        }

        uint32_t CloudscapeComponentController::GetMaxMipLevels()
        {
            return m_configuration.m_shaderConstantData.m_maxMipLevels;
        }

        void CloudscapeComponentController::SetMaxMipLevels(uint32_t maxMipLevels)
        {
            m_configuration.m_shaderConstantData.m_maxMipLevels = maxMipLevels;
            SubmitShaderConstantData();
        }

        AZStd::tuple<uint8_t, uint8_t> CloudscapeComponentController::GetRayMarchingSteps()
        {
            const auto minSteps = AZStd::min(m_configuration.m_shaderConstantData.m_minRayMarchingSteps, m_configuration.m_shaderConstantData.m_maxRayMarchingSteps);
            const auto maxSteps = AZStd::max(m_configuration.m_shaderConstantData.m_minRayMarchingSteps, m_configuration.m_shaderConstantData.m_maxRayMarchingSteps);
            return AZStd::make_tuple(minSteps, maxSteps);
        }

        void CloudscapeComponentController::SetRayMarchingSteps(uint8_t min, uint8_t max)
        {
            m_configuration.m_shaderConstantData.m_minRayMarchingSteps = min;
            m_configuration.m_shaderConstantData.m_maxRayMarchingSteps = max;
            SubmitShaderConstantData();
        }

        float CloudscapeComponentController::GetPlanetRadiusKm()
        {
            return m_configuration.m_shaderConstantData.m_planetRadiusKm;
        }

        void CloudscapeComponentController::SetPlanetRadiusKm(float radiusKm)
        {
            m_configuration.m_shaderConstantData.m_planetRadiusKm = radiusKm;
            SubmitShaderConstantData();
        }

        float CloudscapeComponentController::GetDistanceToCloudSlabKm()
        {
            return m_configuration.m_shaderConstantData.m_cloudSlabDistanceAboveSeaLevelKm;
        }

        void CloudscapeComponentController::SetDistanceToCloudSlabKm(float distanceKm)
        {
            m_configuration.m_shaderConstantData.m_cloudSlabDistanceAboveSeaLevelKm = distanceKm;
            SubmitShaderConstantData();
        }

        float CloudscapeComponentController::GetCloudSlabThicknessKm()
        {
            return m_configuration.m_shaderConstantData.m_cloudSlabThicknessKm;
        }

        void CloudscapeComponentController::SetCloudSlabThicknessKm(float thicknessKm)
        {
            m_configuration.m_shaderConstantData.m_cloudSlabThicknessKm = thicknessKm;
            SubmitShaderConstantData();
        }
        
        AZ::Color CloudscapeComponentController::GetSunLightColorAndIntensity()
        {
            return AZ::Color::CreateFromVector3AndFloat(m_configuration.m_shaderConstantData.m_sunColor,
                m_configuration.m_shaderConstantData.m_sunLightIntensity);
        }
        
        void CloudscapeComponentController::SetSunLightColorAndIntensity(const AZ::Color& rgbColorAlphaIntensity)
        {
            m_configuration.m_shaderConstantData.m_sunColor = rgbColorAlphaIntensity.GetAsVector3();
            m_configuration.m_shaderConstantData.m_sunLightIntensity = rgbColorAlphaIntensity.GetA();
            SubmitShaderConstantData();
        }
        
        AZ::Color CloudscapeComponentController::GetAmbientLightColorAndIntensity()
        {
            AZ::Color color = m_configuration.m_shaderConstantData.m_ambientLightColor;
            color.SetA(m_configuration.m_shaderConstantData.m_ambientLightIntensity);
            return color;
        }
        
        void CloudscapeComponentController::SetAmbientLightColorAndIntensity(const AZ::Color& rgbColorAlphaIntensity)
        {
            m_configuration.m_shaderConstantData.m_ambientLightColor = rgbColorAlphaIntensity;
            m_configuration.m_shaderConstantData.m_ambientLightIntensity = rgbColorAlphaIntensity.GetA();
            SubmitShaderConstantData();
        }

        float CloudscapeComponentController::GetWeatherMapSizeKm()
        {
            return m_configuration.m_shaderConstantData.m_weatherMapSizeKm;
        }

        void CloudscapeComponentController::SetWeatherMapSizeKm(float mapSizeKm)
        {
            m_configuration.m_shaderConstantData.m_weatherMapSizeKm = mapSizeKm;
            SubmitShaderConstantData();

        }

        float CloudscapeComponentController::GetCloudCoverage()
        {
            return m_configuration.m_shaderConstantData.m_globalCloudCoverage;
        }

        void CloudscapeComponentController::SetCloudCoverage(float coverage)
        {
            m_configuration.m_shaderConstantData.m_globalCloudCoverage = coverage;
            SubmitShaderConstantData();
        }

        float CloudscapeComponentController::GetCloudDensity()
        {
            return m_configuration.m_shaderConstantData.m_globalCloudDensity;
        }

        void CloudscapeComponentController::SetCloudDensity(float density)
        {
            m_configuration.m_shaderConstantData.m_globalCloudDensity = density;
            SubmitShaderConstantData();
        }

        AZ::Vector3 CloudscapeComponentController::GetWindVelocity()
        {
            AZ::Vector3 windVelocity = m_configuration.m_shaderConstantData.m_windDirection.GetNormalizedSafe();
            windVelocity *= m_configuration.m_shaderConstantData.m_windSpeedKmPerSec;
            return windVelocity;
        }

        void CloudscapeComponentController::SetWindVelocity(const AZ::Vector3& velocity)
        {
            const float speed = velocity.GetLength();
            m_configuration.m_shaderConstantData.m_windSpeedKmPerSec = speed;
            m_configuration.m_shaderConstantData.m_windDirection = AZ::IsClose(speed, 0.0f) ? AZ::Vector3::CreateAxisX() : velocity / speed;
            SubmitShaderConstantData();
        }

        float CloudscapeComponentController::GetCloudTopShiftKm()
        {
            return m_configuration.m_shaderConstantData.m_cloudTopOffsetKm;
        }

        void CloudscapeComponentController::SetCloudTopShiftKm(float topShiftKm)
        {
            m_configuration.m_shaderConstantData.m_cloudTopOffsetKm = topShiftKm;
            SubmitShaderConstantData();
        }

        const CloudMaterialProperties& CloudscapeComponentController::GetCloudMaterialProperties()
        {
            return m_configuration.m_shaderConstantData.m_cloudMaterialProperties;
        }

        void CloudscapeComponentController::SetCloudMaterialProperties(const CloudMaterialProperties& cmp)
        {
            m_configuration.m_shaderConstantData.m_cloudMaterialProperties = cmp;
            SubmitShaderConstantData();
        }

        void CloudscapeComponentController::EndCallBatch()
        {
            if (!m_isBatchingShaderConstantChanges)
            {
                AZ_Warning(LogName, false, "BeginCallBatch() must be called before calling EndCallBatch()");
                return;
            }
            m_isBatchingShaderConstantChanges = false;
            SubmitShaderConstantData();
        }
        // VolumetricCloudsRequestBus::Handler overrides END
        /////////////////////////////////////////////////////////

} // namespace VolumetricClouds
