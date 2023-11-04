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


class FillHelperComponent : public juce::Component
{
public:
    //==============================================================================
    void paint(juce::Graphics& g) override
    {
        g.setColour(getLookAndFeel().findColour(juce::TextEditor::outlineColourId));
        g.fillRect(getLocalBounds().toFloat());
    }
};


class ComponentContainer : public juce::Component
{
public:
    //==========================================================================
    enum Direction
    {
        Horizontal,
        Vertical
    };

public:
    //==========================================================================
    ComponentContainer(const Direction layoutDirection, const juce::Rectangle<int>& singleSize);
    ~ComponentContainer() override;

    void AddComponent(juce::Component* component);
    void RemoveComponent(juce::Component* component);

    bool IsLayoutingHorizontally();
    void SetFixWidth(int width);
    void SetFixHeight(int height);

    void resized() override;

private:
    //==========================================================================
    void ProcessSize();

    //==========================================================================
    std::vector<juce::Component*>                                       m_containedComponents;
    std::map<juce::Component*, std::unique_ptr<FillHelperComponent>>    m_separatorLines;

    Direction                       m_layoutDirection;
    juce::Rectangle<int>            m_singleSize;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ComponentContainer)
};

};
