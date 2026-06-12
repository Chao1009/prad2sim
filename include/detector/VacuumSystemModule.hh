//
// VacuumSystemModule.hh
// Downstream target chamber, vacuum box and exit tube.
//
//  - kPRad1: the PRad-I system (DetectorCommon.cc AddVaccumBox): chamber +
//    vacuum box + arc-end window + stainless vacuum tube to the world end.
//  - kPRad2: the PRad-II / X17 system (PRadSim_PRad2 AddVaccumBox): extended
//    chamber with the scintillator-housing cutout, repositioned vacuum box,
//    and either the small aluminum vacuum tube (UseHeBag=false) or the
//    kapton He-bag tube with thick/hole windows (UseHeBag=true). The CAD
//    window assembly is handled separately by CadInsertsModule. The optional
//    beam-pipe shielding boxes are ported from PRadSim_X17.
//

#ifndef VacuumSystemModule_h
#define VacuumSystemModule_h 1

#include "detector/DetectorModule.hh"

class G4LogicalVolume;

class VacuumSystemModule : public DetectorModule
{
public:
    enum class Style { kPRad1, kPRad2 };

    // worldSizeZ is needed by the PRad-I tube (runs to the world edge);
    // sciHouseCenter and gemCenter0/1 are needed by the PRad-II chamber
    // cutout and the X17 shielding formulas.
    VacuumSystemModule(Style style, double targetCenter, double worldSizeZ,
                       double sciHouseCenter = 0, bool useHeBag = false,
                       bool useShielding = false,
                       double gemCenter0 = 0, double gemCenter1 = 0);

    void BuildVolumes(G4LogicalVolume *world) override;

private:
    void BuildPRad1(G4LogicalVolume *world);
    void BuildPRad2(G4LogicalVolume *world);

    Style fStyle;
    double fTargetCenter, fWorldSizeZ, fSciHouseCenter;
    bool fUseHeBag, fUseShielding;
    double fGEMCenter0, fGEMCenter1;
};

#endif
