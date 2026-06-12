//
// TargetModule.hh
// Target subsystems:
//  - PRadTargetModule: windowless gas-flow cell (PRad / PRad-II / X17 / test).
//    Copper cell + kapton windows for the gas target; aluminum cell for the
//    LH2 option; bare foil for the Ta option (ported from PRadSim_PRad2).
//  - DRadTargetModule: DRad gas cell with the silicon recoil detector
//    mounted inside the target volume.
//

#ifndef TargetModule_h
#define TargetModule_h 1

#include "detector/DetectorModule.hh"

#include "G4String.hh"

class G4LogicalVolume;

class PRadTargetModule : public DetectorModule
{
public:
    // SD choice: PRad uses a scattering-check SD, the test setup records
    // every step in the target gas.
    enum class SdStyle { kNone, kCheckScattering, kStepRecord };

    // center   : world z of the target center
    // r, halfL : target gas radius and half length
    // material : "H2Gas" (default), "hydrogen", "LH2", "Ta"
    // apertureR: cell window aperture radius (2 mm PRad-I, 1 mm PRad-II)
    PRadTargetModule(double center, double r, double halfL,
                     const G4String &material, double apertureR,
                     SdStyle sdStyle, bool sdOn);

    void BuildVolumes(G4LogicalVolume *world) override;
    void BuildSDs() override;

private:
    double fCenter, fR, fHalfL, fApertureR;
    G4String fMaterial;
    SdStyle fSdStyle;
    bool fSdOn;

    G4LogicalVolume *fTargetLV = nullptr;
};

class DRadTargetModule : public DetectorModule
{
public:
    DRadTargetModule(double center, double r, double halfL, const G4String &material,
                     double recoilCenter, int recoilNSeg, double recoilR,
                     double recoilHalfL, double recoilL1Thickness, double recoilL2Thickness,
                     bool recoilSdOn);

    void BuildVolumes(G4LogicalVolume *world) override;
    void BuildSDs() override;

private:
    double fCenter, fR, fHalfL;
    G4String fMaterial;
    double fRecoilCenter;
    int fRecoilNSeg;
    double fRecoilR, fRecoilHalfL, fRecoilL1Thickness, fRecoilL2Thickness;
    bool fRecoilSdOn;

    G4LogicalVolume *fRecoilDet1LV = nullptr;
    G4LogicalVolume *fRecoilDet2LV = nullptr;
};

#endif
