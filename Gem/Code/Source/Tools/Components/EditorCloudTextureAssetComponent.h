/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <Atom/Feature/Utils/EditorRenderComponentAdapter.h>

#include <Clients/Components/CloudTextureAssetComponent.h>

namespace VolumetricClouds
{
    //! In-editor component for loading a 3D Noise Texture asset.
    class EditorCloudTextureAssetComponent final
        : public AZ::Render::EditorRenderComponentAdapter<CloudTextureAssetComponentController, CloudTextureAssetComponent, CloudTextureAssetComponentConfig>
    {
    public:    
        using BaseClass = EditorRenderComponentAdapter<CloudTextureAssetComponentController, CloudTextureAssetComponent, CloudTextureAssetComponentConfig>;
        AZ_EDITOR_COMPONENT(EditorCloudTextureAssetComponent, EditorCloudTextureAssetComponentTypeId, BaseClass);
    
        static void Reflect(AZ::ReflectContext* context);
    
        EditorCloudTextureAssetComponent() = default;
        EditorCloudTextureAssetComponent(const CloudTextureAssetComponentConfig& config);
    
    private:
        static constexpr char LogName[] = "EditorCloudTextureAssetComponent";

        // BaseClass overrides ...
        void Activate() override;
        void Deactivate() override;
            
        AZ::u32 OnConfigurationChanged() override;

    };
} // namespace VolumetricClouds
