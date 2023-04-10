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

#include "NanoAmpControlProcessor.h"

#include <NanoOcp1.h>
#include <Ocp1ObjectDefinitions.h>


namespace NanoAmpControl
{


//==============================================================================
NanoAmpControlProcessor::NanoAmpControlProcessor(const std::uint16_t ampChannelCount)
    : NanoAmpControlInterface(ampChannelCount)
{
	auto address = juce::String("127.0.0.1");
	auto port = 50014;

	m_nanoOcp1Client = std::make_unique<NanoOcp1::NanoOcp1Client>(address, port);
	m_nanoOcp1Client->onDataReceived = [=](const juce::MemoryBlock& data) {
        return ProcessReceivedOcp1Message(data);
	};
	m_nanoOcp1Client->onConnectionEstablished = [=]() {
        CreateObjectSubscriptions();
		return;
	};
	m_nanoOcp1Client->onConnectionLost = [=]() {
		DBG("onConnectionLost");
		return;
	};
	m_nanoOcp1Client->start();
}

NanoAmpControlProcessor::~NanoAmpControlProcessor()
{
	if (m_nanoOcp1Client)
		m_nanoOcp1Client->stop();
}

bool NanoAmpControlProcessor::UpdateConnectionParameters(const juce::String& address, const std::uint16_t port)
{
	auto success = true;
	success = success && m_nanoOcp1Client->stop();
	m_nanoOcp1Client->setAddress(address);
	m_nanoOcp1Client->setPort(port);
	success = success && m_nanoOcp1Client->start();
	return success;
}

bool NanoAmpControlProcessor::ProcessReceivedOcp1Message(const juce::MemoryBlock& message)
{
    std::unique_ptr<NanoOcp1::Ocp1Message> msgObj = NanoOcp1::Ocp1Message::UnmarshalOcp1Message(message);
    if (msgObj)
    {
        switch (msgObj->GetMessageType())
        {
        case NanoOcp1::Ocp1Message::Notification:
        {
            NanoOcp1::Ocp1Notification* notifObj = static_cast<NanoOcp1::Ocp1Notification*>(msgObj.get());

            DBG("Got an OCA notification message");

            // Update the right GUI element according to the definition of the object 
            // which triggered the notification.

            // Objects without any further addressing
            if (notifObj->MatchesObject(NanoOcp1::GetONo(1, 0, 0, NanoOcp1::BoxAndObjNo::Settings_PwrOn)))
            {
                std::uint16_t switchSetting = NanoOcp1::DataToUint16(notifObj->GetParameterData());

                if (onPwrOnOff)
                    onPwrOnOff(switchSetting > 0);

                return true;
            }

            // Objects with additional channel addressing dimension
            for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
            {
                if (notifObj->MatchesObject(NanoOcp1::GetONo(1, 0, ch, NanoOcp1::BoxAndObjNo::Config_PotiLevel)))
                {
                    std::float_t newGain = NanoOcp1::DataToFloat(notifObj->GetParameterData());

                    if (onChannelGain)
                        onChannelGain(ch, newGain);

                    return true;
                }
                else if (notifObj->MatchesObject(NanoOcp1::GetONo(1, 0, ch, NanoOcp1::BoxAndObjNo::Config_Mute)))
                {
                    std::uint16_t switchSetting = NanoOcp1::DataToUint16(notifObj->GetParameterData());

                    if (onChannelMute)
                        onChannelMute(ch, switchSetting == 1);

                    return true;
                }

                //// Objects with additional record addressing dimension
                //for (std::uint16_t rec = 1; rec <= GetAmpRecordCount(); ch++)
                //{
                //}
            }

            return true;
        }
        case NanoOcp1::Ocp1Message::Response:
        {
            NanoOcp1::Ocp1Response* responseObj = static_cast<NanoOcp1::Ocp1Response*>(msgObj.get());

            if (responseObj->GetResponseStatus() != 0)
            {
                DBG("Got an OCA response for handle " << juce::String(responseObj->GetResponseHandle()) <<
                    " with status " << NanoOcp1::StatusToString(responseObj->GetResponseStatus()));
            }

            return true;
        }
        case NanoOcp1::Ocp1Message::KeepAlive:
        {
            // Reset online timer
            DBG("Got an OCA keepalive message");

            return true;
        }
        default:
            break;
        }
    }

    return false;
}

bool NanoAmpControlProcessor::CreateObjectSubscriptions()
{
    if (!m_nanoOcp1Client || !m_nanoOcp1Client->isConnected())
        return false;

    bool success = true;

    std::uint32_t handle = 0;

    NanoOcp1::Ocp1CommandParameters cmdDef(NanoOcp1::dbOcaObjectDef_Dy_AddSubscription_Settings_PwrOn);
    // subscribe pwrOn
    success = success && m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(cmdDef, handle).GetMemoryBlock());

    cmdDef = NanoOcp1::dbOcaObjectDef_Dy_AddSubscription_Config_PotiLevel;
    // subscribe potilevel for all channels
    for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
    {
        cmdDef.parameterData = NanoOcp1::DataFromOnoForSubscription(NanoOcp1::GetONo(1, 0, ch, NanoOcp1::BoxAndObjNo::Config_PotiLevel));
        success = success && m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(cmdDef, handle).GetMemoryBlock());
    }

    cmdDef = NanoOcp1::dbOcaObjectDef_Dy_AddSubscription_Config_Mute;
    // subscribe mute for all channels
    for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
    {
        cmdDef.parameterData = NanoOcp1::DataFromOnoForSubscription(NanoOcp1::GetONo(1, 0, ch, NanoOcp1::BoxAndObjNo::Config_Mute));
        success = success && m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(cmdDef, handle).GetMemoryBlock());
    }

    return success;
}

bool NanoAmpControlProcessor::SetPwrOnOff(const bool on)
{
    std::uint32_t handle;
    NanoOcp1::Ocp1CommandParameters cmdDef(NanoOcp1::dbOcaObjectDef_Dy_Settings_PwrOn);
    cmdDef.parameterData = NanoOcp1::DataFromUint16(static_cast<std::uint16_t>(on ? 1 : 0));

    if (m_nanoOcp1Client && m_nanoOcp1Client->isConnected())
        return m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(cmdDef, handle).GetMemoryBlock());
    
    return false;
}

bool NanoAmpControlProcessor::SetChannelMute(const std::uint16_t channel, const bool mute)
{
    std::uint32_t handle;
    NanoOcp1::Ocp1CommandParameters cmdDef(NanoOcp1::dbOcaObjectDef_Dy_Config_Mute);
    cmdDef.targetOno = NanoOcp1::GetONo(1, 0, channel, NanoOcp1::BoxAndObjNo::Config_Mute),
    cmdDef.parameterData = NanoOcp1::DataFromUint8(static_cast<std::uint8_t>(mute ? 1 : 2));
    
    if (m_nanoOcp1Client && m_nanoOcp1Client->isConnected())
        return m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(cmdDef, handle).GetMemoryBlock());

    return false;
}

bool NanoAmpControlProcessor::SetChannelGain(const std::uint16_t channel, const float gain)
{
	std::uint32_t handle;
	NanoOcp1::Ocp1CommandParameters cmdDef(NanoOcp1::dbOcaObjectDef_Dy_Config_PotiLevel);
    cmdDef.targetOno = NanoOcp1::GetONo(1, 0, channel, NanoOcp1::BoxAndObjNo::Config_PotiLevel),
	cmdDef.parameterData = NanoOcp1::DataFromFloat(static_cast<std::float_t>(gain));

	if (m_nanoOcp1Client && m_nanoOcp1Client->isConnected())
		return m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(cmdDef, handle).GetMemoryBlock());

    return false;
}


} // namespace SurroundFieldMixer
