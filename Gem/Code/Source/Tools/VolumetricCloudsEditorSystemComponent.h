/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <AzToolsFramework/API/ToolsApplicationAPI.h>

#include <Clients/VolumetricCloudsSystemComponent.h>

namespace VolumetricClouds
{
    /// System component for VolumetricClouds editor
    class VolumetricCloudsEditorSystemComponent
        : public VolumetricCloudsSystemComponent
        , protected AzToolsFramework::EditorEvents::Bus::Handler
    {
        using BaseSystemComponent = VolumetricCloudsSystemComponent;
    public:
        AZ_COMPONENT_DECL(VolumetricCloudsEditorSystemComponent);

        static void Reflect(AZ::ReflectContext* context);

        VolumetricCloudsEditorSystemComponent();
        ~VolumetricCloudsEditorSystemComponent();

    private:
        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        // AZ::Component
        void Activate() override;
        void Deactivate() override;
    };
} // namespace VolumetricClouds
