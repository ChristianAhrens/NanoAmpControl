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

#pragma once

#include <JuceHeader.h>


/**
 * Fwd. Decls
 */
namespace JUCEAppBasics
{
    class ZeroconfDiscoverComponent;
}

namespace NanoAmpControl
{


//==============================================================================
/*
*/
class NanoAmpControlUI :    public juce::Component, juce::Button::Listener, juce::Slider::Listener, juce::TextEditor::Listener
{
public:
    //==========================================================================
    NanoAmpControlUI();
    ~NanoAmpControlUI();

    //==============================================================================
    void paint(Graphics&) override;
    void resized() override;

    //==========================================================================
    void lookAndFeelChanged() override;

    //==============================================================================
    void buttonClicked(Button* button) override;

    //==============================================================================
    void sliderValueChanged(Slider* slider) override;

    //==============================================================================
    void textEditorReturnKeyPressed(TextEditor& editor) override;

    //==============================================================================
    std::function<bool(const juce::String&, const int)> onConnectionParametersEdited;

private:
    //==========================================================================

    //==========================================================================
    std::unique_ptr<TextEditor>                                 m_ipAndPortEditor;
    std::unique_ptr<JUCEAppBasics::ZeroconfDiscoverComponent>   m_zeroconfDiscoverButton;

    std::unique_ptr<TextButton>                                 m_AmpPowerOnButton;

    std::unique_ptr<Slider>                                     m_AmpGainSliderCh1;
    std::unique_ptr<Slider>                                     m_AmpGainSliderCh2;
    std::unique_ptr<Slider>                                     m_AmpGainSliderCh3;
    std::unique_ptr<Slider>                                     m_AmpGainSliderCh4;

    std::unique_ptr<TextButton>                                 m_AmpMuteButtonCh1;
    std::unique_ptr<TextButton>                                 m_AmpMuteButtonCh2;
    std::unique_ptr<TextButton>                                 m_AmpMuteButtonCh3;
    std::unique_ptr<TextButton>                                 m_AmpMuteButtonCh4;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NanoAmpControlUI)
};

}
