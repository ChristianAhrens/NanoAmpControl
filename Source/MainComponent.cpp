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

#include "MainComponent.h"

#include "NanoAmpControl.h"
#include "ComponentContainer.h"

#include <iOS_utils.h>

MainComponent::MainComponent(int ampCount, const juce::Rectangle<int> initSize)
    : juce::Component()
{
    m_componentsContainer = std::make_unique<NanoAmpControl::ComponentContainer>(
        NanoAmpControl::ComponentContainer::Direction::Horizontal,
        initSize);

    m_viewPort = std::make_unique<juce::Viewport>();
    m_viewPort->setViewedComponent(m_componentsContainer.get());
    addAndMakeVisible(m_viewPort.get());

    for (int i = 0; i < ampCount; i++)
        AddAmpControlInstance();

    setSize(initSize.getWidth(), initSize.getHeight());
}

MainComponent::~MainComponent()
{
    auto activeIndices = std::vector<int>();
    for (auto const& ampControlInstanceKV : m_ampControls)
        activeIndices.push_back(ampControlInstanceKV.first);
    for (auto const& activeIndex : activeIndices)
        RemoveAmpControlInstance(activeIndex);

    m_componentsContainer->removeAllChildren();
    m_componentsContainer.reset();

    m_viewPort->removeAllChildren();
    m_viewPort.reset();
}

int MainComponent::AddAmpControlInstance()
{
    jassert(m_componentsContainer);
    if (!m_componentsContainer)
        return -1;

    auto newIndex = m_ampControlsIndexCount++;

    m_ampControls[newIndex] = std::make_unique<NanoAmpControl::NanoAmpControl>();
    m_componentsContainer->AddComponent(m_ampControls.at(newIndex)->getUIComponent());

    return newIndex;
}

void MainComponent::RemoveAmpControlInstance(int index)
{
    jassert(m_componentsContainer);
    if (!m_componentsContainer)
        return;

    if (1 == m_ampControls.count(index) && m_ampControls.at(index)->getUIComponent())
    {
        m_componentsContainer->RemoveComponent(m_ampControls.at(index)->getUIComponent());
        m_ampControls.erase(index);
    }
    else
        jassertfalse;
}

void MainComponent::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto safety = JUCEAppBasics::iOS_utils::getDeviceSafetyMargins();
    auto safeBounds = getLocalBounds();
    safeBounds.removeFromTop(safety._top);
    safeBounds.removeFromBottom(safety._bottom);
    safeBounds.removeFromLeft(safety._left);
    safeBounds.removeFromRight(safety._right);

    m_viewPort->setBounds(safeBounds);

    if (m_componentsContainer->IsLayoutingHorizontally())
        m_componentsContainer->SetFixHeight(safeBounds.getHeight());
    else
        m_componentsContainer->SetFixWidth(safeBounds.getWidth());
}

