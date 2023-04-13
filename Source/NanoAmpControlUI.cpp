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
	m_helpButton = std::make_unique<DrawableButton>("Help", DrawableButton::ButtonStyle::ImageFitted);
	m_helpButton->onClick = []() {
		URL("https://github.com/ChristianAhrens/" + JUCEApplication::getInstance()->getApplicationName() + "/blob/main/README.md").launchInDefaultBrowser();
	};
	std::unique_ptr<XmlElement> svg_xml = XmlDocument::parse(BinaryData::help24px_svg);
	std::unique_ptr<juce::Drawable> image = Drawable::createFromSVG(*(svg_xml.get()));
	image->replaceColour(Colours::black, getLookAndFeel().findColour(juce::TextEditor::textColourId));
	m_helpButton->setImages(image.get());
	m_helpButton->setColour(juce::DrawableButton::backgroundColourId, juce::Colours::transparentWhite);
	m_helpButton->setColour(juce::DrawableButton::backgroundColourId, juce::Colours::transparentWhite);
	addAndMakeVisible(m_helpButton.get());

	auto address = juce::String("127.0.0.1");
	auto port = 50014;

	m_ipAndPortEditor = std::make_unique<TextEditor>();
	m_ipAndPortEditor->setTextToShowWhenEmpty(address + ";" + juce::String(port), getLookAndFeel().findColour(juce::TextEditor::ColourIds::textColourId).darker().darker());
	m_ipAndPortEditor->setJustification(juce::Justification::centred);
	m_ipAndPortEditor->addListener(this);
	addAndMakeVisible(m_ipAndPortEditor.get());

	m_zeroconfDiscoverButton = std::make_unique<JUCEAppBasics::ZeroconfDiscoverComponent>(false, false);
	m_zeroconfDiscoverButton->onServiceSelected = [=](JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType type, ZeroconfSearcher::ZeroconfSearcher::ServiceInfo* info) {
		ignoreUnused(type);
		if (m_ipAndPortEditor)
		{
			m_ipAndPortEditor->setTooltip(juce::String(info->ip) + ";" + juce::String(info->port));
			m_ipAndPortEditor->setText(juce::String(info->name).upToFirstOccurrenceOf("._oca",false, true));
		}
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

	m_RelativeGainSlider = std::make_unique<Slider>();
	m_RelativeGainSlider->setRange(-57.5, 6.0, 0.1);
	m_RelativeGainSlider->setTextValueSuffix("db");
	m_RelativeGainSlider->setNumDecimalPlacesToDisplay(1);
	m_RelativeGainSlider->setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
	m_RelativeGainSlider->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxAbove, false, 60, 15);
	m_RelativeGainSlider->setColour(juce::Slider::backgroundColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	m_RelativeGainSlider->setColour(juce::Slider::trackColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	m_RelativeGainSlider->addListener(this);
	addAndMakeVisible(m_RelativeGainSlider.get());

	m_RelativeMuteButton = std::make_unique<TextButton>();
	m_RelativeMuteButton->setClickingTogglesState(true);
	m_RelativeMuteButton->setButtonText("Mute");
	m_RelativeMuteButton->setColour(juce::TextButton::buttonOnColourId, juce::Colours::indianred);
	m_RelativeMuteButton->addListener(this);
	addAndMakeVisible(m_RelativeMuteButton.get());

	m_RelativeLabel = std::make_unique<Label>("AmpChannelLabel", "Rel.");
	m_RelativeLabel->setJustificationType(juce::Justification::centred);
	addAndMakeVisible(m_RelativeLabel.get());
}

NanoAmpControlUI::~NanoAmpControlUI()
{
}

void NanoAmpControlUI::paint (Graphics& g)
{
    juce::Component::paint(g);

	auto connectionParamsHeight = 41;
	auto infoIconsWidth = connectionParamsHeight;
	auto buttonHeight = 41;
	auto channelWidth = getWidth() / (GetAmpChannelCount() + 1);

	auto bounds = getLocalBounds();
	auto connectionParamsBounds = bounds.removeFromTop(connectionParamsHeight);
	auto gnrlCtrlBounds = bounds.removeFromTop(buttonHeight);
	auto infoIconsBounds = connectionParamsBounds;
	infoIconsBounds = infoIconsBounds.removeFromRight(infoIconsWidth);

	// fill rects first to not have them overpaint follwing line drawing
	g.setColour(getLookAndFeel().findColour(juce::TextEditor::backgroundColourId));
	g.fillRect(connectionParamsBounds);
	g.fillRect(bounds.removeFromRight(channelWidth));
	g.fillRect(gnrlCtrlBounds);

	// draw lines
	g.setColour(getLookAndFeel().findColour(juce::TextEditor::outlineColourId));
	g.drawLine(juce::Line<float>(connectionParamsBounds.getBottomLeft().toFloat(), infoIconsBounds.getBottomRight().toFloat()));
	g.drawLine(juce::Line<float>(infoIconsBounds.getTopLeft().toFloat(), infoIconsBounds.getBottomLeft().toFloat()));
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

	auto marginS = 4;
	auto margin = 6;
	auto marginEx = 8;
	auto connectionParamsHeight = 41;
	auto infoIconsWidth = connectionParamsHeight;
	auto labelHeight = 24;
	auto buttonHeight = 41;
	auto channelWidth = getWidth() / (GetAmpChannelCount() + 1);

	auto bounds = getLocalBounds();
	auto headerBounds = bounds.removeFromTop(connectionParamsHeight);
	auto infoIconsBounds = headerBounds.removeFromRight(infoIconsWidth);
	auto zeroconfButtonBounds = headerBounds.removeFromRight(connectionParamsHeight);
	auto stateLedBounds = headerBounds.removeFromLeft(connectionParamsHeight);
	m_helpButton->setBounds(infoIconsBounds.reduced(margin));
	m_ipAndPortEditor->setBounds(headerBounds.reduced(margin));
	m_zeroconfDiscoverButton->setBounds(zeroconfButtonBounds.reduced(margin));
	m_stateLed->setBounds(stateLedBounds.reduced(marginEx));

	auto pwrOnBounds = bounds.removeFromTop(buttonHeight).reduced(margin);
	m_AmpPowerOnButton->setBounds(pwrOnBounds);

	auto channelLabelBounds = bounds.removeFromTop(labelHeight);
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

	m_RelativeLabel->setBounds(channelLabelBounds.reduced(margin));
	m_RelativeMuteButton->setBounds(muteBounds.reduced(margin));
	m_RelativeGainSlider->setBounds(gainBounds.reduced(margin));
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
	else if (button == m_RelativeMuteButton.get())
	{
		for (auto const& muteButtonKV : m_AmpChannelMuteButtons)
		{
			if (muteButtonKV.second)
				muteButtonKV.second->setToggleState(m_RelativeMuteButton->getToggleState(), juce::sendNotification);
		}
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
	if (slider == m_RelativeGainSlider.get())
	{
		auto relativeGainSliderValue = m_RelativeGainSlider->getValue();
		auto gainDeltaValue = relativeGainSliderValue - m_lastKnownRelativeGainSliderValue;
		for (auto const& gainSliderKV : m_AmpChannelGainSliders)
		{
			if (gainSliderKV.second)
			{
				auto sliderValue = gainSliderKV.second->getValue();
				gainSliderKV.second->setValue(sliderValue + gainDeltaValue, juce::sendNotification);
			}
		}
		m_lastKnownRelativeGainSliderValue = relativeGainSliderValue;
	}
	else
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
}

void NanoAmpControlUI::textEditorReturnKeyPressed(TextEditor& editor)
{
	if (&editor == m_ipAndPortEditor.get())
	{
		auto ip = juce::IPAddress(editor.getText().upToFirstOccurrenceOf(";", false, true));
		auto port = editor.getText().fromLastOccurrenceOf(";", false, true).getIntValue();

		juce::Range<int> tcpPortRange{ 1, 0xffff };
		if (!ip.isNull() && tcpPortRange.contains(port))
			if (onConnectionParametersEdited)
				onConnectionParametersEdited(ip.toString(), static_cast<std::uint16_t>(port));
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
