//
// VirtualDetModule.hh
// A thin vacuum-like disk that records anything crossing it
// (StandardDetectorSD "VirtualSD", branch prefix "VD").
//
// Used as:
//  - PRad: full disk (IR 0, OR 50 cm) at target + 60 mm (scattering check)
//  - test: annulus spanning 0.5..10 deg at z = 99 cm
//  - prad2/x17: annulus (IR 2 cm, OR 100 cm) just downstream of the last
//    GEM station — the "VD" plane consumed by the prad2 sim2replay tool
//    as the HyCal cluster source (VD.X/Y/Z/P).
//

#ifndef VirtualDetModule_h
#define VirtualDetModule_h 1

#include "detector/DetectorModule.hh"

class G4LogicalVolume;

class VirtualDetModule : public DetectorModule
{
public:
    // z: world z of the disk center; halfT: half thickness
    VirtualDetModule(double z, double innerR, double outerR, double halfT, bool sdOn);

    void BuildVolumes(G4LogicalVolume *world) override;
    void BuildSDs() override;

private:
    double fZ, fInnerR, fOuterR, fHalfT;
    bool fSdOn;

    G4LogicalVolume *fVirtualDetLV = nullptr;
};

#endif
