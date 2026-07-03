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

#pragma once

#include <JuceHeader.h>

#include "NanoAmpControl.h"

#include <AmpController.h>


namespace NanoAmpControl
{


//==============================================================================
/**
 * @brief Protocol bridge between the JUCE NanoAmpControl UI layer and
 *        NanoOcp1::AmpController.
 *
 * Derives from NanoAmpControlInterface (app-level JUCE callback API) and
 * holds an AmpController (JUCE-free OCP.1 transport).  The constructor
 * wires the AmpController's typed callbacks to the NanoAmpControlInterface
 * callbacks so that the UI layer is unaffected by the protocol implementation.
 *
 * Connection lifecycle is fully managed by AmpController: auto-subscribe,
 * auto-query-values, and auto-reconnect on connection loss.
 *
 * ## Threading
 * AmpController callbacks fire on the NanoOcp1 socket thread.  The JUCE UI
 * layer receives them via the juce::MessageManager if posting from a non-
 * message thread is required; see NanoAmpControlUI for that bridging.
 */
class NanoAmpControlProcessor : public NanoAmpControlInterface
{
public:
    //==============================================================================
    explicit NanoAmpControlProcessor(std::uint16_t ampChannelCount);
    ~NanoAmpControlProcessor() override;

    //==============================================================================
    /**
     * Reconfigure the target device and reconnect.
     * Disconnects the current session, updates amp type and I/O parameters,
     * then reconnects to the new address/port.
     */
    bool UpdateConnectionParameters(const juce::String& address,
                                    std::uint16_t       port,
                                    AmpType             ampType);

    //==============================================================================
    bool SetPwrOnOff(bool on) override;
    bool SetChannelISP(std::uint16_t channel, bool isp) override;
    bool SetChannelGR(std::uint16_t channel, bool gr) override;
    bool SetChannelOVL(std::uint16_t channel, bool ovl) override;
    bool SetChannelHeadroom(std::uint16_t channel, float headroom) override;
    bool SetChannelMute(std::uint16_t channel, bool mute) override;
    bool SetChannelGain(std::uint16_t channel, float gain) override;

    //==============================================================================
    void SetConnectionState(ConnectionState state) override;

private:
    //==============================================================================
    static NanoOcp1::AmpController::AmpType toCtrlAmpType(AmpType t);
    static ConnectionState fromCtrlState(NanoOcp1::Ocp1Controller::State s);

    //==============================================================================
    std::unique_ptr<NanoOcp1::AmpController> m_amp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NanoAmpControlProcessor)
};


} // namespace NanoAmpControl
