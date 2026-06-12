//
// CadInsertsModule.hh
// Config-driven import of CAD (STL) volumes via CADMesh.
//
// Each insert is one STL file placed at a z offset from the target center
// (the convention used by PRadSim_PRad2 / PRadSim_X17, whose STL files
// carry their absolute transverse position internally). The insert list
// comes from the JSON configuration:
//
//   "cad": {
//     "model_dir": "database/CADmodel",
//     "inserts": [
//       {"stl": "PRad_Housing.stl", "material": "SSteel",
//        "name": "HOUSING", "z_offset_mm": 308.38},
//       {"stl": "Solid008.stl", "material": "Aluminum",
//        "name": "Window9", "z_offset_mm": -1280.0},
//       ...
//     ]
//   }
//
// Built only when prad2sim is compiled with CADMesh support
// (PRAD2SIM_USE_CADMESH); otherwise inserts are reported and skipped.
// Missing STL files are skipped with a loud warning so the simulation can
// run without the (large, not version-controlled) CAD model files.
//

#ifndef CadInsertsModule_h
#define CadInsertsModule_h 1

#include "detector/DetectorModule.hh"

#include <string>
#include <vector>

class G4LogicalVolume;

struct CadInsert {
    std::string stl;      // file name inside modelDir
    std::string material; // material name (MaterialBuilder)
    std::string name;     // base name for the LV ("<name>LV") and placement
    double zOffset;       // placement offset from target center (G4 units)
};

class CadInsertsModule : public DetectorModule
{
public:
    CadInsertsModule(double targetCenter, const std::string &modelDir,
                     std::vector<CadInsert> inserts);

    void BuildVolumes(G4LogicalVolume *world) override;

private:
    double fTargetCenter;
    std::string fModelDir;
    std::vector<CadInsert> fInserts;
};

#endif
