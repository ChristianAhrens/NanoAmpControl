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

/**
 * Fwd. decls
 */
class NanoAmpControlUI;
class NanoAmpControlProcessor;

//==============================================================================
/**
 *
 */
class NanoAmpControl
{
public:
    NanoAmpControl();
    ~NanoAmpControl();
    
    //==========================================================================
    juce::Component* getUIComponent();

    //==========================================================================

private:
    //==========================================================================

    //==========================================================================
    std::unique_ptr<NanoAmpControlProcessor>    m_NanoAmpControlProcessor;
    std::unique_ptr<NanoAmpControlUI>           m_NanoAmpControlUI;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NanoAmpControl)
};

};
