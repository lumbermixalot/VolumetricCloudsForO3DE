/*
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
*/

#pragma once

#include <AzCore/Console/Console.h>

#include <Atom/Utils/PngFile.h>

#include "PngCloudTextureWriter.h"

namespace VolumetricClouds
{
    PngCloudTextureWriter::PngCloudTextureWriter(uint16_t mipLevels, AZ::RHI::Format pixelFormat, const AZ::IO::Path& outputDir, const AZStd::string& stemPrefix)
        : ICloudTextureWriter(mipLevels, pixelFormat, outputDir, stemPrefix)
    {

    }


    PngCloudTextureWriter::~PngCloudTextureWriter()
    {

    }

    //////////////////////////////////////////////////////////////
    // ICloudTextureWriter Overrides ....
    bool PngCloudTextureWriter::SaveMipLevel(uint16_t mipLevel, AZStd::vector<AZ::IO::Path>* savedFiles)
    {
        if (mipLevel >= GetMipLevels())
        {
            AZ_Error(LogName, false, "Invalid mip level=%hu. Max Level is %hu.\n", mipLevel, GetMipLevels());
            return false;
        }

        const auto& mipLevelData = GetMipLevelDataList()[mipLevel];
        if (!mipLevelData.m_dataBuffer)
        {
            AZ_Error(LogName, false, "There's no data for mip level=%hu\n", mipLevel);
            return false;
        }

        AZ::Utils::PngFile::SaveSettings saveSettings;
        if (auto console = AZ::Interface<AZ::IConsole>::Get(); console != nullptr)
        {
            console->GetCvarValue("r_pngCompressionLevel", saveSettings.m_compressionLevel);
        }
        saveSettings.m_stripAlpha = false; 

        const uint8_t* const mipDataBuffer = mipLevelData.m_dataBuffer->data();

        const auto mipSize = mipLevelData.m_mipSize;
        const uint32_t numSlices = mipSize.m_depth;
        const uint32_t numRows = mipSize.m_height;
        const uint32_t numColumns = mipSize.m_width;
        const uint32_t bytesPerRow = numColumns * 4;
        const uint32_t bytesPerSlice = bytesPerRow * numRows;
        //const uint8_t* bytes = imageCpuBytes->data();
        for (uint32_t sliceIdx = 0; sliceIdx < numSlices; ++sliceIdx)
        {
            //const uint8_t* slicePtr = bytes + (bytesPerSlice * sliceIdx);
            const auto itBegin = mipDataBuffer + (bytesPerSlice * sliceIdx);
            const auto itEnd = itBegin + bytesPerSlice;
            AZStd::span<const uint8_t> spanBytes = { itBegin, itEnd };
            AZ::Utils::PngFile pngImage = AZ::Utils::PngFile::Create(mipSize, GetPixelFormat(), spanBytes);
            if (!pngImage)
            {
                AZ_Error(LogName, false, "Failed to create png image for slice number=%u\n", sliceIdx);
                return false;
            }
            AZ::IO::Path outputFilePath = GetOuputDir();
            outputFilePath.Append(AZStd::string::format("%s_%u_%u.png", GetStemPrefix().c_str(), mipLevel, sliceIdx));
            if (!pngImage.Save(outputFilePath.c_str(), saveSettings))
            {
                AZ_Error(LogName, false, "Failed to save png image=%s\n", outputFilePath.c_str());
                return false;
            }

            if (savedFiles)
            {
                savedFiles->push_back(outputFilePath);
            }
            m_savedFiles.push_back(outputFilePath);
        }
        SetMipLevelSaved(mipLevel);
        return true;
    }
    //////////////////////////////////////////////////////////////

} // namespace VolumetricClouds