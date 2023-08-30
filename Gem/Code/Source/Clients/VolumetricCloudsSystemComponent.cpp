
/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <AzCore/Serialization/SerializeContext.h>

#include <Atom/RPI.Public/Scene.h>

#include <VolumetricClouds/VolumetricCloudsTypeIds.h>
#include <Renderer/Passes/CloudTextureComputePass.h>
#include <Renderer/Passes/CloudscapeComputePass.h>
#include <Renderer/Passes/CloudscapeRenderPass.h>
#include <Renderer/CloudTexturesComputeFeatureProcessor.h>
#include <Renderer/CloudTexturesDebugViewerFeatureProcessor.h>
#include <Renderer/CloudscapeFeatureProcessor.h>

#include "VolumetricCloudsSystemComponent.h"


namespace VolumetricClouds
{
    AZ_COMPONENT_IMPL(VolumetricCloudsSystemComponent, "VolumetricCloudsSystemComponent",
        VolumetricCloudsSystemComponentTypeId);

    void VolumetricCloudsSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        CloudTexturesComputeFeatureProcessor::Reflect(context);
        CloudTexturesDebugViewerFeatureProcessor::Reflect(context);
        CloudscapeFeatureProcessor::Reflect(context);

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<VolumetricCloudsSystemComponent, AZ::Component>()
                ->Version(0)
                ;
        }
    }

    void VolumetricCloudsSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC_CE("VolumetricCloudsService"));
    }

    void VolumetricCloudsSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC_CE("VolumetricCloudsService"));
    }

    void VolumetricCloudsSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void VolumetricCloudsSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    VolumetricCloudsSystemComponent::VolumetricCloudsSystemComponent()
    {
    }

    VolumetricCloudsSystemComponent::~VolumetricCloudsSystemComponent()
    {
    }

    void VolumetricCloudsSystemComponent::Init()
    {
    }

    void VolumetricCloudsSystemComponent::Activate()
    {
        auto* passSystem = AZ::RPI::PassSystemInterface::Get();
        AZ_Assert(passSystem, "Cannot get the pass system.");

        // Register volumetric clouds related custom passes
        passSystem->AddPassCreator(AZ::Name("CloudTextureComputePass"), &CloudTextureComputePass::Create);
        passSystem->AddPassCreator(AZ::Name("CloudscapeComputePass"), &CloudscapeComputePass::Create);
        passSystem->AddPassCreator(AZ::Name("CloudscapeRenderPass"), &CloudscapeRenderPass::Create);

        // Setup handler for load pass templates mappings
        m_loadTemplatesHandler = AZ::RPI::PassSystemInterface::OnReadyLoadTemplatesEvent::Handler([this]() { this->LoadPassTemplateMappings(); });
        passSystem->ConnectEvent(m_loadTemplatesHandler);
    }

    void VolumetricCloudsSystemComponent::LoadPassTemplateMappings()
    {
        auto* passSystem = AZ::RPI::PassSystemInterface::Get();
        AZ_Assert(passSystem, "Cannot get the pass system.");

        const char* passTemplatesFile = "Passes/VolumetricCloudsPassTemplates.azasset";
        passSystem->LoadPassTemplateMappings(passTemplatesFile);
    }

    void VolumetricCloudsSystemComponent::Deactivate()
    {
        m_loadTemplatesHandler.Disconnect();
    }


} // namespace VolumetricClouds
