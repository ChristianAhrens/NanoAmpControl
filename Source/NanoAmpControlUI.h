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

#pragma once

#include <JuceHeader.h>

#include "NanoAmpControl.h"


/**
 * Fwd. Decls
 */
namespace JUCEAppBasics
{
    class ZeroconfDiscoverComponent;
}

namespace NanoAmpControl
{


/**
 * Fwd. Decls
 */
class LedComponent;
class LevelMeterWithISPGROVL;

//==============================================================================
/*
*/
class NanoAmpControlUI :    public NanoAmpControlInterface, 
                            public juce::Component, 
                            public juce::Button::Listener, 
                            public juce::Slider::Listener, 
                            public juce::TextEditor::Listener
{
public:
    //==========================================================================
    NanoAmpControlUI(const std::uint16_t ampChannelCount);
    ~NanoAmpControlUI();

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    //==========================================================================
    void lookAndFeelChanged() override;

    //==============================================================================
    void buttonClicked(juce::Button* button) override;

    //==============================================================================
    void sliderValueChanged(juce::Slider* slider) override;

    //==============================================================================
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;

    //==============================================================================
    std::function<bool(const juce::String&, const std::uint16_t, const AmpType)> onConnectionParametersEdited;
    std::function<void()> onAddClicked;
    std::function<void()> onRemoveClicked;
    std::function<void()> onToggleVisuOnlyClicked;

    //==============================================================================
    bool SetPwrOnOff(const bool on) override;
    bool SetChannelISP(const std::uint16_t channel, const bool isp) override;
    bool SetChannelGR(const std::uint16_t channel, const bool gr) override;
    bool SetChannelOVL(const std::uint16_t channel, const bool ovl) override;
    bool SetChannelHeadroom(const std::uint16_t channel, const float headroom) override;
    bool SetChannelMute(const std::uint16_t channel, const bool mute) override;
    bool SetChannelGain(const std::uint16_t channel, const float gain) override;
    void SetConnectionState(const NanoAmpControlInterface::ConnectionState state) override;

protected:
    //==========================================================================
    bool IsVisuOnlyModeActive();
    void SetVisuOnlyModeActive(bool active);
    void ToggleVisuOnlyMode();

    //==========================================================================
    void SetCtrlComponentsVisible(bool visible);

private:
    //==========================================================================
    std::unique_ptr<juce::PopupMenu>                                    m_OptionsPopup;
    std::unique_ptr<juce::DrawableButton>								m_OptionsButton;

    std::unique_ptr<juce::TextEditor>                                   m_IpAndPortEditor;
    std::unique_ptr<JUCEAppBasics::ZeroconfDiscoverComponent>           m_ZeroconfDiscoverButton;

    std::unique_ptr<LedComponent>                                       m_StateLed;

    std::unique_ptr<juce::TextButton>                                   m_AmpPowerOnButton;

    std::map<std::uint16_t, std::unique_ptr<juce::Slider>>              m_AmpChannelGainSliders;
    std::map<std::uint16_t, std::unique_ptr<juce::TextButton>>          m_AmpChannelMuteButtons;
    std::map<std::uint16_t, std::unique_ptr<juce::Label>>               m_AmpChannelLabels;
    std::map<std::uint16_t, std::unique_ptr<LevelMeterWithISPGROVL>>    m_AmpChannelLevelMeters;

    std::unique_ptr<Slider>                                             m_RelativeGainSlider;
    std::unique_ptr<TextButton>                                         m_RelativeMuteButton;
    std::unique_ptr<Label>                                              m_RelativeLabel;

    //==========================================================================
    double  m_LastKnownRelativeGainSliderValue{ 0.0 };
    bool    m_VisuOnlyModeActive{ false };


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NanoAmpControlUI)
};

}
