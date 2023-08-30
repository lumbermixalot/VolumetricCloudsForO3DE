/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <AzCore/Console/Console.h>

#include <Atom/Utils/DdsFile.h>

#include "DdsCloudTextureWriter.h"

namespace VolumetricClouds
{
    DdsCloudTextureWriter::DdsCloudTextureWriter(uint16_t mipLevels, AZ::RHI::Format pixelFormat, const AZ::IO::Path& outputDir, const AZStd::string& stemPrefix)
        : ICloudTextureWriter(mipLevels, pixelFormat, outputDir, stemPrefix)
    {

    }


    DdsCloudTextureWriter::~DdsCloudTextureWriter()
    {

    }

    // Number of bytes per row.
    // Calculated per https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dx-graphics-dds-pguide
    // ( width * bits-per-pixel + 7 ) / 8
    static uint32_t DdsCalculateRowSizeForWidth(uint32_t width, AZ::RHI::Format pixelFormat)
    {
        constexpr uint32_t BitsInAByte = 8;
        return ((width * AZ::RHI::GetFormatSize(pixelFormat) * BitsInAByte) + (BitsInAByte - 1)) / BitsInAByte;
    }

    static uint32_t  DdsCalculateMipSizeInBytes(const AZ::RHI::Size& mipSize, AZ::RHI::Format pixelFormat)
    {
        const uint32_t rowPitch = DdsCalculateRowSizeForWidth(mipSize.m_width, pixelFormat);
        const uint32_t depthSliceSize = rowPitch * mipSize.m_height;
        return  depthSliceSize * mipSize.m_depth;
    }

    uint32_t DdsCloudTextureWriter::CalculatePixelBufferSize()
    {
        // See https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dds-file-layout-for-volume-textures.
        uint32_t totalBufferSize = 0;
        for (uint16_t mipIdx = 0; mipIdx < GetMipLevels(); mipIdx++)
        {
            const auto & mipLevelData = GetMipLevelDataList()[mipIdx];
            totalBufferSize += DdsCalculateMipSizeInBytes(mipLevelData.m_mipSize, GetPixelFormat());
        }
        return totalBufferSize;
    }

    //////////////////////////////////////////////////////////////
    // ICloudTextureWriter Overrides ....
    bool DdsCloudTextureWriter::SaveMipLevel(uint16_t mipLevel, AZStd::vector<AZ::IO::Path>* savedFiles)
    {
        if (mipLevel >= GetMipLevels())
        {
            AZ_Error(LogName, false, "Invalid mip level=%hu. Max Level is %hu.\n", mipLevel, GetMipLevels());
            return false;
        }

        if (!GetMipLevelDataList()[mipLevel].m_dataBuffer)
        {
            AZ_Error(LogName, false, "Can't save mip level %hu if there's no data buffer.\n", mipLevel);
            return false;
        }

        // Mark the mip level as saved.
        SetMipLevelSaved(mipLevel);

        if (GetMipLevelsWithDataCount() != GetMipLevels())
        {
            // If we don't have all the data for all mip levels, then do nothing
            return true;
        }

        if (!m_savedFiles.empty())
        {
            return true; // DDS file already created.
        }

        AZStd::vector<uint8_t> ddsPixelBuffer;
        ddsPixelBuffer.resize_no_construct(CalculatePixelBufferSize());

        // Time to dump all the mips into a single buffer
        // per https://learn.microsoft.com/en-us/windows/win32/direct3ddds/dds-file-layout-for-volume-textures
        uint8_t* currentPixelPtr = ddsPixelBuffer.data();
        for (uint16_t mipIdx = 0; mipIdx < GetMipLevels(); mipIdx++)
        {
            const auto& mipLevelData = GetMipLevelDataList()[mipIdx];
            memcpy(currentPixelPtr, mipLevelData.m_dataBuffer->data(), mipLevelData.m_dataBuffer->size());
            currentPixelPtr += mipLevelData.m_dataBuffer->size();
        }

        const auto & mipLevel0Data = GetMipLevelDataList()[0];
        // We have everything we need to create the one and only DDS File.
        AZ::DdsFile::DdsFileData ddsFileData;
        ddsFileData.m_size = mipLevel0Data.m_mipSize;
        ddsFileData.m_format = GetPixelFormat();
        ddsFileData.m_mipLevels = GetMipLevels();
        ddsFileData.m_buffer = &ddsPixelBuffer;

        AZ::IO::Path outputFile = GetOuputDir();
        outputFile.Append(AZStd::string::format("%s.dds", GetStemPrefix().c_str()));
        auto outcome = AZ::DdsFile::WriteFile(outputFile.String(), ddsFileData);
        if (!outcome)
        {
            AZ_Error(LogName, false, outcome.GetError().m_message.c_str());
            return false;
        }

        if (savedFiles)
        {
            savedFiles->push_back(outputFile);
        }
        m_savedFiles.emplace_back(AZStd::move(outputFile));
        return true;
    }

    //////////////////////////////////////////////////////////////

} // namespace VolumetricClouds