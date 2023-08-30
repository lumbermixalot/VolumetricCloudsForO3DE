/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/
#pragma once

#include <VolumetricClouds/VolumetricCloudsTypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

#include <Atom/RPI.Reflect/Image/Image.h>
#include <AtomCore/Instance/Instance.h>


namespace VolumetricClouds
{
    class CloudTextureProviderRequest
    {
    public:
        AZ_RTTI(CloudTextureProviderRequest, CloudTextureProviderRequestTypeId);
        virtual ~CloudTextureProviderRequest() = default;

        // The returned image must be a 3D Texture.
        virtual AZ::Data::Instance<AZ::RPI::Image> GetCloudTextureImage() = 0;
    };

    class CloudTextureProviderRequestBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;

        /**
        * Overrides the default AZ::EBusTraits ID type so that unsigned ints are 
        * used to access the addresses of the bus.
        */
        typedef AZ::EntityId BusIdType;

        //////////////////////////////////////////////////////////////////////////
    };

    using CloudTextureProviderRequestBus = AZ::EBus<CloudTextureProviderRequest, CloudTextureProviderRequestBusTraits>;


    class CloudTextureProviderNotification
    {
    public:
        AZ_RTTI(CloudTextureProviderNotification, CloudTextureProviderNotificationTypeId);
        virtual ~CloudTextureProviderNotification() = default;

        // Put your public methods here

        // @param image This is a Texture3D (along with all mip levels) that is ready to be used
        //        for example as a readonly SRV in shader etc. 
        virtual void OnCloudTextureImageReady(AZ::Data::Instance<AZ::RPI::Image> image) = 0;

    };

    class CloudTextureProviderNotificationBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Multiple;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::ById;

        /**
        * Overrides the default AZ::EBusTraits ID type so that unsigned ints are 
        * used to access the addresses of the bus.
        */
        typedef AZ::EntityId BusIdType;

        //////////////////////////////////////////////////////////////////////////
    };

    using CloudTextureProviderNotificationBus = AZ::EBus<CloudTextureProviderNotification, CloudTextureProviderNotificationBusTraits>;

} // namespace VolumetricClouds
