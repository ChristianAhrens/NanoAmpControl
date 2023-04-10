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

#include "NanoAmpControl.h"


/**
 * Fwd. Decls
 */
namespace NanoOcp1
{
class NanoOcp1;
class NanoOcp1Client;
class Ocp1Notification;
class Ocp1Response;
}

namespace NanoAmpControl
{


//==============================================================================
/**
 *
 */
class NanoAmpControlProcessor : public NanoAmpControlInterface
{
public:
    //==============================================================================
    NanoAmpControlProcessor(const std::uint16_t ampChannelCount);
    ~NanoAmpControlProcessor();

    //==============================================================================
    bool UpdateConnectionParameters(const juce::String& address, const std::uint16_t port);

    //==============================================================================
    bool SetPwrOnOff(const bool on) override;
    bool SetChannelMute(const std::uint16_t channel, const bool mute) override;
    bool SetChannelGain(const std::uint16_t channel, const float gain) override;
    void SetConnectionState(const NanoAmpControlInterface::ConnectionState state) override;

protected:
    //==============================================================================
    bool ProcessReceivedOcp1Message(const juce::MemoryBlock& message);
    bool CreateObjectSubscriptions();
    bool QueryObjectValues();

    //==============================================================================
    void AddPendingSubscriptionHandle(const std::uint32_t handle);
    bool PopPendingSubscriptionHandle(const std::uint32_t handle);
    bool HasPendingSubscriptions();
    //==============================================================================
    void AddPendingGetValueHandle(const std::uint32_t handle, const std::uint32_t ONo);
    const std::uint32_t PopPendingGetValueHandle(const std::uint32_t handle);
    bool HasPendingGetValues();

private:
    //==============================================================================
    bool UpdateObjectValues(const NanoOcp1::Ocp1Notification* notifObj);
    bool UpdateObjectValues(const std::uint32_t ONo, const NanoOcp1::Ocp1Response* responseObj);

    //==============================================================================
    std::unique_ptr<NanoOcp1::NanoOcp1Client>   m_nanoOcp1Client;
    ConnectionState                             m_connectionState{ Unknown };
    std::vector<std::uint32_t>                  m_pendingSubscriptionHandles;
    std::map<std::uint32_t, std::uint32_t>      m_pendingGetValueHandlesWithONo;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NanoAmpControlProcessor)
};

}
