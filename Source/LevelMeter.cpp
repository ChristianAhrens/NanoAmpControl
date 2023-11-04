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

#include "LevelMeter.h"


namespace NanoAmpControl
{


LevelMeter::LevelMeter()
    : juce::Component()
{
}

LevelMeter::~LevelMeter()
{
}

const juce::Colour LevelMeter::GetColourForState(const State stateValue)
{
    switch (stateValue)
    {
    case White:
        return juce::Colour(0xff, 0xff, 0xff);//juce::Colours::white;
    case Grey:
        return juce::Colour(0x7d, 0x7d, 0x7d);//juce::Colours::grey;
    case Red:
        return juce::Colour(0xf3, 0x00, 0x14);//juce::Colours::red;
    case Yellow:
        return juce::Colour(0xfe, 0xff, 0x00);//juce::Colours::yellowgreen;
    case Green:
        return juce::Colour(0x54, 0xad, 0x20);//juce::Colours::forestgreen;
    case Off:
    default:
        return juce::Colour(0x43, 0x43, 0x43);//juce::Colours::darkgrey;
    }
}

void LevelMeter::SetLevelValue(const float level)
{
    m_currentLevelValue = level;

    repaint();
}

void LevelMeter::SetLevelPeakValue(const float levelPeak)
{
    m_currentLevelPeakValue = levelPeak;

    repaint();
}

void LevelMeter::SetLevelRange(const juce::Range<float>& range)
{
    m_levelRange = juce::NormalisableRange<float>(range);

    repaint();
}

void LevelMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    paintLevelMeter(g, bounds);
}

void LevelMeter::paintLevelMeter(juce::Graphics& g, juce::Rectangle<int>& bounds)
{
    g.setColour(getLookAndFeel().findColour(juce::TextEditor::outlineColourId));
    g.drawRect(bounds);

    auto levelRectBounds = bounds.reduced(1);
    g.setColour(getLookAndFeel().findColour(juce::TextEditor::backgroundColourId));
    g.fillRect(levelRectBounds);

    auto levelRectHeight = levelRectBounds.getHeight();

    auto level0to1 = m_levelRange.convertTo0to1(m_currentLevelValue);
    auto levelRect = levelRectBounds.removeFromBottom(static_cast<int>(levelRectHeight * level0to1));
    auto levelGreenColour = GetColourForState(State::Green);
    g.setColour(levelGreenColour);
    g.fillRect(levelRect);

    auto levelPeak0to1 = m_levelRange.convertTo0to1(m_currentLevelPeakValue);
    auto levelPeakRect = levelRectBounds.removeFromBottom(static_cast<int>(levelRectHeight * (levelPeak0to1 - level0to1)));
    g.setColour(getLookAndFeel().findColour(juce::TextEditor::textColourId));
    g.fillRect(levelPeakRect.removeFromTop(1));

}


LevelMeterWithISP::LevelMeterWithISP()
    : LevelMeter()
{
}

LevelMeterWithISP::~LevelMeterWithISP()
{
}

void LevelMeterWithISP::SetISPState(const bool ispState)
{
    m_currentISPState = ispState;

    repaint();
}

void LevelMeterWithISP::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    paintLevelMeterWithISP(g, bounds);
}

void LevelMeterWithISP::paintLevelMeterWithISP(juce::Graphics& g, juce::Rectangle<int>& bounds)
{
    auto ispBounds = bounds;
    ispBounds = ispBounds.removeFromBottom(getWidth());
    paintISP(g, ispBounds);

    bounds.removeFromBottom(getWidth() - 1);
    paintLevelMeter(g, bounds);
}

void LevelMeterWithISP::paintISP(juce::Graphics& g, juce::Rectangle<int>& bounds)
{
    auto ispBounds = bounds;

    g.setColour(getLookAndFeel().findColour(juce::TextEditor::outlineColourId));
    g.drawRect(ispBounds);

    auto ispFillBounds = ispBounds.reduced(1);
    if (m_currentISPState)
    {
        auto ispGreenColour = GetColourForState(State::Green);
        g.setColour(ispGreenColour);
    }
    else
    {
        g.setColour(getLookAndFeel().findColour(juce::TextEditor::backgroundColourId));
    }
    g.fillRect(ispFillBounds);

    g.setColour(getLookAndFeel().findColour(juce::TextEditor::outlineColourId));
    auto font = g.getCurrentFont();
    font.setHeight(0.5f * ispBounds.getHeight());
    g.setFont(font);
    g.drawText("ISP", ispBounds, juce::Justification::centred);
}


LevelMeterWithISPGROVL::LevelMeterWithISPGROVL()
    : LevelMeterWithISP()
{
}

LevelMeterWithISPGROVL::~LevelMeterWithISPGROVL()
{
}

void LevelMeterWithISPGROVL::SetGRState(const bool grState)
{
    m_currentGRState = grState;

    repaint();
}

void LevelMeterWithISPGROVL::SetOVLState(const bool ovlState)
{
    m_currentOVLState = ovlState;

    repaint();
}

void LevelMeterWithISPGROVL::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    paintLevelMeterWithISPGROVL(g, bounds);
}

void LevelMeterWithISPGROVL::paintLevelMeterWithISPGROVL(juce::Graphics& g, juce::Rectangle<int>& bounds)
{
    auto ovlBounds = bounds;
    ovlBounds = ovlBounds.removeFromTop(getWidth() - 1);
    paintOVL(g, ovlBounds);

    auto grBounds = bounds;
    grBounds.removeFromTop(ovlBounds.getHeight() - 1);
    grBounds = grBounds.removeFromTop(getWidth() - 1);
    paintGR(g, grBounds);
    
    auto levelMeterWithISPBounds = bounds;
    levelMeterWithISPBounds.removeFromTop(ovlBounds.getHeight() - 1);
    levelMeterWithISPBounds.removeFromTop(grBounds.getHeight() - 1);
    paintLevelMeterWithISP(g, levelMeterWithISPBounds);
}

void LevelMeterWithISPGROVL::paintGR(juce::Graphics& g, juce::Rectangle<int>& bounds)
{
    auto grBounds = bounds;

    g.setColour(getLookAndFeel().findColour(juce::TextEditor::outlineColourId));
    g.drawRect(grBounds);

    auto grFillBounds = grBounds.reduced(1);
    if (m_currentGRState)
    {
        auto grYellowColour = GetColourForState(State::Yellow);
        g.setColour(grYellowColour);
    }
    else
    {
        g.setColour(getLookAndFeel().findColour(juce::TextEditor::backgroundColourId));
    }
    g.fillRect(grFillBounds);

    g.setColour(getLookAndFeel().findColour(juce::TextEditor::outlineColourId));
    auto font = g.getCurrentFont();
    font.setHeight(0.5f * grBounds.getHeight());
    g.setFont(font);
    g.drawText("GR", grBounds, juce::Justification::centred);
}

void LevelMeterWithISPGROVL::paintOVL(juce::Graphics& g, juce::Rectangle<int>& bounds)
{
    auto ovlBounds = bounds;

    g.setColour(getLookAndFeel().findColour(juce::TextEditor::outlineColourId));
    g.drawRect(ovlBounds);

    auto ovlFillBounds = ovlBounds.reduced(1);
    if (m_currentOVLState)
    {
        auto ovlRedColour = GetColourForState(State::Red);
        g.setColour(ovlRedColour);
    }
    else
    {
        g.setColour(getLookAndFeel().findColour(juce::TextEditor::backgroundColourId));
    }
    g.fillRect(ovlFillBounds);

    g.setColour(getLookAndFeel().findColour(juce::TextEditor::outlineColourId));
    auto font = g.getCurrentFont();
    font.setHeight(0.5f * ovlBounds.getHeight());
    g.setFont(font);
    g.drawText("OVL", ovlBounds, juce::Justification::centred);
}


};
