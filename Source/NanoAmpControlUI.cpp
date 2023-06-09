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
#include "LevelMeter.h"

#include <ZeroconfDiscoverComponent.h>


namespace NanoAmpControl
{

//==============================================================================
NanoAmpControlUI::NanoAmpControlUI(const std::uint16_t ampChannelCount)
    :	NanoAmpControlInterface(ampChannelCount),
		juce::Component()
{
	m_helpButton = std::make_unique<juce::DrawableButton>("Help", juce::DrawableButton::ButtonStyle::ImageFitted);
	m_helpButton->onClick = []() {
		juce::URL("https://github.com/ChristianAhrens/" + juce::JUCEApplication::getInstance()->getApplicationName() + "/blob/main/README.md").launchInDefaultBrowser();
	};
	std::unique_ptr<XmlElement> svg_xml = juce::XmlDocument::parse(BinaryData::help24px_svg);
	std::unique_ptr<juce::Drawable> image = juce::Drawable::createFromSVG(*(svg_xml.get()));
	image->replaceColour(Colours::black, getLookAndFeel().findColour(juce::TextEditor::textColourId));
	m_helpButton->setImages(image.get());
	m_helpButton->setColour(juce::DrawableButton::backgroundColourId, juce::Colours::transparentWhite);
	m_helpButton->setColour(juce::DrawableButton::backgroundColourId, juce::Colours::transparentWhite);
	m_helpButton->setTooltip(juce::JUCEApplication::getInstance()->getApplicationName() + " v"
		+ juce::JUCEApplication::getInstance()->getApplicationVersion());
	addAndMakeVisible(m_helpButton.get());

	auto address = juce::String("127.0.0.1");
	auto port = 50014;

	m_ipAndPortEditor = std::make_unique<juce::TextEditor>();
	m_ipAndPortEditor->setTextToShowWhenEmpty(address + ":" + juce::String(port), getLookAndFeel().findColour(juce::TextEditor::ColourIds::textColourId).darker().darker());
	m_ipAndPortEditor->setJustification(juce::Justification::centred);
	m_ipAndPortEditor->addListener(this);
	addAndMakeVisible(m_ipAndPortEditor.get());

	m_zeroconfDiscoverButton = std::make_unique<JUCEAppBasics::ZeroconfDiscoverComponent>(false, false);
	m_zeroconfDiscoverButton->onServiceSelected = [=](JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType type, ZeroconfSearcher::ZeroconfSearcher::ServiceInfo* info) {
		ignoreUnused(type);
		if (m_ipAndPortEditor)
		{
			m_ipAndPortEditor->setTooltip(juce::String(info->ip) + ":" + juce::String(info->port));
			m_ipAndPortEditor->setText(juce::String(info->name).upToFirstOccurrenceOf("._oca",false, true));
		}
		if (onConnectionParametersEdited)
			onConnectionParametersEdited(juce::String(info->ip), static_cast<std::uint16_t>(info->port), juce::String(info->name).contains("5D") ? AmpType::Amp5D : AmpType::DxDy);
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
		m_AmpChannelGainSliders.insert(std::make_pair(ch, std::make_unique<juce::Slider>()));
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
		m_AmpChannelLevelMeters.insert(std::make_pair(ch, std::make_unique<LevelMeter>()));
		m_AmpChannelLevelMeters.at(ch)->SetLevelRange(juce::Range<float>(-32.0f, 32.0f));
		m_AmpChannelLevelMeters.at(ch)->SetLevelValue(-32.0f);
		m_AmpChannelLevelMeters.at(ch)->SetLevelPeakValue(-32.0f);
		addAndMakeVisible(m_AmpChannelLevelMeters.at(ch).get());
	}

	for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
	{
		m_AmpChannelMuteButtons.insert(std::make_pair(ch, std::make_unique<juce::TextButton>()));
		m_AmpChannelMuteButtons.at(ch)->setClickingTogglesState(true);
		m_AmpChannelMuteButtons.at(ch)->setButtonText("Mute");
		m_AmpChannelMuteButtons.at(ch)->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colour(0xf1, 0x00, 0x15));
		m_AmpChannelMuteButtons.at(ch)->addListener(this);
		addAndMakeVisible(m_AmpChannelMuteButtons.at(ch).get());
	}

	for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
	{
		m_AmpChannelIspLeds.insert(std::make_pair(ch, std::make_unique<LedComponent>()));
		m_AmpChannelIspLeds.at(ch)->SetTextLabel("ISP");
		m_AmpChannelIspLeds.at(ch)->SetOutlineThickness(1.0f);
		m_AmpChannelIspLeds.at(ch)->SetState(LedComponent::Grey);
		addAndMakeVisible(m_AmpChannelIspLeds.at(ch).get());
	}

	for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
	{
		m_AmpChannelGrLeds.insert(std::make_pair(ch, std::make_unique<LedComponent>()));
		m_AmpChannelGrLeds.at(ch)->SetTextLabel("GR");
		m_AmpChannelGrLeds.at(ch)->SetOutlineThickness(1.0f);
		m_AmpChannelGrLeds.at(ch)->SetState(LedComponent::Grey);
		addAndMakeVisible(m_AmpChannelGrLeds.at(ch).get());
	}

	for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
	{
		m_AmpChannelOvlLeds.insert(std::make_pair(ch, std::make_unique<LedComponent>()));
		m_AmpChannelOvlLeds.at(ch)->SetTextLabel("OVL");
		m_AmpChannelOvlLeds.at(ch)->SetOutlineThickness(1.0f);
		m_AmpChannelOvlLeds.at(ch)->SetState(LedComponent::Grey);
		addAndMakeVisible(m_AmpChannelOvlLeds.at(ch).get());
	}

	for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
	{
		m_AmpChannelLabels.insert(std::make_pair(ch, std::make_unique<juce::Label>("AmpChannelLabel", "Ch " + juce::String(ch))));
		m_AmpChannelLabels.at(ch)->setJustificationType(juce::Justification::centred);
		addAndMakeVisible(m_AmpChannelLabels.at(ch).get());
	}

	m_RelativeGainSlider = std::make_unique<juce::Slider>();
	m_RelativeGainSlider->setRange(-57.5, 6.0, 0.1);
	m_RelativeGainSlider->setTextValueSuffix("db");
	m_RelativeGainSlider->setNumDecimalPlacesToDisplay(1);
	m_RelativeGainSlider->setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
	m_RelativeGainSlider->setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxAbove, false, 60, 15);
	m_RelativeGainSlider->setColour(juce::Slider::backgroundColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	m_RelativeGainSlider->setColour(juce::Slider::trackColourId, getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	m_RelativeGainSlider->addListener(this);
	addAndMakeVisible(m_RelativeGainSlider.get());

	m_RelativeMuteButton = std::make_unique<juce::TextButton>();
	m_RelativeMuteButton->setClickingTogglesState(true);
	m_RelativeMuteButton->setButtonText("Mute");
	m_RelativeMuteButton->setColour(juce::TextButton::buttonOnColourId, juce::Colours::indianred);
	m_RelativeMuteButton->addListener(this);
	addAndMakeVisible(m_RelativeMuteButton.get());

	m_RelativeLabel = std::make_unique<juce::Label>("AmpChannelLabel", "Rel.");
	m_RelativeLabel->setJustificationType(juce::Justification::centred);
	addAndMakeVisible(m_RelativeLabel.get());

}

NanoAmpControlUI::~NanoAmpControlUI()
{
}

void NanoAmpControlUI::paint (juce::Graphics& g)
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

	auto ispAndGrAndOvlLedSize = buttonHeight < (channelWidth / 3) ? buttonHeight : (channelWidth / 3);
	auto ispAndGrAndOvlLedBounds = bounds.removeFromTop(ispAndGrAndOvlLedSize);
	auto channelLabelBounds = bounds.removeFromTop(labelHeight);
	auto muteBounds = bounds.removeFromTop(buttonHeight);
	auto gainAndLevelsBounds = bounds;
	auto ledMargin = ispAndGrAndOvlLedSize / 8;

	for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
	{
		auto ledChBounds = ispAndGrAndOvlLedBounds.removeFromLeft(channelWidth);
		ledChBounds.reduce((ledChBounds.getWidth() - 3 * ispAndGrAndOvlLedSize) / 2, 0);

		if (m_AmpChannelIspLeds.find(ch) != m_AmpChannelIspLeds.end())
			m_AmpChannelIspLeds.at(ch)->setBounds(ledChBounds
				.removeFromLeft(ispAndGrAndOvlLedSize)
				.reduced(ledMargin));

		if (m_AmpChannelGrLeds.find(ch) != m_AmpChannelGrLeds.end())
			m_AmpChannelGrLeds.at(ch)->setBounds(ledChBounds
				.removeFromLeft(ispAndGrAndOvlLedSize)
				.reduced(ledMargin));

		if (m_AmpChannelOvlLeds.find(ch) != m_AmpChannelOvlLeds.end())
			m_AmpChannelOvlLeds.at(ch)->setBounds(ledChBounds
				.removeFromLeft(ispAndGrAndOvlLedSize)
				.reduced(ledMargin));

		if (m_AmpChannelLabels.find(ch) != m_AmpChannelLabels.end())
			m_AmpChannelLabels.at(ch)->setBounds(channelLabelBounds
				.removeFromLeft(channelWidth)
				.reduced(marginS));

		if (m_AmpChannelMuteButtons.find(ch) != m_AmpChannelMuteButtons.end())
			m_AmpChannelMuteButtons.at(ch)->setBounds(muteBounds
				.removeFromLeft(channelWidth)
				.reduced(margin));

		if (m_AmpChannelGainSliders.find(ch) != m_AmpChannelGainSliders.end())
			m_AmpChannelGainSliders.at(ch)->setBounds(gainAndLevelsBounds
				.removeFromLeft(static_cast<int>(channelWidth * 0.7f))
				.reduced(margin / 2));

		if (m_AmpChannelLevelMeters.find(ch) != m_AmpChannelLevelMeters.end())
			m_AmpChannelLevelMeters.at(ch)->setBounds(gainAndLevelsBounds
				.removeFromLeft(static_cast<int>(channelWidth * 0.3f))
				.reduced(margin / 2));
	}

	m_RelativeLabel->setBounds(channelLabelBounds.reduced(margin));
	m_RelativeMuteButton->setBounds(muteBounds.reduced(margin));
	m_RelativeGainSlider->setBounds(gainAndLevelsBounds.reduced(margin));
}

void NanoAmpControlUI::lookAndFeelChanged()
{
    juce::Component::lookAndFeelChanged();
}

void NanoAmpControlUI::buttonClicked(juce::Button* button)
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

void NanoAmpControlUI::sliderValueChanged(juce::Slider* slider)
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

void NanoAmpControlUI::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
	if (&editor == m_ipAndPortEditor.get())
	{
		auto ip = juce::IPAddress(editor.getText().upToFirstOccurrenceOf(":", false, true));
		auto port = editor.getText().fromLastOccurrenceOf(":", false, true).getIntValue();

		juce::Range<int> tcpPortRange{ 1, 0xffff };
		if (!ip.isNull() && tcpPortRange.contains(port))
			if (onConnectionParametersEdited)
				onConnectionParametersEdited(ip.toString(), static_cast<std::uint16_t>(port), AmpType::DxDy);
	}
}

bool NanoAmpControlUI::SetPwrOnOff(const bool on)
{
	if (m_AmpPowerOnButton)
		m_AmpPowerOnButton->setToggleState(on, juce::dontSendNotification);

	return true;
}

bool NanoAmpControlUI::SetChannelHeadroom(const std::uint16_t channel, const float headroom)
{
	if (m_AmpChannelLevelMeters.find(channel) != m_AmpChannelLevelMeters.end())
	{
		m_AmpChannelLevelMeters.at(channel)->SetLevelValue(headroom);
		m_AmpChannelLevelMeters.at(channel)->SetLevelPeakValue(headroom);
		return true;
	}
	else
		return false;
}

bool NanoAmpControlUI::SetChannelISP(const std::uint16_t channel, const bool isp)
{
	if (m_AmpChannelIspLeds.find(channel) != m_AmpChannelIspLeds.end())
	{
		m_AmpChannelIspLeds.at(channel)->SetState(isp ? LedComponent::State::Green : LedComponent::Grey);
		return true;
	}
	else
		return false;
}

bool NanoAmpControlUI::SetChannelGR(const std::uint16_t channel, const bool gr)
{
	if (m_AmpChannelGrLeds.find(channel) != m_AmpChannelGrLeds.end())
	{
		m_AmpChannelGrLeds.at(channel)->SetState(gr ? LedComponent::State::Yellow : LedComponent::Grey);
		return true;
	}
	else
		return false;
}

bool NanoAmpControlUI::SetChannelOVL(const std::uint16_t channel, const bool ovl)
{
	if (m_AmpChannelOvlLeds.find(channel) != m_AmpChannelOvlLeds.end())
	{
		m_AmpChannelOvlLeds.at(channel)->SetState(ovl ? LedComponent::State::Red : LedComponent::Grey);
		return true;
	}
	else
		return false;
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
