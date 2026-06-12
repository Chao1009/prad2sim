//
// VirtualDetModule.cc
//

#include "detector/VirtualDetModule.hh"

#include "StandardDetectorSD.hh"

#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4PVPlacement.hh"
#include "G4SDManager.hh"
#include "G4Tubs.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

VirtualDetModule::VirtualDetModule(double z, double innerR, double outerR, double halfT, bool sdOn)
    : DetectorModule("VirtualDet"), fZ(z), fInnerR(innerR), fOuterR(outerR), fHalfT(halfT), fSdOn(sdOn)
{
}

void VirtualDetModule::BuildVolumes(G4LogicalVolume *world)
{
    G4Material *VirtualDetM = G4Material::GetMaterial("VirtualDetM");

    G4VSolid *solidVirtualDet = new G4Tubs("VirtualDetS", fInnerR, fOuterR, fHalfT, 0, twopi);
    fVirtualDetLV = new G4LogicalVolume(solidVirtualDet, VirtualDetM, "VirtualDetLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, fZ), fVirtualDetLV, "Virtual Detector", world, false, 0);
}

void VirtualDetModule::BuildSDs()
{
    if (!fSdOn || !fVirtualDetLV) return;

    StandardDetectorSD *VirtualSD = new StandardDetectorSD("VirtualSD", "VD");
    G4SDManager::GetSDMpointer()->AddNewDetector(VirtualSD);
    fVirtualDetLV->SetSensitiveDetector(VirtualSD);
}
