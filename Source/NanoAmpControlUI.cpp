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

#include "LedComponent.h"

#include <ZeroconfDiscoverComponent.h>


namespace NanoAmpControl
{

//==============================================================================
NanoAmpControlUI::NanoAmpControlUI(const std::uint16_t ampChannelCount)
    :	NanoAmpControlInterface(ampChannelCount),
		juce::Component()
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
			onConnectionParametersEdited(juce::String(info->ip), static_cast<std::uint16_t>(info->port));
	};
	m_zeroconfDiscoverButton->clearServices();
	m_zeroconfDiscoverButton->addDiscoverService(JUCEAppBasics::ZeroconfDiscoverComponent::ZST_OCA);
	addAndMakeVisible(m_zeroconfDiscoverButton.get());

	m_stateLed = std::make_unique<LedComponent>();
	m_stateLed->SetOutlineThickness(1.0f);
	addAndMakeVisible(m_stateLed.get());

	m_AmpPowerOnButton = std::make_unique<TextButton>();
	m_AmpPowerOnButton->setClickingTogglesState(true);
	m_AmpPowerOnButton->setButtonText("PwrON");
	m_AmpPowerOnButton->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::forestgreen);
	m_AmpPowerOnButton->addListener(this);
	addAndMakeVisible(m_AmpPowerOnButton.get());

	for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
	{
		m_AmpChannelGainSliders.insert(std::make_pair(ch, std::make_unique<Slider>()));
		m_AmpChannelGainSliders.at(ch)->setRange(-57.5, 6.0, 0.1);
		m_AmpChannelGainSliders.at(ch)->setTextValueSuffix("db");
		m_AmpChannelGainSliders.at(ch)->setNumDecimalPlacesToDisplay(1);
		m_AmpChannelGainSliders.at(ch)->setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
		m_AmpChannelGainSliders.at(ch)->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxAbove, false, 60, 15);
		m_AmpChannelGainSliders.at(ch)->addListener(this);
		addAndMakeVisible(m_AmpChannelGainSliders.at(ch).get());
	}

	for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
	{
		m_AmpChannelMuteButtons.insert(std::make_pair(ch, std::make_unique<TextButton>()));
		m_AmpChannelMuteButtons.at(ch)->setClickingTogglesState(true);
		m_AmpChannelMuteButtons.at(ch)->setButtonText("Mute");
		m_AmpChannelMuteButtons.at(ch)->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::indianred);
		m_AmpChannelMuteButtons.at(ch)->addListener(this);
		addAndMakeVisible(m_AmpChannelMuteButtons.at(ch).get());
	}

	for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
	{
		m_AmpChannelLabels.insert(std::make_pair(ch, std::make_unique<Label>("AmpChannelLabel", "Ch " + juce::String(ch))));
		m_AmpChannelLabels.at(ch)->setJustificationType(juce::Justification::centred);
		addAndMakeVisible(m_AmpChannelLabels.at(ch).get());
	}
}

NanoAmpControlUI::~NanoAmpControlUI()
{
}

void NanoAmpControlUI::paint (Graphics& g)
{
    juce::Component::paint(g);

	auto connectionParamsHeight = 35;
	auto buttonHeight = 35;
	auto channelWidth = getWidth() / GetAmpChannelCount();

	auto bounds = getLocalBounds();
	auto headerBounds = bounds.removeFromTop(connectionParamsHeight);
	g.setColour(getLookAndFeel().findColour(juce::TextEditor::ColourIds::backgroundColourId));
	g.fillRect(headerBounds);
	g.setColour(getLookAndFeel().findColour(juce::TextEditor::ColourIds::outlineColourId));
	g.drawLine(juce::Line<float>(headerBounds.getBottomLeft().toFloat(), headerBounds.getBottomRight().toFloat()));

	auto gnrlCtrlBounds = bounds.removeFromTop(buttonHeight);
	g.drawLine(juce::Line<float>(gnrlCtrlBounds.getBottomLeft().toFloat(), gnrlCtrlBounds.getBottomRight().toFloat()));

	for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
	{
		auto channelCtrlBounds = bounds.removeFromLeft(channelWidth);
		g.drawLine(juce::Line<float>(channelCtrlBounds.getTopRight().toFloat(), channelCtrlBounds.getBottomRight().toFloat()));
	}
}

void NanoAmpControlUI::resized()
{
	if (GetAmpChannelCount() < 1)
		return;

	auto margin = 5;
	auto connectionParamsHeight = 35;
	auto buttonHeight = 35;
	auto channelWidth = getWidth() / GetAmpChannelCount();

	auto bounds = getLocalBounds();

	auto textEditorBounds = bounds.removeFromTop(connectionParamsHeight);
	auto zeroconfButtonBounds = textEditorBounds.removeFromRight(connectionParamsHeight);
	auto stateLedBounds = textEditorBounds.removeFromLeft(connectionParamsHeight);
	m_ipAndPortEditor->setBounds(textEditorBounds.reduced(5));
	m_zeroconfDiscoverButton->setBounds(zeroconfButtonBounds.reduced(5));
	m_stateLed->setBounds(stateLedBounds.reduced(7));

	auto pwrOnBounds = bounds.removeFromTop(buttonHeight).reduced(5);
	m_AmpPowerOnButton->setBounds(pwrOnBounds);

	auto channelLabelBounds = bounds.removeFromTop(buttonHeight);
	auto muteBounds = bounds.removeFromTop(buttonHeight);
	auto gainBounds = bounds;

	for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
	{
		if (m_AmpChannelLabels.find(ch) != m_AmpChannelLabels.end())
			m_AmpChannelLabels.at(ch)->setBounds(channelLabelBounds.removeFromLeft(channelWidth).reduced(margin));

		if (m_AmpChannelMuteButtons.find(ch) != m_AmpChannelMuteButtons.end())
			m_AmpChannelMuteButtons.at(ch)->setBounds(muteBounds.removeFromLeft(channelWidth).reduced(margin));

		if (m_AmpChannelGainSliders.find(ch) != m_AmpChannelGainSliders.end())
			m_AmpChannelGainSliders.at(ch)->setBounds(gainBounds.removeFromLeft(channelWidth).reduced(margin));
	}
}

void NanoAmpControlUI::lookAndFeelChanged()
{
    juce::Component::lookAndFeelChanged();
}

void NanoAmpControlUI::buttonClicked(Button* button)
{
	if (button == m_AmpPowerOnButton.get())
	{
		if (onPwrOnOff)
			onPwrOnOff(button->getToggleState());
	}
	else
	{
		auto muteButtonKV = std::find_if(m_AmpChannelMuteButtons.begin(), m_AmpChannelMuteButtons.end(), [button](const auto& val) { return val.second.get() == button; });
		if (muteButtonKV != m_AmpChannelMuteButtons.end() && muteButtonKV->second)
		{
			auto& channel = muteButtonKV->first;
			auto& muteButton = muteButtonKV->second;
			if (onChannelMute)
				onChannelMute(channel, muteButton->getToggleState());
		}
	}
}

void NanoAmpControlUI::sliderValueChanged(Slider* slider)
{
	auto gainSliderKV = std::find_if(m_AmpChannelGainSliders.begin(), m_AmpChannelGainSliders.end(), [slider](const auto& val) { return val.second.get() == slider; });
	if (gainSliderKV != m_AmpChannelGainSliders.end() && gainSliderKV->second)
	{
		auto& channel = gainSliderKV->first;
		auto& gainSlider = gainSliderKV->second;
		if (onChannelGain)
			onChannelGain(channel, static_cast<float>(gainSlider->getValue()));
	}
}

void NanoAmpControlUI::textEditorReturnKeyPressed(TextEditor& editor)
{
	if (&editor == m_ipAndPortEditor.get())
	{
		auto ip = editor.getText().upToFirstOccurrenceOf(";", false, true);
		auto port = editor.getText().fromLastOccurrenceOf(";", false, true).getIntValue();

		if (onConnectionParametersEdited)
			onConnectionParametersEdited(ip, static_cast<std::uint16_t>(port));
	}
}

bool NanoAmpControlUI::SetPwrOnOff(const bool on)
{
	if (m_AmpPowerOnButton)
		m_AmpPowerOnButton->setToggleState(on, juce::dontSendNotification);

	return true;
}

bool NanoAmpControlUI::SetChannelMute(const std::uint16_t channel, const bool mute)
{
	if (m_AmpChannelMuteButtons.find(channel) != m_AmpChannelMuteButtons.end())
	{
		m_AmpChannelMuteButtons.at(channel)->setToggleState(mute, juce::dontSendNotification);
		return true;
	}
	else
		return false;
}

bool NanoAmpControlUI::SetChannelGain(const std::uint16_t channel, const float gain)
{
	if (m_AmpChannelGainSliders.find(channel) != m_AmpChannelGainSliders.end())
	{
		m_AmpChannelGainSliders.at(channel)->setValue(gain, juce::dontSendNotification);
		return true;
	}
	else
		return false;
}

void NanoAmpControlUI::SetConnectionState(const NanoAmpControlInterface::ConnectionState state)
{
	if (m_stateLed)
	{
		switch (state)
		{
		case Disconnected:
			m_stateLed->SetState(LedComponent::State::Grey);
			break;
		case Connected:
			m_stateLed->SetState(LedComponent::State::Yellow);
			break;
		case Subscribed:
			m_stateLed->SetState(LedComponent::State::Green);
			break;
		case Active:
			m_stateLed->SetState(LedComponent::State::Yellow);
			break;
		case Unknown:
		default:
			m_stateLed->SetState(LedComponent::State::Off);
			break;
		}
	}
}


}
