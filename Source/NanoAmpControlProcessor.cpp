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


namespace NanoAmpControl
{


//==============================================================================
NanoAmpControlProcessor::NanoAmpControlProcessor()
{
	auto address = juce::String("127.0.0.1");
	auto port = 50014;

	m_nanoOcp1Client = std::make_unique<NanoOcp1::NanoOcp1Client>(address, port);
	m_nanoOcp1Client->onDataReceived = [=](const juce::MemoryBlock& data) {
		DBG("onDataReceived - dl " + juce::String(data.getSize()));
		return true;
	};
	m_nanoOcp1Client->onConnectionEstablished = [=]() {
		DBG("onConnectionEstablished");
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
	m_nanoOcp1Client->stop();
}

bool NanoAmpControlProcessor::UpdateConnectionParameters(const juce::String& address, const int port)
{
	auto success = true;
	success = success && m_nanoOcp1Client->stop();
	m_nanoOcp1Client->setAddress(address);
	m_nanoOcp1Client->setPort(port);
	success = success && m_nanoOcp1Client->start();
	return success;
}


} // namespace SurroundFieldMixer
