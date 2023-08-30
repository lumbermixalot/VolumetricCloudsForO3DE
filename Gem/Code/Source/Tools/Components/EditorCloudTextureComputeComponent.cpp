/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <AzCore/RTTI/BehaviorContext.h>

#include <AzToolsFramework/UI/PropertyEditor/PropertyFilePathCtrl.h>

#include <Tools/Utils/PngCloudTextureWriter.h>
#include <Tools/Utils/DdsCloudTextureWriter.h>
#include <Renderer/Passes/CloudTextureComputePass.h> // To get function that calculates num mips.
#include "EditorCloudTextureComputeComponent.h"

AZ_PUSH_DISABLE_WARNING(4251 4800, "-Wunknown-warning-option") // disable warnings spawned by QT
#include <QApplication.h>
#include <QMessageBox>
#include <QPushButton.h>
AZ_POP_DISABLE_WARNING

namespace VolumetricClouds
{
    void SaveToDiskConfig::Reflect(AZ::ReflectContext* context)
    {
        AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context);
        if (serialize)
        {
            serialize->Class<SaveToDiskConfig, AZ::ComponentConfig>()
                ->Version(1)
                ->Field("OutputImagePath", &SaveToDiskConfig::m_outputImagePath)
                ;

            AZ::EditContext* edit = serialize->GetEditContext();
            if (edit)
            {
                edit->Class<SaveToDiskConfig>("Save To Disk", "")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::ShowChildrenOnly)
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->DataElement(
                        AZ::Edit::UIHandlers::Default, &SaveToDiskConfig::m_outputImagePath, "Output Path",
                        "Output path to save the image(s) to.")
                        ->Attribute(AZ::Edit::Attributes::SourceAssetFilterPattern, SaveToDiskConfig::GetSupportedImagesFilter())
                    ;
            }
        }
    }

    void EditorCloudTextureComputeComponent::Reflect(AZ::ReflectContext* context)
    {
        BaseClass::Reflect(context);
        SaveToDiskConfig::Reflect(context);

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<EditorCloudTextureComputeComponent, BaseClass>()
                ->Version(1)
                ->Field("SaveToDiskConfig", &EditorCloudTextureComputeComponent::m_saveToDiskConfig)
                ;

            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<EditorCloudTextureComputeComponent>(
                    "Cloud Texture Compute", "Helps generate the Perlin Worley noise textures used for rendering cloudscapes.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "Graphics/Environment")
                        ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                        ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Icons/Components/Viewport/Component_Placeholder.svg")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ->Attribute(AZ::Edit::Attributes::HelpPageURL, "")
                    ->UIElement(AZ::Edit::UIHandlers::Button, "ResetValues", "Resets all parameters to default values.")
                        ->Attribute(AZ::Edit::Attributes::NameLabelOverride, "")
                        ->Attribute(AZ::Edit::Attributes::ButtonText, "Reset To Default")
                        ->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorCloudTextureComputeComponent::OnResetNoiseParameters)
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Save To Disk")
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ->DataElement(AZ::Edit::UIHandlers::Default, &EditorCloudTextureComputeComponent::m_saveToDiskConfig, "Configuration", "")
                            ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::Show)
                        ->UIElement(AZ::Edit::UIHandlers::Button, "SaveToDisk", "Save the generated 3D Texture to disk.")
                            ->Attribute(AZ::Edit::Attributes::NameLabelOverride, "")
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Save")
                            ->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorCloudTextureComputeComponent::OnSaveToDisk)
                            ->Attribute(AZ::Edit::Attributes::ReadOnly, &EditorCloudTextureComputeComponent::IsSaveToDiskDisabled)
                    ->EndGroup()
                    ;
            }
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->ConstantProperty("EditorCloudTextureComputeComponentTypeId", BehaviorConstant(AZ::Uuid(EditorCloudTextureComputeComponentTypeId)))
                ->Attribute(AZ::Script::Attributes::Module, "render")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Automation)
                ;
        }
    }

    EditorCloudTextureComputeComponent::EditorCloudTextureComputeComponent(const CloudTextureComputeComponentConfig& config)
        : BaseClass(config)
    {
    }

    void EditorCloudTextureComputeComponent::Activate()
    {
        BaseClass::Activate();

        m_readbackHandler = CloudTexturesComputeFeatureProcessor::ReadbackEvent::Handler(
            [this](AZ::Data::Instance<AZ::RPI::Image> image,
                AZStd::shared_ptr<AZStd::vector<uint8_t>> mipDataBuffer, uint16_t mipSlice, const AZ::RHI::Size& mipSize)
            {
                if (!m_cloudTextureWriter)
                {
                    return;
                }

                if (!m_cloudTextureWriter->SetDataBufferForMipLevel(mipDataBuffer, mipSlice, mipSize))
                {
                    AZ_Error(LogName, false, "%s Failed to add data buffer for level=%hu.", __FUNCTION__, mipSlice);
                    m_cloudTextureWriter.reset();
                    m_progressDialog->reset();
                    return;
                }

                if (m_cloudTextureWriter->GetMipLevelsWithDataCount() == m_cloudTextureWriter->GetMipLevels())
                {
                    // We got all the data. We can start the Tick events and save mip level to disk.
                    m_progressDialog->setValue(1);
                    AZ::SystemTickBus::Handler::BusConnect();
                }
            }
        );
    }


    void EditorCloudTextureComputeComponent::Deactivate()
    {
        BaseClass::Deactivate();
        AZ::SystemTickBus::Handler::BusDisconnect();
    }


    void EditorCloudTextureComputeComponent::OnEntityVisibilityChanged(bool visibility)
    {
        EditorRenderComponentAdapter::OnEntityVisibilityChanged(visibility);
    }


    AZ::u32 EditorCloudTextureComputeComponent::OnConfigurationChanged()
    {
        m_controller.OnConfigurationChanged();
        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }


    AZ::u32 EditorCloudTextureComputeComponent::OnResetNoiseParameters()
    {
        m_controller.m_configuration = {};
        return OnConfigurationChanged();
    }


    bool EditorCloudTextureComputeComponent::IsSaveToDiskDisabled()
    {
        return m_cloudTextureWriter != nullptr;
    }


    AZ::u32 EditorCloudTextureComputeComponent::OnSaveToDisk()
    {
        if (!m_controller.GetCloudTextureImage())
        {
            QString msg("No cloud texture image has been generated so far.");
            QMessageBox::information(
                QApplication::activeWindow(),
                "Error",
                msg,
                QMessageBox::Ok);
            return AZ::Edit::PropertyRefreshLevels::None;
        }

        AZ::IO::Path fullPathIO = AzToolsFramework::GetAbsolutePathFromRelativePath(m_saveToDiskConfig.m_outputImagePath);
        if (m_saveToDiskConfig.m_outputImagePath.empty() || fullPathIO.empty())
        {
            QString msg("Invalid output path.");
            QMessageBox::information(
                QApplication::activeWindow(),
                "Error",
                msg,
                QMessageBox::Ok);
            return AZ::Edit::PropertyRefreshLevels::None;
        }
        fullPathIO = fullPathIO.LexicallyNormal();

        AZStd::string parentPath = fullPathIO.ParentPath().String();
        // Make sure the output directory exists:
        if (!AZ::IO::SystemFile::Exists(parentPath.c_str()))
        {
            QString msg = QString::asprintf("Output directory=<%s> doesn't exist!", parentPath.c_str());
            QMessageBox::information(
                QApplication::activeWindow(),
                "Error",
                msg,
                QMessageBox::Ok);
            return AZ::Edit::PropertyRefreshLevels::None;
        }

        AZStd::string prefix = fullPathIO.Stem().String();

        const uint16_t mipLevels = CloudTextureComputePass::CalculateMipCount(m_controller.m_configuration.m_computeData.m_pixelSize);
        const AZ::RHI::Format pixFormat = m_controller.GetCloudTextureImage()->GetDescriptor().m_format;
        m_cloudTextureWriter.reset();
        m_cloudTextureWriter = AZStd::make_unique<DdsCloudTextureWriter>(
            mipLevels, pixFormat, parentPath, prefix);

        m_controller.ForceCloudTextureRegeneration(&m_readbackHandler);
        ShowProgressDialog();

        return AZ::Edit::PropertyRefreshLevels::AttributesAndValues;
    }


    void EditorCloudTextureComputeComponent::ShowProgressDialog()
    {
        if (m_progressDialog)
        {
            m_progressDialog.reset();
        }

        m_progressDialog = AZStd::make_unique<QProgressDialog>();
        m_progressDialog->setWindowModality(Qt::WindowModal);
        m_progressDialog->setWindowFlags(m_progressDialog->windowFlags() & ~Qt::WindowCloseButtonHint);
        m_progressDialog->setLabelText("Saving volumetric texture...");
        m_progressDialog->setWindowModality(Qt::WindowModal);
        m_progressDialog->setMaximumSize(QSize(256, 96));
        m_progressDialog->setMinimum(0);
        m_progressDialog->setMinimumDuration(0);
        m_progressDialog->setAutoClose(true);
        uint32_t numImages = m_controller.m_configuration.m_computeData.m_pixelSize;
        m_progressDialog->setMaximum(numImages);


        class MyCancelButton final : public QPushButton {
        public:
            MyCancelButton(EditorCloudTextureComputeComponent* myComponent) : QPushButton("Cancel"),
                m_component(myComponent) {
                connect(this, &QPushButton::released, this, &MyCancelButton::myClicked);
            }

            void myClicked()
            {
                m_component->OnProgressDialogCanceled();
            }

        private:
            EditorCloudTextureComputeComponent* m_component = nullptr;
        };
        m_progressDialog->setCancelButton(new MyCancelButton(this));

        m_progressDialog->show();
    }

    void EditorCloudTextureComputeComponent::OnProgressDialogCanceled()
    {
        m_cloudTextureWriter.reset();

        // Force UI refresh of the component so the "Save To Disk" button becomes
        // enabled again.
        AzToolsFramework::ToolsApplicationNotificationBus::Broadcast(
            &AzToolsFramework::ToolsApplicationEvents::InvalidatePropertyDisplay, AzToolsFramework::Refresh_AttributesAndValues);

    }


    // AZ::SystemTickBus::Handler overrides ...
    void EditorCloudTextureComputeComponent::OnSystemTick()
    {
        if (!m_cloudTextureWriter)
        {
            AZ::SystemTickBus::Handler::BusDisconnect();
            return;
        }

        uint16_t nextMipLevelToSave = m_cloudTextureWriter->GetSavedMipLevelsCount();
        if (nextMipLevelToSave < m_cloudTextureWriter->GetMipLevels())
        {
            if (!m_cloudTextureWriter->SaveMipLevel(nextMipLevelToSave))
            {
                m_cloudTextureWriter.reset();
                m_progressDialog->reset();
                AZ::SystemTickBus::Handler::BusDisconnect();

                QString msg = QString::asprintf("Saving cloud texture to disk failed at mip level=%hu!", nextMipLevelToSave);
                QMessageBox::information(
                    QApplication::activeWindow(),
                    "Error",
                    msg,
                    QMessageBox::Ok);

                // Force UI refresh of the component so the "Save To Disk" button becomes
                // enabled again.
                AzToolsFramework::ToolsApplicationNotificationBus::Broadcast(
                    &AzToolsFramework::ToolsApplicationEvents::InvalidatePropertyDisplay, AzToolsFramework::Refresh_AttributesAndValues);
                return;
            }

            m_progressDialog->setValue(m_progressDialog->value() + 1);

            if (m_cloudTextureWriter->GetSavedMipLevelsCount() == m_cloudTextureWriter->GetMipLevels())
            {
                m_progressDialog->setValue(m_progressDialog->value() + 1);

                const auto& fileList = m_cloudTextureWriter->GetListOfSavedFiles();

                QString msg = QString::asprintf("Successfully saved all mip levels=%hu for cloud texture to disk.\n%zu files were created. First file was:\n%s",
                    m_cloudTextureWriter->GetMipLevels(), fileList.size(), fileList[0].c_str());
                QMessageBox::information(
                    QApplication::activeWindow(),
                    "Error",
                    msg,
                    QMessageBox::Ok);

                m_cloudTextureWriter.reset();
                m_progressDialog->reset();
                AZ::SystemTickBus::Handler::BusDisconnect();
                // Force UI refresh of the component so the "Save To Disk" button becomes
                // enabled again.
                AzToolsFramework::ToolsApplicationNotificationBus::Broadcast(
                    &AzToolsFramework::ToolsApplicationEvents::InvalidatePropertyDisplay, AzToolsFramework::Refresh_AttributesAndValues);
            }

            
        }
    }

} // namespace VolumetricClouds
