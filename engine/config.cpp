#include "engine/config.h"
#include "engine/logger.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>

namespace ASCIIpaper::Engine {

std::string Config::Trim(const std::string& str) {
    auto start = std::find_if_not(str.begin(), str.end(), [](int c) { return std::isspace(c); });
    auto end =
        std::find_if_not(str.rbegin(), str.rend(), [](int c) { return std::isspace(c); }).base();
    return (start < end) ? std::string(start, end) : std::string();
}

bool Config::Load(const std::string& filename) {
    // Create default config file if there isn't one
    // This ensures the CLI can always be used and that the app can be ran immediately
    std::ifstream checkFile(filename);
    if (!checkFile.good()) {
        checkFile.close();
        ASCII_COUT << "config.ini not found. Generating default configuration at: " << filename
                   << '\n';

        std::ofstream newConfig(filename);
        if (newConfig.is_open()) {
            newConfig << "# scene: aquarium/city\n";
            newConfig << "scene = aquarium\n\n";

            newConfig << "# Target FPS\n";
            newConfig << "target_fps = 30\n\n";

            newConfig << "# System sync hookup\n";
            newConfig << "system_sync = true\n\n";

            newConfig << "# Aquarium config variables\n";
            newConfig << "fish_count = 6\n";
            newConfig << "bubble_count = 15\n";
            newConfig << "jellyfish_count = 3\n\n";

            newConfig << "# City config variables\n";
            newConfig << "car_count = 12\n";
            newConfig << "star_count = 40\n";
            newConfig << "weather = storm\n";
            newConfig.close();
        } else {
            ASCII_CERR << "Failed to generate default config.ini!" << '\n';
            return false;
        }
    } else {
        checkFile.close();
    }

    std::ifstream file(filename);
    if (!file.is_open())
        return false;
    return LoadFromStream(file);
}

bool Config::LoadFromStream(std::istream& stream) {
    std::string line;
    while (std::getline(stream, line)) {
        line = Trim(line);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#')
            continue;

        size_t delimiterPos = line.find('=');
        if (delimiterPos != std::string::npos) {
            std::string key = Trim(line.substr(0, delimiterPos));
            std::string value = Trim(line.substr(delimiterPos + 1));
            m_values[key] = value;
        }
    }
    return true;
}

// Helper functions
int Config::GetInt(const std::string& key, int defaultValue) const {
    auto it = m_values.find(key);
    if (it != m_values.end()) {
        try {
            return std::stoi(it->second);
        } catch (const std::exception& e) {
            ASCII_CERR << "Config Parse Error: " << e.what() << '\n';
        }
    }
    return defaultValue;
}

float Config::GetFloat(const std::string& key, float defaultValue) const {
    auto it = m_values.find(key);
    if (it != m_values.end()) {
        try {
            return std::stof(it->second);
        } catch (const std::exception& e) {
            ASCII_CERR << "Config Parse Error: " << e.what() << '\n';
        }
    }
    return defaultValue;
}

std::string Config::GetString(const std::string& key, const std::string& defaultValue) const {
    auto it = m_values.find(key);
    if (it != m_values.end()) {
        return it->second;
    }
    return defaultValue;
}

bool Config::GetBool(const std::string& key, bool defaultValue) const {
    std::string val = GetString(key, "");
    if (val.empty()) {
        return defaultValue;
    }

    // Convert the string to lowercase so "True", "TRUE", and "true" all work
    std::transform(val.begin(), val.end(), val.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    return (val == "true" || val == "1" || val == "yes" || val == "on");
}

} // namespace ASCIIpaper::Engine