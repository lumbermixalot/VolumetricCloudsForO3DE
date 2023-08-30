/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

AZ_PUSH_DISABLE_WARNING(4251 4800, "-Wunknown-warning-option") // disable warnings spawned by QT
#include <QProgressDialog>
AZ_POP_DISABLE_WARNING

#include <AzCore/Component/TickBus.h>

#include <Atom/Feature/Utils/EditorRenderComponentAdapter.h>

#include <Clients/Components/CloudTextureComputeComponent.h>
#include <Tools/Utils/ICloudTextureWriter.h>
#include <Renderer/CloudTexturesComputeFeatureProcessor.h>

namespace VolumetricClouds
{
    class SaveToDiskConfig : public AZ::ComponentConfig
    {
    public:
        AZ_CLASS_ALLOCATOR(SaveToDiskConfig, AZ::SystemAllocator);
        AZ_RTTI(SaveToDiskConfig, "{89E8DDC6-E887-4730-B780-AD01C11566AB}", AZ::ComponentConfig);
        static void Reflect(AZ::ReflectContext* context);

        AZ::IO::Path m_outputImagePath;

        static AZStd::string GetSupportedImagesFilter()
        {
            return "Volume Texture (*.dds)";
        }
    };

    //! In-editor component for generating a 3D Noise Texture with the help of CloudTextureComputePass.
    class EditorCloudTextureComputeComponent final
        : public AZ::Render::EditorRenderComponentAdapter<CloudTextureComputeComponentController, CloudTextureComputeComponent, CloudTextureComputeComponentConfig>
        , private AZ::SystemTickBus::Handler
    {
    public:    
        using BaseClass = EditorRenderComponentAdapter<CloudTextureComputeComponentController, CloudTextureComputeComponent, CloudTextureComputeComponentConfig>;
        AZ_EDITOR_COMPONENT(EditorCloudTextureComputeComponent, EditorCloudTextureComputeComponentTypeId, BaseClass);
    
        static void Reflect(AZ::ReflectContext* context);
    
        EditorCloudTextureComputeComponent() = default;
        EditorCloudTextureComputeComponent(const CloudTextureComputeComponentConfig& config);

        void OnProgressDialogCanceled();
    
    private:
        static constexpr char LogName[] = "EditorCloudTextureComputeComponent";

        // BaseClass overrides ...
        void Activate() override;
        void Deactivate() override;

        // AZ::SystemTickBus::Handler overrides ...
        void OnSystemTick() override;
            
        AZ::u32 OnConfigurationChanged() override;
        
        // Returns one of the values from namespace PropertyRefreshLevels
        AZ::u32 OnResetNoiseParameters();


        // Swapns a job that saves the Texture3D to disk.
        // Returns one of the values from namespace PropertyRefreshLevels
        AZ::u32 OnSaveToDisk();
        // Helper function that makes the SaveToDisk button disabled or enabled.
        bool IsSaveToDiskDisabled();
        void ShowProgressDialog();
    
        // AzToolsFramework::EditorEntityVisibilityNotificationBus::Handler overrides
        void OnEntityVisibilityChanged(bool visibility) override;

        SaveToDiskConfig m_saveToDiskConfig;
        std::unique_ptr<QProgressDialog> m_progressDialog; // Will be visible when we are writing the 3D Texture to disk.
        // If this smart ptr is different than null it means we are saving a cloud texture
        // to disk.
        AZStd::unique_ptr<ICloudTextureWriter> m_cloudTextureWriter;
        CloudTexturesComputeFeatureProcessor::ReadbackEvent::Handler m_readbackHandler;

    };
} // namespace VolumetricClouds
