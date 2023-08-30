/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <AzCore/RTTI/BehaviorContext.h>

#include <AzToolsFramework/UI/PropertyEditor/PropertyFilePathCtrl.h>

#include "EditorCloudTextureAssetComponent.h"

namespace VolumetricClouds
{
    void EditorCloudTextureAssetComponent::Reflect(AZ::ReflectContext* context)
    {
        BaseClass::Reflect(context);

        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<EditorCloudTextureAssetComponent, BaseClass>()
                ->Version(1)
                ;

            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<EditorCloudTextureAssetComponent>(
                    "Cloud Texture Asset", "Creates a volume texture for cloud rendering from an streaming image asset.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::Category, "Graphics/Environment")
                        ->Attribute(AZ::Edit::Attributes::Icon, "Icons/Components/Component_Placeholder.svg")
                        ->Attribute(AZ::Edit::Attributes::ViewportIcon, "Icons/Components/Viewport/Component_Placeholder.svg")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("Game", 0x232b318c))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ->Attribute(AZ::Edit::Attributes::HelpPageURL, "")
                    ;
            }
        }

        if (auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context))
        {
            behaviorContext->ConstantProperty("EditorCloudTextureAssetComponentTypeId", BehaviorConstant(AZ::Uuid(EditorCloudTextureAssetComponentTypeId)))
                ->Attribute(AZ::Script::Attributes::Module, "render")
                ->Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Automation)
                ;
        }
    }

    EditorCloudTextureAssetComponent::EditorCloudTextureAssetComponent(const CloudTextureAssetComponentConfig& config)
        : BaseClass(config)
    {
    }

    void EditorCloudTextureAssetComponent::Activate()
    {
        BaseClass::Activate();
    }

    void EditorCloudTextureAssetComponent::Deactivate()
    {
        BaseClass::Deactivate();
    }


    AZ::u32 EditorCloudTextureAssetComponent::OnConfigurationChanged()
    {
        m_controller.OnConfigurationChanged();
        return AZ::Edit::PropertyRefreshLevels::ValuesOnly;
    }

} // namespace VolumetricClouds
