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
    m_NanoAmpControlProcessor = std::make_unique<NanoAmpControlProcessor>();
    m_NanoAmpControlUI = std::make_unique<NanoAmpControlUI>();
    m_NanoAmpControlUI->onConnectionParametersEdited = [=](const juce::String& address, const int port) {
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
