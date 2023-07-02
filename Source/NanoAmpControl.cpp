/* Copyright (c) 2022, Christian Ahrens
 *
 * This file is part of NanoAmpControl <https://github.com/ChristianAhrens/NanoAmpControl>
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

#include "NanoAmpControl.h"

#include "NanoAmpControlUI.h"
#include "NanoAmpControlProcessor.h"

namespace NanoAmpControl
{

//==============================================================================
NanoAmpControl::NanoAmpControl()
{
    // create processor and ui objects
    m_NanoAmpControlProcessor = std::make_unique<NanoAmpControlProcessor>(s_channelCount);
    m_NanoAmpControlUI = std::make_unique<NanoAmpControlUI>(s_channelCount);

    // interconnect the lambdas of both
    // processor to ui
    m_NanoAmpControlProcessor->onPwrOnOff = [=](bool on) {
        if (m_NanoAmpControlUI)
            m_NanoAmpControlUI->SetPwrOnOff(on);
    };
    m_NanoAmpControlProcessor->onChannelHeadroom = [=](std::uint16_t channel, float headroom) {
        if (m_NanoAmpControlUI)
            m_NanoAmpControlUI->SetChannelHeadroom(channel, headroom);
    };
    m_NanoAmpControlProcessor->onChannelISP = [=](std::uint16_t channel, bool isp) {
        if (m_NanoAmpControlUI)
            m_NanoAmpControlUI->SetChannelISP(channel, isp);
    };
    m_NanoAmpControlProcessor->onChannelGR = [=](std::uint16_t channel, bool gr) {
        if (m_NanoAmpControlUI)
            m_NanoAmpControlUI->SetChannelGR(channel, gr);
    };
    m_NanoAmpControlProcessor->onChannelOVL = [=](std::uint16_t channel, bool ovl) {
        if (m_NanoAmpControlUI)
            m_NanoAmpControlUI->SetChannelOVL(channel, ovl);
    };
    m_NanoAmpControlProcessor->onChannelMute = [=](std::uint16_t channel, bool mute) {
        if (m_NanoAmpControlUI)
            m_NanoAmpControlUI->SetChannelMute(channel, mute);
    };
    m_NanoAmpControlProcessor->onChannelGain = [=](std::uint16_t channel, float gain) {
        if (m_NanoAmpControlUI)
            m_NanoAmpControlUI->SetChannelGain(channel, gain);
    };
    m_NanoAmpControlProcessor->onConnectionStateChanged = [=](NanoAmpControlProcessor::ConnectionState state) {
        if (m_NanoAmpControlUI)
            m_NanoAmpControlUI->SetConnectionState(state);
    };
    // ui to processor
    m_NanoAmpControlUI->onPwrOnOff = [=](bool on) {
        if (m_NanoAmpControlProcessor)
            m_NanoAmpControlProcessor->SetPwrOnOff(on);
    };
    m_NanoAmpControlUI->onChannelMute = [=](std::uint16_t channel, bool mute) {
        if (m_NanoAmpControlProcessor)
            m_NanoAmpControlProcessor->SetChannelMute(channel, mute);
    };
    m_NanoAmpControlUI->onChannelGain = [=](std::uint16_t channel, float gain) {
        if (m_NanoAmpControlProcessor)
            m_NanoAmpControlProcessor->SetChannelGain(channel, gain);
    };
    //m_NanoAmpControlUI->onConnectionStateChanged is intentionally not connected here!
    m_NanoAmpControlUI->onConnectionParametersEdited = [=](const juce::String& address, const std::uint16_t port, const NanoAmpControlInterface::AmpType ampType) {
        if (m_NanoAmpControlProcessor)
            return m_NanoAmpControlProcessor->UpdateConnectionParameters(address, port, ampType);
        else
            return false;
    };
}

NanoAmpControl::~NanoAmpControl()
{
    m_NanoAmpControlProcessor->onPwrOnOff = nullptr;
    m_NanoAmpControlProcessor->onChannelISP = nullptr;
    m_NanoAmpControlProcessor->onChannelGR = nullptr;
    m_NanoAmpControlProcessor->onChannelOVL = nullptr;
    m_NanoAmpControlProcessor->onChannelHeadroom = nullptr;
    m_NanoAmpControlProcessor->onChannelMute = nullptr;
    m_NanoAmpControlProcessor->onChannelGain = nullptr;
    m_NanoAmpControlProcessor->onConnectionStateChanged = nullptr;
    m_NanoAmpControlUI->onPwrOnOff = nullptr;
    m_NanoAmpControlUI->onChannelISP = nullptr;
    m_NanoAmpControlUI->onChannelGR = nullptr;
    m_NanoAmpControlUI->onChannelOVL = nullptr;
    m_NanoAmpControlUI->onChannelHeadroom = nullptr;
    m_NanoAmpControlUI->onChannelMute = nullptr;
    m_NanoAmpControlUI->onChannelGain = nullptr;
    m_NanoAmpControlUI->onConnectionParametersEdited = nullptr;
}

juce::Component* NanoAmpControl::getUIComponent()
{
    if (m_NanoAmpControlUI)
        return m_NanoAmpControlUI.get();
    else
        return nullptr;
}


}
