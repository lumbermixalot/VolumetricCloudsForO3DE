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
    ICloudTextureWriter::ICloudTextureWriter(uint16_t mipLevels, AZ::RHI::Format pixelFormat,
        const AZ::IO::Path& outputDir, const AZStd::string& stemPrefix)
        : m_mipLevels(mipLevels), m_pixelFormat(pixelFormat)
        , m_outputDir(outputDir), m_stemPrefix(stemPrefix)
    {
        m_mipLevelsDataList.reserve(mipLevels);
        m_mipLevelsDataList.resize(mipLevels);
    }

    bool ICloudTextureWriter::SetDataBufferForMipLevel(AZStd::shared_ptr<AZStd::vector<uint8_t>> dataBuffer,
        uint16_t mipLevel, AZ::RHI::Size mipSize)
    {
        if (mipLevel >= m_mipLevels)
        {
            AZ_Warning(GetLogName(), false, "Invalid mip level index=%hu. Max level is %hu\n",
                mipLevel, m_mipLevels);
            return false;
        }
        AZ_Assert(mipLevel < m_mipLevelsDataList.size(),
            "The mip level data list has not been resized properly. mip level index=%hu, current size=%zu",
            mipLevel, m_mipLevelsDataList.size() );
        AZ_Warning(GetLogName(), !m_mipLevelsDataList[mipLevel].m_dataBuffer, "mipLevel %hu already had a data buffer and will be replaced.\n", mipLevel);
        
        MipLevelData& mipData = m_mipLevelsDataList[mipLevel];
        mipData.m_dataBuffer = dataBuffer;
        mipData.m_mipLevel = mipLevel;
        mipData.m_mipSize = mipSize;

        m_mipLevelsWithData[mipLevel] = true;

        return DataBufferForMipLevelAdded(mipData);
    }

    void ICloudTextureWriter::SetMipLevelSaved(uint16_t mipLevel)
    {
        m_savedMipLevels[mipLevel] = true;
    }

} // namespace VolumetricClouds