//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// DetectorDRad.cc
// Developer : Chao Peng, Chao Gu
// History:
//   Aug 2012, C. Peng, Original version.
//   Jan 2017, C. Gu, Rewrite with ROOT support.
//   Mar 2017, C. Gu, Add DRad configuration.
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "DetectorConstruction.hh"

#include "CalorimeterSD.hh"
#include "StandardDetectorSD.hh"
#include "TrackingDetectorSD.hh"

#include "TROOT.h"
#include "TError.h"
#include "TObject.h"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4Polyhedra.hh"
#include "G4PVPlacement.hh"
#include "G4SubtractionSolid.hh"
#include "G4Tubs.hh"
#include "G4VPhysicalVolume.hh"

#include "G4SDManager.hh"

#include "G4Colour.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"

#include <cmath>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume *DetectorConstruction::DefineDRadVolumes()
{
    G4Material *DefaultM = G4Material::GetMaterial("Galaxy");
    G4Material *TargetM = G4Material::GetMaterial("D2Gas");

    if (fTargetMat == "H2Gas" || fTargetMat == "hydrogen")
        TargetM = G4Material::GetMaterial("H2Gas");

    G4Material *TargetCellM = G4Material::GetMaterial("Kapton");
    G4Material *TargetWindowM = G4Material::GetMaterial("Kapton");
    G4Material *RecoilDetectorM = G4Material::GetMaterial("Silicon");
    G4Material *RecoilDetCoverM = G4Material::GetMaterial("SiO2");
    G4Material *HeBagM = G4Material::GetMaterial("HeGas");
    G4Material *ScintillatorPlaneM = G4Material::GetMaterial("EJ204");

    // World
    G4VSolid *solidWorld = new G4Box("WorldS", fWorldSizeXY, fWorldSizeXY, fWorldSizeZ);
    G4LogicalVolume *logicWorld = new G4LogicalVolume(solidWorld, DefaultM, "WorldLV");
    G4VPhysicalVolume *physiWorld = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicWorld, "World", 0, false, 0);

    // Target
    G4double CellXY = 15.0 * cm;

    // Target Container
    G4VSolid *solidTargetCon = new G4Box("TargetContainerS", CellXY + 0.1 * cm, CellXY + 0.1 * cm, fTargetHalfL + 0.1 * cm);
    G4LogicalVolume *logicTargetCon = new G4LogicalVolume(solidTargetCon, DefaultM, "TargetContainerLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, fTargetCenter), logicTargetCon, "Target Container", logicWorld, false, 0);

    // Target material
    G4VSolid *solidTarget = new G4Tubs("TargetS", 0, fTargetR, fTargetHalfL, 0, twopi);
    G4LogicalVolume *logicTarget = new G4LogicalVolume(solidTarget, TargetM, "TargetLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicTarget, "Target Material", logicTargetCon, false, 0);

    // Target cell
    G4Box *CellBox = new G4Box("CellBox", CellXY, CellXY, fTargetHalfL);
    G4Tubs *CellTube = new G4Tubs("CellTube", 0, fTargetR, fTargetHalfL + 1.0 * mm, 0, twopi);
    G4SubtractionSolid *solidCell = new G4SubtractionSolid("TargetCellS", CellBox, CellTube);
    G4LogicalVolume *logicCell = new G4LogicalVolume(solidCell, TargetCellM, "TargetCellLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicCell, "Target Cell", logicTargetCon, false, 0);

    // Target cell windows
    G4double CellApertureR = 2.0 * mm;
    G4double CellWinThickness = 7.5 * um;
    G4Box *CellWinBox = new G4Box("CellWinBox", CellXY, CellXY, CellWinThickness / 2.0);
    G4Tubs *CellWinTube = new G4Tubs("CellWinTube", 0, CellApertureR, CellWinThickness + 1.0 * mm, 0, twopi);
    G4SubtractionSolid *solidCellWin = new G4SubtractionSolid("TargetWindowS", CellWinBox, CellWinTube);
    G4LogicalVolume *logicCellWin = new G4LogicalVolume(solidCellWin, TargetWindowM, "TargetWindowLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, -fTargetHalfL - CellWinThickness / 2.0), logicCellWin, "Target Window", logicTargetCon, false, 0);
    new G4PVPlacement(0, G4ThreeVector(0, 0, +fTargetHalfL + CellWinThickness / 2.0), logicCellWin, "Target Window", logicTargetCon, false, 1);

    // Recoil detector
    G4double RecoilDetCenter = fRecoilDetCenter - fTargetCenter;
    G4double CoverThickness = 0.5 * um;
    G4double RecoilDetAng = twopi / fRecoilDetNSeg;
    G4double RecoilDetL2OR = fRecoilDetR * cos(RecoilDetAng / 2.0);
    G4double RecoilDetL2IRC = RecoilDetL2OR - fRecoilDetL2Thickness + CoverThickness;
    G4double RecoilDetL2IR = RecoilDetL2OR - fRecoilDetL2Thickness;
    G4double RecoilDetL1OR = RecoilDetL2IR;
    G4double RecoilDetL1IRC = RecoilDetL1OR - fRecoilDetL1Thickness + CoverThickness;
    G4double RecoilDetL1IR = RecoilDetL1OR - fRecoilDetL1Thickness;

    G4double rInnerRDL2[] = {RecoilDetL2IRC, RecoilDetL2IRC};
    G4double rOuterRDL2[] = {RecoilDetL2OR, RecoilDetL2OR};
    G4double zPlaneRDL2[] = {-fRecoilDetHalfL, fRecoilDetHalfL};
    G4VSolid *solidRecoilDet2 = new G4Polyhedra("RecoilDet2S", 0, twopi, fRecoilDetNSeg, 2, zPlaneRDL2, rInnerRDL2, rOuterRDL2);

    G4double rInnerRDL2Cover[] = {RecoilDetL2IR, RecoilDetL2IR};
    G4double rOuterRDL2Cover[] = {RecoilDetL2IRC, RecoilDetL2IRC};
    G4double zPlaneRDL2Cover[] = {-fRecoilDetHalfL, fRecoilDetHalfL};
    G4VSolid *solidRecoilDet2Cover = new G4Polyhedra("RecoilDet2CoverS", 0, twopi, fRecoilDetNSeg, 2, zPlaneRDL2Cover, rInnerRDL2Cover, rOuterRDL2Cover);

    G4double rInnerRDL1[] = {RecoilDetL1IRC, RecoilDetL1IRC};
    G4double rOuterRDL1[] = {RecoilDetL1OR, RecoilDetL1OR};
    G4double zPlaneRDL1[] = {-fRecoilDetHalfL, fRecoilDetHalfL};
    G4VSolid *solidRecoilDet1 = new G4Polyhedra("RecoilDet1S", 0, twopi, fRecoilDetNSeg, 2, zPlaneRDL1, rInnerRDL1, rOuterRDL1);

    G4double rInnerRDL1Cover[] = {RecoilDetL1IR, RecoilDetL1IR};
    G4double rOuterRDL1Cover[] = {RecoilDetL1IRC, RecoilDetL1IRC};
    G4double zPlaneRDL1Cover[] = {-fRecoilDetHalfL, fRecoilDetHalfL};
    G4VSolid *solidRecoilDet1Cover = new G4Polyhedra("RecoilDet1CoverS", 0, twopi, fRecoilDetNSeg, 2, zPlaneRDL1Cover, rInnerRDL1Cover, rOuterRDL1Cover);

    G4LogicalVolume *logicRecoilDet1 = new G4LogicalVolume(solidRecoilDet1, RecoilDetectorM, "RecoilDet1LV");
    G4LogicalVolume *logicRecoilDet1Cover = new G4LogicalVolume(solidRecoilDet1Cover, RecoilDetCoverM, "RecoilDet1CoverLV");
    G4LogicalVolume *logicRecoilDet2 = new G4LogicalVolume(solidRecoilDet2, RecoilDetectorM, "RecoilDet2LV");
    G4LogicalVolume *logicRecoilDet2Cover = new G4LogicalVolume(solidRecoilDet2Cover, RecoilDetCoverM, "RecoilDet2CoverLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, RecoilDetCenter), logicRecoilDet1Cover, "Recoil Detector 1 Cover", logicTarget, false, 0);
    new G4PVPlacement(0, G4ThreeVector(0, 0, RecoilDetCenter), logicRecoilDet1, "Recoil Detector 1", logicTarget, false, 0);
    new G4PVPlacement(0, G4ThreeVector(0, 0, RecoilDetCenter), logicRecoilDet2Cover, "Recoil Detector 2 Cover", logicTarget, false, 1);
    new G4PVPlacement(0, G4ThreeVector(0, 0, RecoilDetCenter), logicRecoilDet2, "Recoil Detector 2", logicTarget, false, 1);

    AddVaccumBox(logicWorld);

    AddGEM(logicWorld, 0, true);
    AddGEM(logicWorld, 1, false);

    // He bag (Only He gas for now)
    G4Box *HeBagBox = new G4Box("HeBagBox", 1.0 * m, 1.0 * m, (fGEMCenter[1] - fGEMCenter[0] - 5.65 * cm) / 2.0);
    G4Tubs *HeBagTube = new G4Tubs("HeBagTube", 0, 22.0 * mm, (fGEMCenter[1] - fGEMCenter[0] - 5.65 * cm + 1.0 * mm) / 2.0, 0, twopi);
    G4SubtractionSolid *solidHeBag = new G4SubtractionSolid("HeBagS", HeBagBox, HeBagTube);
    G4LogicalVolume *logicHeBag = new G4LogicalVolume(solidHeBag, HeBagM, "HeBagLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, (fGEMCenter[0] + fGEMCenter[1]) / 2.0), logicHeBag, "He Bag", logicWorld, false, 0);

    // Scintillator plane
    G4double SciPlaneThickness = 5.0 * mm;
    G4double SciPlaneHalfX = 75.0 * cm;
    G4double SciPlaneHalfY = 75.0 * cm;
    G4Box *SciPlaneBox = new G4Box("ScintillatorPlaneBox", SciPlaneHalfX, SciPlaneHalfY, SciPlaneThickness / 2.0);
    G4Tubs *SciPlaneTube = new G4Tubs("ScintillatorPlaneTube", 0, 22.0 * mm, (SciPlaneThickness + 1.0 * mm) / 2.0, 0, twopi);
    G4SubtractionSolid *solidSciPlane = new G4SubtractionSolid("ScintillatorPlaneS", SciPlaneBox, SciPlaneTube);
    G4LogicalVolume *logicSciPlane = new G4LogicalVolume(solidSciPlane, ScintillatorPlaneM, "ScintillatorPlaneLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, fSciPlaneCenter), logicSciPlane, "Scintillator Plane", logicWorld, false, 0);

    AddHyCal(logicWorld);

    G4LogicalVolumeStore *pLogicalVolume = G4LogicalVolumeStore::GetInstance();

    for (unsigned long i = 0; i < pLogicalVolume->size(); i++)
        (*pLogicalVolume)[i]->SetVisAttributes(fVisAtts[(*pLogicalVolume)[i]->GetMaterial()->GetName()]);

    // Always return the physical World
    return physiWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::DefineDRadSDs()
{
    if (fRecoilDetSDOn) {
        TrackingDetectorSD *RecoilDetSD = new TrackingDetectorSD("RecoilDetectorSD", "RD");
        G4SDManager::GetSDMpointer()->AddNewDetector(RecoilDetSD);
        SetSensitiveDetector("RecoilDet1LV", RecoilDetSD);
        SetSensitiveDetector("RecoilDet2LV", RecoilDetSD);
    }

    if (fGEMSDOn) {
        TrackingDetectorSD *GEMSD = new TrackingDetectorSD("GEMSD", "GEM");
        G4SDManager::GetSDMpointer()->AddNewDetector(GEMSD);
        SetSensitiveDetector("GEM0CathodeLV", GEMSD);
        SetSensitiveDetector("GEM1CathodeLV", GEMSD);
    }

    if (fSciPlaneSDOn) {
        StandardDetectorSD *SciPlaneSD = new StandardDetectorSD("ScintillatorPlaneSD", "SP");
        G4SDManager::GetSDMpointer()->AddNewDetector(SciPlaneSD);
        SetSensitiveDetector("ScintillatorPlaneLV", SciPlaneSD);
    }

    if (fHyCalSDOn) {
        CalorimeterSD *HyCalSD = new CalorimeterSD("HyCalSD", "HC", "database/pwo_attenuation.dat");
        HyCalSD->SetAttenuationLG(fAttenuationLG);
        G4SDManager::GetSDMpointer()->AddNewDetector(HyCalSD);

        for (int i = 0; i < 1152; i++)
            SetSensitiveDetector(Form("PbWO4Absorber%04dLV", i), HyCalSD);

        for (int i = 0; i < 576; i++)
            SetSensitiveDetector(Form("PbGlassAbsorber%04dLV", i), HyCalSD);
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
