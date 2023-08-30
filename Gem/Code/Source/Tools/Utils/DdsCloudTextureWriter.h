/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include "ICloudTextureWriter.h"

namespace VolumetricClouds
{
    class DdsCloudTextureWriter final : public ICloudTextureWriter
    {
    public:
        DdsCloudTextureWriter() = delete;
        DdsCloudTextureWriter(uint16_t mipLevels, AZ::RHI::Format pixelFormat, const AZ::IO::Path& outputDir, const AZStd::string& stemPrefix);
        virtual ~DdsCloudTextureWriter();

        static constexpr char LogName[] = "PngCloudTextureWriter";

        //////////////////////////////////////////////////////////////
        // ICloudTextureWriter Overrides ....
        const char* GetLogName() const override { return LogName; }
        bool SaveMipLevel(uint16_t mipLevel, AZStd::vector<AZ::IO::Path>* savedFiles = nullptr) override;
        const AZStd::vector<AZ::IO::Path>& GetListOfSavedFiles() const override { return m_savedFiles; }
        //////////////////////////////////////////////////////////////


    private:
        // Returns the expected size of the buffer that contains all the mips and slices
        // as required for a volume texture.
        uint32_t CalculatePixelBufferSize();

        // REMARK: In a single DDS file we save all mip levels of a volume texture.
        AZStd::vector<AZ::IO::Path> m_savedFiles;
    };
} // namespace VolumetricClouds