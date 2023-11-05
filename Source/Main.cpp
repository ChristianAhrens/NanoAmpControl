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

#include <JuceHeader.h>

#include "MainComponent.h"
#include "LookAndFeel.h"

//==============================================================================
class MainApplication  : public juce::JUCEApplication
{
public:
    //==============================================================================
    MainApplication() {}

    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return true; }

    //==============================================================================
    void initialise(const String& commandLine) override
    {
        mainWindow.reset(new MainWindow(getApplicationName(), commandLine));
    }

    void shutdown() override
    {
        mainWindow.reset();
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    }

    void anotherInstanceStarted (const String& commandLine) override
    {
        ignoreUnused(commandLine);
    }

    //==============================================================================
    /*
        This struct implements minimal commandline string parsing
    */
    struct CommandLineParser
    {
        CommandLineParser(){}
        CommandLineParser(const String& commandLine) {
            if (!ReadCommandLine(commandLine))
                juce::JUCEApplication::quit();
        }

        bool ReadCommandLine(const String& commandLine){
            // no commandline input is totally valid
            if (commandLine.isEmpty())
                return true;

            // if help was requested, user should be informed, but startup should be aborted
            if (commandLine.containsWholeWord("-h") || commandLine.containsWholeWord("--help"))
            {
                PrintHelp();
                return false;
            }

            // parse the rest for -i and -s parameter values
            juce::StringArray paramStringArray;
            paramStringArray.addTokens(commandLine, " ", "'");
            for (auto iter = paramStringArray.begin(); iter != paramStringArray.end(); iter++)
            {
                if (iter->containsWholeWord("-i") || iter->containsWholeWord("--instances"))
                {
                    auto dataIter = (iter + 1);
                    if (dataIter)
                        _ampCount = dataIter->getIntValue();
                }

                if (iter->containsWholeWord("-s") || iter->containsWholeWord("--size"))
                {
                    auto dataIter = (iter + 1);
                    if (dataIter)
                    {
                        juce::StringArray sizeValStringArray;
                        sizeValStringArray.addTokens(*dataIter, ",", "'");
                        jassert(sizeValStringArray.size() == 2);
                        if (2 == sizeValStringArray.size())
                            _initSize = juce::Rectangle<int>(sizeValStringArray[0].getIntValue(), sizeValStringArray[1].getIntValue());
                    }
                }
            }

            return true;
        }

        void PrintHelp() {
            std::cout << juce::JUCEApplication::getInstance()->getApplicationName().toStdString() << " commandline options:" << std::endl
                << std::endl
                << "-h | --help         Print this help." << std::endl
                << "-i | --instances    Specify integer value for how many NanoAmpControl instances should be created." << std::endl
                << "-s | --size         Specify two comma separated integer values as initial width and height per NanoAmpControl Instance" << std::endl;
        }

        int                     _ampCount = { 1 };
        juce::Rectangle<int>    _initSize = { 300, 533 };
    };

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow    : public DocumentWindow
    {
    public:
        MainWindow (const String& name, const String& commandLine)  : DocumentWindow (name,
                                                    Desktop::getInstance().getDefaultLookAndFeel()
                                                                          .findColour (ResizableWindow::backgroundColourId),
                                                    DocumentWindow::allButtons)
        {
            CommandLineParser cmdP(commandLine);

            m_customLookAndFeel = std::unique_ptr<LookAndFeel>(new NanoAmpControl::DarkLookAndFeel);
            juce::Desktop::getInstance().setDefaultLookAndFeel(m_customLookAndFeel.get());
            
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent(cmdP._ampCount, cmdP._initSize), true);

           #if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
           #elif JUCE_LINUX
            juce::Desktop::getInstance().setKioskModeComponent(getTopLevelComponent());
           #else
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
           #endif

            setVisible (true);
        }

        void closeButtonPressed() override
        {
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need.
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */

    private:
        std::unique_ptr<LookAndFeel>	m_customLookAndFeel; // our own look and feel implementation instance

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (MainApplication)
