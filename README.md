![Showreel.001.png](Resources/Documentation/Showreel/Showreel.001.png "NanoAmpControl Headline Icons")

NanoAmpControl is a **PRIVATELY** created and driven project and is a programming experiment to try out ideas on a deskopt/mobile app to control 4-ch amplifiers via AES70/OCP.1 using JUCE framework.

It uses multiple submodules to integrate the required functionality:
| Submodule | Purpose | Weblink |
|:----------|:--------|:--------|
| NanoOcp | minimal AES70/OCP.1 protocol implementation incl. TCP/IP socket handling through JUCE Framework | https://github.com/ChristianAhrens/NanoOcp |
| JUCE-AppBasics | Various JUCE Framework utility extention classes | https://github.com/ChristianAhrens/JUCE-AppBasics |
| ZeroconfSearcher | UI element based on JUCE Framework to discover mdns announcements | https://github.com/ChristianAhrens/ZeroconfSearcher |
| JUCE | UI Framework | https://github.com/juce-framework/JUCE |
| create-dmg | Helper tool for macOS DiskImage creation | https://github.com/create-dmg/create-dmg |

Its sourcecode and prebuilt binaries are made publicly available to enable interested users to experiment, extend and create own adaptations.

Use what is provided here at your own risk!

See [LATEST RELEASE](../../releases/latest) for available binary packages or join iOS TestFlight Beta:
<img src="Resources/AppStore/TestFlightQRCode.png" alt="TestFlight QR Code" width="25%">

![Showreel.002.png](Resources/Documentation/Showreel/Showreel.002.png "NanoAmpControl UI Overview")