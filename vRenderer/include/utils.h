#pragma once

#include <stdio.h>
#include <json.hpp>
#include <filesystem>

using json = nlohmann::json;

static const char* PREFS_FILE = "user_preferences.json";

// Save preferences to JSON file
template <typename T>
static bool SavePrefs(const std::string& key, const T& value) {
    json j;

    // Try to read existing file
    std::ifstream in(PREFS_FILE);
    if (in.good()) {
        try {
            in >> j;
        }
        catch (...) {
            // If JSON is invalid, start fresh
            j = json::object();
        }
    }
    in.close();

    // Set the value
    j[key] = value;

    // Write to file
    std::ofstream out(PREFS_FILE);
    if (!out) return false;
    // TODO TEMPORARY
    //out << j.dump(4);
    return true;
}

// Get preferences from JSON file
template <typename T>
static bool GetPrefs(const std::string& key, T& outValue) {
    json j;

    std::ifstream in(PREFS_FILE);
    if (!in) return false;

    try {
        in >> j;
    }
    catch (...) {
        return false;
    }

    if (!j.contains(key)) return false;

    try {
        outValue = j[key].get<T>();
        return true;
    }
    catch (...) {
        return false;
    }
}