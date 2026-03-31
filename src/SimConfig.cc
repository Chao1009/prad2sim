//
// SimConfig.cc
// JSON-based configuration loader for PRadSim.
//

#include "SimConfig.hh"

#include <fstream>
#include <iostream>

SimConfig::SimConfig() : fLoaded(false)
{
}

bool SimConfig::Load(const std::string &path)
{
    std::ifstream ifs(path);

    if (!ifs.is_open()) {
        std::cerr << "SimConfig: cannot open " << path << std::endl;
        fLoaded = false;
        return false;
    }

    fPath = path;

    nlohmann::json config;

    try {
        ifs >> config;
    } catch (const nlohmann::json::parse_error &e) {
        std::cerr << "SimConfig: JSON parse error in " << path << ": " << e.what() << std::endl;
        fLoaded = false;
        return false;
    }

    ifs.close();

    // If the config references a base file, load and merge it
    if (config.contains("_base") && config["_base"].is_string()) {
        std::string baseFile = config["_base"].get<std::string>();
        std::string dir = DirName(path);

        if (!dir.empty())
            baseFile = dir + "/" + baseFile;

        std::ifstream bfs(baseFile);

        if (bfs.is_open()) {
            nlohmann::json base;

            try {
                bfs >> base;
            } catch (const nlohmann::json::parse_error &e) {
                std::cerr << "SimConfig: JSON parse error in base file " << baseFile << ": " << e.what() << std::endl;
            }

            bfs.close();
            fData = Merge(base, config);
        } else {
            std::cerr << "SimConfig: cannot open base file " << baseFile << std::endl;
            fData = config;
        }
    } else {
        fData = config;
    }

    fLoaded = true;
    std::cout << "SimConfig: loaded configuration from " << path << std::endl;
    return true;
}

double SimConfig::GetDouble(const std::string &key, double defaultVal) const
{
    if (!fLoaded) return defaultVal;

    try {
        if (fData.contains(key))
            return fData[key].get<double>();
    } catch (...) {
    }

    return defaultVal;
}

std::string SimConfig::GetString(const std::string &key, const std::string &defaultVal) const
{
    if (!fLoaded) return defaultVal;

    try {
        if (fData.contains(key))
            return fData[key].get<std::string>();
    } catch (...) {
    }

    return defaultVal;
}

double SimConfig::GetDouble(const std::string &section, const std::string &key, double defaultVal) const
{
    if (!fLoaded) return defaultVal;

    try {
        if (fData.contains(section) && fData[section].contains(key))
            return fData[section][key].get<double>();
    } catch (...) {
    }

    return defaultVal;
}

int SimConfig::GetInt(const std::string &section, const std::string &key, int defaultVal) const
{
    if (!fLoaded) return defaultVal;

    try {
        if (fData.contains(section) && fData[section].contains(key))
            return fData[section][key].get<int>();
    } catch (...) {
    }

    return defaultVal;
}

bool SimConfig::GetBool(const std::string &section, const std::string &key, bool defaultVal) const
{
    if (!fLoaded) return defaultVal;

    try {
        if (fData.contains(section) && fData[section].contains(key))
            return fData[section][key].get<bool>();
    } catch (...) {
    }

    return defaultVal;
}

std::string SimConfig::GetString(const std::string &section, const std::string &key, const std::string &defaultVal) const
{
    if (!fLoaded) return defaultVal;

    try {
        if (fData.contains(section) && fData[section].contains(key))
            return fData[section][key].get<std::string>();
    } catch (...) {
    }

    return defaultVal;
}

std::vector<double> SimConfig::GetDoubleArray(const std::string &section, const std::string &key, const std::vector<double> &defaultVal) const
{
    if (!fLoaded) return defaultVal;

    try {
        if (fData.contains(section) && fData[section].contains(key) && fData[section][key].is_array()) {
            std::vector<double> result;

            for (const auto &v : fData[section][key])
                result.push_back(v.get<double>());

            return result;
        }
    } catch (...) {
    }

    return defaultVal;
}

void SimConfig::Print() const
{
    if (!fLoaded) {
        std::cout << "SimConfig: no configuration loaded" << std::endl;
        return;
    }

    std::cout << "SimConfig [" << fPath << "]:" << std::endl;
    std::cout << fData.dump(2) << std::endl;
}

nlohmann::json SimConfig::Merge(const nlohmann::json &base, const nlohmann::json &over)
{
    nlohmann::json result = base;

    for (auto it = over.begin(); it != over.end(); ++it) {
        if (it.key() == "_base")
            continue;

        if (it->is_object() && result.contains(it.key()) && result[it.key()].is_object())
            result[it.key()] = Merge(result[it.key()], *it);
        else
            result[it.key()] = *it;
    }

    return result;
}

std::string SimConfig::DirName(const std::string &path)
{
    size_t pos = path.find_last_of("/\\");

    if (pos != std::string::npos)
        return path.substr(0, pos);

    return "";
}
