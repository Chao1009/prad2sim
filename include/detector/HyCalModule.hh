//
// HyCalModule.hh
// The HyCal hybrid calorimeter: 1152 PbWO4 crystals + 576 lead-glass
// modules with reflectors, brass plates/strips, Rohacell box and the
// tungsten collimator. Module sizes and positions are read from a
// geometry table (database/hycal_module_shuffled.dat by default).
//
// One CalorimeterSD ("HyCalSD", branch prefix "HC") spans all absorbers;
// per-module energies are indexed by the copy-number sum, giving
// 0..575 for lead glass and 576..1727 for PbWO4.
//

#ifndef HyCalModule_h
#define HyCalModule_h 1

#include "detector/DetectorModule.hh"

#include "G4String.hh"

#include <vector>

class G4LogicalVolume;

class HyCalModule : public DetectorModule
{
public:
    // crystalSurf: world z of the PbWO4 front surface
    HyCalModule(double crystalSurf, double attenuationLG,
                bool sdOn, const G4String &moduleFile = "database/hycal_module_shuffled.dat",
                const G4String &attenuationFile = "database/pwo_attenuation.dat");

    void BuildVolumes(G4LogicalVolume *world) override;
    void BuildSDs() override;

private:
    double fCrystalSurf;
    double fAttenuationLG;
    bool fSdOn;
    G4String fModuleFile;
    G4String fAttenuationFile;

    std::vector<G4LogicalVolume *> fAbsorberLVs;
};

#endif
