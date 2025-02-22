/***************************************************************************
 *   Copyright (C) 2016-2022 by Ilya Kotov                                 *
 *   forkotov02@ya.ru                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include <QObject>
#include <QSettings>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <initguid.h>
#include <audioclient.h>
#include <endpointvolume.h>
#include <mmdeviceapi.h>
#include <mmreg.h>
#include <functiondiscoverykeys_devpkey.h>
#include <math.h>
#include <qmmp/buffer.h>
#include "outputwasapi.h"

#define WASAPI_BUFSIZE 10000000LL //1s

#ifndef AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM
#define AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM 0x80000000
#endif

OutputWASAPI *OutputWASAPI::instance = nullptr;
VolumeWASAPI *OutputWASAPI::volumeControl = nullptr;
OutputWASAPI::DWASAPIChannels OutputWASAPI::m_wasapi_pos[10]  = {
   {Qmmp::CHAN_FRONT_LEFT, SPEAKER_FRONT_LEFT},
   {Qmmp::CHAN_FRONT_RIGHT, SPEAKER_FRONT_RIGHT},
   {Qmmp::CHAN_FRONT_CENTER, SPEAKER_FRONT_CENTER},
   {Qmmp::CHAN_LFE, SPEAKER_LOW_FREQUENCY},
   {Qmmp::CHAN_REAR_LEFT, SPEAKER_BACK_LEFT},
   {Qmmp::CHAN_REAR_RIGHT, SPEAKER_BACK_RIGHT},
   {Qmmp::CHAN_REAR_CENTER, SPEAKER_BACK_CENTER},
   {Qmmp::CHAN_SIDE_LEFT, SPEAKER_SIDE_LEFT},
   {Qmmp::CHAN_SIDE_RIGHT, SPEAKER_BACK_RIGHT},
   {Qmmp::CHAN_NULL, 0}
};

OutputWASAPI::OutputWASAPI() : Output()
{
    instance = this;
    QSettings settings;
    m_id = settings.value("WASAPI/device", "default").toString();
    m_exclusive = settings.value("WASAPI/exclusive_mode", false).toBool();
}

OutputWASAPI::~OutputWASAPI()
{
    instance = nullptr;
    uninitialize();
}

bool OutputWASAPI::initialize(quint32 freq, ChannelMap map, Qmmp::AudioFormat format)
{
    HRESULT result = CoCreateInstance(CLSID_MMDeviceEnumerator, nullptr, CLSCTX_ALL, IID_IMMDeviceEnumerator, (void**)&m_pEnumerator);
    if(result != S_OK)
    {
        qWarning("OutputWASAPI: CoCreateInstance failed, error code = 0x%lx", result);
        m_pEnumerator = nullptr;
        return false;
    }

    if(m_id != "default" && !m_id.isEmpty())
    {
        WCHAR id[m_id.length() + 1];
        m_id.toWCharArray(id);
        id[m_id.length()] = 0;

        if((result = m_pEnumerator->GetDevice(id, &m_pDevice)) != S_OK)
        {
            qWarning("OutputWASAPI: IMMDeviceEnumerator::GetDevice failed, error code = 0x%lx", result);
            m_pDevice = nullptr;
        }
        else
            qDebug("OutputWASAPI: using device id: %s", qPrintable(m_id));
    }

    if(!m_pDevice)
    {
        if((result = m_pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_pDevice)) != S_OK)
        {
            qWarning("OutputWASAPI: IMMDeviceEnumerator::GetDefaultAudioEndpoint failed, error code = 0x%lx", result);
            m_pDevice = nullptr;
            return false;
        }
    }

    if((result = m_pDevice->Activate(IID_IAudioClient, CLSCTX_ALL, nullptr, (void**)&m_pAudioClient)) != S_OK)
    {
        qWarning("OutputWASAPI: IMMDevice::Activate failed, error code = 0x%lx", result);
        m_pAudioClient = nullptr;
        return false;
    }

    WAVEFORMATEXTENSIBLE wfex;
    wfex.Format.wFormatTag      = WAVE_FORMAT_EXTENSIBLE;
    wfex.Format.nChannels       = map.count();
    wfex.Format.nSamplesPerSec  = freq;
    wfex.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE);

    if(format == Qmmp::PCM_S16LE)
    {
        wfex.Format.wBitsPerSample = 16;
        wfex.Samples.wValidBitsPerSample = 16;
    }
    else if(format == Qmmp::PCM_S24LE)
    {
        wfex.Format.wBitsPerSample  = 32;
        wfex.Samples.wValidBitsPerSample = 24;
    }
    else if(format == Qmmp::PCM_S32LE)
    {
        wfex.Format.wBitsPerSample  = 32;
        wfex.Samples.wValidBitsPerSample = 32;
    }
    else
    {
        format = Qmmp::PCM_S16LE;
        wfex.Format.wBitsPerSample  = 16;
        wfex.Samples.wValidBitsPerSample = 16;
    }

    wfex.Format.nBlockAlign     = (wfex.Format.wBitsPerSample / 8) * wfex.Format.nChannels;
    wfex.Format.nAvgBytesPerSec = wfex.Format.nSamplesPerSec * wfex.Format.nBlockAlign;

    //generate channel order
    ChannelMap out_map;
    int i = 0;
    DWORD mask = 0;
    while(m_wasapi_pos[i].pos != Qmmp::CHAN_NULL)
    {
        if(map.contains(m_wasapi_pos[i].pos))
        {
            mask |= m_wasapi_pos[i].chan_mask;
            out_map << m_wasapi_pos[i].pos;
        }
        i++;
    }

    wfex.dwChannelMask = mask;
    wfex.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

    AUDCLNT_SHAREMODE mode = m_exclusive ? AUDCLNT_SHAREMODE_EXCLUSIVE :  AUDCLNT_SHAREMODE_SHARED;
    DWORD streamFlags = 0;
    //enable channel matrixer and a sample rate converter if format is unsupported
    if(m_pAudioClient->IsFormatSupported(mode, (WAVEFORMATEX *)&wfex, nullptr) != S_OK)
    {
        streamFlags |= AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM;
        qDebug("OutputWASAPI: format is not supported, using converter");
    }

    if((result = m_pAudioClient->Initialize(mode, streamFlags, WASAPI_BUFSIZE, 0, (WAVEFORMATEX *)&wfex, nullptr)) != S_OK)
    {
        qWarning("OutputWASAPI: IAudioClient::Initialize failed, error code = 0x%lx", result);
        return false;
    }

    if((result = m_pAudioClient->GetBufferSize(&m_bufferFrames)) != S_OK)
    {
        qWarning("OutputWASAPI: IAudioClient::GetBufferSize failed, error code = 0x%lx", result);
        return false;
    }

    if((result = m_pAudioClient->GetService(IID_IAudioRenderClient, (void**)&m_pRenderClient)) != S_OK)
    {
        qWarning("OutputWASAPI: IAudioClient::GetService failed, error code = 0x%lx", result);
        return false;
    }

    if((result = m_pAudioClient->GetService(IID_ISimpleAudioVolume, (void**)&m_pSimpleAudioVolume)) != S_OK)
    {
        qWarning("OutputWASAPI: IAudioClient::GetService failed, error code = 0x%lx", result);
        return false;
    }


    if((result = m_pAudioClient->Start()) != S_OK)
    {
        qWarning("OutputWASAPI: IAudioClient::Start failed, error code = 0x%lx", result);
        return false;
    }

    configure(freq, out_map, format);
    m_frameSize = sampleSize() * channels();
    if(volumeControl)
        volumeControl->restore();
    return true;
}

qint64 OutputWASAPI::latency()
{
    UINT32 frames = 0;
    m_pAudioClient->GetCurrentPadding(&frames);
    return frames * 1000 / sampleRate();
}

qint64 OutputWASAPI::writeAudio(unsigned char *data, qint64 len)
{
    UINT32 frames = 0;
    BYTE *pData = nullptr;
    DWORD flags = 0;
    DWORD result = 0;
    m_pAudioClient->GetCurrentPadding(&frames);

    UINT32 framesAvailable = m_bufferFrames - frames;

    UINT32 framesToWrite = qMin(framesAvailable, (UINT32)len / m_frameSize);

    //wait until buffer is not full
    if(framesToWrite == 0)
    {
        usleep(len * 1000000L / sampleRate() / m_frameSize / 2);
        m_pAudioClient->GetCurrentPadding(&frames);
        framesAvailable = m_bufferFrames - frames;
        framesToWrite = qMin(framesAvailable, (UINT32)len / m_frameSize);
    }

    if(format() == Qmmp::PCM_S24LE)
    {
        for(DWORD i = 0; i < framesToWrite * channels(); ++i)
        {
            ((quint32*) data)[i] <<= 8;
        }
    }

    if((result = m_pRenderClient->GetBuffer(framesToWrite, &pData)) != S_OK)
    {
        qWarning("OutputWASAPI: IAudioClient::GetBuffer failed, error code = 0x%lx", result);
        return -1;
    }
    memcpy(pData, data, framesToWrite * m_frameSize);
    m_pRenderClient->ReleaseBuffer(framesToWrite, flags);
    return framesToWrite * m_frameSize;
}

void OutputWASAPI::drain()
{
    UINT32 frames = 0;
    while((m_pAudioClient->GetCurrentPadding(&frames) == S_OK) && (frames > 0))
        usleep(50);
}

void OutputWASAPI::suspend()
{
    m_pAudioClient->Stop();
}

void OutputWASAPI::resume()
{
   m_pAudioClient->Start();
}

void OutputWASAPI::reset()
{
    m_pAudioClient->Stop();
    m_pAudioClient->Reset();
    m_pAudioClient->Start();
}

ISimpleAudioVolume *OutputWASAPI::simpleAudioVolume()
{
    return m_pSimpleAudioVolume;
}

void OutputWASAPI::uninitialize()
{
    if(m_pAudioClient)
    {
        m_pAudioClient->Stop();
        m_pAudioClient->Release();
        m_pAudioClient = nullptr;
    }
    if(m_pEnumerator)
    {
        m_pEnumerator->Release();
        m_pEnumerator = nullptr;
    }
    if(m_pDevice)
    {
        m_pDevice->Release();
        m_pDevice = nullptr;
    }
    if(m_pRenderClient)
    {
        m_pRenderClient->Release();
        m_pRenderClient = nullptr;
    }
    if(m_pSimpleAudioVolume)
    {
        m_pSimpleAudioVolume->Release();
        m_pSimpleAudioVolume = nullptr;
    }
}

/***** MIXER *****/
VolumeWASAPI::VolumeWASAPI()
{
    OutputWASAPI::volumeControl = this;
    QSettings settings;
    m_volume.left = settings.value("WASAPI/left_volume", 100).toInt();
    m_volume.right = settings.value("WASAPI/right_volume", 100).toInt();
}

VolumeWASAPI::~VolumeWASAPI()
{
    m_volume = volume();
    QSettings settings;
    settings.setValue("WASAPI/left_volume", m_volume.left);
    settings.setValue("WASAPI/right_volume", m_volume.right);
    OutputWASAPI::volumeControl = nullptr;
}

void VolumeWASAPI::setVolume(const VolumeSettings &vol)
{
    if(OutputWASAPI::instance && OutputWASAPI::instance->simpleAudioVolume())
    {
        OutputWASAPI::instance->simpleAudioVolume()->SetMasterVolume(float(qMax(vol.left, vol.right)) / 100.0f, nullptr);
    }
    m_volume = vol;
}

VolumeSettings VolumeWASAPI::volume() const
{
    VolumeSettings vol;
    if(OutputWASAPI::instance && OutputWASAPI::instance->simpleAudioVolume())
    {
        float level = 0;
        OutputWASAPI::instance->simpleAudioVolume()->GetMasterVolume(&level);
        vol.left = ceilf(level * 100.0f);
        vol.right = vol.left;
        return vol;
    }
    return m_volume;
}

void VolumeWASAPI::restore()
{
    setVolume(m_volume);
}
