/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <AzCore/RTTI/TypeInfoSimple.h>
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Math/Vector3.h>

namespace VolumetricClouds
{
    // Provides some of the data used by CloudscapeCS.azsl that describe
    // the value of the parameters for the physical properties
    // of clouds in terms of light propagation and scattering.
    struct CloudMaterialProperties
    {
        AZ_CLASS_ALLOCATOR_DECL;
        AZ_TYPE_INFO_WITH_NAME_DECL(CloudMaterialProperties);
        AZ_RTTI_NO_TYPE_INFO_DECL();

        static void Reflect(AZ::ReflectContext* reflection);

        bool operator==(const CloudMaterialProperties& rhs) const;
        bool operator!=(const CloudMaterialProperties& rhs) const;

        static constexpr float DEFAULT_ABSORPTION_COEFFICIENT = 0.01f; //[m-1]

        // NOTE: Beer's Law Attenuation Coefficient = Absorption + Scattering coefficients.
        // Attenuation coefficient is typically 0.04[m-1] for clouds.
        // For clouds, Scattering coefficient is typically 3x Absorption coeffcient.

        // Beer's Law Absorption coefficient. in units [m-1] == [1/m].
        // For clouds it is typically 0.01[m-1]
        float m_absorptionCoefficient = DEFAULT_ABSORPTION_COEFFICIENT;

        // Beer's Law Scattering coefficient. in units [m-1] == [1/m].
        // For clouds it is typically 3x(Absorption coefficient) 0.03[m-1]
        float m_scatteringCoefficient = 3.0 * DEFAULT_ABSORPTION_COEFFICIENT;

        // Excentricity constant in HenyeyGreenstein phase function. Typically 0.2.
        // A number between [-1,1]. The closer it is to 1.0 the more forward scattering.
        // The closer it is to -1.0 the more backward scattering.
        float m_henyeyGreensteinG = 0.2;

        // These are the a, b, c coefficients that will be used
        // to modulate each multi scaterring octave of light contribution
        // as proposed in the paper "Oz: The Great and Volumetric"
        // http://magnuswrenninge.com/wp-content/uploads/2010/03/Wrenninge-OzTheGreatAndVolumetric.pdf
        // a: Attenuation that affects the optical depth in Beer's Law evaluation.
        // b: Contribution to the scattering coefficient.
        // c: Excentricity Attenuation to the excentricity constant "g" (see @m_henyeyGreensteinG above).
        float m_multiScatteringA = 0.5f;
        float m_multiScatteringB = 0.5f;
        float m_multiScatteringC = 0.5f;

        // Added for scripting reasons.
        float GetAbsorptionCoefficient();
        void  SetAbsorptionCoefficient(float aCoeff);
        float GetScatteringCoefficient();
        void  SetScatteringCoefficient(float sCoeff);
        float GetExcentricity();
        void  SetExcentricity(float excentricity);
        AZ::Vector3 GetMultiScatteringABC();
        void SetMultiScatteringABC(const AZ::Vector3& abc);
    };

} // namespace VolumetricClouds
