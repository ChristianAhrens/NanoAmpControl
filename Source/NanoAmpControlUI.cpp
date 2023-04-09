/* Copyright (c) 2022, Christian Ahrens
 *
 * This file is part of SurroundFieldMixer <https://github.com/ChristianAhrens/SurroundFieldMixer>
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

#include "NanoAmpControlUI.h"

#include <ZeroconfDiscoverComponent.h>


namespace NanoAmpControl
{

//==============================================================================
NanoAmpControlUI::NanoAmpControlUI()
    : juce::Component()
{
	auto address = juce::String("127.0.0.1");
	auto port = 50014;

	m_ipAndPortEditor = std::make_unique<TextEditor>();
	m_ipAndPortEditor->setTextToShowWhenEmpty(address + ";" + juce::String(port), getLookAndFeel().findColour(juce::TextEditor::ColourIds::textColourId).darker().darker());
	m_ipAndPortEditor->addListener(this);
	addAndMakeVisible(m_ipAndPortEditor.get());

	m_zeroconfDiscoverButton = std::make_unique<JUCEAppBasics::ZeroconfDiscoverComponent>(false, false);
	m_zeroconfDiscoverButton->onServiceSelected = [=](JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType type, ZeroconfSearcher::ZeroconfSearcher::ServiceInfo* info) {
		ignoreUnused(type);
		if (m_ipAndPortEditor)
			m_ipAndPortEditor->setText(juce::String(info->ip) + ";" + juce::String(info->port));
		if (onConnectionParametersEdited)
			onConnectionParametersEdited(juce::String(info->ip), info->port);
	};
	m_zeroconfDiscoverButton->clearServices();
	m_zeroconfDiscoverButton->addDiscoverService(JUCEAppBasics::ZeroconfDiscoverComponent::ZST_OCA);
	addAndMakeVisible(m_zeroconfDiscoverButton.get());

	m_AmpPowerOnButton = std::make_unique<TextButton>();
	m_AmpPowerOnButton->setClickingTogglesState(true);
	m_AmpPowerOnButton->setButtonText("PwrON");
	m_AmpPowerOnButton->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::forestgreen);
	m_AmpPowerOnButton->addListener(this);
	addAndMakeVisible(m_AmpPowerOnButton.get());

	m_AmpGainSliderCh1 = std::make_unique<Slider>();
	m_AmpGainSliderCh1->setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
	m_AmpGainSliderCh1->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 15);
	m_AmpGainSliderCh1->addListener(this);
	addAndMakeVisible(m_AmpGainSliderCh1.get());
	m_AmpGainSliderCh2 = std::make_unique<Slider>();
	m_AmpGainSliderCh2->setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
	m_AmpGainSliderCh2->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 15);
	m_AmpGainSliderCh2->addListener(this);
	addAndMakeVisible(m_AmpGainSliderCh2.get());
	m_AmpGainSliderCh3 = std::make_unique<Slider>();
	m_AmpGainSliderCh3->setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
	m_AmpGainSliderCh3->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 15);
	m_AmpGainSliderCh3->addListener(this);
	addAndMakeVisible(m_AmpGainSliderCh3.get());
	m_AmpGainSliderCh4 = std::make_unique<Slider>();
	m_AmpGainSliderCh4->setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
	m_AmpGainSliderCh4->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, false, 60, 15);
	m_AmpGainSliderCh4->addListener(this);
	addAndMakeVisible(m_AmpGainSliderCh4.get());

	m_AmpMuteButtonCh1 = std::make_unique<TextButton>();
	m_AmpMuteButtonCh1->setClickingTogglesState(true);
	m_AmpMuteButtonCh1->setButtonText("MUTE Ch1");
	m_AmpMuteButtonCh1->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::indianred);
	m_AmpMuteButtonCh1->addListener(this);
	addAndMakeVisible(m_AmpMuteButtonCh1.get());
	m_AmpMuteButtonCh2 = std::make_unique<TextButton>();
	m_AmpMuteButtonCh2->setClickingTogglesState(true);
	m_AmpMuteButtonCh2->setButtonText("MUTE Ch2");
	m_AmpMuteButtonCh2->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::red);
	m_AmpMuteButtonCh2->addListener(this);
	addAndMakeVisible(m_AmpMuteButtonCh2.get());
	m_AmpMuteButtonCh3 = std::make_unique<TextButton>();
	m_AmpMuteButtonCh3->setClickingTogglesState(true);
	m_AmpMuteButtonCh3->setButtonText("MUTE Ch3");
	m_AmpMuteButtonCh3->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::red);
	m_AmpMuteButtonCh3->addListener(this);
	addAndMakeVisible(m_AmpMuteButtonCh3.get());
	m_AmpMuteButtonCh4 = std::make_unique<TextButton>();
	m_AmpMuteButtonCh4->setClickingTogglesState(true);
	m_AmpMuteButtonCh4->setButtonText("MUTE Ch4");
	m_AmpMuteButtonCh4->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::red);
	m_AmpMuteButtonCh4->addListener(this);
	addAndMakeVisible(m_AmpMuteButtonCh4.get());
}

NanoAmpControlUI::~NanoAmpControlUI()
{
}

void NanoAmpControlUI::paint (Graphics& g)
{
    juce::Component::paint(g);
}

void NanoAmpControlUI::resized()
{
	auto connectionParamsHeight = 35;
	auto buttonHeight = 50;
	auto channelWidth = getWidth() / 4;

	auto bounds = getLocalBounds();

	auto textEditorBounds = bounds.removeFromTop(connectionParamsHeight);
	auto zeroconfButtonBounds = textEditorBounds.removeFromRight(connectionParamsHeight);
	m_ipAndPortEditor->setBounds(textEditorBounds.reduced(5));
	m_zeroconfDiscoverButton->setBounds(zeroconfButtonBounds.reduced(5));

	auto pwrOnBounds = bounds.removeFromTop(buttonHeight).reduced(5);
	m_AmpPowerOnButton->setBounds(pwrOnBounds);

	auto muteBounds = bounds.removeFromBottom(buttonHeight);
	m_AmpMuteButtonCh1->setBounds(muteBounds.removeFromLeft(channelWidth).reduced(5));
	m_AmpMuteButtonCh2->setBounds(muteBounds.removeFromLeft(channelWidth).reduced(5));
	m_AmpMuteButtonCh3->setBounds(muteBounds.removeFromLeft(channelWidth).reduced(5));
	m_AmpMuteButtonCh4->setBounds(muteBounds.reduced(5));

	auto gainBounds = bounds;
	m_AmpGainSliderCh1->setBounds(gainBounds.removeFromLeft(channelWidth).reduced(5));
	m_AmpGainSliderCh2->setBounds(gainBounds.removeFromLeft(channelWidth).reduced(5));
	m_AmpGainSliderCh3->setBounds(gainBounds.removeFromLeft(channelWidth).reduced(5));
	m_AmpGainSliderCh4->setBounds(gainBounds.reduced(5));
}

void NanoAmpControlUI::lookAndFeelChanged()
{
    juce::Component::lookAndFeelChanged();
}

void NanoAmpControlUI::buttonClicked(Button* button)
{
	if (button == m_AmpMuteButtonCh1.get())
	{
		//auto& muteData = CreateMute5DData(1, button->getToggleState());
		//if (!m_nanoOcp1Client->isConnected())
		//	DBG("client not connected");
		//else if (!m_nanoOcp1Client->sendData(juce::MemoryBlock((const char*)muteData.data(), muteData.size())))
		//	DBG("sending failed");
	}
	else if (button == m_AmpMuteButtonCh2.get())
	{
		//auto& muteData = CreateMute5DData(2, button->getToggleState());
		//if (!m_nanoOcp1Client->isConnected())
		//	DBG("client not connected");
		//else if (!m_nanoOcp1Client->sendData(juce::MemoryBlock((const char*)muteData.data(), muteData.size())))
		//	DBG("sending failed");
	}
	else if (button == m_AmpMuteButtonCh3.get())
	{
		//auto& muteData = CreateMute5DData(3, button->getToggleState());
		//if (!m_nanoOcp1Client->isConnected())
		//	DBG("client not connected");
		//else if (!m_nanoOcp1Client->sendData(juce::MemoryBlock((const char*)muteData.data(), muteData.size())))
		//	DBG("sending failed");
	}
	else if (button == m_AmpMuteButtonCh4.get())
	{
		//auto& muteData = CreateMute5DData(4, button->getToggleState());
		//if (!m_nanoOcp1Client->isConnected())
		//	DBG("client not connected");
		//else if (!m_nanoOcp1Client->sendData(juce::MemoryBlock((const char*)muteData.data(), muteData.size())))
		//	DBG("sending failed");
	}
	else if (button == m_AmpPowerOnButton.get())
	{
		//auto& pwrOnData = CreatePowerOn5DData(button->getToggleState());
		//if (!m_nanoOcp1Client->isConnected())
		//	DBG("client not connected");
		//else if (!m_nanoOcp1Client->sendData(juce::MemoryBlock((const char*)pwrOnData.data(), pwrOnData.size())))
		//	DBG("sending failed");
	}
}

void NanoAmpControlUI::sliderValueChanged(Slider* slider)
{
	if (slider == m_AmpGainSliderCh1.get())
	{
		//auto& gainData = CreateGain5DData(1, slider->getValue());
		//if (!m_nanoOcp1Client->isConnected())
		//	DBG("client not connected");
		//else if (!m_nanoOcp1Client->sendData(juce::MemoryBlock((const char*)gainData.data(), gainData.size())))
		//	DBG("sending failed");
	}
	else if (slider == m_AmpGainSliderCh2.get())
	{
		//auto& gainData = CreateGain5DData(2, slider->getValue());
		//if (!m_nanoOcp1Client->isConnected())
		//	DBG("client not connected");
		//else if (!m_nanoOcp1Client->sendData(juce::MemoryBlock((const char*)gainData.data(), gainData.size())))
		//	DBG("sending failed");
	}
	else if (slider == m_AmpGainSliderCh3.get())
	{
		//auto& gainData = CreateGain5DData(3, slider->getValue());
		//if (!m_nanoOcp1Client->isConnected())
		//	DBG("client not connected");
		//else if (!m_nanoOcp1Client->sendData(juce::MemoryBlock((const char*)gainData.data(), gainData.size())))
		//	DBG("sending failed");
	}
	else if (slider == m_AmpGainSliderCh4.get())
	{
		//auto& gainData = CreateGain5DData(4, slider->getValue());
		//if (!m_nanoOcp1Client->isConnected())
		//	DBG("client not connected");
		//else if (!m_nanoOcp1Client->sendData(juce::MemoryBlock((const char*)gainData.data(), gainData.size())))
		//	DBG("sending failed");
	}
}

void NanoAmpControlUI::textEditorReturnKeyPressed(TextEditor& editor)
{
	if (&editor == m_ipAndPortEditor.get())
	{
		auto ip = editor.getText().upToFirstOccurrenceOf(";", false, true);
		auto port = editor.getText().fromLastOccurrenceOf(";", false, true).getIntValue();

		if (onConnectionParametersEdited)
			onConnectionParametersEdited(ip, port);
	}
}


}
