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
	onToggleVisuOnlyClicked = [=]() {
		ToggleVisuOnlyMode();
	};

	// set up the options button that triggers a popup menu for user interaction
	m_OptionsPopup = std::make_unique<juce::PopupMenu>();
	m_OptionsButton = std::make_unique<juce::DrawableButton>("Options", juce::DrawableButton::ButtonStyle::ImageFitted);
	m_OptionsButton->onClick = [this]() {
		// clear the popup first before populating it anew
		m_OptionsPopup->dismissAllActiveMenus();
		m_OptionsPopup->clear();

		// some info as a title header, since we currently dont have anywhere else to present this info (name+version)
		m_OptionsPopup->addSectionHeader(juce::JUCEApplication::getInstance()->getApplicationName() + " v"
			+ juce::JUCEApplication::getInstance()->getApplicationVersion());

		m_OptionsPopup->addSeparator();

		// add '+' popup menu entry, incl. prep the corresp. icon
		std::unique_ptr<juce::Drawable> normalAddImage = juce::Drawable::createFromSVG(*(juce::XmlDocument::parse(BinaryData::add_circle24px_svg).get()));
		normalAddImage->replaceColour(Colours::black, getLookAndFeel().findColour(juce::TextEditor::textColourId));
		m_OptionsPopup->addItem(1, "Add amp ctrl", true, false, std::move(normalAddImage));

		// add '-' popup menu entry, incl. prep the corresp. icon
		std::unique_ptr<juce::Drawable> normalRemoveImage = juce::Drawable::createFromSVG(*(juce::XmlDocument::parse(BinaryData::remove_circle24px_svg).get()));
		normalRemoveImage->replaceColour(Colours::black, getLookAndFeel().findColour(juce::TextEditor::textColourId));
		m_OptionsPopup->addItem(2, "Remove this amp ctrl", true, false, std::move(normalRemoveImage));

		m_OptionsPopup->addSeparator();

		// add 'toggle visu only' popup menu entry
		m_OptionsPopup->addItem(3, "Show levelmeters only", true, IsVisuOnlyModeActive());
		// add 'toggle fullscreen' popup menu entry
		m_OptionsPopup->addItem(4, "Fullscreen window mode", true, (nullptr != juce::Desktop::getInstance().getKioskModeComponent()));

		m_OptionsPopup->addSeparator();

		// add 'github readme' popup menu entry, incl. prep the corresp. icon
		std::unique_ptr<juce::Drawable> normalHelpImage = juce::Drawable::createFromSVG(*(juce::XmlDocument::parse(BinaryData::help24px_svg).get()));
		normalHelpImage->replaceColour(Colours::black, getLookAndFeel().findColour(juce::TextEditor::textColourId));
		m_OptionsPopup->addItem(5, "Github README", true, false, std::move(normalHelpImage));

		// show the popup and handle its result in a lambda
		m_OptionsPopup->showMenuAsync(PopupMenu::Options(), [this](int resultingAssiIdx) {
			switch (resultingAssiIdx)
			{
			case 1:
				if (onAddClicked)
					onAddClicked();
				break;
			case 2:
				if (onRemoveClicked)
					onRemoveClicked();
				break;
			case 3:
				if (onToggleVisuOnlyClicked)
					onToggleVisuOnlyClicked();
				break;
			case 4:
				if (nullptr != juce::Desktop::getInstance().getKioskModeComponent())
					juce::Desktop::getInstance().setKioskModeComponent(nullptr);
				else
					juce::Desktop::getInstance().setKioskModeComponent(getTopLevelComponent());
				break;
			case 5:
				juce::URL("https://github.com/ChristianAhrens/" + juce::JUCEApplication::getInstance()->getApplicationName() + "/blob/main/README.md").launchInDefaultBrowser();
				break;
			default:
				break;
			};
		});
	};
	std::unique_ptr<XmlElement> svg_xml = juce::XmlDocument::parse(BinaryData::settings24px_svg);
	std::unique_ptr<juce::Drawable> normalImage = juce::Drawable::createFromSVG(*(svg_xml.get()));
	normalImage->replaceColour(Colours::black, getLookAndFeel().findColour(juce::TextEditor::textColourId));
	std::unique_ptr<juce::Drawable> overImage = juce::Drawable::createFromSVG(*(svg_xml.get()));
	overImage->replaceColour(Colours::black, getLookAndFeel().findColour(juce::TextEditor::highlightedTextColourId));
	m_OptionsButton->setImages(normalImage.get(), overImage.get());
	m_OptionsButton->setColour(juce::DrawableButton::backgroundColourId, juce::Colours::transparentWhite);
	addAndMakeVisible(m_OptionsButton.get());

	auto address = juce::String("127.0.0.1");
	auto port = 50014;

	m_IpAndPortEditor = std::make_unique<juce::TextEditor>();
	m_IpAndPortEditor->setTextToShowWhenEmpty(address + ":" + juce::String(port), getLookAndFeel().findColour(juce::TextEditor::ColourIds::textColourId).darker().darker());
	m_IpAndPortEditor->setJustification(juce::Justification::centred);
	m_IpAndPortEditor->addListener(this);
	addAndMakeVisible(m_IpAndPortEditor.get());

	m_ZeroconfDiscoverButton = std::make_unique<JUCEAppBasics::ZeroconfDiscoverComponent>("AmpDiscovery");
	m_ZeroconfDiscoverButton->onServiceSelected = [=](JUCEAppBasics::ZeroconfDiscoverComponent::ZeroconfServiceType serviceType, ZeroconfSearcher::ZeroconfSearcher::ServiceInfo* info) {
		ignoreUnused(serviceType);
		if (m_IpAndPortEditor)
		{
			m_IpAndPortEditor->setTooltip(juce::String(info->ip) + ":" + juce::String(info->port));
			m_IpAndPortEditor->setText(juce::String(info->name).upToFirstOccurrenceOf("._oca",false, true));
		}

		auto ampType = AmpType::_Dy;
		auto ampName = juce::String(info->name);
		if (ampName.contains("5D"))
		{
			ampType = AmpType::_5D;
		}
		else if (ampName.contains("D20")
			|| ampName.contains("D80")
			|| ampName.contains("10D")
			|| ampName.contains("20D"))
		{
			ampType = AmpType::_Dx;
		}
		else if (ampName.contains("D40")
			|| ampName.contains("40D"))
		{
			ampType = AmpType::_Dy;
		}

		if (onConnectionParametersEdited)
			onConnectionParametersEdited(juce::String(info->ip), static_cast<std::uint16_t>(info->port), ampType);
	};
	m_ZeroconfDiscoverButton->clearServices();
	m_ZeroconfDiscoverButton->addDiscoverService(JUCEAppBasics::ZeroconfDiscoverComponent::ZST_OCA);
	addAndMakeVisible(m_ZeroconfDiscoverButton.get());

	m_StateLed = std::make_unique<LedComponent>();
	m_StateLed->SetOutlineThickness(1.0f);
	addAndMakeVisible(m_StateLed.get());

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

		m_AmpChannelLevelMeters.insert(std::make_pair(ch, std::make_unique<LevelMeterWithISPGROVL>()));
		m_AmpChannelLevelMeters.at(ch)->SetLevelRange(juce::Range<float>(-32.0f, 32.0f));
		m_AmpChannelLevelMeters.at(ch)->SetLevelValue(-32.0f);
		m_AmpChannelLevelMeters.at(ch)->SetLevelPeakValue(-32.0f);
		m_AmpChannelLevelMeters.at(ch)->SetISPState(false);
		m_AmpChannelLevelMeters.at(ch)->SetGRState(false);
		m_AmpChannelLevelMeters.at(ch)->SetOVLState(false);
		addAndMakeVisible(m_AmpChannelLevelMeters.at(ch).get());

		m_AmpChannelMuteButtons.insert(std::make_pair(ch, std::make_unique<juce::TextButton>()));
		m_AmpChannelMuteButtons.at(ch)->setClickingTogglesState(true);
		m_AmpChannelMuteButtons.at(ch)->setButtonText("Mute");
		m_AmpChannelMuteButtons.at(ch)->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colour(0xf1, 0x00, 0x15));
		m_AmpChannelMuteButtons.at(ch)->addListener(this);
		addAndMakeVisible(m_AmpChannelMuteButtons.at(ch).get());

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
	
	auto bounds = getLocalBounds();
	auto connectionParamsBounds = bounds.removeFromTop(connectionParamsHeight);

	// fill rects first to not have them overpaint follwing line drawing
	g.setColour(getLookAndFeel().findColour(juce::TextEditor::shadowColourId));
	g.fillRect(connectionParamsBounds);
	g.setColour(getLookAndFeel().findColour(juce::TextEditor::backgroundColourId));

	// draw lines
	if (IsVisuOnlyModeActive())
	{
		auto channelWidth = getWidth() / (GetAmpChannelCount());

		auto infoIconsBounds = connectionParamsBounds;
		infoIconsBounds = infoIconsBounds.removeFromRight(infoIconsWidth);

		g.setColour(getLookAndFeel().findColour(juce::TextEditor::outlineColourId));
		g.drawLine(juce::Line<float>(connectionParamsBounds.getBottomLeft().toFloat(), infoIconsBounds.getBottomRight().toFloat()));
		g.drawLine(juce::Line<float>(infoIconsBounds.getTopLeft().toFloat(), infoIconsBounds.getBottomLeft().toFloat()));
		for (std::uint16_t ch = 1; ch < GetAmpChannelCount(); ch++)
		{
			auto channelCtrlBounds = bounds.removeFromLeft(channelWidth);
			g.drawLine(juce::Line<float>(channelCtrlBounds.getTopRight().toFloat(), channelCtrlBounds.getBottomRight().toFloat()));
		}
	}
	else
	{
		auto channelWidth = getWidth() / (GetAmpChannelCount() + 1);

		auto gnrlCtrlBounds = bounds.removeFromTop(buttonHeight);
		auto infoIconsBounds = connectionParamsBounds;
		infoIconsBounds = infoIconsBounds.removeFromRight(infoIconsWidth);

		g.fillRect(bounds.removeFromRight(channelWidth));
		g.fillRect(gnrlCtrlBounds);

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

	auto bounds = getLocalBounds();
	auto headerBounds = bounds.removeFromTop(connectionParamsHeight);
	auto infoIconsBounds = headerBounds.removeFromRight(infoIconsWidth);
	auto zeroconfButtonBounds = headerBounds.removeFromRight(connectionParamsHeight);
	auto stateLedBounds = headerBounds.removeFromLeft(connectionParamsHeight);
	m_OptionsButton->setBounds(infoIconsBounds.reduced(margin));
	m_IpAndPortEditor->setBounds(headerBounds.reduced(margin));
	m_ZeroconfDiscoverButton->setBounds(zeroconfButtonBounds.reduced(margin));
	m_StateLed->setBounds(stateLedBounds.reduced(marginEx));

	if (IsVisuOnlyModeActive())
	{
		auto channelWidth = getWidth() / GetAmpChannelCount();

		auto channelLabelBounds = bounds.removeFromTop(labelHeight);
		auto levelsBounds = bounds;

		for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
		{
			if (m_AmpChannelLabels.find(ch) != m_AmpChannelLabels.end())
				m_AmpChannelLabels.at(ch)->setBounds(channelLabelBounds
					.removeFromLeft(channelWidth)
					.reduced(marginS));

			if (m_AmpChannelLevelMeters.find(ch) != m_AmpChannelLevelMeters.end())
				m_AmpChannelLevelMeters.at(ch)->setBounds(levelsBounds
					.removeFromLeft(static_cast<int>(channelWidth))
					.reduced(2 * margin));
		}
	}
	else
	{
		auto channelWidth = getWidth() / (GetAmpChannelCount() + 1);

		auto pwrOnBounds = bounds.removeFromTop(buttonHeight).reduced(margin);
		m_AmpPowerOnButton->setBounds(pwrOnBounds);

		auto channelLabelBounds = bounds.removeFromTop(labelHeight);
		auto muteBounds = bounds.removeFromTop(buttonHeight);
		auto gainAndLevelsBounds = bounds;

		for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
		{
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
		auto gainDeltaValue = relativeGainSliderValue - m_LastKnownRelativeGainSliderValue;
		for (auto const& gainSliderKV : m_AmpChannelGainSliders)
		{
			if (gainSliderKV.second)
			{
				auto sliderValue = gainSliderKV.second->getValue();
				gainSliderKV.second->setValue(sliderValue + gainDeltaValue, juce::sendNotification);
			}
		}
		m_LastKnownRelativeGainSliderValue = relativeGainSliderValue;
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
	if (&editor == m_IpAndPortEditor.get())
	{
		auto ip = juce::IPAddress(editor.getText().upToFirstOccurrenceOf(":", false, true));
		auto port = editor.getText().fromLastOccurrenceOf(":", false, true).getIntValue();

		juce::Range<int> tcpPortRange{ 1, 0xffff };
		if (!ip.isNull() && tcpPortRange.contains(port))
			if (onConnectionParametersEdited)
				onConnectionParametersEdited(ip.toString(), static_cast<std::uint16_t>(port), AmpType::_Dy);
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
	if (m_AmpChannelLevelMeters.find(channel) != m_AmpChannelLevelMeters.end())
	{
		m_AmpChannelLevelMeters.at(channel)->SetISPState(isp);
		return true;
	}
	else
		return false;
}

bool NanoAmpControlUI::SetChannelGR(const std::uint16_t channel, const bool gr)
{
	if (m_AmpChannelLevelMeters.find(channel) != m_AmpChannelLevelMeters.end())
	{
		m_AmpChannelLevelMeters.at(channel)->SetGRState(gr);
		return true;
	}
	else
		return false;
}

bool NanoAmpControlUI::SetChannelOVL(const std::uint16_t channel, const bool ovl)
{
	if (m_AmpChannelLevelMeters.find(channel) != m_AmpChannelLevelMeters.end())
	{
		m_AmpChannelLevelMeters.at(channel)->SetOVLState(ovl);
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
	if (m_StateLed)
	{
		switch (state)
		{
		case Disconnected:
			m_StateLed->SetState(LedComponent::State::Grey);
			break;
		case Connected:
			m_StateLed->SetState(LedComponent::State::Yellow);
			break;
		case Subscribed:
			m_StateLed->SetState(LedComponent::State::Green);
			break;
		case Active:
			m_StateLed->SetState(LedComponent::State::Yellow);
			break;
		case Unknown:
		default:
			m_StateLed->SetState(LedComponent::State::Off);
			break;
		}
	}
}

bool NanoAmpControlUI::IsVisuOnlyModeActive()
{
	return m_VisuOnlyModeActive;
}

void NanoAmpControlUI::SetVisuOnlyModeActive(bool active)
{
	m_VisuOnlyModeActive = active;

	SetCtrlComponentsVisible(!active);
	
	resized();
	repaint();
}

void NanoAmpControlUI::ToggleVisuOnlyMode()
{
	SetVisuOnlyModeActive(!IsVisuOnlyModeActive());
}

void NanoAmpControlUI::SetCtrlComponentsVisible(bool visible)
{
	m_AmpPowerOnButton->setVisible(visible);

	for (std::uint16_t ch = 1; ch <= GetAmpChannelCount(); ch++)
	{
		if (m_AmpChannelMuteButtons.find(ch) != m_AmpChannelMuteButtons.end())
			m_AmpChannelMuteButtons.at(ch)->setVisible(visible);

		if (m_AmpChannelGainSliders.find(ch) != m_AmpChannelGainSliders.end())
			m_AmpChannelGainSliders.at(ch)->setVisible(visible);
	}

	m_RelativeLabel->setVisible(visible);
	m_RelativeMuteButton->setVisible(visible);
	m_RelativeGainSlider->setVisible(visible);
}


}
