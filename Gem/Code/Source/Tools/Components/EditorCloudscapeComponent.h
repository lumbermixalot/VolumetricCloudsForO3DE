/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <Atom/Feature/Utils/EditorRenderComponentAdapter.h>
#include <Clients/Components/CloudscapeComponent.h>

namespace VolumetricClouds
{
    //! In-editor component for displaying and editing cloudscapes.
    class EditorCloudscapeComponent final
        : public AZ::Render::EditorRenderComponentAdapter<CloudscapeComponentController, CloudscapeComponent, CloudscapeComponentConfig>
    {
    public:    
        using BaseClass = EditorRenderComponentAdapter<CloudscapeComponentController, CloudscapeComponent, CloudscapeComponentConfig>;
        AZ_EDITOR_COMPONENT(EditorCloudscapeComponent, EditorCloudscapeComponentTypeId, BaseClass);
    
        static void Reflect(AZ::ReflectContext* context);
    
        EditorCloudscapeComponent() = default;
        EditorCloudscapeComponent(const CloudscapeComponentConfig& config);
    
    private:
        // BaseClass overrides ...
        void Activate() override;
        void Deactivate() override;

        AZ::u32 OnConfigurationChanged() override;
        AZ::u32 OnResetConfigData();

        // Only becomes true if there's no previous
        // volumetric cloudscape component in the level.
        bool m_isActive = false;

    };
} // namespace VolumetricClouds
