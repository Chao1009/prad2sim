//
// HeBagModule.cc
// Faithful port of the He bag from DetectorDRad.cc.
//

#include "detector/HeBagModule.hh"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4PVPlacement.hh"
#include "G4SubtractionSolid.hh"
#include "G4Tubs.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

HeBagModule::HeBagModule(double gemCenter0, double gemCenter1)
    : DetectorModule("HeBag"), fGEMCenter0(gemCenter0), fGEMCenter1(gemCenter1)
{
}

void HeBagModule::BuildVolumes(G4LogicalVolume *world)
{
    G4Material *HeBagM = G4Material::GetMaterial("HeGas");

    // He bag (Only He gas for now)
    G4Box *HeBagBox = new G4Box("HeBagBox", 1.0 * m, 1.0 * m, (fGEMCenter1 - fGEMCenter0 - 5.65 * cm) / 2.0);
    G4Tubs *HeBagTube = new G4Tubs("HeBagTube", 0, 22.0 * mm, (fGEMCenter1 - fGEMCenter0 - 5.65 * cm + 1.0 * mm) / 2.0, 0, twopi);
    G4SubtractionSolid *solidHeBag = new G4SubtractionSolid("HeBagS", HeBagBox, HeBagTube);
    G4LogicalVolume *logicHeBag = new G4LogicalVolume(solidHeBag, HeBagM, "HeBagLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, (fGEMCenter0 + fGEMCenter1) / 2.0), logicHeBag, "He Bag", world, false, 0);
}
