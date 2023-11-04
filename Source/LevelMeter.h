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

#pragma once

#include <JuceHeader.h>

namespace NanoAmpControl
{


class LevelMeter : public juce::Component
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
    LevelMeter();
    virtual ~LevelMeter() override;

    //==========================================================================
    virtual void paint(juce::Graphics& g) override;

    //==========================================================================
    void SetLevelValue(const float level);
    void SetLevelPeakValue(const float levelPeak);

    void SetLevelRange(const juce::Range<float>& range);

protected:
    //==========================================================================
    const juce::Colour GetColourForState(const State stateValue);

    //==========================================================================
    void paintLevelMeter(juce::Graphics& g, juce::Rectangle<int>& bounds);

private:
    //==========================================================================
    float m_currentLevelValue{ 0.0f };
    float m_currentLevelPeakValue{ 0.0f };

    juce::NormalisableRange<float>  m_levelRange;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};

class LevelMeterWithISP : public LevelMeter
{
public:
    //==========================================================================
    LevelMeterWithISP();
    virtual ~LevelMeterWithISP() override;

    //==========================================================================
    virtual void paint(juce::Graphics& g) override;

    //==========================================================================
    void SetISPState(const bool ispState);

protected:
    //==========================================================================
    void paintLevelMeterWithISP(juce::Graphics& g, juce::Rectangle<int>& bounds);

private:
    //==========================================================================
    void paintISP(juce::Graphics& g, juce::Rectangle<int>& bounds);

    //==========================================================================
    bool m_currentISPState{ false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeterWithISP)
};

class LevelMeterWithISPGROVL : public LevelMeterWithISP
{
public:
    //==========================================================================
    LevelMeterWithISPGROVL();
    virtual ~LevelMeterWithISPGROVL() override;

    //==========================================================================
    virtual void paint(juce::Graphics& g) override;

    //==========================================================================
    void SetGRState(const bool grState);
    void SetOVLState(const bool ovlState);

protected:
    //==========================================================================
    void paintLevelMeterWithISPGROVL(juce::Graphics& g, juce::Rectangle<int>& bounds);

private:
    //==========================================================================
    void paintGR(juce::Graphics& g, juce::Rectangle<int>& bounds);
    void paintOVL(juce::Graphics& g, juce::Rectangle<int>& bounds);

    //==========================================================================
    bool m_currentGRState{ false };
    bool m_currentOVLState{ false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeterWithISPGROVL)
};


};
