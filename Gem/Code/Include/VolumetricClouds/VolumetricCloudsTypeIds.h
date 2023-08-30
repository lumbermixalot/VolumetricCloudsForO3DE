/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/
#pragma once

namespace VolumetricClouds
{
    inline constexpr char ScriptingModuleName[] = "volumetric_clouds";

    // System Component TypeIds
    inline constexpr const char* VolumetricCloudsSystemComponentTypeId = "{523C52D4-A099-4EB4-941D-C5AF9EE6CD66}";
    inline constexpr const char* VolumetricCloudsEditorSystemComponentTypeId = "{E7F643AE-A5E0-49A7-B420-C69D40981CF6}";

    // Module derived classes TypeIds
    inline constexpr const char* VolumetricCloudsModuleInterfaceTypeId = "{720C24BB-1F0E-47EC-9663-79585B4CBE01}";
    inline constexpr const char* VolumetricCloudsModuleTypeId = "{D4DBA861-B571-4A01-97C2-C06EB40C5104}";
    // The Editor Module by default is mutually exclusive with the Client Module
    // so they use the Same TypeId
    inline constexpr const char* VolumetricCloudsEditorModuleTypeId = VolumetricCloudsModuleTypeId;

    // Interface TypeIds
    inline constexpr const char* VolumetricCloudsRequestsTypeId = "{01F82831-F461-41A3-A116-5E6FAC87038F}";
    inline constexpr const char* CloudTextureSystemRequestsTypeId = "{08CF7D92-BF8B-4589-97E4-050861E25EDB}";

    // Interface TypeIds
    inline constexpr const char* CloudscapeComponentTypeId = "{2B9B1A59-B803-4150-9AAD-784427250678}";
    inline constexpr const char* EditorCloudscapeComponentTypeId = "{1932D2C2-6861-4613-9A07-6DAD4DA594FC}";

    inline constexpr const char* CloudTextureComputeComponentTypeId = "{CDF1B0AE-1FA5-4023-AFF3-7832D624212D}";
    inline constexpr const char* EditorCloudTextureComputeComponentTypeId = "{BC14B4C5-D5B5-4650-9E14-4DEDBBAACFD4}";
    inline constexpr const char* CloudTextureComputeDataTypeId = "{7B80A2DC-340F-42C8-B4CE-41A48774427C}";
    inline constexpr const char* CloudTexturePresentationDataTypeId = "{299BBB68-B5AC-4D81-B78D-F03BFA012F09}";

    inline constexpr const char* CloudTextureAssetComponentTypeId = "{BC2E899F-D153-4108-A953-071F6765BA9B}";
    inline constexpr const char* EditorCloudTextureAssetComponentTypeId = "{354CE04D-5D9D-4194-8757-88AF7472DDAA}";

    inline constexpr const char* CloudMaterialPropertiesTypeId = "{515030BE-B95D-4A3D-87F4-F5249AF086AB}";
    inline constexpr const char* CloudscapeShaderConstantDataTypeId = "{9940E82C-AD4D-418E-B98C-FDB89FFE4BA5}";


    // Interface TypeIds
    inline constexpr const char* CloudTextureProviderRequestTypeId = "{C0DC5305-DE2C-4F94-899B-19127619179E}";
    inline constexpr const char* CloudTextureProviderNotificationTypeId = "{90CDDC65-2F2E-4053-A42E-C3B38723AD28}";


} // namespace VolumetricClouds
