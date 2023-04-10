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

#pragma once

#include <JuceHeader.h>

namespace NanoAmpControl
{


class LedComponent : public juce::Component
{
public:
    enum State
    {
        Off,
        White,
        Grey,
        Red,
        Yellow,
        Green
    };

public:
    //==========================================================================
    LedComponent();
    ~LedComponent() override;

    //==========================================================================
    void paint(juce::Graphics& g) override;

    //==========================================================================
    void SetState(const State stateValue);
    void SetOutlineThickness(float thickness);

private:
    //==========================================================================
    const juce::Colour GetColourForState(const State stateValue);

    //==========================================================================
    State m_currentStateValue{ State::Off };
    float m_outlineThickness{ 2.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LedComponent)
};

};
