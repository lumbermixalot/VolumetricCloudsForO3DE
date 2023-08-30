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
#include "CloudMaterialProperties.h"

namespace VolumetricClouds
{
    AZ_CLASS_ALLOCATOR_IMPL(CloudMaterialProperties, AZ::SystemAllocator);
    AZ_TYPE_INFO_WITH_NAME_IMPL(CloudMaterialProperties, "VolumetricClouds::CloudMaterialProperties", CloudMaterialPropertiesTypeId);
    AZ_RTTI_NO_TYPE_INFO_IMPL(CloudMaterialProperties);

    static AZStd::string CloudMaterialPropertiesToString(const CloudMaterialProperties& cmp)
    {
        return AZStd::string::format("<aCoeff=%.3f, sCoeff=%.3f, G=%.3f, abc=(%.3f, %.3f, %.3f)>"
            , cmp.m_absorptionCoefficient, cmp.m_scatteringCoefficient, cmp.m_henyeyGreensteinG
            , cmp.m_multiScatteringA, cmp.m_multiScatteringB, cmp.m_multiScatteringC);
    }

    void CloudMaterialProperties::Reflect(AZ::ReflectContext* context)
    {
        if (auto serializeContext = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serializeContext->Class<CloudMaterialProperties>()
                ->Version(1)
                ->Field("AbsorptionCoefficent", &CloudMaterialProperties::m_absorptionCoefficient)
                ->Field("ScatteringCoefficient", &CloudMaterialProperties::m_scatteringCoefficient)
                ->Field("HenyeyGreensteinG", &CloudMaterialProperties::m_henyeyGreensteinG)
                ->Field("AttenuationA", &CloudMaterialProperties::m_multiScatteringA)
                ->Field("ContributionB", &CloudMaterialProperties::m_multiScatteringB)
                ->Field("ExcentricityAttenuationC", &CloudMaterialProperties::m_multiScatteringC)
                ;

            if (auto editContext = serializeContext->GetEditContext())
            {
                editContext->Class<CloudMaterialProperties>(
                    "CloudMaterialProperties", "Physical properties of the clouds.")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ->Attribute(AZ::Edit::Attributes::Visibility, AZ::Edit::PropertyVisibility::Show)
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudMaterialProperties::m_absorptionCoefficient, "Absorption Coefficient", "Beer's Law Absorption coefficient. For clouds it is typically 0.01[m-1].")
                        ->Attribute(AZ::Edit::Attributes::Suffix, " m-1")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.0)
                        ->Attribute(AZ::Edit::Attributes::Max, 0.1)
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudMaterialProperties::m_scatteringCoefficient, "Scattering Coefficient", "Beer's Law Scattering coefficient. For clouds it is typically 3x(Absorption Coefficient).")
                        ->Attribute(AZ::Edit::Attributes::Suffix, " m-1")
                        ->Attribute(AZ::Edit::Attributes::Min, 0.0)
                        ->Attribute(AZ::Edit::Attributes::Max, 0.1)
                    ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudMaterialProperties::m_henyeyGreensteinG, "Excentricity", "Excentricity constant, g, in HenyeyGreenstein phase function. Typically 0.2. The closer to 1.0 the more forward scattering.")
                        ->Attribute(AZ::Edit::Attributes::Min, -1.0)
                        ->Attribute(AZ::Edit::Attributes::Max, 1.0)
                    ->ClassElement(AZ::Edit::ClassElements::Group, "Multiple Scattering Constants")
                    ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                        ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudMaterialProperties::m_multiScatteringA, "a", "Octave Attenuation factor that affects the optical depth in Beer's Law evaluation. Default is 0.5")
                            ->Attribute(AZ::Edit::Attributes::Min, 0.0000001)
                            ->Attribute(AZ::Edit::Attributes::Max, 1.0)
                        ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudMaterialProperties::m_multiScatteringB, "b", "Octave Contribution to the scattering coefficient. Default is 0.5")
                            ->Attribute(AZ::Edit::Attributes::Min, 0.0000001)
                            ->Attribute(AZ::Edit::Attributes::Max, 1.0)
                        ->DataElement(AZ::Edit::UIHandlers::Slider, &CloudMaterialProperties::m_multiScatteringC, "c", "Octave Excentricity Attenuation in Henyey-Greenstein Phase Function. Default is 0.5")
                            ->Attribute(AZ::Edit::Attributes::Min, 0.0000001)
                            ->Attribute(AZ::Edit::Attributes::Max, 1.0)
                    ->EndGroup()
                    ;
            }
        }
        auto behaviorContext = azrtti_cast<AZ::BehaviorContext*>(context);
        if (behaviorContext)
        {
            behaviorContext->Class<CloudMaterialProperties>()->
                Attribute(AZ::Script::Attributes::Scope, AZ::Script::Attributes::ScopeFlags::Common)->
                Attribute(AZ::Script::Attributes::Module, ScriptingModuleName)->
                Constructor<>()->
                    Attribute(AZ::Script::Attributes::Storage, AZ::Script::Attributes::StorageType::Value)->
                Property("absorptionCoefficient", &CloudMaterialProperties::GetAbsorptionCoefficient, &CloudMaterialProperties::SetAbsorptionCoefficient)->
                Property("scatteringCoefficient", &CloudMaterialProperties::GetScatteringCoefficient, &CloudMaterialProperties::SetScatteringCoefficient)->
                Property("excentricity", &CloudMaterialProperties::GetExcentricity, &CloudMaterialProperties::SetExcentricity)->
                Property("multiScatteringABC", &CloudMaterialProperties::GetMultiScatteringABC, &CloudMaterialProperties::SetMultiScatteringABC)->
                Method("ToString", &CloudMaterialPropertiesToString)->
                    Attribute(AZ::Script::Attributes::Operator, AZ::Script::Attributes::OperatorType::ToString)->
                Method("Clone", [](const CloudMaterialProperties& rhs) -> CloudMaterialProperties { return rhs; })
                ;
        }
    }

    bool CloudMaterialProperties::operator==(const CloudMaterialProperties& rhs) const
    {
        return AZ::IsClose(m_absorptionCoefficient, rhs.m_absorptionCoefficient) &&
               AZ::IsClose(m_scatteringCoefficient, rhs.m_scatteringCoefficient) &&
               AZ::IsClose(m_henyeyGreensteinG, rhs.m_henyeyGreensteinG) &&
               AZ::IsClose(m_multiScatteringA, rhs.m_multiScatteringA) &&
               AZ::IsClose(m_multiScatteringB, rhs.m_multiScatteringB) &&
               AZ::IsClose(m_multiScatteringC, rhs.m_multiScatteringC)
               ;
    }

    bool CloudMaterialProperties::operator!=(const CloudMaterialProperties& rhs) const
    {
        return !(*this == rhs);
    }

    // Added for scripting reasons.
    float CloudMaterialProperties::GetAbsorptionCoefficient()
    {
        return m_absorptionCoefficient;
    }

    void  CloudMaterialProperties::SetAbsorptionCoefficient(float aCoeff)
    {
        m_absorptionCoefficient = aCoeff;
    }

    float CloudMaterialProperties::GetScatteringCoefficient()
    {
        return m_scatteringCoefficient;
    }
    
    void  CloudMaterialProperties::SetScatteringCoefficient(float sCoeff)
    {
        m_scatteringCoefficient = sCoeff;
    }
    
    float CloudMaterialProperties::GetExcentricity()
    {
        return m_henyeyGreensteinG;
    }

    void  CloudMaterialProperties::SetExcentricity(float excentricity)
    {
        m_henyeyGreensteinG = excentricity;
    }

    AZ::Vector3 CloudMaterialProperties::GetMultiScatteringABC()
    {
        return AZ::Vector3(m_multiScatteringA, m_multiScatteringB, m_multiScatteringC);
    }

    void CloudMaterialProperties::SetMultiScatteringABC(const AZ::Vector3& abc)
    {
        m_multiScatteringA = abc.GetX();
        m_multiScatteringB = abc.GetY();
        m_multiScatteringC = abc.GetZ();
    }

} // namespace VolumetricClouds
