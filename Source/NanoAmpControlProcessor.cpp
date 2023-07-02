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
        SetConnectionState(Connected);
        CreateObjectSubscriptions();
        QueryObjectValues();
		return;
	};
	m_nanoOcp1Client->onConnectionLost = [=]() {
        SetConnectionState(Disconnected);
		return;
	};
	m_nanoOcp1Client->start();
}

NanoAmpControlProcessor::~NanoAmpControlProcessor()
{
	if (m_nanoOcp1Client)
		m_nanoOcp1Client->stop();
}

bool NanoAmpControlProcessor::UpdateConnectionParameters(const juce::String& address, const std::uint16_t port, const AmpType ampType)
{
	auto success = true;
	success = success && m_nanoOcp1Client->stop();
    SetAmpType(ampType);
	m_nanoOcp1Client->setAddress(address);
	m_nanoOcp1Client->setPort(port);
	success = success && m_nanoOcp1Client->start();
	return success;
}

void NanoAmpControlProcessor::SetConnectionState(const ConnectionState state)
{
    if (m_connectionState != state)
    {
        m_connectionState = state;

        if (onConnectionStateChanged)
            onConnectionStateChanged(state);
    }
}

void NanoAmpControlProcessor::SetAmpType(const NanoAmpControlInterface::AmpType ampType)
{
    m_ampType = ampType;
}

NanoAmpControlInterface::AmpType NanoAmpControlProcessor::GetAmpType()
{
    return m_ampType;
}

bool NanoAmpControlProcessor::ProcessReceivedOcp1Message(const juce::MemoryBlock& message)
{
    std::unique_ptr<NanoOcp1::Ocp1Message> msgObj = NanoOcp1::Ocp1Message::UnmarshalOcp1Message(message);
    if (msgObj)
    {
        if (m_connectionState < Active)
            SetConnectionState(Active);

        switch (msgObj->GetMessageType())
        {
        case NanoOcp1::Ocp1Message::Notification:
        {
            NanoOcp1::Ocp1Notification* notifObj = static_cast<NanoOcp1::Ocp1Notification*>(msgObj.get());

            if (!UpdateObjectValues(notifObj))
                DBG("Got an unhandled OCA notification message");

            return true;
        }
        case NanoOcp1::Ocp1Message::Response:
        {
            NanoOcp1::Ocp1Response* responseObj = static_cast<NanoOcp1::Ocp1Response*>(msgObj.get());

            auto handle = responseObj->GetResponseHandle();
            if (responseObj->GetResponseStatus() != 0)
            {
                DBG("Got an OCA response for handle " << juce::String(handle) <<
                    " with status " << NanoOcp1::StatusToString(responseObj->GetResponseStatus()));
            }
            else if (PopPendingSubscriptionHandle(handle) && !HasPendingSubscriptions())
            {
                // All subscriptions were confirmed
                if (onConnectionStateChanged)
                    onConnectionStateChanged(Subscribed);
            }
            else
            {
                auto ONo = PopPendingGetValueHandle(handle);
                if (0x00 != ONo)
                {
                    if (!UpdateObjectValues(ONo, responseObj))
                        DBG("Got an unhandled OCA getvalue response message");
                }
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

bool NanoAmpControlProcessor::UpdateObjectValues(const NanoOcp1::Ocp1Notification* notifObj)
{
    // Update the right GUI element according to the definition of the object 
    // which triggered the notification.

    // Objects without any further addressing
    auto cmdDef = NanoOcp1::Ocp1CommandDefinition();
    if (GetAmpType() == AmpType::Amp5D)
        cmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_Settings_PwrOn();
    else if (GetAmpType() == AmpType::DxDy)
        cmdDef = NanoOcp1::DxDy::dbOcaObjectDef_Settings_PwrOn();
    if (notifObj->MatchesObject(&cmdDef))
    {
        std::uint16_t switchSetting = NanoOcp1::DataToUint16(notifObj->GetParameterData());

        if (onPwrOnOff)
            onPwrOnOff(switchSetting > 0);

        return true;
    }

    // Objects with additional channel addressing dimension
    for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
    {
        auto potCmdDef = NanoOcp1::DxDy::dbOcaObjectDef_Config_PotiLevel(ch);
        auto muteCmdDef = NanoOcp1::DxDy::dbOcaObjectDef_Config_Mute(ch);

        auto ispCmdDef = NanoOcp1::Ocp1CommandDefinition();
        auto grCmdDef = NanoOcp1::Ocp1CommandDefinition();
        auto ovlCmdDef = NanoOcp1::Ocp1CommandDefinition();
        auto headCmdDef = NanoOcp1::Ocp1CommandDefinition();
        if (GetAmpType() == AmpType::Amp5D)
        {
            ispCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_Isp(ch);
            grCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_Gr(ch);
            ovlCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_Ovl(ch);
            headCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_GrHead(ch);
        }
        else if (GetAmpType() == AmpType::DxDy)
        {
            ispCmdDef = NanoOcp1::DxDy::dbOcaObjectDef_ChStatus_Isp(ch);
            grCmdDef = NanoOcp1::DxDy::dbOcaObjectDef_ChStatus_Gr(ch);
            ovlCmdDef = NanoOcp1::DxDy::dbOcaObjectDef_ChStatus_Ovl(ch);
            headCmdDef = NanoOcp1::DxDy::dbOcaObjectDef_ChStatus_GrHead(ch);
        }
        
        if (notifObj->MatchesObject(&potCmdDef))
        {
            std::float_t newGain = NanoOcp1::DataToFloat(notifObj->GetParameterData());

            if (onChannelGain)
                onChannelGain(ch, newGain);

            return true;
        }
        else if (notifObj->MatchesObject(&muteCmdDef))
        {
            std::uint16_t switchSetting = NanoOcp1::DataToUint16(notifObj->GetParameterData());

            if (onChannelMute)
                onChannelMute(ch, switchSetting == 1);

            return true;
        }
        else if (notifObj->MatchesObject(&headCmdDef))
        {
            std::float_t newHeadroom = NanoOcp1::DataToFloat(notifObj->GetParameterData());

            if (onChannelHeadroom)
                onChannelHeadroom(ch, newHeadroom);

            return true;
        }
        else if (notifObj->MatchesObject(&ispCmdDef))
        {
            std::uint16_t switchSetting = NanoOcp1::DataToUint16(notifObj->GetParameterData());

            if (onChannelISP)
                onChannelISP(ch, switchSetting == 1);

            return true;
        }
        else if (notifObj->MatchesObject(&grCmdDef))
        {
            std::uint16_t switchSetting = NanoOcp1::DataToUint16(notifObj->GetParameterData());

            if (onChannelGR)
                onChannelGR(ch, switchSetting == 1);

            return true;
        }
        else if (notifObj->MatchesObject(&ovlCmdDef))
        {
            std::uint16_t switchSetting = NanoOcp1::DataToUint16(notifObj->GetParameterData());
            
            if (onChannelOVL)
                onChannelOVL(ch, switchSetting == 1);

            return true;
        }
    }

    return false;
}

bool NanoAmpControlProcessor::UpdateObjectValues(const std::uint32_t ONo, const NanoOcp1::Ocp1Response* responseObj)
{
    // Update the right GUI element according to the definition of the object 
    // to which the response refers.
    
    // Objects without any further addressing
    if (ONo == NanoOcp1::GetONo(1, 0, 0, NanoOcp1::DxDy::Settings_PwrOn))
    {
        std::uint16_t switchSetting = NanoOcp1::DataToUint16(responseObj->GetParameterData());
    
        if (onPwrOnOff)
            onPwrOnOff(switchSetting > 0);
    
        return true;
    }
    
    // Objects with additional channel addressing dimension
    for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
    {
        if (ONo == NanoOcp1::GetONo(1, 0, ch, NanoOcp1::DxDy::Config_PotiLevel))
        {
            std::float_t newGain = NanoOcp1::DataToFloat(responseObj->GetParameterData());
    
            if (onChannelGain)
                onChannelGain(ch, newGain);
    
            return true;
        }
        else if (ONo == NanoOcp1::GetONo(1, 0, ch, NanoOcp1::DxDy::Config_Mute))
        {
            std::uint16_t switchSetting = NanoOcp1::DataToUint16(responseObj->GetParameterData());
    
            if (onChannelMute)
                onChannelMute(ch, switchSetting == 1);
    
            return true;
        }
        else if (ONo == NanoOcp1::GetONo(1, 0, ch, NanoOcp1::DxDy::ChStatus_Isp)
            || ONo == NanoOcp1::GetONo(1, 0, ch, NanoOcp1::Amp5D::ChStatus_Isp))
        {
            std::uint16_t switchSetting = NanoOcp1::DataToUint16(responseObj->GetParameterData());

            if (onChannelISP)
                onChannelISP(ch, switchSetting == 1);

            return true;
        }
        else if (ONo == NanoOcp1::GetONo(1, 0, ch, NanoOcp1::DxDy::ChStatus_Gr)
            || ONo == NanoOcp1::GetONo(1, 0, ch, NanoOcp1::Amp5D::ChStatus_Gr))
        {
            std::uint16_t switchSetting = NanoOcp1::DataToUint16(responseObj->GetParameterData());

            if (onChannelGR)
                onChannelGR(ch, switchSetting == 1);

            return true;
        }
        else if (ONo == NanoOcp1::GetONo(1, 0, ch, NanoOcp1::DxDy::ChStatus_Ovl)
            || ONo == NanoOcp1::GetONo(1, 0, ch, NanoOcp1::DxDy::ChStatus_Ovl))
        {
            std::uint16_t switchSetting = NanoOcp1::DataToUint16(responseObj->GetParameterData());
            
            if (onChannelOVL)
                onChannelOVL(ch, switchSetting == 1);

            return true;
        }
        else if (ONo == NanoOcp1::GetONo(1, ch, 0, NanoOcp1::DxDy::ChStatus_GrHead)
            || ONo == NanoOcp1::GetONo(1, ch, 0, NanoOcp1::Amp5D::ChStatus_GrHead))
        {
            std::float_t newHeadroom = NanoOcp1::DataToFloat(responseObj->GetParameterData());

            if (onChannelHeadroom)
                onChannelHeadroom(ch, newHeadroom);

            return true;
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

    // subscribe pwrOn
    auto pwrOnCmdDef = NanoOcp1::Ocp1CommandDefinition();
    if (GetAmpType() == AmpType::Amp5D)
        pwrOnCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_Settings_PwrOn().AddSubscriptionCommand();
    else if (GetAmpType() == AmpType::DxDy)
        pwrOnCmdDef = NanoOcp1::DxDy::dbOcaObjectDef_Settings_PwrOn().AddSubscriptionCommand();
    success = success && m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(pwrOnCmdDef, handle).GetMemoryBlock());
    AddPendingSubscriptionHandle(handle);

    // subscribe potilevel for all channels
    for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
    {
        success = success && m_nanoOcp1Client->sendData(
            NanoOcp1::Ocp1CommandResponseRequired(
                NanoOcp1::DxDy::dbOcaObjectDef_Config_PotiLevel(ch).AddSubscriptionCommand(), handle).GetMemoryBlock());
        AddPendingSubscriptionHandle(handle);
    }

    // subscribe mute for all channels
    for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
    {
        success = success && m_nanoOcp1Client->sendData(
            NanoOcp1::Ocp1CommandResponseRequired(
                NanoOcp1::DxDy::dbOcaObjectDef_Config_Mute(ch).AddSubscriptionCommand(), handle).GetMemoryBlock());
        AddPendingSubscriptionHandle(handle);
    }

    // subscribe isp for all channels
    for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
    {
        auto ispCmdDef = NanoOcp1::Ocp1CommandDefinition();
        if (GetAmpType() == AmpType::Amp5D)
            ispCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_Isp(ch).AddSubscriptionCommand();
        else if (GetAmpType() == AmpType::DxDy)
            ispCmdDef = NanoOcp1::DxDy::dbOcaObjectDef_ChStatus_Isp(ch).AddSubscriptionCommand();
        success = success && m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(ispCmdDef, handle).GetMemoryBlock());
        AddPendingSubscriptionHandle(handle);
    }

    // subscribe gr for all channels
    for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
    {
        auto grCmdDef = NanoOcp1::Ocp1CommandDefinition();
        if (GetAmpType() == AmpType::Amp5D)
            grCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_Gr(ch).AddSubscriptionCommand();
        else if (GetAmpType() == AmpType::DxDy)
            grCmdDef = NanoOcp1::DxDy::dbOcaObjectDef_ChStatus_Gr(ch).AddSubscriptionCommand();
        success = success && m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(grCmdDef, handle).GetMemoryBlock());
        AddPendingSubscriptionHandle(handle);
    }

    // subscribe ovl for all channels
    for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
    {
        auto ovlCmdDef = NanoOcp1::Ocp1CommandDefinition();
        if (GetAmpType() == AmpType::Amp5D)
            ovlCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_Ovl(ch).AddSubscriptionCommand();
        else if (GetAmpType() == AmpType::DxDy)
            ovlCmdDef = NanoOcp1::DxDy::dbOcaObjectDef_ChStatus_Ovl(ch).AddSubscriptionCommand();
        success = success && m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(ovlCmdDef, handle).GetMemoryBlock());
        AddPendingSubscriptionHandle(handle);
    }

    // subscribe headroom for all channels
    for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
    {
        auto headCmdDef = NanoOcp1::Ocp1CommandDefinition();
        if (GetAmpType() == AmpType::Amp5D)
            headCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_GrHead(ch).AddSubscriptionCommand();
        else if (GetAmpType() == AmpType::DxDy)
            headCmdDef = NanoOcp1::DxDy::dbOcaObjectDef_ChStatus_GrHead(ch).AddSubscriptionCommand();
        success = success && m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(headCmdDef, handle).GetMemoryBlock());
        AddPendingSubscriptionHandle(handle);
    }

    return success;
}

bool NanoAmpControlProcessor::QueryObjectValues()
{
    if (!m_nanoOcp1Client || !m_nanoOcp1Client->isConnected())
        return false;

    bool success = true;

    std::uint32_t handle = 0;

    // query pwrOn
    auto pwrOnCmdDef = NanoOcp1::Ocp1CommandDefinition();
    if (GetAmpType() == AmpType::Amp5D)
        pwrOnCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_Settings_PwrOn();
    else if (GetAmpType() == AmpType::DxDy)
        pwrOnCmdDef = NanoOcp1::DxDy::dbOcaObjectDef_Settings_PwrOn();
    success = success && m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(pwrOnCmdDef, handle).GetMemoryBlock());
    AddPendingGetValueHandle(handle, pwrOnCmdDef.m_targetOno);

    // query potilevel for all channels
    for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
    {
        auto potCmdDef = NanoOcp1::DxDy::dbOcaObjectDef_Config_PotiLevel(ch);
        success = success && m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(potCmdDef, handle).GetMemoryBlock());
        AddPendingGetValueHandle(handle, potCmdDef.m_targetOno);
    }

    // query mute for all channels
    for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
    {
        auto muteCmdDef = NanoOcp1::DxDy::dbOcaObjectDef_Config_Mute(ch);
        success = success && m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(muteCmdDef, handle).GetMemoryBlock());
        AddPendingGetValueHandle(handle, muteCmdDef.m_targetOno);
    }

    // query digital level for all channels
    for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
    {
        auto levelCmdDef = NanoOcp1::DxDy::dbOcaObjectDef_Input_Digital_Level(ch);
        success = success && m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(levelCmdDef, handle).GetMemoryBlock());
        AddPendingGetValueHandle(handle, levelCmdDef.m_targetOno);
    }

    // query isp for all channels
    for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
    {
        auto ispCmdDef = NanoOcp1::Ocp1CommandDefinition();
        if (GetAmpType() == AmpType::Amp5D)
            ispCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_Isp(ch);
        else if (GetAmpType() == AmpType::DxDy)
            ispCmdDef = NanoOcp1::DxDy::dbOcaObjectDef_ChStatus_Isp(ch);
        success = success && m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(ispCmdDef, handle).GetMemoryBlock());
        AddPendingGetValueHandle(handle, ispCmdDef.m_targetOno);
    }

    // query gr for all channels
    for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
    {
        auto grCmdDef = NanoOcp1::Ocp1CommandDefinition();
        if (GetAmpType() == AmpType::Amp5D)
            grCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_Gr(ch);
        else if (GetAmpType() == AmpType::DxDy)
            grCmdDef = NanoOcp1::DxDy::dbOcaObjectDef_ChStatus_Gr(ch);
        success = success && m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(grCmdDef, handle).GetMemoryBlock());
        AddPendingGetValueHandle(handle, grCmdDef.m_targetOno);
    }

    // query ovl for all channels
    for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
    {
        auto ovlCmdDef = NanoOcp1::Ocp1CommandDefinition();
        if (GetAmpType() == AmpType::Amp5D)
            ovlCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_Ovl(ch);
        else if (GetAmpType() == AmpType::DxDy)
            ovlCmdDef = NanoOcp1::DxDy::dbOcaObjectDef_ChStatus_Ovl(ch);
        success = success && m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(ovlCmdDef, handle).GetMemoryBlock());
        AddPendingGetValueHandle(handle, ovlCmdDef.m_targetOno);
    }

    // query headroom for all channels
    for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
    {
        auto headCmdDef = NanoOcp1::Ocp1CommandDefinition();
        if (GetAmpType() == AmpType::Amp5D)
            headCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_GrHead(ch);
        else if (GetAmpType() == AmpType::DxDy)
            headCmdDef = NanoOcp1::DxDy::dbOcaObjectDef_ChStatus_GrHead(ch);
        success = success && m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(headCmdDef, handle).GetMemoryBlock());
        AddPendingGetValueHandle(handle, headCmdDef.m_targetOno);
    }

    return success;
}

void NanoAmpControlProcessor::AddPendingSubscriptionHandle(const std::uint32_t handle)
{
    m_pendingSubscriptionHandles.push_back(handle);
}

bool NanoAmpControlProcessor::PopPendingSubscriptionHandle(const std::uint32_t handle)
{
    auto it = std::find(m_pendingSubscriptionHandles.begin(), m_pendingSubscriptionHandles.end(), handle);
    if (it != m_pendingSubscriptionHandles.end())
    {
        m_pendingSubscriptionHandles.erase(it);
        return true;
    }
    else
        return false;
}

bool NanoAmpControlProcessor::HasPendingSubscriptions()
{
    return m_pendingSubscriptionHandles.empty();
}

void NanoAmpControlProcessor::AddPendingGetValueHandle(const std::uint32_t handle, const std::uint32_t ONo)
{
    m_pendingGetValueHandlesWithONo.insert(std::make_pair(handle, ONo));
}

const std::uint32_t NanoAmpControlProcessor::PopPendingGetValueHandle(const std::uint32_t handle)
{
    auto it = std::find_if(m_pendingGetValueHandlesWithONo.begin(), m_pendingGetValueHandlesWithONo.end(), [handle](const auto& val) { return val.first == handle; });
    if (it != m_pendingGetValueHandlesWithONo.end())
    {
        auto ONo = it->second;
        m_pendingGetValueHandlesWithONo.erase(it);
        return ONo;
    }
    else
        return 0x00;
}

bool NanoAmpControlProcessor::HasPendingGetValues()
{
    return m_pendingGetValueHandlesWithONo.empty();
}

bool NanoAmpControlProcessor::SetPwrOnOff(const bool on)
{
    std::uint32_t handle;
    if (m_nanoOcp1Client && m_nanoOcp1Client->isConnected())
    {
        if (GetAmpType() == AmpType::Amp5D)
        {
            return m_nanoOcp1Client->sendData(
                NanoOcp1::Ocp1CommandResponseRequired(
                    NanoOcp1::Amp5D::dbOcaObjectDef_Settings_PwrOn().SetValueCommand(on ? 1 : 0), handle).GetMemoryBlock());
        }
        else if (GetAmpType() == AmpType::DxDy)
        {
            return m_nanoOcp1Client->sendData(
                NanoOcp1::Ocp1CommandResponseRequired(
                    NanoOcp1::DxDy::dbOcaObjectDef_Settings_PwrOn().SetValueCommand(on ? 1 : 0), handle).GetMemoryBlock());
        }
    }
    
    return false;
}

bool NanoAmpControlProcessor::SetChannelISP(const std::uint16_t, const bool)
{
    return false;
}

bool NanoAmpControlProcessor::SetChannelGR(const std::uint16_t, const bool)
{
    return false;
}

bool NanoAmpControlProcessor::SetChannelOVL(const std::uint16_t, const bool)
{
    return false;
}

bool NanoAmpControlProcessor::SetChannelHeadroom(const std::uint16_t, const float)
{
    return false;
}

bool NanoAmpControlProcessor::SetChannelMute(const std::uint16_t channel, const bool mute)
{
    std::uint32_t handle;
    if (m_nanoOcp1Client && m_nanoOcp1Client->isConnected())
        return m_nanoOcp1Client->sendData(
            NanoOcp1::Ocp1CommandResponseRequired(
                NanoOcp1::DxDy::dbOcaObjectDef_Config_Mute(channel).SetValueCommand(mute ? 1: 2), handle).GetMemoryBlock());

    return false;
}

bool NanoAmpControlProcessor::SetChannelGain(const std::uint16_t channel, const float gain)
{
	std::uint32_t handle;
	if (m_nanoOcp1Client && m_nanoOcp1Client->isConnected())
		return m_nanoOcp1Client->sendData(
            NanoOcp1::Ocp1CommandResponseRequired(
                NanoOcp1::DxDy::dbOcaObjectDef_Config_PotiLevel(channel).SetValueCommand(gain), handle).GetMemoryBlock());

    return false;
}


} // namespace SurroundFieldMixer
