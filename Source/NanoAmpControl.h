/* Copyright (c) 2023, Christian Ahrens
 *
 * This file is part of SurroundFieldMixer <https://github.com/ChristianAhrens/NanoAmpControl>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#pragma once

#include <JuceHeader.h>


namespace NanoAmpControl
{

/**
 * Fwd. decls
 */
class NanoAmpControlUI;
class NanoAmpControlProcessor;


//==============================================================================
/**
 *
 */
class NanoAmpControlInterface
{
public:
    enum ConnectionState
    {
        Unknown,
        Disconnected,
        Connected,
        Active,
        Subscribed,
    };

public:
    NanoAmpControlInterface(const std::uint16_t ampChannelCount) : m_ampChannelCount(ampChannelCount) {};

    //==============================================================================
    virtual bool SetPwrOnOff(const bool on) = 0;
    virtual bool SetChannelISP(const std::uint16_t channel, const bool isp) = 0;
    virtual bool SetChannelGR(const std::uint16_t channel, const bool gr) = 0;
    virtual bool SetChannelOVL(const std::uint16_t channel, const bool ovl) = 0;
    virtual bool SetChannelMute(const std::uint16_t channel, const bool mute) = 0;
    virtual bool SetChannelGain(const std::uint16_t channel, const float gain) = 0;

    //==============================================================================
    virtual void SetConnectionState(const ConnectionState state) = 0;

    //==============================================================================
    std::function<void(const bool on)>                                  onPwrOnOff;
    std::function<void(const std::uint16_t channel, const bool isp)>    onChannelISP;
    std::function<void(const std::uint16_t channel, const bool gr)>     onChannelGR;
    std::function<void(const std::uint16_t channel, const bool ovl)>    onChannelOVL;
    std::function<void(const std::uint16_t channel, const bool mute)>   onChannelMute;
    std::function<void(const std::uint16_t channel, const float gain)>  onChannelGain;
    std::function<void(const ConnectionState state)>                    onConnectionStateChanged;

protected:
    //==============================================================================
    std::uint16_t GetAmpChannelCount() { return m_ampChannelCount; };

private:
    //==============================================================================
    std::uint16_t   m_ampChannelCount{ 0 };

};

//==============================================================================
/**
 *
 */
class NanoAmpControl
{
public:
    NanoAmpControl();
    ~NanoAmpControl();
    
    //==========================================================================
    juce::Component* getUIComponent();

    //==========================================================================
    static constexpr std::uint16_t s_channelCount{ 4 };

private:
    //==========================================================================

    //==========================================================================
    std::unique_ptr<NanoAmpControlProcessor>    m_NanoAmpControlProcessor;
    std::unique_ptr<NanoAmpControlUI>           m_NanoAmpControlUI;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NanoAmpControl)
};

};
