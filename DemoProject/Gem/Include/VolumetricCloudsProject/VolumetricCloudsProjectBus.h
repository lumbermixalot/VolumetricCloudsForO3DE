
#pragma once

#include <VolumetricCloudsProject/VolumetricCloudsProjectTypeIds.h>

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace VolumetricCloudsProject
{
    class VolumetricCloudsProjectRequests
    {
    public:
        AZ_RTTI(VolumetricCloudsProjectRequests, VolumetricCloudsProjectRequestsTypeId);
        virtual ~VolumetricCloudsProjectRequests() = default;
        // Put your public methods here
    };

    class VolumetricCloudsProjectBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using VolumetricCloudsProjectRequestBus = AZ::EBus<VolumetricCloudsProjectRequests, VolumetricCloudsProjectBusTraits>;
    using VolumetricCloudsProjectInterface = AZ::Interface<VolumetricCloudsProjectRequests>;

} // namespace VolumetricCloudsProject
