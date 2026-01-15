# LSL Application Template

This is the reference template for building Lab Streaming Layer (LSL) applications in C++ with Qt6. Use this as a starting point for creating new LSL-compatible applications.

## Features

- **Modern CMake** (3.28+) with clean, documented structure
- **Automatic liblsl**: fetched via FetchContent, or use a pre-installed version
- **CLI and GUI separation** with shared core library
- **Qt6** for the GUI (with Qt5 intentionally dropped for simplicity)
- **Cross-platform**: Linux, macOS, Windows
- **macOS code signing** with entitlements for network capabilities
- **Automated CI/CD** via GitHub Actions

## Project Structure

```
LSLTemplate/
├── CMakeLists.txt           # Root build configuration
├── app.entitlements         # macOS network capabilities
├── LSLTemplate.cfg          # Default configuration file
├── src/
│   ├── core/                # Qt-independent core library
│   │   ├── include/lsltemplate/
│   │   │   ├── Device.hpp       # Device interface
│   │   │   ├── LSLOutlet.hpp    # LSL outlet wrapper
│   │   │   ├── Config.hpp       # Configuration management
│   │   │   └── StreamThread.hpp # Background streaming
│   │   └── src/
│   ├── cli/                 # Command-line application
│   │   └── main.cpp
│   └── gui/                 # Qt6 GUI application
│       ├── MainWindow.hpp/cpp
│       ├── MainWindow.ui
│       └── main.cpp
├── scripts/
│   └── sign_and_notarize.sh # macOS signing script
└── .github/workflows/
    └── build.yml            # CI/CD workflow
```

## Building

### Prerequisites

- CMake 3.28 or later
- C++20 compatible compiler
- Qt6.8 (for GUI build)
- liblsl (optional - will be fetched automatically if not found)

### Installing Qt 6.8 on Ubuntu

Ubuntu's default repositories don't include Qt 6.8. Use [aqtinstall](https://github.com/miurahr/aqtinstall) to install it:

```bash
# Install aqtinstall
pip install aqtinstall

# Install Qt 6.8 (adjust version as needed)
aqt install-qt linux desktop 6.8.3 gcc_64 -O ~/Qt

# Set environment for CMake to find Qt
export CMAKE_PREFIX_PATH=~/Qt/6.8.3/gcc_64

# Install system dependencies
sudo apt-get install libgl1-mesa-dev libxkbcommon-dev libxcb-cursor0
```

To run the GUI application, ensure Qt libraries are in your library path:

```bash
export LD_LIBRARY_PATH=~/Qt/6.8.3/gcc_64/lib:$LD_LIBRARY_PATH
```

Alternatively, build CLI-only with `-DLSLTEMPLATE_BUILD_GUI=OFF` to avoid the Qt dependency.

### Quick Start

```bash
# Clone and build
git clone https://github.com/labstreaminglayer/AppTemplate_cpp_qt.git
cd AppTemplate_cpp_qt

# Configure (liblsl will be fetched automatically)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --parallel

# Install
cmake --install build --prefix build/install

# Package
cd build && cpack
```

### Build Options

| Option                  | Default              | Description                                      |
|-------------------------|----------------------|--------------------------------------------------|
| `LSLTEMPLATE_BUILD_GUI` | ON                   | Build the GUI application                        |
| `LSLTEMPLATE_BUILD_CLI` | ON                   | Build the CLI application                        |
| `LSL_INSTALL_ROOT`      | -                    | Path to installed liblsl (skips FetchContent)    |
| `LSL_FETCH_REF`         | (see CMakeLists.txt) | liblsl git ref to fetch (tag, branch, or commit) |

### liblsl Discovery

By default, liblsl is fetched automatically from GitHub using CMake's FetchContent. To use a pre-installed liblsl instead, set `LSL_INSTALL_ROOT`:

```bash
cmake -S . -B build -DLSL_INSTALL_ROOT=/path/to/liblsl
```

### CLI-Only Build

For headless systems or servers:

```bash
cmake -S . -B build -DLSLTEMPLATE_BUILD_GUI=OFF
cmake --build build
```

### Parallel Development with liblsl

If you need to develop against a local liblsl source tree (e.g., testing liblsl changes alongside your app), see [App-LabRecorder](https://github.com/labstreaminglayer/App-LabRecorder/) for an example of a more advanced liblsl discovery setup that supports `LSL_SOURCE_DIR`.

## Usage

### GUI Application

```bash
./LSLTemplate                    # Use default config
./LSLTemplate myconfig.cfg       # Use custom config
```

### CLI Application

```bash
./LSLTemplateCLI --help
./LSLTemplateCLI --name MyStream --rate 256 --channels 8
./LSLTemplateCLI --config myconfig.cfg
```

## Customizing for Your Device

1. **Fork/copy this template**
2. **Rename the project** in `CMakeLists.txt`
3. **Implement your device class** by deriving from `IDevice` in `src/core/include/lsltemplate/Device.hpp`
4. **Update the GUI** for device-specific settings in `src/gui/MainWindow.ui`
5. **Update configuration** fields in `src/core/include/lsltemplate/Config.hpp`

## macOS Code Signing

For local development, the build automatically applies ad-hoc signing with network entitlements, allowing the app to use LSL's multicast discovery.

For distribution, apps must be signed with a Developer ID certificate and notarized. See `scripts/sign_and_notarize.sh` and Apple's [Notarizing macOS Software Before Distribution](https://developer.apple.com/documentation/security/notarizing_macos_software_before_distribution) guide.

The GitHub Actions workflow handles signing and notarization automatically using organization secrets:

| Secret                             | Description                                                |
|------------------------------------|------------------------------------------------------------|
| `PROD_MACOS_CERTIFICATE`           | Base64-encoded Developer ID Application certificate (.p12) |
| `PROD_MACOS_CERTIFICATE_PWD`       | Certificate password                                       |
| `PROD_MACOS_NOTARIZATION_APPLE_ID` | Apple ID email for notarization                            |
| `PROD_MACOS_NOTARIZATION_PWD`      | Apple ID App-specific password for the notary tool         |
| `PROD_MACOS_NOTARIZATION_TEAM_ID`  | Apple Developer Team ID                                    |

These secrets must be shared with your repository under Organization Settings → Secrets and variables → Actions → Repository access.

The GHA workflow imports the certificate and stores notarization credentials, then sets environment variables for `sign_and_notarize.sh`:

| Script Environment Variable        | Source                                                                                 |
|------------------------------------|----------------------------------------------------------------------------------------|
| `APPLE_CODE_SIGN_IDENTITY_APP`     | Extracted from imported certificate (e.g., `Developer ID Application: Name (TEAMID)`)  |
| `APPLE_NOTARIZE_KEYCHAIN_PROFILE`  | Created via `notarytool store-credentials` using the three `NOTARIZATION` secrets      |
| `ENTITLEMENTS_FILE`                | Set to `app.entitlements` in the repo                                                  |

To run the script locally, install your Developer ID certificate in Keychain, create a notarization profile with `xcrun notarytool store-credentials`, and set these environment variables.

## License

MIT License - see LICENSE
