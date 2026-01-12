#include "lsltemplate/Config.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <unistd.h>
#include <pwd.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#else
#include <linux/limits.h>
#endif
#endif

namespace lsltemplate {

namespace {

// Simple INI parser helpers
std::string trim(const std::string& str) {
    auto start = std::find_if_not(str.begin(), str.end(),
        [](unsigned char c) { return std::isspace(c); });
    auto end = std::find_if_not(str.rbegin(), str.rend(),
        [](unsigned char c) { return std::isspace(c); }).base();
    return (start < end) ? std::string(start, end) : std::string();
}

std::filesystem::path getExecutablePath() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path();
#elif defined(__APPLE__)
    char buffer[PATH_MAX];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) == 0) {
        return std::filesystem::path(buffer).parent_path();
    }
    return {};
#else
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        return std::filesystem::path(buffer).parent_path();
    }
    return {};
#endif
}

std::filesystem::path getConfigDirectory() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(nullptr, CSIDL_APPDATA, nullptr, 0, buffer))) {
        return std::filesystem::path(buffer);
    }
    return {};
#elif defined(__APPLE__)
    const char* home = getenv("HOME");
    if (home) {
        return std::filesystem::path(home) / "Library" / "Preferences";
    }
    return {};
#else
    // XDG Base Directory Specification
    const char* xdg_config = getenv("XDG_CONFIG_HOME");
    if (xdg_config && *xdg_config) {
        return std::filesystem::path(xdg_config);
    }
    const char* home = getenv("HOME");
    if (home) {
        return std::filesystem::path(home) / ".config";
    }
    return {};
#endif
}

} // anonymous namespace

std::optional<AppConfig> ConfigManager::load(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return std::nullopt;
    }

    AppConfig config;
    std::string line;
    std::string current_section;

    while (std::getline(file, line)) {
        line = trim(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }

        // Section header
        if (line.front() == '[' && line.back() == ']') {
            current_section = line.substr(1, line.size() - 2);
            continue;
        }

        // Key=value pair
        auto eq_pos = line.find('=');
        if (eq_pos != std::string::npos) {
            std::string key = trim(line.substr(0, eq_pos));
            std::string value = trim(line.substr(eq_pos + 1));

            // Remove quotes if present
            if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
                value = value.substr(1, value.size() - 2);
            }

            // Map to config fields (customize for your application)
            if (key == "name" || key == "stream_name") {
                config.stream_name = value;
            } else if (key == "type" || key == "stream_type") {
                config.stream_type = value;
            } else if (key == "channels" || key == "channel_count") {
                config.channel_count = std::stoi(value);
            } else if (key == "sample_rate" || key == "srate") {
                config.sample_rate = std::stod(value);
            } else if (key == "device" || key == "device_param") {
                config.device_param = std::stoi(value);
            }
        }
    }

    return config;
}

bool ConfigManager::save(const AppConfig& config, const std::filesystem::path& path) {
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }

    file << "# LSL Application Configuration\n";
    file << "[Stream]\n";
    file << "name=" << config.stream_name << "\n";
    file << "type=" << config.stream_type << "\n";
    file << "channels=" << config.channel_count << "\n";
    file << "sample_rate=" << config.sample_rate << "\n";
    file << "\n";
    file << "[Device]\n";
    file << "device_param=" << config.device_param << "\n";

    return file.good();
}

std::filesystem::path ConfigManager::findConfigFile(
    const std::string& filename,
    const std::optional<std::filesystem::path>& hint
) {
    std::vector<std::filesystem::path> search_paths;

    // 1. Hint path
    if (hint && std::filesystem::exists(*hint)) {
        return *hint;
    }

    // 2. Current working directory
    search_paths.push_back(std::filesystem::current_path());

    // 3. Executable directory
    auto exe_path = getExecutablePath();
    if (!exe_path.empty()) {
        search_paths.push_back(exe_path);
    }

    // 4. Platform config directory
    auto config_dir = getConfigDirectory();
    if (!config_dir.empty()) {
        search_paths.push_back(config_dir);
    }

    // Search for the file
    for (const auto& dir : search_paths) {
        auto full_path = dir / filename;
        if (std::filesystem::exists(full_path)) {
            return full_path;
        }
    }

    return {};
}

} // namespace lsltemplate
