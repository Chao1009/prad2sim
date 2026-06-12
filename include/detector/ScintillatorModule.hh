//
// ScintillatorModule.hh
// Scintillator planes:
//  - kSinglePlane: the DRad 150x150 cm EJ204 plane with a beam hole
//    (DetectorDRad.cc), SD "ScintillatorPlaneSD" / branch prefix "SP".
//  - kFourPlane: the PRad-II veto layout (PRadSim_PRad2): four EJ204
//    paddles around the beam axis at two z planes, each backed by a thin
//    virtual plane. SDs "SciDetectorSD1"/"Sci1" (horizontal pair),
//    "SciDetectorSD2"/"Sci2" (vertical pair), "SciVirtualSD"/"SciVD".
//

#ifndef ScintillatorModule_h
#define ScintillatorModule_h 1

#include "detector/DetectorModule.hh"

#include <vector>

class G4LogicalVolume;

class ScintillatorModule : public DetectorModule
{
public:
    enum class Style { kSinglePlane, kFourPlane };

    // kSinglePlane: refCenter = plane center (world z), sdOn = SP flag
    // kFourPlane:   refCenter = target center,
    //               sdOn = paddle SDs, vdOn = virtual-plane SD
    ScintillatorModule(Style style, double refCenter, bool sdOn, bool vdOn = false);

    void BuildVolumes(G4LogicalVolume *world) override;
    void BuildSDs() override;

private:
    Style fStyle;
    double fRefCenter;
    bool fSdOn, fVdOn;

    G4LogicalVolume *fSinglePlaneLV = nullptr;
    std::vector<G4LogicalVolume *> fPlaneLVs;  // ScintillatorLV1..4
    std::vector<G4LogicalVolume *> fSciVdLVs;  // SciVDLV1..4
};

#endif
