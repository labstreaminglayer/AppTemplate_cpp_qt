# LSL Application Template

This is the reference template for building Lab Streaming Layer (LSL) applications in C++ with Qt6. Use this as a starting point for creating new LSL-compatible applications.

## Features

- **Modern CMake** (3.28+) with clean, documented structure
- **4-tier liblsl discovery**: source, install_root, system, FetchContent
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

| Option | Default | Description |
|--------|---------|-------------|
| `LSLTEMPLATE_BUILD_GUI` | ON | Build the GUI application |
| `LSLTEMPLATE_BUILD_CLI` | ON | Build the CLI application |
| `LSL_FETCH_IF_MISSING` | ON | Auto-fetch liblsl from GitHub |
| `LSL_FETCH_REF` | (see CMakeLists.txt) | liblsl git ref to fetch (tag, branch, or commit) |
| `LSL_SOURCE_DIR` | - | Path to liblsl source (for development) |
| `LSL_INSTALL_ROOT` | - | Path to installed liblsl |

### liblsl Discovery Priority

The build system searches for liblsl in this order:

1. **LSL_SOURCE_DIR** - Build from local source (for parallel liblsl development)
2. **LSL_INSTALL_ROOT** - Explicit installation path
3. **System** - Standard CMake search paths
4. **FetchContent** - Automatic download from GitHub

### CLI-Only Build

For headless systems or servers:

```bash
cmake -S . -B build -DLSLTEMPLATE_BUILD_GUI=OFF
cmake --build build
```

### Building with Local liblsl

For parallel development with liblsl:

```bash
cmake -S . -B build -DLSL_SOURCE_DIR=/path/to/liblsl
```

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

For local development, the build automatically applies ad-hoc signing with network entitlements. This allows the app to use LSL's multicast discovery.

For distribution, use the signing script:

```bash
# Sign only
./scripts/sign_and_notarize.sh build/install/LSLTemplate.app

# Sign and notarize
export APPLE_CODE_SIGN_IDENTITY_APP="Developer ID Application: Your Name"
export APPLE_NOTARIZE_KEYCHAIN_PROFILE="your-profile"
./scripts/sign_and_notarize.sh build/install/LSLTemplate.app --notarize
```

## GitHub Actions Secrets

For automated signing and notarization, the workflow expects these secrets from the `labstreaminglayer` organization:

| Secret | Description |
|--------|-------------|
| `PROD_MACOS_CERTIFICATE` | Base64-encoded Developer ID Application certificate (.p12) |
| `PROD_MACOS_CERTIFICATE_PWD` | Certificate password |
| `PROD_MACOS_CI_KEYCHAIN_PWD` | Password for temporary CI keychain |
| `PROD_MACOS_NOTARIZATION_APPLE_ID` | Apple ID email for notarization |
| `PROD_MACOS_NOTARIZATION_PWD` | App-specific password for Apple ID |
| `PROD_MACOS_NOTARIZATION_TEAM_ID` | Apple Developer Team ID |

**Important:** These organization secrets must be shared with your repository. In GitHub:
1. Go to Organization Settings → Secrets and variables → Actions
2. For each secret, click to edit and under "Repository access" select the repositories that need access

## License

MIT License - see LICENSE
