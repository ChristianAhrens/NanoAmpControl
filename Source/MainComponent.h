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

/**
 * Fwd. decl.
 */
namespace NanoAmpControl
{
    class NanoAmpControl;
    class ComponentContainer;
}

class MainComponent   :  public juce::Component
{
public:
    //==========================================================================
    MainComponent(int ampCount, const juce::Rectangle<int> initSize);
    ~MainComponent() override;
    
    //==========================================================================
    void paint(juce::Graphics& g) override;
    void resized() override;

protected:
    //==========================================================================
    int AddAmpControlInstance();
    void RemoveAmpControlInstance(int id);

private:
    //==========================================================================
    int                                                             m_ampControlsIdCount{ 0 };
    std::map<int, std::unique_ptr<NanoAmpControl::NanoAmpControl>>  m_ampControls;
    std::unique_ptr<NanoAmpControl::ComponentContainer>             m_componentsContainer;
    std::unique_ptr<juce::Viewport>                                 m_viewPort;
    juce::SharedResourcePointer<juce::TooltipWindow>                m_tooltipWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
