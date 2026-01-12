/**
 * @file main.cpp
 * @brief CLI entry point for LSL Template application
 *
 * This provides a headless (no GUI) version of the application,
 * useful for servers, embedded systems, or automated testing.
 */

#include <lsltemplate/Config.hpp>
#include <lsltemplate/Device.hpp>
#include <lsltemplate/StreamThread.hpp>

#include <atomic>
#include <csignal>
#include <iostream>
#include <string>

namespace {

std::atomic<bool> g_shutdown{false};

void signalHandler(int /*signum*/) {
    std::cout << "\nShutdown requested..." << std::endl;
    g_shutdown = true;
}

void printUsage(const char* program_name) {
    std::cout << "Usage: " << program_name << " [options]\n"
              << "\n"
              << "Options:\n"
              << "  -h, --help           Show this help message\n"
              << "  -c, --config FILE    Load configuration from FILE\n"
              << "  -n, --name NAME      Stream name (default: LSLTemplate)\n"
              << "  -t, --type TYPE      Stream type (default: Counter)\n"
              << "  -r, --rate RATE      Sample rate in Hz (default: 10)\n"
              << "  --channels N         Number of channels (default: 1)\n"
              << "\n"
              << "Example:\n"
              << "  " << program_name << " --name MyDevice --rate 256 --channels 8\n"
              << std::endl;
}

void statusCallback(const std::string& message, bool is_error) {
    if (is_error) {
        std::cerr << "[ERROR] " << message << std::endl;
    } else {
        std::cout << "[INFO] " << message << std::endl;
    }
}

} // anonymous namespace

int main(int argc, char* argv[]) {
    // Parse command line arguments
    lsltemplate::AppConfig config;
    std::string config_file;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        } else if ((arg == "-c" || arg == "--config") && i + 1 < argc) {
            config_file = argv[++i];
        } else if ((arg == "-n" || arg == "--name") && i + 1 < argc) {
            config.stream_name = argv[++i];
        } else if ((arg == "-t" || arg == "--type") && i + 1 < argc) {
            config.stream_type = argv[++i];
        } else if ((arg == "-r" || arg == "--rate") && i + 1 < argc) {
            config.sample_rate = std::stod(argv[++i]);
        } else if (arg == "--channels" && i + 1 < argc) {
            config.channel_count = std::stoi(argv[++i]);
        } else {
            std::cerr << "Unknown option: " << arg << std::endl;
            printUsage(argv[0]);
            return 1;
        }
    }

    // Load config file if specified
    if (!config_file.empty()) {
        auto loaded = lsltemplate::ConfigManager::load(config_file);
        if (loaded) {
            config = *loaded;
            std::cout << "Loaded configuration from: " << config_file << std::endl;
        } else {
            std::cerr << "Failed to load config file: " << config_file << std::endl;
            return 1;
        }
    }

    // Set up signal handling for graceful shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    std::cout << "LSL Template CLI" << std::endl;
    std::cout << "Stream: " << config.stream_name << " (" << config.stream_type << ")" << std::endl;
    std::cout << "Channels: " << config.channel_count << " @ " << config.sample_rate << " Hz" << std::endl;
    std::cout << "Press Ctrl+C to stop..." << std::endl;

    // Create mock device (replace with your actual device)
    lsltemplate::MockDevice::Config device_config{
        .name = config.stream_name,
        .type = config.stream_type,
        .channel_count = config.channel_count,
        .sample_rate = config.sample_rate,
        .start_value = config.device_param
    };
    auto device = std::make_unique<lsltemplate::MockDevice>(device_config);

    // Create and start the stream thread
    lsltemplate::StreamThread stream(std::move(device), statusCallback);

    if (!stream.start()) {
        std::cerr << "Failed to start streaming" << std::endl;
        return 1;
    }

    // Wait for shutdown signal
    while (!g_shutdown && stream.isRunning()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Clean shutdown
    stream.stop();

    std::cout << "Shutdown complete." << std::endl;
    return 0;
}
