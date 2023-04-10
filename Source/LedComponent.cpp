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

const juce::Colour LedComponent::GetColourForState(const State stateValue)
{
    switch (stateValue)
    {
    case White:
        return juce::Colours::white;
    case Grey:
        return juce::Colours::grey;
    case Red:
        return juce::Colours::red;
    case Yellow:
        return juce::Colours::yellow;
    case Green:
        return juce::Colours::green;
    case Off:
    default:
        return juce::Colours::darkgrey;
    }
}

void LedComponent::paint(juce::Graphics& g)
{
    auto ledBounds = getLocalBounds().reduced(static_cast<int>(m_outlineThickness));

    // led fill - state dependant
    g.setColour(GetColourForState(m_currentStateValue));
    g.fillEllipse(ledBounds.toFloat());

    // led outline
    g.setColour(getLookAndFeel().findColour(juce::TextEditor::ColourIds::outlineColourId));
    g.drawEllipse(ledBounds.toFloat(), m_outlineThickness);
}


};