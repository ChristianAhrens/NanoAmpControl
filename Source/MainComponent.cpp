/* Copyright (c) 2022, Christian Ahrens
 *
 * This file is part of SurroundFieldMixer <https://github.com/ChristianAhrens/SurroundFieldMixer>
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

#include "SurroundFieldMixer.h"

#include <iOS_utils.h>

MainComponent::MainComponent()
    : juce::Component()
{
    m_ssm = std::make_unique<SurroundFieldMixer::SurroundFieldMixer>();
    m_ssm->onRemoteOnlineCallback = std::bind(static_cast<void(MainComponent::*)(void)>(&MainComponent::repaint), this);
    addAndMakeVisible(m_ssm->getUIComponent());

    m_setupToggleButton = std::make_unique<TextButton>("Audio Device Setup");
    m_setupToggleButton->onClick = [this] {
        auto setupComponent = m_ssm->getDeviceSetupComponent();
        if (setupComponent)
        {
            if (setupComponent->isVisible())
            {
                removeChildComponent(setupComponent);
                setupComponent->setVisible(false);
            }
            else
            {
                setupComponent->setVisible(true);
                addAndMakeVisible(setupComponent);
            }

            resized();
        }
    };
    addAndMakeVisible(m_setupToggleButton.get());

    m_lockLayoutButton = std::make_unique<TextButton>("Lock Layout");
    m_lockLayoutButton->setClickingTogglesState(true);
    m_lockLayoutButton->onClick = [this] {
        m_ssm->lockCurrentLayout(m_lockLayoutButton->getToggleState());
    };
    addAndMakeVisible(m_lockLayoutButton.get());

    setSize(900, 600);
}

MainComponent::~MainComponent()
{
}

void MainComponent::paint(Graphics &g)
{
    g.fillAll(getLookAndFeel().findColour(AlertWindow::backgroundColourId).darker());
    
    auto safety = JUCEAppBasics::iOS_utils::getDeviceSafetyMargins();
    auto safeBounds = getLocalBounds();
    safeBounds.removeFromTop(safety._top);
    safeBounds.removeFromBottom(safety._bottom);
    safeBounds.removeFromLeft(safety._left);
    safeBounds.removeFromRight(safety._right);
    
    auto setupAreaBounds = safeBounds.removeFromTop(26);
    g.setColour(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    g.fillRect(setupAreaBounds.reduced(6, 0));

    if (m_ssm)
    {
        setupAreaBounds.removeFromLeft(5);
        auto onlineButtonBounds = setupAreaBounds.removeFromLeft(setupAreaBounds.getHeight()).reduced(5).toFloat();
        if (m_ssm->isControlOnline())
        {
            g.setColour(juce::Colours::white.darker());
            g.fillEllipse(onlineButtonBounds);
            g.setColour(juce::Colours::black);
            g.drawEllipse(onlineButtonBounds, 1);
        }
        else
        {
            g.setColour(juce::Colours::grey);
            g.fillEllipse(onlineButtonBounds);
            g.setColour(juce::Colours::black);
            g.drawEllipse(onlineButtonBounds, 1);
        }
    }
}

void MainComponent::resized()
{
    auto safety = JUCEAppBasics::iOS_utils::getDeviceSafetyMargins();
    auto safeBounds = getLocalBounds();
    safeBounds.removeFromTop(safety._top);
    safeBounds.removeFromBottom(safety._bottom);
    safeBounds.removeFromLeft(safety._left);
    safeBounds.removeFromRight(safety._right);

    auto margin = 3;
    auto setupAreaBounds = safeBounds.removeFromTop(26).reduced(9, margin);
    auto contentAreaBounds = safeBounds;

    if (m_setupToggleButton)
        m_setupToggleButton->setBounds(setupAreaBounds.removeFromRight(100).removeFromTop(20));
    setupAreaBounds.removeFromRight(margin);
    if (m_lockLayoutButton)
        m_lockLayoutButton->setBounds(setupAreaBounds.removeFromRight(100).removeFromTop(20));

    auto SurroundFieldMixerComponent = m_ssm->getUIComponent();
    if (SurroundFieldMixerComponent)
        SurroundFieldMixerComponent->setBounds(contentAreaBounds);

    auto setupComponent = m_ssm->getDeviceSetupComponent();
    if (setupComponent && setupComponent->isVisible())
        setupComponent->setBounds(contentAreaBounds.reduced(15));
}

