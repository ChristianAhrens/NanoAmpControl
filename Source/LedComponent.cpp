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

#include "LedComponent.h"


namespace NanoAmpControl
{


LedComponent::LedComponent()
    : juce::Component()
{
}

LedComponent::~LedComponent()
{
}

void LedComponent::SetState(const State stateValue)
{
    m_currentStateValue = stateValue;

    repaint();
}

void LedComponent::SetOutlineThickness(float thickness)
{
    m_outlineThickness = thickness;

    repaint();
}

void LedComponent::SetTextLabel(const juce::String& textLabel)
{
    m_textLabel = textLabel;

    repaint();
}

const juce::Colour LedComponent::GetColourForState(const State stateValue)
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

void LedComponent::paint(juce::Graphics& g)
{
    auto ledBounds = getLocalBounds().reduced(static_cast<int>(m_outlineThickness)).toFloat();

    // led fill - state dependant
    g.setColour(GetColourForState(m_currentStateValue));
    g.fillEllipse(ledBounds);

    // textlabel if present
    if (m_textLabel.isNotEmpty())
    {
        g.setColour(getLookAndFeel().findColour(juce::TextEditor::outlineColourId));
        auto font = g.getCurrentFont();
        font.setHeight(ledBounds.getHeight() / 2);
        g.setFont(font);
        g.drawText(m_textLabel, ledBounds, juce::Justification::centred);
    }

    // led outline
    g.setColour(getLookAndFeel().findColour(juce::TextEditor::outlineColourId));
    g.drawEllipse(ledBounds, m_outlineThickness);
}


};
