/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <AzCore/RTTI/BehaviorContext.h>

#include <VolumetricClouds/VolumetricCloudsBus.h>
#include "EditorCloudscapeComponent.h"

AZ_PUSH_DISABLE_WARNING(4251 4800, "-Wunknown-warning-option") // disable warnings spawned by QT
#include <QApplication.h>
#include <QMessageBox>
AZ_POP_DISABLE_WARNING

namespace VolumetricClouds
{
        void EditorCloudscapeComponent::Reflect(AZ::ReflectContext* context)
        {
            BaseClass::Reflect(context);

            if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
            {
                serializeContext->Class<EditorCloudscapeComponent, BaseClass>()
                    ->Version(1)
                    ;

                if (auto editContext = serializeContext->GetEditContext())
                {
                    editContext->Class<EditorCloudscapeComponent>(
                        "Volumetric Cloudscape", "Adds a volumetric cloudscape.")
                        ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                            ->Attribute(AZ::Edit::Attributes::Category, "Graphics/Environment")
                            ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                            ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Icons/Components/Viewport/Component_Placeholder.svg")
                            ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game"))
                            ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                            ->Attribute(AZ::Edit::Attributes::HelpPageURL, "")
                        ->UIElement(AZ::Edit::UIHandlers::Button, "ResetValues", "Resets all parameters to default values.")
                            ->Attribute(AZ::Edit::Attributes::NameLabelOverride, "")
                            ->Attribute(AZ::Edit::Attributes::ButtonText, "Reset To Default")
                            ->Attribute(AZ::Edit::Attributes::ChangeNotify, &EditorCloudscapeComponent::OnResetConfigData)
                        ;
                }
            }

        }

        EditorCloudscapeComponent::EditorCloudscapeComponent(const CloudscapeComponentConfig& config)
            : BaseClass(config)
        {
        }

        void EditorCloudscapeComponent::Activate()
        {
            if (VolumetricCloudsRequestBus::HasHandlers())
            {
                m_isActive = false;
                // We only allow one of these components per level.
                QString msg("Only one Volumetric Cloudscape component is allowed per level.");
                QMessageBox::information(
                    QApplication::activeWindow(),
                    "Error",
                    msg,
                    QMessageBox::Ok);
            }
            else
            {
                m_isActive = true;
            }
            BaseClass::Activate();
        }

        void EditorCloudscapeComponent::Deactivate()
        {
            BaseClass::Deactivate();
            m_isActive = false;
        }

        AZ::u32 EditorCloudscapeComponent::OnConfigurationChanged()
        {
            AZ_Error("EditorCloudscapeComponent", m_isActive, "As mentioned already this Volumetric Cloudscape component is redundant and should be removed! myEntity Id=%s, name=%s\n",
                    GetEntityId().ToString().c_str(), GetEntity()->GetName().c_str());
            m_controller.OnConfigurationChanged();
            return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
        }

        AZ::u32 EditorCloudscapeComponent::OnResetConfigData()
        {
            AZ_Error("EditorCloudscapeComponent", m_isActive, "As mentioned already this Volumetric Cloudscape component is redundant and should be removed! myEntity Id=%s, name=%s\n",
                    GetEntityId().ToString().c_str(), GetEntity()->GetName().c_str());
            m_controller.m_configuration.m_shaderConstantData.m_cloudMaterialProperties = {};
            return OnConfigurationChanged();
        }

} // namespace VolumetricClouds
