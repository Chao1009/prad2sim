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
// DetectorPRad.cc
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
#include "CheckScatteringSD.hh"
#include "StandardDetectorSD.hh"
#include "TrackingDetectorSD.hh"

#include "TROOT.h"
#include "TError.h"
#include "TObject.h"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4PVPlacement.hh"
#include "G4SubtractionSolid.hh"
#include "G4Tubs.hh"
#include "G4VPhysicalVolume.hh"

#include "G4SDManager.hh"

#include "G4Colour.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume *DetectorConstruction::DefinePRadVolumes()
{
    G4Material *DefaultM = G4Material::GetMaterial("Galaxy");
    G4Material *TargetM = G4Material::GetMaterial("H2Gas");
    G4Material *TargetCellM = G4Material::GetMaterial("Copper");
    G4Material *TargetWindowM = G4Material::GetMaterial("Kapton");
    G4Material *UCollimatorM = G4Material::GetMaterial("Nickel");
    G4Material *VacuumTubeM = G4Material::GetMaterial("SSteel");
    G4Material *VirtualDetM = G4Material::GetMaterial("VirtualDetM");

    // World
    G4VSolid *solidWorld = new G4Box("WorldS", fWorldSizeXY, fWorldSizeXY, fWorldSizeZ);
    G4LogicalVolume *logicWorld = new G4LogicalVolume(solidWorld, DefaultM, "WorldLV");
    G4VPhysicalVolume *physiWorld = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicWorld, "World", 0, false, 0);

    // Target
    // Target Container
    G4VSolid *solidTargetCon = new G4Box("TargetContainerS", 3.5 * cm, 3.5 * cm, 2.1 * cm);
    G4LogicalVolume *logicTargetCon = new G4LogicalVolume(solidTargetCon, DefaultM, "TargetContainerLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, fTargetCenter), logicTargetCon, "Target Container", logicWorld, false, 0);

    // Target material
    G4double TargetR = 25.0 * mm;
    G4double TargetHalfL = 20.0 * mm;
    G4VSolid *solidTarget = new G4Tubs("TargetS", 0, TargetR, TargetHalfL, 0, twopi);
    G4LogicalVolume *logicTarget = new G4LogicalVolume(solidTarget, TargetM, "TargetLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicTarget, "Target Material", logicTargetCon, false, 0);

    // Target cell
    G4double CellXY = 3.5 * cm;
    G4Box *CellBox = new G4Box("CellBox", CellXY, CellXY, TargetHalfL);
    G4Tubs *CellTube = new G4Tubs("CellTube", 0, TargetR, TargetHalfL + 1.0 * mm, 0, twopi);
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
    new G4PVPlacement(0, G4ThreeVector(0, 0, -TargetHalfL - CellWinThickness / 2.0), logicCellWin, "Target Window", logicTargetCon, false, 0);
    new G4PVPlacement(0, G4ThreeVector(0, 0, +TargetHalfL + CellWinThickness / 2.0), logicCellWin, "Target Window", logicTargetCon, false, 1);

    // Upstream collimator
    // Dimension from PRad beam line drawing (search PRad in JLab drawing database)
    G4double UCollimatorHalfL = 11.8 * 2.54 / 2.0 * cm;
    G4double UCollimatorOR = 3.9 * 2.54 / 2.0 * cm;
    G4double UCollimatorIR = 12.7 * mm / 2.0;
    G4double UCollimatorCenter = fTargetCenter - 2.03 * m + UCollimatorHalfL;
    G4VSolid *solidUCollimator = new G4Tubs("UCollimatorS", UCollimatorIR, UCollimatorOR, UCollimatorHalfL, 0, twopi);
    G4LogicalVolume *logicUCollimator = new G4LogicalVolume(solidUCollimator, UCollimatorM, "UCollimatorLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, UCollimatorCenter), logicUCollimator, "Upstream Collimator", logicWorld, false, 0);

    // Upstream beam pipe
    // Dimension from PRad target drawings and PRad beam line drawings
    G4double UBeamPipeIR = 1.87 * 2.54 / 2.0 * cm;
    G4double UBeamPipeOR = 2.0 * 2.54 / 2.0 * cm;
    G4double UBeamPipeHalfL = 0.79 * m;
    G4double UBeamPipeOffset = 0.105 * m;
    G4double UBeamPipeCenter = fTargetCenter - UBeamPipeOffset - UBeamPipeHalfL - 4 * cm; // subtract 4cm for target length
    G4VSolid *solidUBeamPipe = new G4Tubs("UBeamPipeS", UBeamPipeIR, UBeamPipeOR, UBeamPipeHalfL, 0, 2 * pi);
    G4LogicalVolume *logicUBeamPipe = new G4LogicalVolume(solidUBeamPipe, VacuumTubeM, "UBeamPipeLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, UBeamPipeCenter), logicUBeamPipe, "Upstream Beam Pipe", logicWorld, false, 0);

    AddVaccumBox(logicWorld);

    // GEM center: PRad survey value is 229.99575 cm (now set via config/prad.json)
    AddGEM(logicWorld, 0, false);

    // Crystal surface: PRad survey value is 273.515 cm (now set via config/prad.json)
    AddHyCal(logicWorld);

    // Virtual Detector
    G4double VirtualDetR = 50.0 * cm;
    G4double VirtualDetZ = 0.1 * mm;
    G4VSolid *solidVirtualDet = new G4Tubs("VirtualDetS", 0, VirtualDetR, VirtualDetZ / 2.0, 0, twopi);
    G4LogicalVolume *logicVirtualDet = new G4LogicalVolume(solidVirtualDet, VirtualDetM, "VirtualDetLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, fTargetCenter + 60 * mm), logicVirtualDet, "Virtual Detector", logicWorld, false, 0);

    G4LogicalVolumeStore *pLogicalVolume = G4LogicalVolumeStore::GetInstance();

    for (unsigned long i = 0; i < pLogicalVolume->size(); i++)
        (*pLogicalVolume)[i]->SetVisAttributes(fVisAtts[(*pLogicalVolume)[i]->GetMaterial()->GetName()]);

    // Always return the physical World
    return physiWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::DefinePRadSDs()
{
    if (fTargetSDOn) {
        CheckScatteringSD *TargetSD = new CheckScatteringSD("TargetSD", "TG");
        G4SDManager::GetSDMpointer()->AddNewDetector(TargetSD);
        SetSensitiveDetector("TargetLV", TargetSD);
    }

    if (fGEMSDOn) {
        TrackingDetectorSD *GEMSD = new TrackingDetectorSD("GEMSD", "GEM");
        G4SDManager::GetSDMpointer()->AddNewDetector(GEMSD);
        SetSensitiveDetector("GEM0CathodeLV", GEMSD);
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

    if (fVirtualSDOn) {
        StandardDetectorSD *VirtualSD = new StandardDetectorSD("VirtualSD", "VD");
        G4SDManager::GetSDMpointer()->AddNewDetector(VirtualSD);
        SetSensitiveDetector("VirtualDetLV", VirtualSD);
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
