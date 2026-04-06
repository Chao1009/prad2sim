//
// ********************************************************************
// * SimConfig.cc                                                      *
// * JSON-based configuration loader for PRadSim.                      *
// ********************************************************************
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "SimConfig.hh"

#include <fstream>
#include <iostream>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

SimConfig::SimConfig() : fLoaded(false)
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

bool SimConfig::Load(const std::string &path)
{
    nlohmann::json config = ReadJsonFile(path);

    if (config.is_null()) {
        fLoaded = false;
        return false;
    }

    fPath = path;

    // If the config references a base file, load and merge it
    if (config.contains("_base") && config["_base"].is_string()) {
        std::string baseFile = config["_base"].get<std::string>();
        std::string dir = DirName(path);

        if (!dir.empty())
            baseFile = dir + "/" + baseFile;

        nlohmann::json base = ReadJsonFile(baseFile);

        if (!base.is_null())
            fData = Merge(base, config);
        else
            fData = config;
    } else {
        fData = config;
    }

    fLoaded = true;
    std::cout << "SimConfig: loaded configuration from " << path << std::endl;
    return true;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

const nlohmann::json *SimConfig::FindNode(const std::string &key) const
{
    if (!fLoaded || !fData.contains(key))
        return nullptr;

    return &fData[key];
}

const nlohmann::json *SimConfig::FindNode(const std::string &section, const std::string &key) const
{
    if (!fLoaded || !fData.contains(section) || !fData[section].contains(key))
        return nullptr;

    return &fData[section][key];
}

template<typename T>
T SimConfig::Extract(const nlohmann::json *node, const T &defaultVal) const
{
    if (!node) return defaultVal;

    try {
        return node->get<T>();
    } catch (...) {
        return defaultVal;
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

double SimConfig::GetDouble(const std::string &key, double defaultVal) const
{
    return Extract(FindNode(key), defaultVal);
}

std::string SimConfig::GetString(const std::string &key, const std::string &defaultVal) const
{
    return Extract(FindNode(key), defaultVal);
}

double SimConfig::GetDouble(const std::string &section, const std::string &key, double defaultVal) const
{
    return Extract(FindNode(section, key), defaultVal);
}

int SimConfig::GetInt(const std::string &section, const std::string &key, int defaultVal) const
{
    return Extract(FindNode(section, key), defaultVal);
}

bool SimConfig::GetBool(const std::string &section, const std::string &key, bool defaultVal) const
{
    return Extract(FindNode(section, key), defaultVal);
}

std::string SimConfig::GetString(const std::string &section, const std::string &key, const std::string &defaultVal) const
{
    return Extract(FindNode(section, key), defaultVal);
}

std::vector<double> SimConfig::GetDoubleArray(const std::string &section, const std::string &key, const std::vector<double> &defaultVal) const
{
    const nlohmann::json *node = FindNode(section, key);

    if (!node || !node->is_array()) return defaultVal;

    try {
        return node->get<std::vector<double>>();
    } catch (...) {
        return defaultVal;
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void SimConfig::Print() const
{
    if (!fLoaded) {
        std::cout << "SimConfig: no configuration loaded" << std::endl;
        return;
    }

    std::cout << "SimConfig [" << fPath << "]:" << std::endl;
    std::cout << fData.dump(2) << std::endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

nlohmann::json SimConfig::ReadJsonFile(const std::string &path)
{
    std::ifstream ifs(path);

    if (!ifs.is_open()) {
        std::cerr << "SimConfig: cannot open " << path << std::endl;
        return nullptr;
    }

    try {
        nlohmann::json data;
        ifs >> data;
        return data;
    } catch (const nlohmann::json::parse_error &e) {
        std::cerr << "SimConfig: JSON parse error in " << path << ": " << e.what() << std::endl;
        return nullptr;
    }
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

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
