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

#include "ComponentContainer.h"


namespace NanoAmpControl
{


ComponentContainer::ComponentContainer(const Direction layoutDirection, const juce::Rectangle<int>& singleSize)
    : juce::Component(), m_layoutDirection(layoutDirection), m_singleSize(singleSize)
{
}

ComponentContainer::~ComponentContainer()
{
}

void ComponentContainer::resized()
{
    juce::FlexBox fb;
    fb.flexDirection = IsLayoutingHorizontally() ? juce::FlexBox::Direction::row : juce::FlexBox::Direction::column;
    fb.flexWrap = juce::FlexBox::Wrap::wrap;
    fb.justifyContent = juce::FlexBox::JustifyContent::center;
    fb.alignContent = juce::FlexBox::AlignContent::center;
    for (auto const& component : m_containedComponents)
    {
        fb.items.add(juce::FlexItem(*component)
            .withMinWidth(float(m_singleSize.getWidth()))
            .withMinHeight(float(m_singleSize.getHeight())));
        if (1 == m_separatorLines.count(component))
            fb.items.add(juce::FlexItem(*m_separatorLines.at(component).get())
                .withWidth(IsLayoutingHorizontally() ? 1.0f : m_singleSize.getWidth())
                .withHeight(IsLayoutingHorizontally() ? m_singleSize.getHeight() : 1.0f));
    }
    fb.performLayout(getLocalBounds());
}

void ComponentContainer::AddComponent(juce::Component* component)
{
    component->setSize(m_singleSize.getWidth(), m_singleSize.getHeight());

    if (!m_containedComponents.empty())
    {
        m_separatorLines[m_containedComponents.back()] = std::make_unique<FillHelperComponent>();
        addAndMakeVisible(m_separatorLines[m_containedComponents.back()].get());
    }

    m_containedComponents.push_back(component);
    addAndMakeVisible(component);

    ProcessSize();
}

void ComponentContainer::RemoveComponent(juce::Component* component)
{
    auto compoIter = std::find(m_containedComponents.begin(), m_containedComponents.end(), component);
    if (compoIter != m_containedComponents.end())
    {
        if (1 == m_separatorLines.count(*compoIter) && m_separatorLines.at(*compoIter))
        {
            removeChildComponent(m_separatorLines.at(*compoIter).get());
            m_separatorLines.erase(*compoIter);
        }

        removeChildComponent(*compoIter);
        m_containedComponents.erase(compoIter);
    }

    ProcessSize();
}

bool ComponentContainer::IsLayoutingHorizontally()
{
    return (m_layoutDirection == Direction::Horizontal);
}

void ComponentContainer::SetFixWidth(int width)
{
    m_singleSize.setWidth(width);

    ProcessSize();
}

void ComponentContainer::SetFixHeight(int height)
{
    m_singleSize.setHeight(height);

    ProcessSize();
}

void ComponentContainer::ProcessSize()
{
    int newWidthsCount = IsLayoutingHorizontally() ? int(m_containedComponents.size()) : 1;
    int newHeightsCount = IsLayoutingHorizontally() ? 1 : int(m_containedComponents.size());

    int newWidth = newWidthsCount * m_singleSize.getWidth() + (IsLayoutingHorizontally() ? newWidthsCount - 1 : 0);
    int newHeight = newHeightsCount* m_singleSize.getHeight() + (!IsLayoutingHorizontally() ? newHeightsCount - 1 : 0);;

    setSize(newWidth, newHeight);
}


};
