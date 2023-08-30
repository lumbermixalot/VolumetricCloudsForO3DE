/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#include <AzCore/Asset/AssetSerializer.h>
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>

#include <VolumetricClouds/VolumetricCloudsTypeIds.h>
#include "CloudTexturePresentationData.h"

namespace VolumetricClouds
{
    AZ_CLASS_ALLOCATOR_IMPL(CloudTexturePresentationData, AZ::SystemAllocator);
    AZ_TYPE_INFO_WITH_NAME_IMPL(CloudTexturePresentationData, "VolumetricClouds::CloudTexturePresentationData", CloudTexturePresentationDataTypeId);
    AZ_RTTI_NO_TYPE_INFO_IMPL(CloudTexturePresentationData);

    void CloudTexturePresentationData::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CloudTexturePresentationData>()
                ->Version(1)
                ->Field("AlwaysFaceCamera", &CloudTexturePresentationData::m_alwaysFaceCamera)
                ->Field("TexCoordZ", &CloudTexturePresentationData::m_texCoordZ)
                ->Field("VisibleChannel", &CloudTexturePresentationData::m_visibleChannel)
                ;

            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<CloudTexturePresentationData>(
                    "CloudTexturePresentationData", "Configuration data for the Volumetric Clouds Component.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::Show)
                    ->DataElement(AZ::Edit::UIHandlers::Default, &CloudTexturePresentationData::m_alwaysFaceCamera, "Face Camera", "")
                    ->DataElement(AZ::Edit::UIHandlers::ComboBox, &CloudTexturePresentationData::m_visibleChannel, "Visible Channel", "Selects which noise channel to see (x,y,z,w)")
                        ->EnumAttribute(VisibleChannel::PerlinWorley, "Perlin Worley (x)")
                        ->EnumAttribute(VisibleChannel::WorleyFbm1, "WorleyFbm (Freq * 1.0) (y)")
                        ->EnumAttribute(VisibleChannel::WorleyFbm2, "WorleyFbm (Freq * 2.0) (z)")
                        ->EnumAttribute(VisibleChannel::WorleyFbm4, "WorleyFbm (Freq * 4.0) (w)")
                        ->EnumAttribute(VisibleChannel::AllChannels, "All (x, y, z, w)")
                        ->EnumAttribute(VisibleChannel::None, "Hide")
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudTexturePresentationData::m_texCoordZ, "Texture3D.z", "The Texture3D.z coordinate to visualize.")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.0)
                        ->Attribute(AZ::Edit::Attributes::Max, 1.0)
                    ;
            }
        }

    }

    bool CloudTexturePresentationData::operator==(const CloudTexturePresentationData& rhs) const
    {
        return (m_alwaysFaceCamera == rhs.m_alwaysFaceCamera) &&
               (m_texCoordZ == rhs.m_texCoordZ) &&
               (m_visibleChannel == rhs.m_visibleChannel)
               ;
    }

    bool CloudTexturePresentationData::operator!=(const CloudTexturePresentationData& rhs) const
    {
        return !(*this == rhs);
    }

    bool CloudTexturePresentationData::IsHidden() const
    {
        return m_visibleChannel == VisibleChannel::None;
    }

} // namespace VolumetricClouds
