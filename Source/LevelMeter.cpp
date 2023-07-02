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

    g.setColour(getLookAndFeel().findColour(juce::TextEditor::outlineColourId));
    g.drawRect(bounds);

    auto levelRectBounds = bounds.reduced(1);
    g.setColour(getLookAndFeel().findColour(juce::TextEditor::backgroundColourId));
    g.fillRect(levelRectBounds);

    auto levelRectHeight = levelRectBounds.getHeight();

    auto level0to1 = m_levelRange.convertTo0to1(m_currentLevelValue);
    auto levelRect = levelRectBounds.removeFromBottom(static_cast<int>(levelRectHeight * level0to1));
    auto levelGreenColour = juce::Colour(0x54, 0xad, 0x20);
    g.setColour(levelGreenColour);
    g.fillRect(levelRect);

    auto levelPeak0to1 = m_levelRange.convertTo0to1(m_currentLevelPeakValue);
    auto levelPeakRect = levelRectBounds.removeFromBottom(static_cast<int>(levelRectHeight * (levelPeak0to1 - level0to1)));
    g.setColour(getLookAndFeel().findColour(juce::TextEditor::textColourId));
    g.fillRect(levelPeakRect.removeFromTop(1));

}


};
