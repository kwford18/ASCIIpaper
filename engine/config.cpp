#include "engine/config.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace Aquarium::Engine {

    std::string Config::Trim(const std::string& str) {
        auto start = std::find_if_not(str.begin(), str.end(), [](int c) { return std::isspace(c); });
        auto end = std::find_if_not(str.rbegin(), str.rend(), [](int c) { return std::isspace(c); }).base();
        return (start < end) ? std::string(start, end) : std::string();
    }

    bool Config::Load(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return false;
        return LoadFromStream(file);
    }

    bool Config::LoadFromStream(std::istream& stream) {
        std::string line;
        while (std::getline(stream, line)) {
            line = Trim(line);
            
            // Skip empty lines and comments
            if (line.empty() || line[0] == '#') continue;

            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = Trim(line.substr(0, delimiterPos));
                std::string value = Trim(line.substr(delimiterPos + 1));
                m_values[key] = value;
            }
        }
        return true;
    }

    int Config::GetInt(const std::string& key, int defaultValue) const {
        auto it = m_values.find(key);
        if (it != m_values.end()) {
            try { return std::stoi(it->second); } 
            catch (...) {} // Fallback on parsing error
        }
        return defaultValue;
    }

    float Config::GetFloat(const std::string& key, float defaultValue) const {
        auto it = m_values.find(key);
        if (it != m_values.end()) {
            try { return std::stof(it->second); } 
            catch (...) {}
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

} // namespace Aquarium::Engine