//
// ********************************************************************
// * SimConfig.hh                                                      *
// * JSON-based configuration loader for PRadSim.                      *
// *                                                                   *
// * Loads parameters from JSON files with inheritance support:         *
// * each config file may reference a "_base" file whose values        *
// * are merged (config-specific values override base values).         *
// *                                                                   *
// * All lengths in JSON are in cm, energies in MeV, angles in deg.    *
// * Conversion to Geant4 internal units happens in the calling code.  *
// ********************************************************************
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#ifndef SimConfig_h
#define SimConfig_h 1

#include "json.hh"

#include <string>
#include <vector>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

class SimConfig
{
public:
    SimConfig();

    bool Load(const std::string &path);

    bool IsLoaded() const { return fLoaded; }
    const std::string &GetPath() const { return fPath; }

    // Top-level key access
    double GetDouble(const std::string &key, double defaultVal) const;
    std::string GetString(const std::string &key, const std::string &defaultVal) const;

    // Nested section.key access
    double GetDouble(const std::string &section, const std::string &key, double defaultVal) const;
    int GetInt(const std::string &section, const std::string &key, int defaultVal) const;
    bool GetBool(const std::string &section, const std::string &key, bool defaultVal) const;
    std::string GetString(const std::string &section, const std::string &key, const std::string &defaultVal) const;
    std::vector<double> GetDoubleArray(const std::string &section, const std::string &key, const std::vector<double> &defaultVal) const;

    void Print() const;

private:
    // Lookup a JSON node by key path; returns nullptr if not found
    const nlohmann::json *FindNode(const std::string &key) const;
    const nlohmann::json *FindNode(const std::string &section, const std::string &key) const;

    // Typed extraction from a JSON node with default fallback
    template<typename T>
    T Extract(const nlohmann::json *node, const T &defaultVal) const;

    // Read and parse a JSON file; returns empty object on failure
    static nlohmann::json ReadJsonFile(const std::string &path);

    static nlohmann::json Merge(const nlohmann::json &base, const nlohmann::json &over);
    static std::string DirName(const std::string &path);

    nlohmann::json fData;
    bool fLoaded;
    std::string fPath;
};

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#endif
