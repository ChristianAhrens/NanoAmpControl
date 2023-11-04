/* Copyright (c) 2023, Christian Ahrens
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
    if (GetAmpType() == AmpType::_5D)
        cmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_Settings_PwrOn();
    else if (GetAmpType() == AmpType::_Dx || GetAmpType() == AmpType::_Dy)
        cmdDef = NanoOcp1::AmpDxDy::dbOcaObjectDef_Settings_PwrOn();
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
        auto potCmdDef = NanoOcp1::AmpGeneric::dbOcaObjectDef_Config_PotiLevel(ch);
        auto muteCmdDef = NanoOcp1::AmpGeneric::dbOcaObjectDef_Config_Mute(ch);

        auto ispCmdDef = NanoOcp1::Ocp1CommandDefinition();
        auto grCmdDef = NanoOcp1::Ocp1CommandDefinition();
        auto ovlCmdDef = NanoOcp1::Ocp1CommandDefinition();
        auto headCmdDef = NanoOcp1::Ocp1CommandDefinition();
        switch (GetAmpType())
        {
        case AmpType::_5D:
            {
                ispCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_Isp(ch);
                grCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_Gr(ch);
                ovlCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_Ovl(ch);
                headCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_GrHead(ch);
            }
            break;
        case AmpType::_Dx:
            {
                ispCmdDef = NanoOcp1::AmpDxDy::dbOcaObjectDef_ChStatus_Isp(ch);
                grCmdDef = NanoOcp1::AmpDxDy::dbOcaObjectDef_ChStatus_Gr(ch);
                ovlCmdDef = NanoOcp1::AmpDxDy::dbOcaObjectDef_ChStatus_Ovl(ch);
                headCmdDef = NanoOcp1::AmpDx::dbOcaObjectDef_ChStatus_GrHead(ch);
            }
            break;
        case AmpType::_Dy:
            {
                ispCmdDef = NanoOcp1::AmpDxDy::dbOcaObjectDef_ChStatus_Isp(ch);
                grCmdDef = NanoOcp1::AmpDxDy::dbOcaObjectDef_ChStatus_Gr(ch);
                ovlCmdDef = NanoOcp1::AmpDxDy::dbOcaObjectDef_ChStatus_Ovl(ch);
                headCmdDef = NanoOcp1::AmpDy::dbOcaObjectDef_ChStatus_GrHead(ch);
            }
            break;
        default:
            break;
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
            std::uint16_t switchSetting = NanoOcp1::DataToUint8(notifObj->GetParameterData());

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
            std::uint16_t switchSetting = NanoOcp1::DataToUint8(notifObj->GetParameterData());

            if (onChannelISP)
                onChannelISP(ch, switchSetting == 1);

            return true;
        }
        else if (notifObj->MatchesObject(&grCmdDef))
        {
            std::uint16_t switchSetting = NanoOcp1::DataToUint8(notifObj->GetParameterData());

            if (onChannelGR)
                onChannelGR(ch, switchSetting == 1);

            return true;
        }
        else if (notifObj->MatchesObject(&ovlCmdDef))
        {
            std::uint16_t switchSetting = NanoOcp1::DataToUint8(notifObj->GetParameterData());
            
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
    if (((GetAmpType() == AmpType::_Dx || GetAmpType() == AmpType::_Dy) && ONo == NanoOcp1::GetONo(1, 0, 0, NanoOcp1::AmpDxDy::Settings_PwrOn))
        || (GetAmpType() == AmpType::_5D && ONo == NanoOcp1::GetONo(1, 0, 0, NanoOcp1::Amp5D::Settings_PwrOn)))
    {
        std::uint16_t switchSetting = NanoOcp1::DataToUint16(responseObj->GetParameterData());
    
        if (onPwrOnOff)
            onPwrOnOff(switchSetting > 0);
    
        return true;
    }
    
    // Objects with additional channel addressing dimension
    for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
    {
        if (ONo == NanoOcp1::GetONo(1, 0, ch, NanoOcp1::AmpGeneric::Config_PotiLevel))
        {
            std::float_t newGain = NanoOcp1::DataToFloat(responseObj->GetParameterData());
    
            if (onChannelGain)
                onChannelGain(ch, newGain);
    
            return true;
        }
        else if (ONo == NanoOcp1::GetONo(1, 0, ch, NanoOcp1::AmpGeneric::Config_Mute))
        {
            std::uint16_t switchSetting = NanoOcp1::DataToUint8(responseObj->GetParameterData());

            if (onChannelMute)
                onChannelMute(ch, switchSetting == 1);
    
            return true;
        }
        else if (((GetAmpType() == AmpType::_Dx || GetAmpType() == AmpType::_Dy) && ONo == NanoOcp1::GetONo(1, 0, ch, NanoOcp1::AmpDxDy::ChStatus_Isp))
            || (GetAmpType() == AmpType::_5D && ONo == NanoOcp1::GetONo(1, 0, ch, NanoOcp1::Amp5D::ChStatus_Isp)))
        {
            std::uint16_t switchSetting = NanoOcp1::DataToUint8(responseObj->GetParameterData());

            if (onChannelISP)
                onChannelISP(ch, switchSetting == 1);

            return true;
        }
        else if (((GetAmpType() == AmpType::_Dx || GetAmpType() == AmpType::_Dy) && ONo == NanoOcp1::GetONo(1, 0, ch, NanoOcp1::AmpDxDy::ChStatus_Gr))
            || (GetAmpType() == AmpType::_5D && ONo == NanoOcp1::GetONo(1, 0, ch, NanoOcp1::Amp5D::ChStatus_Gr)))
        {
            std::uint16_t switchSetting = NanoOcp1::DataToUint8(responseObj->GetParameterData());

            if (onChannelGR)
                onChannelGR(ch, switchSetting == 1);

            return true;
        }
        else if (((GetAmpType() == AmpType::_Dx || GetAmpType() == AmpType::_Dy) && ONo == NanoOcp1::GetONo(1, 0, ch, NanoOcp1::AmpDxDy::ChStatus_Ovl))
            || (GetAmpType() == AmpType::_5D && ONo == NanoOcp1::GetONo(1, 0, ch, NanoOcp1::Amp5D::ChStatus_Ovl)))
        {
            std::uint16_t switchSetting = NanoOcp1::DataToUint8(responseObj->GetParameterData());
            
            if (onChannelOVL)
                onChannelOVL(ch, switchSetting == 1);

            return true;
        }
        else if ((GetAmpType() == AmpType::_Dx && ONo == NanoOcp1::GetONo(1, 0, ch, NanoOcp1::AmpDx::ChStatus_GrHead))
            || (GetAmpType() == AmpType::_Dy && ONo == NanoOcp1::GetONo(1, 0, ch, NanoOcp1::AmpDy::ChStatus_GrHead))
            || (GetAmpType() == AmpType::_5D && ONo == NanoOcp1::GetONo(1, 0, ch, NanoOcp1::Amp5D::ChStatus_GrHead)))
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
    if (GetAmpType() == AmpType::_5D)
        pwrOnCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_Settings_PwrOn().AddSubscriptionCommand();
    else if (GetAmpType() == AmpType::_Dx || GetAmpType() == AmpType::_Dy)
        pwrOnCmdDef = NanoOcp1::AmpDxDy::dbOcaObjectDef_Settings_PwrOn().AddSubscriptionCommand();
    success = m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(pwrOnCmdDef, handle).GetMemoryBlock()) && success;
    AddPendingSubscriptionHandle(handle);

    for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
    {
        // subscribe potilevel for all channels
        success = m_nanoOcp1Client->sendData(
            NanoOcp1::Ocp1CommandResponseRequired(
                NanoOcp1::AmpGeneric::dbOcaObjectDef_Config_PotiLevel(ch).AddSubscriptionCommand(), handle).GetMemoryBlock()) && success;
        AddPendingSubscriptionHandle(handle);

        // subscribe mute for all channels
        success = m_nanoOcp1Client->sendData(
            NanoOcp1::Ocp1CommandResponseRequired(
                NanoOcp1::AmpGeneric::dbOcaObjectDef_Config_Mute(ch).AddSubscriptionCommand(), handle).GetMemoryBlock()) && success;
        AddPendingSubscriptionHandle(handle);

        // subscribe isp for all channels
        auto ispCmdDef = NanoOcp1::Ocp1CommandDefinition();
        if (GetAmpType() == AmpType::_5D)
            ispCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_Isp(ch).AddSubscriptionCommand();
        else if (GetAmpType() == AmpType::_Dx || GetAmpType() == AmpType::_Dy)
            ispCmdDef = NanoOcp1::AmpDxDy::dbOcaObjectDef_ChStatus_Isp(ch).AddSubscriptionCommand();
        success = m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(ispCmdDef, handle).GetMemoryBlock()) && success;
        AddPendingSubscriptionHandle(handle);

        // subscribe gr for all channels
        auto grCmdDef = NanoOcp1::Ocp1CommandDefinition();
        if (GetAmpType() == AmpType::_5D)
            grCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_Gr(ch).AddSubscriptionCommand();
        else if (GetAmpType() == AmpType::_Dx || GetAmpType() == AmpType::_Dy)
            grCmdDef = NanoOcp1::AmpDxDy::dbOcaObjectDef_ChStatus_Gr(ch).AddSubscriptionCommand();
        success = m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(grCmdDef, handle).GetMemoryBlock()) && success;
        AddPendingSubscriptionHandle(handle);

        // subscribe ovl for all channels
        auto ovlCmdDef = NanoOcp1::Ocp1CommandDefinition();
        if (GetAmpType() == AmpType::_5D)
            ovlCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_Ovl(ch).AddSubscriptionCommand();
        else if (GetAmpType() == AmpType::_Dx || GetAmpType() == AmpType::_Dy)
            ovlCmdDef = NanoOcp1::AmpDxDy::dbOcaObjectDef_ChStatus_Ovl(ch).AddSubscriptionCommand();
        success = m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(ovlCmdDef, handle).GetMemoryBlock()) && success;
        AddPendingSubscriptionHandle(handle);

        // subscribe headroom for all channels
        auto headCmdDef = NanoOcp1::Ocp1CommandDefinition();
        if (GetAmpType() == AmpType::_5D)
            headCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_GrHead(ch).AddSubscriptionCommand();
        else if (GetAmpType() == AmpType::_Dx)
            headCmdDef = NanoOcp1::AmpDx::dbOcaObjectDef_ChStatus_GrHead(ch).AddSubscriptionCommand();
        else if (GetAmpType() == AmpType::_Dy)
            headCmdDef = NanoOcp1::AmpDy::dbOcaObjectDef_ChStatus_GrHead(ch).AddSubscriptionCommand();
        success = m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(headCmdDef, handle).GetMemoryBlock()) && success;
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
    if (GetAmpType() == AmpType::_5D)
        pwrOnCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_Settings_PwrOn();
    else if (GetAmpType() == AmpType::_Dx || GetAmpType() == AmpType::_Dx)
        pwrOnCmdDef = NanoOcp1::AmpDxDy::dbOcaObjectDef_Settings_PwrOn();
    success = m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(pwrOnCmdDef, handle).GetMemoryBlock()) && success;
    AddPendingGetValueHandle(handle, pwrOnCmdDef.m_targetOno);

    for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
    {
        // query potilevel for all channels
        auto potCmdDef = NanoOcp1::AmpGeneric::dbOcaObjectDef_Config_PotiLevel(ch);
        success = m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(potCmdDef, handle).GetMemoryBlock()) && success;
        AddPendingGetValueHandle(handle, potCmdDef.m_targetOno);

        // query mute for all channels
        auto muteCmdDef = NanoOcp1::AmpGeneric::dbOcaObjectDef_Config_Mute(ch);
        success = m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(muteCmdDef, handle).GetMemoryBlock()) && success;
        AddPendingGetValueHandle(handle, muteCmdDef.m_targetOno);

        // query isp for all channels
        auto ispCmdDef = NanoOcp1::Ocp1CommandDefinition();
        if (GetAmpType() == AmpType::_5D)
            ispCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_Isp(ch);
        else if (GetAmpType() == AmpType::_Dx || GetAmpType() == AmpType::_Dy)
            ispCmdDef = NanoOcp1::AmpDxDy::dbOcaObjectDef_ChStatus_Isp(ch);
        success = m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(ispCmdDef, handle).GetMemoryBlock()) && success;
        AddPendingGetValueHandle(handle, ispCmdDef.m_targetOno);

        // query gr for all channels
        auto grCmdDef = NanoOcp1::Ocp1CommandDefinition();
        if (GetAmpType() == AmpType::_5D)
            grCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_Gr(ch);
        else if (GetAmpType() == AmpType::_Dx || GetAmpType() == AmpType::_Dy)
            grCmdDef = NanoOcp1::AmpDxDy::dbOcaObjectDef_ChStatus_Gr(ch);
        success = m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(grCmdDef, handle).GetMemoryBlock()) && success;
        AddPendingGetValueHandle(handle, grCmdDef.m_targetOno);

        // query ovl for all channels
        auto ovlCmdDef = NanoOcp1::Ocp1CommandDefinition();
        if (GetAmpType() == AmpType::_5D)
            ovlCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_Ovl(ch);
        else if (GetAmpType() == AmpType::_Dx || GetAmpType() == AmpType::_Dy)
            ovlCmdDef = NanoOcp1::AmpDxDy::dbOcaObjectDef_ChStatus_Ovl(ch);
        success = m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(ovlCmdDef, handle).GetMemoryBlock()) && success;
        AddPendingGetValueHandle(handle, ovlCmdDef.m_targetOno);

        // query headroom for all channels
        auto headCmdDef = NanoOcp1::Ocp1CommandDefinition();
        if (GetAmpType() == AmpType::_5D)
            headCmdDef = NanoOcp1::Amp5D::dbOcaObjectDef_ChStatus_GrHead(ch);
        else if (GetAmpType() == AmpType::_Dx)
            headCmdDef = NanoOcp1::AmpDx::dbOcaObjectDef_ChStatus_GrHead(ch);
        else if (GetAmpType() == AmpType::_Dy)
            headCmdDef = NanoOcp1::AmpDy::dbOcaObjectDef_ChStatus_GrHead(ch);
        success = m_nanoOcp1Client->sendData(NanoOcp1::Ocp1CommandResponseRequired(headCmdDef, handle).GetMemoryBlock()) && success;
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
        if (GetAmpType() == AmpType::_5D)
        {
            return m_nanoOcp1Client->sendData(
                NanoOcp1::Ocp1CommandResponseRequired(
                    NanoOcp1::Amp5D::dbOcaObjectDef_Settings_PwrOn().SetValueCommand(on ? 1 : 0), handle).GetMemoryBlock());
        }
        else if (GetAmpType() == AmpType::_Dx || GetAmpType() == AmpType::_Dy)
        {
            return m_nanoOcp1Client->sendData(
                NanoOcp1::Ocp1CommandResponseRequired(
                    NanoOcp1::AmpDxDy::dbOcaObjectDef_Settings_PwrOn().SetValueCommand(on ? 1 : 0), handle).GetMemoryBlock());
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
                NanoOcp1::AmpGeneric::dbOcaObjectDef_Config_Mute(channel).SetValueCommand(mute ? 1: 2), handle).GetMemoryBlock());

    return false;
}

bool NanoAmpControlProcessor::SetChannelGain(const std::uint16_t channel, const float gain)
{
	std::uint32_t handle;
	if (m_nanoOcp1Client && m_nanoOcp1Client->isConnected())
		return m_nanoOcp1Client->sendData(
            NanoOcp1::Ocp1CommandResponseRequired(
                NanoOcp1::AmpGeneric::dbOcaObjectDef_Config_PotiLevel(channel).SetValueCommand(gain), handle).GetMemoryBlock());

    return false;
}


} // namespace NanoAmpControl
