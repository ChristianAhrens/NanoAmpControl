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


/**
 * Fwd. Decls
 */
namespace NanoOcp1
{
class NanoOcp1;
class NanoOcp1Client;
}

namespace NanoAmpControl
{


//==============================================================================
/**
 *
 */
class NanoAmpControlProcessor
{
public:
    //==============================================================================
    NanoAmpControlProcessor();
    ~NanoAmpControlProcessor();

    //==============================================================================
    bool UpdateConnectionParameters(const juce::String& address, const int port);

protected:
    //==============================================================================


private:
    //==============================================================================

    //==============================================================================
    std::unique_ptr<NanoOcp1::NanoOcp1Client>  m_nanoOcp1Client;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NanoAmpControlProcessor)
};

} // namespace SurroundFieldMixer
