#pragma once
#include <string>
#include <unordered_map>
#include <istream>

namespace ASCIIpaper::Engine {

    class Config {
    public:
        Config() = default;

        // Loads settings from a file on disk
        bool Load(const std::string& filename);
        
        // Loads settings from an open stream (great for Unit Testing!)
        bool LoadFromStream(std::istream& stream);

        // Value Getters with Fallbacks
        int GetInt(const std::string& key, int defaultValue) const;
        float GetFloat(const std::string& key, float defaultValue) const;
        std::string GetString(const std::string& key, const std::string& defaultValue) const;

    private:
        std::unordered_map<std::string, std::string> m_values;
        
        // Helper to strip whitespace
        static std::string Trim(const std::string& str);
    };

} // namespace ASCIIpaper::Engine