//
// ScintillatorModule.cc
// kSinglePlane: faithful port from DetectorDRad.cc.
// kFourPlane:   faithful port from PRadSim_PRad2 DefinePRadVolumes() /
//               DefinePRadSDs().
//

#include "detector/ScintillatorModule.hh"

#include "StandardDetectorSD.hh"

#include "TString.h"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4Material.hh"
#include "G4PVPlacement.hh"
#include "G4SDManager.hh"
#include "G4SubtractionSolid.hh"
#include "G4Tubs.hh"

#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

ScintillatorModule::ScintillatorModule(Style style, double refCenter, bool sdOn, bool vdOn)
    : DetectorModule("Scintillator"), fStyle(style), fRefCenter(refCenter), fSdOn(sdOn), fVdOn(vdOn)
{
}

void ScintillatorModule::BuildVolumes(G4LogicalVolume *world)
{
    G4Material *ScintillatorPlaneM = G4Material::GetMaterial("EJ204");

    if (fStyle == Style::kSinglePlane) {
        // Scintillator plane
        G4double SciPlaneThickness = 5.0 * mm;
        G4double SciPlaneHalfX = 75.0 * cm;
        G4double SciPlaneHalfY = 75.0 * cm;
        G4Box *SciPlaneBox = new G4Box("ScintillatorPlaneBox", SciPlaneHalfX, SciPlaneHalfY, SciPlaneThickness / 2.0);
        G4Tubs *SciPlaneTube = new G4Tubs("ScintillatorPlaneTube", 0, 22.0 * mm, (SciPlaneThickness + 1.0 * mm) / 2.0, 0, twopi);
        G4SubtractionSolid *solidSciPlane = new G4SubtractionSolid("ScintillatorPlaneS", SciPlaneBox, SciPlaneTube);
        fSinglePlaneLV = new G4LogicalVolume(solidSciPlane, ScintillatorPlaneM, "ScintillatorPlaneLV");
        new G4PVPlacement(0, G4ThreeVector(0, 0, fRefCenter), fSinglePlaneLV, "Scintillator Plane", world, false, 0);
        return;
    }

    // kFourPlane (PRad-II): four EJ204 paddles around the beam axis,
    // horizontal pair at target + 304.880 mm, vertical pair at + 311.880 mm,
    // each preceded by a 2 um virtual plane.
    G4Material *VirtualDetM = G4Material::GetMaterial("VirtualDetM");

    G4double SciPlaneHalfX = 32.5 * mm;
    G4double SciPlaneHalfY = 50. * mm;
    G4double SciPlaneHalfZ = 1.5 * mm;
    G4double HorizontalCenter = fRefCenter + 304.880 * mm;
    G4double VerticalCenter = fRefCenter + 311.880 * mm;
    G4double SciPlaneHoleXY = 40. * mm;
    G4double shiftX = 0. * mm, shiftY = 0. * mm;

    G4Box *SolidSciPlane1 = new G4Box("SciPlane1S", SciPlaneHalfX, SciPlaneHalfY, SciPlaneHalfZ);
    G4LogicalVolume *logicSciPlane1 = new G4LogicalVolume(SolidSciPlane1, ScintillatorPlaneM, "ScintillatorLV1");
    new G4PVPlacement(0, G4ThreeVector(0.5 * SciPlaneHoleXY + SciPlaneHalfX + shiftX, 0 + shiftY, HorizontalCenter), logicSciPlane1, "Scintillator", world, false, 0);
    G4Box *SolidSciPlane2 = new G4Box("SciPlane2S", SciPlaneHalfX, SciPlaneHalfY, SciPlaneHalfZ);
    G4LogicalVolume *logicSciPlane2 = new G4LogicalVolume(SolidSciPlane2, ScintillatorPlaneM, "ScintillatorLV2");
    new G4PVPlacement(0, G4ThreeVector(-0.5 * SciPlaneHoleXY - SciPlaneHalfX + shiftX, 0 + shiftY, HorizontalCenter), logicSciPlane2, "Scintillator", world, false, 0);
    G4Box *SolidSciPlane3 = new G4Box("SciPlane3S", SciPlaneHalfY, SciPlaneHalfX, SciPlaneHalfZ);
    G4LogicalVolume *logicSciPlane3 = new G4LogicalVolume(SolidSciPlane3, ScintillatorPlaneM, "ScintillatorLV3");
    new G4PVPlacement(0, G4ThreeVector(0 + shiftX, 0.5 * SciPlaneHoleXY + SciPlaneHalfX + shiftY, VerticalCenter), logicSciPlane3, "Scintillator", world, false, 0);
    G4Box *SolidSciPlane4 = new G4Box("SciPlane4S", SciPlaneHalfY, SciPlaneHalfX, SciPlaneHalfZ);
    G4LogicalVolume *logicSciPlane4 = new G4LogicalVolume(SolidSciPlane4, ScintillatorPlaneM, "ScintillatorLV4");
    new G4PVPlacement(0, G4ThreeVector(0 + shiftX, -0.5 * SciPlaneHoleXY - SciPlaneHalfX + shiftY, VerticalCenter), logicSciPlane4, "Scintillator", world, false, 0);

    fPlaneLVs = {logicSciPlane1, logicSciPlane2, logicSciPlane3, logicSciPlane4};

    G4Box *SolidSciVD1 = new G4Box("SciVD1S", SciPlaneHalfX, SciPlaneHalfY, 0.001 * mm);
    G4LogicalVolume *logicSciVD1 = new G4LogicalVolume(SolidSciVD1, VirtualDetM, "SciVDLV1");
    new G4PVPlacement(0, G4ThreeVector(0.5 * SciPlaneHoleXY + SciPlaneHalfX + shiftX, 0 + shiftY, HorizontalCenter - SciPlaneHalfZ - 0.005 * mm), logicSciVD1, "SciVD", world, false, 0);
    G4Box *SolidSciVD2 = new G4Box("SciVD2S", SciPlaneHalfX, SciPlaneHalfY, 0.001 * mm);
    G4LogicalVolume *logicSciVD2 = new G4LogicalVolume(SolidSciVD2, VirtualDetM, "SciVDLV2");
    new G4PVPlacement(0, G4ThreeVector(-0.5 * SciPlaneHoleXY - SciPlaneHalfX + shiftX, 0 + shiftY, HorizontalCenter - SciPlaneHalfZ - 0.005 * mm), logicSciVD2, "SciVD", world, false, 0);
    G4Box *SolidSciVD3 = new G4Box("SciVD3S", SciPlaneHalfY, SciPlaneHalfX, 0.001 * mm);
    G4LogicalVolume *logicSciVD3 = new G4LogicalVolume(SolidSciVD3, VirtualDetM, "SciVDLV3");
    new G4PVPlacement(0, G4ThreeVector(0 + shiftX, 0.5 * SciPlaneHoleXY + SciPlaneHalfX + shiftY, VerticalCenter - SciPlaneHalfZ - 0.005 * mm), logicSciVD3, "SciVD", world, false, 0);
    G4Box *SolidSciVD4 = new G4Box("SciVD4S", SciPlaneHalfY, SciPlaneHalfX, 0.001 * mm);
    G4LogicalVolume *logicSciVD4 = new G4LogicalVolume(SolidSciVD4, VirtualDetM, "SciVDLV4");
    new G4PVPlacement(0, G4ThreeVector(0 + shiftX, -0.5 * SciPlaneHoleXY - SciPlaneHalfX + shiftY, VerticalCenter - SciPlaneHalfZ - 0.005 * mm), logicSciVD4, "SciVD", world, false, 0);

    fSciVdLVs = {logicSciVD1, logicSciVD2, logicSciVD3, logicSciVD4};
}

void ScintillatorModule::BuildSDs()
{
    if (fStyle == Style::kSinglePlane) {
        if (!fSdOn || !fSinglePlaneLV) return;

        StandardDetectorSD *SciPlaneSD = new StandardDetectorSD("ScintillatorPlaneSD", "SP");
        G4SDManager::GetSDMpointer()->AddNewDetector(SciPlaneSD);
        fSinglePlaneLV->SetSensitiveDetector(SciPlaneSD);
        return;
    }

    if (fSdOn && fPlaneLVs.size() == 4) {
        StandardDetectorSD *SciDetectorSD1 = new StandardDetectorSD("SciDetectorSD1", "Sci1");
        G4SDManager::GetSDMpointer()->AddNewDetector(SciDetectorSD1);
        fPlaneLVs[0]->SetSensitiveDetector(SciDetectorSD1);
        fPlaneLVs[1]->SetSensitiveDetector(SciDetectorSD1);

        StandardDetectorSD *SciDetectorSD2 = new StandardDetectorSD("SciDetectorSD2", "Sci2");
        G4SDManager::GetSDMpointer()->AddNewDetector(SciDetectorSD2);
        fPlaneLVs[2]->SetSensitiveDetector(SciDetectorSD2);
        fPlaneLVs[3]->SetSensitiveDetector(SciDetectorSD2);
    }

    if (fVdOn && fSciVdLVs.size() == 4) {
        StandardDetectorSD *SciVirtualSD = new StandardDetectorSD("SciVirtualSD", "SciVD");
        G4SDManager::GetSDMpointer()->AddNewDetector(SciVirtualSD);

        for (auto *lv : fSciVdLVs)
            lv->SetSensitiveDetector(SciVirtualSD);
    }
}
