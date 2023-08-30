/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <AzCore/std/containers/vector.h>
#include <AzCore/std/containers/bitset.h>
#include <AzCore/std/smart_ptr/shared_ptr.h>
#include <AzCore/IO/Path/Path.h>

#include <Atom/RHI.Reflect/Size.h>
#include <Atom/RHI.Reflect/Format.h>

namespace VolumetricClouds
{
    class ICloudTextureWriter
    {
    public:
        ICloudTextureWriter() = delete;
        ICloudTextureWriter(uint16_t mipLevels, AZ::RHI::Format pixelFormat, const AZ::IO::Path& outputDir, const AZStd::string& stemPrefix);
        virtual ~ICloudTextureWriter() {};

        bool SetDataBufferForMipLevel(AZStd::shared_ptr<AZStd::vector<uint8_t>> dataBuffer,
                                      uint16_t mipLevel, AZ::RHI::Size mipSize);

        // Used for AZ_Warning, AZ_Printf, etc
        virtual const char* GetLogName() const = 0;

        //! Returns true is successfully saved or processed a mip level.
        //! For some formats like PNG several files will be saved to disk. one per mip level, per depth slice
        //! If the caller provides a valid pointer to a vector, then the list of created files will be added to it.
        //! Other formats like DDS support all depth slices and all mips of a volume texture in a single file,
        //! in this case even if the mip was processed successfully the CloudTextureWriter may not save 
        //! new files to disk.
        //! Returns false in case that there's missing mip level data or the requested @mipLevel
        //! is out of bounds.
        virtual bool SaveMipLevel(uint16_t mipLevel, AZStd::vector<AZ::IO::Path>* savedFiles = nullptr) = 0;

        //! Returns a list of all created files. If an empty list is returned it is not necessarily
        //! a sign of error. For example for DDS files it may mean that not all mips data buffers
        //! are available
        virtual const AZStd::vector<AZ::IO::Path>& GetListOfSavedFiles() const = 0;

        uint16_t GetSavedMipLevelsCount() const { return static_cast<uint16_t>(m_savedMipLevels.count()); }
        uint16_t GetMipLevelsWithDataCount() const { return static_cast<uint16_t>(m_mipLevelsWithData.count()); }
        uint16_t GetMipLevels() const { return m_mipLevels; }
        AZ::RHI::Format GetPixelFormat() const { return m_pixelFormat; }
        const AZ::IO::Path& GetOuputDir() const { return m_outputDir; }
        const AZStd::string& GetStemPrefix() const { return m_stemPrefix; }

    protected:
        struct MipLevelData
        {
            uint16_t m_mipLevel = 0;
            AZ::RHI::Size m_mipSize = {};
            AZStd::shared_ptr<AZStd::vector<uint8_t>> m_dataBuffer;
        };
        const AZStd::vector<MipLevelData>& GetMipLevelDataList() const { return m_mipLevelsDataList; }
        void SetMipLevelSaved(uint16_t mipLevel);

        virtual bool DataBufferForMipLevelAdded([[maybe_unused]] const MipLevelData& mipLevelData) { return true; }

    private:
        uint16_t m_mipLevels;
        AZ::RHI::Format m_pixelFormat;
        AZ::IO::Path m_outputDir;
        AZStd::string m_stemPrefix;
        AZStd::vector<MipLevelData> m_mipLevelsDataList;
        AZStd::bitset<16> m_savedMipLevels;
        AZStd::bitset<16> m_mipLevelsWithData;
    };
} // namespace VolumetricClouds