//
// DetectorModule.hh
// Base class for composable detector subsystems.
//
// Each module builds one subsystem's volumes into the world and attaches its
// sensitive detectors. DetectorConstruction owns an ordered list of modules
// per experiment configuration (prad / drad / prad2 / x17 / test) — adding a
// new configuration means assembling a new module list, not editing a
// monolithic geometry function.
//
// Conventions:
//  - BuildVolumes() is called from DetectorConstruction::Construct(); all
//    positions are in the world frame (mm internally, Geant4 units).
//  - BuildSDs() is called from ConstructSDandField(); modules keep pointers
//    to the logical volumes they created and attach SDs directly via
//    G4LogicalVolume::SetSensitiveDetector().
//  - Volume names, copy numbers and SD names/abbreviations are part of the
//    data contract (branch names, detector IDs) — change them deliberately.
//

#ifndef DetectorModule_h
#define DetectorModule_h 1

#include "G4String.hh"

class G4LogicalVolume;

class DetectorModule
{
public:
    explicit DetectorModule(const G4String &name) : fName(name) {}
    virtual ~DetectorModule() = default;

    const G4String &Name() const { return fName; }

    // Build geometry into the world volume
    virtual void BuildVolumes(G4LogicalVolume *world) = 0;

    // Create and attach sensitive detectors (optional)
    virtual void BuildSDs() {}

protected:
    G4String fName;
};

#endif
