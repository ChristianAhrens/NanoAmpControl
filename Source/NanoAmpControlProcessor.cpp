/* Copyright (c) 2023-2026, Christian Ahrens
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

#include "NanoAmpControlProcessor.h"


namespace NanoAmpControl
{


//==============================================================================
NanoAmpControlProcessor::NanoAmpControlProcessor(const std::uint16_t ampChannelCount)
    : NanoAmpControlInterface(ampChannelCount)
{
    m_amp = std::make_unique<NanoOcp1::AmpController>();
    m_amp->setAmpType(NanoOcp1::AmpController::AmpType::Dy,
                      static_cast<std::uint16_t>(ampChannelCount));

    // ── Wire AmpController callbacks → NanoAmpControlInterface callbacks ──────

    m_amp->onStateChanged = [this](NanoOcp1::Ocp1Controller::State s) {
        SetConnectionState(fromCtrlState(s));
    };

    m_amp->onPower = [this](bool on) {
        if (onPwrOnOff)
            onPwrOnOff(on);
    };

    m_amp->onChannelGain = [this](std::uint16_t ch, float dB) {
        if (onChannelGain)
            onChannelGain(ch, dB);
    };

    m_amp->onChannelMute = [this](std::uint16_t ch, bool muted) {
        if (onChannelMute)
            onChannelMute(ch, muted);
    };

    m_amp->onChannelISP = [this](std::uint16_t ch, bool active) {
        if (onChannelISP)
            onChannelISP(ch, active);
    };

    m_amp->onChannelGR = [this](std::uint16_t ch, bool active) {
        if (onChannelGR)
            onChannelGR(ch, active);
    };

    m_amp->onChannelOVL = [this](std::uint16_t ch, bool active) {
        if (onChannelOVL)
            onChannelOVL(ch, active);
    };

    m_amp->onChannelHeadroom = [this](std::uint16_t ch, float hr) {
        if (onChannelHeadroom)
            onChannelHeadroom(ch, hr);
    };

    // Start with a connection attempt to the default target.
    m_amp->connect("127.0.0.1", 50014);
}

NanoAmpControlProcessor::~NanoAmpControlProcessor()
{
    // AmpController destructor calls disconnect() automatically.
}

//==============================================================================
bool NanoAmpControlProcessor::UpdateConnectionParameters(const juce::String& address,
                                                         const std::uint16_t port,
                                                         const AmpType       ampType)
{
    m_amp->disconnect();
    m_amp->setAmpType(toCtrlAmpType(ampType),
                      static_cast<std::uint16_t>(GetAmpChannelCount()));
    m_amp->connect(address.toStdString(), static_cast<int>(port));
    return true;
}

//==============================================================================
bool NanoAmpControlProcessor::SetPwrOnOff(const bool on)
{
    return m_amp ? m_amp->setPower(on) : false;
}

bool NanoAmpControlProcessor::SetChannelISP(const std::uint16_t, const bool)
{
    return false; // ISP is a read-only status; not settable from UI.
}

bool NanoAmpControlProcessor::SetChannelGR(const std::uint16_t, const bool)
{
    return false; // GR is a read-only status; not settable from UI.
}

bool NanoAmpControlProcessor::SetChannelOVL(const std::uint16_t, const bool)
{
    return false; // OVL is a read-only status; not settable from UI.
}

bool NanoAmpControlProcessor::SetChannelHeadroom(const std::uint16_t, const float)
{
    return false; // Headroom is a read-only status; not settable from UI.
}

bool NanoAmpControlProcessor::SetChannelMute(const std::uint16_t channel, const bool mute)
{
    return m_amp ? m_amp->setChannelMute(channel, mute) : false;
}

bool NanoAmpControlProcessor::SetChannelGain(const std::uint16_t channel, const float gain)
{
    return m_amp ? m_amp->setChannelGain(channel, gain) : false;
}

//==============================================================================
void NanoAmpControlProcessor::SetConnectionState(const ConnectionState state)
{
    if (onConnectionStateChanged)
        onConnectionStateChanged(state);
}

//==============================================================================
NanoOcp1::AmpController::AmpType
NanoAmpControlProcessor::toCtrlAmpType(const AmpType t)
{
    switch (t)
    {
    case AmpType::_Dx:  return NanoOcp1::AmpController::AmpType::Dx;
    case AmpType::_Dy:  return NanoOcp1::AmpController::AmpType::Dy;
    case AmpType::_5D:  return NanoOcp1::AmpController::AmpType::FiveD;
    default:            return NanoOcp1::AmpController::AmpType::Dy;
    }
}

NanoAmpControlInterface::ConnectionState
NanoAmpControlProcessor::fromCtrlState(const NanoOcp1::Ocp1Controller::State s)
{
    switch (s)
    {
    case NanoOcp1::Ocp1Controller::State::Disconnected: return ConnectionState::Disconnected;
    case NanoOcp1::Ocp1Controller::State::Connecting:   return ConnectionState::Disconnected;
    case NanoOcp1::Ocp1Controller::State::Subscribing:  return ConnectionState::Connected;
    case NanoOcp1::Ocp1Controller::State::Subscribed:   return ConnectionState::Active;
    case NanoOcp1::Ocp1Controller::State::GetValues:    return ConnectionState::Active;
    case NanoOcp1::Ocp1Controller::State::Connected:    return ConnectionState::Subscribed;
    default:                                             return ConnectionState::Unknown;
    }
}


} // namespace NanoAmpControl
