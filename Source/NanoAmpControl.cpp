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
    m_NanoAmpControlProcessor->onChannelMute = [=](std::uint16_t channel, bool mute) {
        if (m_NanoAmpControlUI)
            m_NanoAmpControlUI->SetChannelMute(channel, mute);
    };
    m_NanoAmpControlProcessor->onChannelGain = [=](std::uint16_t channel, float gain) {
        if (m_NanoAmpControlUI)
            m_NanoAmpControlUI->SetChannelGain(channel, gain);
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
    m_NanoAmpControlUI->onConnectionParametersEdited = [=](const juce::String& address, const std::uint16_t port) {
        if (m_NanoAmpControlProcessor)
            return m_NanoAmpControlProcessor->UpdateConnectionParameters(address, port);
        else
            return false;
    };
}

NanoAmpControl::~NanoAmpControl()
{
}

juce::Component* NanoAmpControl::getUIComponent()
{
    if (m_NanoAmpControlUI)
        return m_NanoAmpControlUI.get();
    else
        return nullptr;
}


}
