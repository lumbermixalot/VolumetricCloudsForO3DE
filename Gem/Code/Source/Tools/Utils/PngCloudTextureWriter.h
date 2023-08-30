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
    class PngCloudTextureWriter final : public ICloudTextureWriter
    {
    public:
        PngCloudTextureWriter() = delete;
        PngCloudTextureWriter(uint16_t mipLevels, AZ::RHI::Format pixelFormat, const AZ::IO::Path& outputDir, const AZStd::string& stemPrefix);
        virtual ~PngCloudTextureWriter();

        static constexpr char LogName[] = "PngCloudTextureWriter";

        //////////////////////////////////////////////////////////////
        // ICloudTextureWriter Overrides ....
        const char* GetLogName() const override { return LogName; }
        bool SaveMipLevel(uint16_t mipLevel, AZStd::vector<AZ::IO::Path>* savedFiles = nullptr) override;
        const AZStd::vector<AZ::IO::Path>& GetListOfSavedFiles() const override { return m_savedFiles; }
        //////////////////////////////////////////////////////////////


    private:
        AZStd::vector<AZ::IO::Path> m_savedFiles;
    };
} // namespace VolumetricClouds