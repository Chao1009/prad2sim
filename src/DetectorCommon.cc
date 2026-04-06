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
// DetectorCommon.cc
// Developer : Chao Peng, Chao Gu
// History:
//   Aug 2012, C. Peng, Original version.
//   Jan 2017, C. Gu, Rewrite with ROOT support.
//   Mar 2017, C. Gu, Add DRad configuration.
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "DetectorConstruction.hh"

#include "StandardDetectorSD.hh"
#include "StepRecordSD.hh"

#include "TROOT.h"
#include "TError.h"
#include "TObject.h"

#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4Polycone.hh"
#include "G4PVParameterised.hh"
#include "G4PVPlacement.hh"
#include "G4RotationMatrix.hh"
#include "G4Sphere.hh"
#include "G4SubtractionSolid.hh"
#include "G4Tubs.hh"
#include "G4UnionSolid.hh"
#include "G4UserLimits.hh"
#include "G4VPhysicalVolume.hh"

#include "G4SDManager.hh"

#include "G4Colour.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"

#include <cmath>
#include <fstream>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume *DetectorConstruction::DefineTestVolumes()
{
    G4Material *DefaultM = G4Material::GetMaterial("Galaxy");
    G4Material *TargetM = G4Material::GetMaterial("H2Gas");
    G4Material *TargetCellM = G4Material::GetMaterial("Copper");
    G4Material *TargetWindowM = G4Material::GetMaterial("Kapton");
    G4Material *VirtualDetM = G4Material::GetMaterial("VirtualDetM");

    // World (size set via config/test.json, defaults to 10x100 cm)
    G4VSolid *solidWorld = new G4Box("WorldS", fWorldSizeXY, fWorldSizeXY, fWorldSizeZ);
    G4LogicalVolume *logicWorld = new G4LogicalVolume(solidWorld, DefaultM, "WorldLV");
    G4VPhysicalVolume *physiWorld = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicWorld, "World", 0, false, 0);

    // Target
    // Target Container
    G4VSolid *solidTargetCon = new G4Box("TargetContainerS", 3.5 * cm, 3.5 * cm, 2.1 * cm);
    G4LogicalVolume *logicTargetCon = new G4LogicalVolume(solidTargetCon, DefaultM, "TargetContainerLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicTargetCon, "Target Container", logicWorld, false, 0);

    // Target material
    G4double TargetR = 25.0 * mm;
    G4double TargetHalfL = 20.0 * mm;
    G4VSolid *solidTarget = new G4Tubs("TargetS", 0, TargetR, TargetHalfL, 0, twopi);
    G4LogicalVolume *logicTarget = new G4LogicalVolume(solidTarget, TargetM, "TargetLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicTarget, "Target Material", logicTargetCon, false, 0);
    //logicTarget->SetUserLimits(new G4UserLimits(0.5 * mm));

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

    // Virtual Detector
    G4double VirtualDetZ = 0.1 * mm;
    G4double VirtualDetL = 99.0 * cm;
    G4double VirtualDetIR = (VirtualDetL - 20.0 * mm) * tan(0.5 / 180.0 * pi);
    G4double VirtualDetOR = (VirtualDetL + 20.0 * mm) * tan(10.0 / 180.0 * pi);
    G4VSolid *solidVirtualDet = new G4Tubs("VirtualDetS", VirtualDetIR, VirtualDetOR, VirtualDetZ, 0, twopi);
    G4LogicalVolume *logicVirtualDet = new G4LogicalVolume(solidVirtualDet, VirtualDetM, "VirtualDetLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, VirtualDetL), logicVirtualDet, "Virtual Detector", logicWorld, false, 0);

    G4LogicalVolumeStore *pLogicalVolume = G4LogicalVolumeStore::GetInstance();

    for (unsigned long i = 0; i < pLogicalVolume->size(); i++)
        (*pLogicalVolume)[i]->SetVisAttributes(fVisAtts[(*pLogicalVolume)[i]->GetMaterial()->GetName()]);

    return physiWorld;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::DefineTestSDs()
{
    StepRecordSD *TargetSD = new StepRecordSD("TargetSD", "TG");
    G4SDManager::GetSDMpointer()->AddNewDetector(TargetSD);
    SetSensitiveDetector("TargetLV", TargetSD);

    StandardDetectorSD *VirtualSD = new StandardDetectorSD("VirtualSD", "VD");
    G4SDManager::GetSDMpointer()->AddNewDetector(VirtualSD);
    SetSensitiveDetector("VirtualDetLV", VirtualSD);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::AddVaccumBox(G4LogicalVolume *mother)
{
    G4Material *ChamberM = G4Material::GetMaterial("Aluminum");
    G4Material *ChamberWindowM = G4Material::GetMaterial("Kapton");
    G4Material *VacuumBoxM = G4Material::GetMaterial("Aluminum");
    G4Material *VacuumTubeM = G4Material::GetMaterial("SSteel");

    // Target chamber
    // For now, only built the downstream chamber with window
    // The downstream chamber window should locate at -3000.0 + 89.0 + 74.0  = -2837.0 mm
    // The length of the downstream chamber is 381.7 mm
    // The total length of the downstream chamber and the tube in total is 710.0 mm
    // Here the downstream chamber and the tube are built together to be the new downstream chamber.
    // So the center of this geometry should be at -2837.0 + 710.0 / 2 = -2482.0 mm
    fDownChamberCenter = fTargetCenter + 74.0 * mm + 71.0 * cm / 2.0;
    G4double DownChamberHalfL = 71.0 / 2.0 * cm;
    G4double DownChamberUR = 8.00 * cm;

    // Downstream chamber
    G4double rInnerDC[] = {7.56 * cm, 7.56 * cm, 7.56 * cm, 7.56 * cm, 17.30 * cm, 17.30 * cm};
    G4double rOuterDC[] = {8.00 * cm, 8.00 * cm, 17.78 * cm, 17.78 * cm, 17.78 * cm, 17.78 * cm};
    G4double zPlaneDC[] = {0, 32.83 * cm, 32.83 * cm, 35.37 * cm, 35.37 * cm, 71.00 * cm};
    G4VSolid *solidDownChamber = new G4Polycone("DownstreamChamberS", 0, twopi, 6, zPlaneDC, rInnerDC, rOuterDC);
    G4LogicalVolume *logicDownChamber = new G4LogicalVolume(solidDownChamber, ChamberM, "DownstreamChamberLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, fDownChamberCenter - DownChamberHalfL), logicDownChamber, "Downstream Chamber", mother, false, 0);

    // Downstream chamber window
    G4double DownChamberApertureR = 22.8 * mm;
    G4double DownChamberWinThickness = 7.5 * um;
    G4Tubs *solidDownChamberWin = new G4Tubs("DownstreamChamberWindowS", DownChamberApertureR, DownChamberUR, DownChamberWinThickness / 2.0, 0, twopi);
    G4LogicalVolume *logicDownChamberWin = new G4LogicalVolume(solidDownChamberWin, ChamberWindowM, "DownstreamChamberWindowLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, fDownChamberCenter - DownChamberHalfL - DownChamberWinThickness / 2.0), logicDownChamberWin, "Downstream Chamber Window", mother, false, 0);

    // Vacuum box
    // The length of the vacuum box is 4251.7 mm
    // So the center of this geometry should be at -3000.0 + 89.0 + 74.0 + 710.0 + 2125.85 = -1.15 mm
    fVacBoxCenter = fTargetCenter + 74.0 * mm + 71.0 * cm + 425.17 * cm / 2.0;
    G4double VacBoxHalfL = 425.17 * cm / 2.0;
    G4double VacBoxMaxR = 78.11 * cm;
    G4double rInner2[] = {17.30 * cm, 17.30 * cm, 50.17 * cm, 50.17 * cm, 78.11 * cm, 78.11 * cm};
    G4double rOuter2[] = {17.78 * cm, 17.78 * cm, 50.80 * cm, 50.80 * cm, 78.74 * cm, 78.74 * cm};
    G4double zPlane2[] = {0, 6.8 * cm, 17.6 * cm, 215.3 * cm, 229.5 * cm, 425.17 * cm};
    G4VSolid *solidVacBox = new G4Polycone("VacuumBoxS", 0, twopi, 6, zPlane2, rInner2, rOuter2);
    G4LogicalVolume *logicVacBox = new G4LogicalVolume(solidVacBox, VacuumBoxM, "VacuumBoxLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, fVacBoxCenter - VacBoxHalfL), logicVacBox, "Vacuum Box", mother, false, 0);

    // Vacuum box window
    G4double VacBoxWinFlangeOffset = 3.81 * cm;
    G4double ArcDistance = 5.59 * cm;
    G4double ArcEndR = (ArcDistance * ArcDistance + VacBoxMaxR * VacBoxMaxR) / (2 * ArcDistance);
    G4double ArcEndThickness = 1.6 * mm;
    G4double VacBoxWinApertureR = 3.0 * cm;
    G4VSolid *solidVacBoxWin = new G4Sphere("VacuumBoxWindowS", ArcEndR - ArcEndThickness, ArcEndR, 0, twopi, pi - asin(VacBoxMaxR / ArcEndR), asin(VacBoxMaxR / ArcEndR) - asin((VacBoxWinApertureR + 0.1 * mm) / ArcEndR));
    G4LogicalVolume *logicVacBoxWin = new G4LogicalVolume(solidVacBoxWin, VacuumBoxM, "VacuumBoxWindowLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, fVacBoxCenter + VacBoxHalfL + ArcEndR - ArcDistance - VacBoxWinFlangeOffset), logicVacBoxWin, "Vacuum Box Window", mother, false, 0);

    // Vacuum Tube
    G4double VacTubeOR = 1.9 * cm;
    G4double VacTubeIR = VacTubeOR - 0.12446 * cm; // 0.049 in = 0.12446 cm from Eugene
    G4double VacTubeL = fWorldSizeZ - 10.0 * cm - fVacBoxCenter - VacBoxHalfL + ArcDistance;
    G4VSolid *solidVacTube = new G4Tubs("VacuumTubeS", VacTubeIR, VacTubeOR, VacTubeL / 2.0, 0, twopi);
    G4LogicalVolume *logicVacTube = new G4LogicalVolume(solidVacTube, VacuumTubeM, "VacuumTubeLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, fWorldSizeZ - 10.0 * cm - VacTubeL / 2.0), logicVacTube, "Vacuum Tube", mother, false, 0);

    // Flange on vacuum tube
    G4double FlangeOR = VacBoxWinApertureR;
    G4double FlangeIR = VacTubeOR;
    G4double FlangeHalfL = 0.5 * cm;
    G4VSolid *solidFlange = new G4Tubs("FlangeS", FlangeIR, FlangeOR, FlangeHalfL, 0, twopi);
    G4LogicalVolume *logicFlange = new G4LogicalVolume(solidFlange, VacuumTubeM, "FlangeLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, fVacBoxCenter + VacBoxHalfL - ArcDistance + FlangeHalfL), logicFlange, "Flange", mother, false, 0);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::AddGEM(G4LogicalVolume *mother, int layerid, bool culess)
{
    G4Material *DefaultM = G4Material::GetMaterial("Galaxy");
    G4Material *GEMFrameM = G4Material::GetMaterial("NemaG10");
    G4Material *GEMGasM = G4Material::GetMaterial("ArCO2");
    G4Material *GEMFoilM = G4Material::GetMaterial("Kapton");
    G4Material *GEMFoil0d2M = G4Material::GetMaterial("Kapton0.2");
    G4Material *GEMFoil0d8M = G4Material::GetMaterial("Kapton0.8");
    G4Material *GEMCuM = G4Material::GetMaterial("Copper");
    G4Material *GEMCu0d2M = G4Material::GetMaterial("Copper0.2");
    G4Material *GEMCu0d75M = G4Material::GetMaterial("Copper0.75");
    G4Material *GEMCu0d8M = G4Material::GetMaterial("Copper0.8");
    G4Material *GEMGlueM = G4Material::GetMaterial("Kapton"); // TODO: Add actual Glue material

    // GEM
    G4double GEMCenter = fGEMCenter[layerid];
    G4double GEMGap = 3.971 * cm; // Gap between two GEM // 3.971 from Weizhi
    G4double GEMHalfX = 55.04 * cm / 2.0;
    G4double GEMHalfY = 122.88 * cm / 2.0;
    G4double GEMHalfT = (15.0 * mm + 455.0 * um) / 2.0; // 2 * 25 + 5 + 50 (win) + 6 * 5 + 3 * 50 (foil) + 5 + 5 + 50 + 50 + 60 (readout)

    if (culess) GEMHalfT = (15.0 * mm + 410.0 * um) / 2.0; // 2 * 25 + 50 (win) + 3 * 50 (foil) + 50 + 50 + 60 (readout)

    G4double GEMSpacerWh = 0.3 * mm / 2.0;
    G4double GEMSpacerWv = 0.5 * mm / 2.0;
    G4double GEMSpacerT = (2.0 - 0.1) * mm;
    G4double GEMHoleR = 2.2 * cm;
    G4double GEMCenterHalfXY = 7.4 * cm / 2.0;
    G4double GEMFrameWidth = 1.5 * cm;
    G4double GEMCenterOffset = GEMHalfX + GEMFrameWidth - GEMCenterHalfXY;

    // GEM Container
    G4Box *GEMConBox = new G4Box(Form("GEM%dConBox", layerid), 1.0 * m, 1.0 * m, (GEMGap + 2.0 * GEMHalfT + 1.0 * mm) / 2.0);
    G4Tubs *GEMConTube = new G4Tubs(Form("GEM%dConTube", layerid), 0, GEMHoleR, (GEMGap + 2.0 * GEMHalfT + 1.0 * mm) / 2.0 + 0.1 * mm, 0, twopi);
    G4SubtractionSolid *solidGEMCon = new G4SubtractionSolid(Form("GEM%dContainerS", layerid), GEMConBox, GEMConTube);
    G4LogicalVolume *logicGEMCon = new G4LogicalVolume(solidGEMCon, DefaultM, Form("GEM%dContainerLV", layerid));
    new G4PVPlacement(0, G4ThreeVector(0, 0, GEMCenter), logicGEMCon, Form("GEM %d Container", layerid), mother, false, 2 * layerid);

    // GEM
    G4Box *GEMBox = new G4Box(Form("GEM%dBox", layerid), GEMHalfX + GEMFrameWidth, GEMHalfY + GEMFrameWidth * 2.0, GEMHalfT);
    G4Tubs *GEMTube = new G4Tubs(Form("GEM%dTube", layerid), 0, GEMHoleR, GEMHalfT + 0.1 * mm, 0, twopi);
    G4SubtractionSolid *solidGEM = new G4SubtractionSolid(Form("GEM%dS", layerid), GEMBox, GEMTube, 0, G4ThreeVector(-GEMCenterOffset, 0, 0));
    G4LogicalVolume *logicGEM = new G4LogicalVolume(solidGEM, DefaultM, Form("GEM%dLV", layerid));
    new G4PVPlacement(0, G4ThreeVector(GEMCenterOffset, 0, GEMGap / 2.0), logicGEM, Form("GEM %d L", layerid), logicGEMCon, false, 0);
    G4RotationMatrix rmGEM;
    rmGEM.rotateZ(180.0 * deg);
    new G4PVPlacement(G4Transform3D(rmGEM, G4ThreeVector(-GEMCenterOffset, 0, -GEMGap / 2.0)), logicGEM, Form("GEM %d R", layerid), logicGEMCon, false, 1);

    // GEM Gas
    G4Box *GEMGasBox = new G4Box(Form("GEM%dGasBox", layerid), GEMHalfX, GEMHalfY, GEMHalfT);
    G4Box *GEMSubBox = new G4Box(Form("GEM%dSubBox", layerid), GEMCenterHalfXY, GEMCenterHalfXY, GEMHalfT + 0.1 * mm);
    G4SubtractionSolid *solidGEMGas = new G4SubtractionSolid(Form("GEM%dGasS", layerid), GEMGasBox, GEMSubBox, 0, G4ThreeVector(-GEMCenterOffset, 0, 0));
    G4LogicalVolume *logicGEMGas = new G4LogicalVolume(solidGEMGas, GEMGasM, Form("GEM%dGasLV", layerid));
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicGEMGas, Form("GEM %d Gas", layerid), logicGEM, false, 0);

    // GEM Frame
    G4Box *GEMFrameBox1 = new G4Box(Form("GEM%dFrameBox1", layerid), GEMHalfX + GEMFrameWidth, GEMHalfY + GEMFrameWidth * 2.0, GEMHalfT);
    G4Box *GEMFrameBox2 = new G4Box(Form("GEM%dFrameBox2", layerid), GEMHalfX, GEMHalfY, GEMHalfT + 0.1 * mm);
    G4SubtractionSolid *solidGEMFrame = new G4SubtractionSolid(Form("GEM%dFrameS", layerid), GEMFrameBox1, GEMFrameBox2);
    G4LogicalVolume *logicGEMFrame = new G4LogicalVolume(solidGEMFrame, GEMFrameM, Form("GEM%dFrameLV", layerid));
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicGEMFrame, Form("GEM %d Frame", layerid), logicGEM, false, 0);

    G4Box *GEMPipeBox = new G4Box(Form("GEM%dPipeBox", layerid), GEMCenterHalfXY - GEMFrameWidth / 2.0, GEMCenterHalfXY, GEMHalfT);
    G4SubtractionSolid *solidGEMPipe = new G4SubtractionSolid(Form("GEM%dPipeS", layerid), GEMPipeBox, GEMTube, 0, G4ThreeVector(-GEMFrameWidth / 2.0, 0, 0));
    G4LogicalVolume *logicGEMPipe = new G4LogicalVolume(solidGEMPipe, GEMFrameM, Form("GEM%dPipeLV", layerid));
    new G4PVPlacement(0, G4ThreeVector(-GEMCenterOffset + GEMFrameWidth / 2.0, 0, 0), logicGEMPipe, Form("GEM %d Pipe", layerid), logicGEM, false, 0);

    // GEM Spacer
    G4double GEMSpacerOffset = 91.3 * mm;
    G4Box *GEMHSpacerBox1 = new G4Box(Form("GEM%dHSpacerBox1", layerid), GEMHalfX, GEMSpacerWh, GEMSpacerT / 2.0);
    G4Box *GEMHSpacerBox2 = new G4Box(Form("GEM%dHSpacerBox2", layerid), GEMHalfX - GEMCenterHalfXY + GEMFrameWidth / 2.0, GEMSpacerWh, GEMSpacerT / 2.0);
    G4Box *GEMVSpacerBox = new G4Box(Form("GEM%dVSpacerBox", layerid), GEMSpacerWv, GEMHalfY, GEMSpacerT / 2.0);
    G4UnionSolid *GEMSpacerPiece1 = new G4UnionSolid(Form("GEM%dSpacerPiece1", layerid), GEMVSpacerBox, GEMHSpacerBox1, 0, G4ThreeVector(GEMSpacerOffset, 204.0 * mm, 0));
    G4UnionSolid *GEMSpacerPiece2 = new G4UnionSolid(Form("GEM%dSpacerPiece2", layerid), GEMSpacerPiece1, GEMHSpacerBox1, 0, G4ThreeVector(GEMSpacerOffset, 409.3 * mm, 0));
    G4UnionSolid *GEMSpacerPiece3 = new G4UnionSolid(Form("GEM%dSpacerPiece3", layerid), GEMSpacerPiece2, GEMHSpacerBox2, 0, G4ThreeVector(GEMSpacerOffset + GEMCenterHalfXY - GEMFrameWidth / 2.0, 0, 0));
    G4UnionSolid *GEMSpacerPiece4 = new G4UnionSolid(Form("GEM%dSpacerPiece4", layerid), GEMSpacerPiece3, GEMHSpacerBox1, 0, G4ThreeVector(GEMSpacerOffset, -204.0 * mm, 0));
    G4UnionSolid *GEMSpacerPiece5 = new G4UnionSolid(Form("GEM%dSpacerPiece5", layerid), GEMSpacerPiece4, GEMHSpacerBox1, 0, G4ThreeVector(GEMSpacerOffset, -409.3 * mm, 0));
    G4UnionSolid *solidGEMSpacer = new G4UnionSolid(Form("GEM%dSpacerS", layerid), GEMSpacerPiece5, GEMVSpacerBox, 0, G4ThreeVector(GEMSpacerOffset * 2.0, 0, 0));
    G4LogicalVolume *logicGEMSpacer = new G4LogicalVolume(solidGEMSpacer, GEMFrameM, Form("GEM%dSpacerLV", layerid));

    // GEM Foil
    G4double GEMWinT = 25.0 * um;
    G4double GEMFoilT = 50.0 * um;
    G4double GEMCuT = 5.0 * um;
    G4double GEMGlueT = 60.0 * um;

    G4Box *GEMWinBox = new G4Box(Form("GEM%dWinBox", layerid), GEMHalfX, GEMHalfY, GEMWinT / 2.0);
    G4SubtractionSolid *solidGEMWin = new G4SubtractionSolid(Form("GEM%dWinS", layerid), GEMWinBox, GEMSubBox, 0, G4ThreeVector(-GEMCenterOffset, 0, 0));
    G4LogicalVolume *logicGEMWin = new G4LogicalVolume(solidGEMWin, GEMFoilM, Form("GEM%dWinLV", layerid));

    G4Box *GEMFoilBox = new G4Box(Form("GEM%dFoilBox", layerid), GEMHalfX, GEMHalfY, GEMFoilT / 2.0);
    G4SubtractionSolid *solidGEMFoil = new G4SubtractionSolid(Form("GEM%dFoilS", layerid), GEMFoilBox, GEMSubBox, 0, G4ThreeVector(-GEMCenterOffset, 0, 0));
    G4LogicalVolume *logicGEMFoil = new G4LogicalVolume(solidGEMFoil, GEMFoil0d8M, Form("GEM%dFoilLV", layerid));
    G4LogicalVolume *logicGEMFoil80 = new G4LogicalVolume(solidGEMFoil, GEMFoil0d2M, Form("GEM%dFoil80LV", layerid));
    G4LogicalVolume *logicGEMFoil350 = new G4LogicalVolume(solidGEMFoil, GEMFoilM, Form("GEM%dFoil350LV", layerid));
    G4LogicalVolume *logicGEMCathode = new G4LogicalVolume(solidGEMFoil, GEMFoilM, Form("GEM%dCathodeLV", layerid));

    G4Box *GEMCuBox = new G4Box(Form("GEM%dCuBox", layerid), GEMHalfX, GEMHalfY, GEMCuT / 2.0);
    G4SubtractionSolid *solidGEMCu = new G4SubtractionSolid(Form("GEM%dCuS", layerid), GEMCuBox, GEMSubBox, 0, G4ThreeVector(-GEMCenterOffset, 0, 0));
    G4LogicalVolume *logicGEMCu = new G4LogicalVolume(solidGEMCu, GEMCu0d8M, Form("GEM%dCuLV", layerid));
    G4LogicalVolume *logicGEMCu80 = new G4LogicalVolume(solidGEMCu, GEMCu0d2M, Form("GEM%dCu80LV", layerid));
    G4LogicalVolume *logicGEMCu350 = new G4LogicalVolume(solidGEMCu, GEMCu0d75M, Form("GEM%dCu350LV", layerid));
    G4LogicalVolume *logicGEMCathodeCu = new G4LogicalVolume(solidGEMCu, GEMCuM, Form("GEM%dCathodeCuLV", layerid));

    G4Box *GEMGlueBox = new G4Box(Form("GEM%dGlueBox", layerid), GEMHalfX, GEMHalfY, GEMGlueT / 2.0);
    G4SubtractionSolid *solidGEMGlue = new G4SubtractionSolid(Form("GEM%dGlueS", layerid), GEMGlueBox, GEMSubBox, 0, G4ThreeVector(-GEMCenterOffset, 0, 0));
    G4LogicalVolume *logicGEMGlue = new G4LogicalVolume(solidGEMGlue, GEMGlueM, Form("GEM%dGlueLV", layerid));

    G4double zoff = -GEMHalfT;

    new G4PVPlacement(0, G4ThreeVector(0, 0, zoff + GEMWinT / 2.0), logicGEMWin, Form("GEM %d Window", layerid), logicGEMGas, false, 0);
    zoff += GEMWinT;

    new G4PVPlacement(0, G4ThreeVector(-GEMSpacerOffset, 0, zoff + GEMSpacerT / 2.0), logicGEMSpacer, Form("GEM %d Spacer", layerid), logicGEMGas, false, 0);
    zoff += 3.0 * mm;

    new G4PVPlacement(0, G4ThreeVector(0, 0, zoff + GEMFoilT / 2.0), logicGEMCathode, Form("GEM %d Cathode", layerid), logicGEMGas, false, 0);
    zoff += GEMFoilT;

    if (!culess) {
        new G4PVPlacement(0, G4ThreeVector(0, 0, zoff + GEMCuT / 2.0), logicGEMCathodeCu, Form("GEM %d Copper", layerid), logicGEMGas, false, 0);
        zoff += GEMCuT;
    }

    new G4PVPlacement(0, G4ThreeVector(-GEMSpacerOffset, 0, zoff + GEMSpacerT / 2.0), logicGEMSpacer, Form("GEM %d Spacer", layerid), logicGEMGas, false, 1);
    zoff += 3.0 * mm;

    if (!culess) {
        new G4PVPlacement(0, G4ThreeVector(0, 0, zoff + GEMCuT / 2.0), logicGEMCu, Form("GEM %d Copper", layerid), logicGEMGas, false, 1);
        zoff += GEMCuT;
    }

    new G4PVPlacement(0, G4ThreeVector(0, 0, zoff + GEMFoilT / 2.0), logicGEMFoil, Form("GEM %d Foil", layerid), logicGEMGas, false, 0);
    zoff += GEMFoilT;

    if (!culess) {
        new G4PVPlacement(0, G4ThreeVector(0, 0, zoff + GEMCuT / 2.0), logicGEMCu, Form("GEM %d Copper", layerid), logicGEMGas, false, 2);
        zoff += GEMCuT;
    }

    new G4PVPlacement(0, G4ThreeVector(-GEMSpacerOffset, 0, zoff + GEMSpacerT / 2.0), logicGEMSpacer, Form("GEM %d Spacer", layerid), logicGEMGas, false, 2);
    zoff += 2.0 * mm;

    if (!culess) {
        new G4PVPlacement(0, G4ThreeVector(0, 0, zoff + GEMCuT / 2.0), logicGEMCu, Form("GEM %d Copper", layerid), logicGEMGas, false, 3);
        zoff += GEMCuT;
    }

    new G4PVPlacement(0, G4ThreeVector(0, 0, zoff + GEMFoilT / 2.0), logicGEMFoil, Form("GEM %d Foil", layerid), logicGEMGas, false, 1);
    zoff += GEMFoilT;

    if (!culess) {
        new G4PVPlacement(0, G4ThreeVector(0, 0, zoff + GEMCuT / 2.0), logicGEMCu, Form("GEM %d Copper", layerid), logicGEMGas, false, 4);
        zoff += GEMCuT;
    }

    new G4PVPlacement(0, G4ThreeVector(-GEMSpacerOffset, 0, zoff + GEMSpacerT / 2.0), logicGEMSpacer, Form("GEM %d Spacer", layerid), logicGEMGas, false, 3);
    zoff += 2.0 * mm;

    if (!culess) {
        new G4PVPlacement(0, G4ThreeVector(0, 0, zoff + GEMCuT / 2.0), logicGEMCu, Form("GEM %d Copper", layerid), logicGEMGas, false, 5);
        zoff += GEMCuT;
    }

    new G4PVPlacement(0, G4ThreeVector(0, 0, zoff + GEMFoilT / 2.0), logicGEMFoil, Form("GEM %d Foil", layerid), logicGEMGas, false, 2);
    zoff += GEMFoilT;

    if (!culess) {
        new G4PVPlacement(0, G4ThreeVector(0, 0, zoff + GEMCuT / 2.0), logicGEMCu, Form("GEM %d Copper", layerid), logicGEMGas, false, 6);
        zoff += GEMCuT;
    }

    new G4PVPlacement(0, G4ThreeVector(-GEMSpacerOffset, 0, zoff + GEMSpacerT / 2.0 + 0.01 * mm), logicGEMSpacer, Form("GEM %d Spacer", layerid), logicGEMGas, false, 4);
    zoff += 2.0 * mm;

    if (!culess) {
        new G4PVPlacement(0, G4ThreeVector(0, 0, zoff + GEMCuT / 2.0), logicGEMCu80, Form("GEM %d Copper", layerid), logicGEMGas, false, 7);
        zoff += GEMCuT;
    }

    new G4PVPlacement(0, G4ThreeVector(0, 0, zoff + GEMFoilT / 2.0), logicGEMFoil80, Form("GEM %d Foil", layerid), logicGEMGas, false, 3);
    zoff += GEMFoilT;

    if (!culess) {
        new G4PVPlacement(0, G4ThreeVector(0, 0, zoff + GEMCuT / 2.0), logicGEMCu350, Form("GEM %d Copper", layerid), logicGEMGas, false, 8);
        zoff += GEMCuT;
    }

    new G4PVPlacement(0, G4ThreeVector(0, 0, zoff + GEMFoilT / 2.0), logicGEMFoil350, Form("GEM %d Foil", layerid), logicGEMGas, false, 4);
    zoff += GEMFoilT;
    new G4PVPlacement(0, G4ThreeVector(0, 0, zoff + GEMGlueT / 2.0), logicGEMGlue, Form("GEM %d Glue", layerid), logicGEMGas, false, 0);
    zoff += GEMGlueT;

    new G4PVPlacement(0, G4ThreeVector(0, 0, GEMHalfT - GEMWinT / 2.0), logicGEMWin, Form("GEM %d Window", layerid), logicGEMGas, false, 1);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::AddHyCal(G4LogicalVolume *mother)
{
    G4Material *HyCalConM = G4Material::GetMaterial("Air");
    G4Material *HyCalBoxM = G4Material::GetMaterial("Rohacell");
    G4Material *HyCalBoxWinM = G4Material::GetMaterial("Tedlar");
    G4Material *CollimatorM = G4Material::GetMaterial("Tungsten");
    G4Material *PbWO4ModuleM = G4Material::GetMaterial("PbWO4");
    G4Material *PbGlassModuleM = G4Material::GetMaterial("PbGlass");
    G4Material *TedlarTapeM = G4Material::GetMaterial("Tedlar");
    G4Material *ReflectorM = G4Material::GetMaterial("Polyester");
    G4Material *PlateM = G4Material::GetMaterial("Brass");

    std::ifstream dimension_file;
    dimension_file.open("database/hycal_module_shuffled.dat");
    G4double pwo[1152][4], lg[576][4];

    for (int i = 0; i < 576; i++)
        dimension_file >> lg[i][0] >> lg[i][1] >> lg[i][2] >> lg[i][3];

    for (int i = 0; i < 1152; i++)
        dimension_file >> pwo[i][0] >> pwo[i][1] >> pwo[i][2] >> pwo[i][3];

    dimension_file.close();

    G4double MaxStep = 1.0 * mm;

    // HyCal
    G4double CrystalL = 18.0 * cm;
    G4double PbGlassL = 45.0 * cm;
    G4double CrystalDiffL = 9.73 * cm; // according to last survey (april 2017)
    G4double CrystalCenter = fCrystalSurf + CrystalL / 2.0;
    G4double PbGlassCenter = fCrystalSurf - CrystalDiffL + PbGlassL / 2.0;

    // HyCal container
    G4double HyCalBoxCenter = PbGlassCenter - 9.0 * cm + 30.0 * cm; // Check
    G4Box *HyCalConNoHole = new G4Box("HyCalConNoHole", 80.0 * cm, 80.0 * cm, 65.0 * cm);
    G4Tubs *HyCalConHole = new G4Tubs("HyCalConHole", 0, 31.75 * mm, 66.0 * cm, 0, twopi);
    G4SubtractionSolid *solidHyCalCon = new G4SubtractionSolid("HyCalConS", HyCalConNoHole, HyCalConHole);
    G4LogicalVolume *logicHyCalCon = new G4LogicalVolume(solidHyCalCon, HyCalConM, "HyCalConLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, HyCalBoxCenter), logicHyCalCon, "HyCal Container", mother, false, 0);
    logicHyCalCon->SetUserLimits(new G4UserLimits(MaxStep));

    // HyCal box
    G4Box *HyCalBoxOuter = new G4Box("HyCalBoxOuter", 72.54 * cm, 72.54 * cm, 62.54 * cm);
    G4Box *HyCalBoxInner = new G4Box("HyCalBoxInner", 70.0 * cm, 70.0 * cm, 60.0 * cm);
    G4SubtractionSolid *HyCalBoxNoHole = new G4SubtractionSolid("HyCalBoxNoHole", HyCalBoxOuter, HyCalBoxInner);
    G4Tubs *HyCalBoxHole = new G4Tubs("HyCalBoxHole", 0, 3.175 * cm, 65.0 * cm, 0, twopi);
    G4SubtractionSolid *solidHyCalBox = new G4SubtractionSolid("HyCalBoxS", HyCalBoxNoHole, HyCalBoxHole);
    G4LogicalVolume *logicHyCalBox = new G4LogicalVolume(solidHyCalBox, HyCalBoxM, "HyCalBoxLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicHyCalBox, "HyCal Box", logicHyCalCon, false, 0);

    // HyCal box window
    G4VSolid *solidHyCalBoxWin = new G4Tubs("HyCalBoxWinS", 1.90 * cm, 5.08 * cm, 19.0 * um, 0, twopi);
    G4LogicalVolume *logicHyCalBoxWin = new G4LogicalVolume(solidHyCalBoxWin, HyCalBoxWinM, "HyCalBoxWinLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, -62.54 * cm - 19.0 * um), logicHyCalBoxWin, "HyCal Box Window", logicHyCalCon, false, 0);
    new G4PVPlacement(0, G4ThreeVector(0, 0, -60.00 * cm + 19.0 * um), logicHyCalBoxWin, "HyCal Box Window", logicHyCalCon, false, 1);

    // Lead glass module container
    G4double PlateThickness = 25.4 * 0.08 * mm; // 0.08 in
    G4double PbGlassPlateHoleR = 25.4 * 0.5 * mm; // 0.5 in
    G4double ReflectorT = 25.0 * um;
    G4double StripThickness = 25.4 * 0.001 * mm; // 0.001 in

    G4Box *PbGlassConBox = new G4Box("PbGlassConBox", 58.199 * cm, 58.165 * cm, PbGlassL / 2.0 + PlateThickness);
    G4Box *PbGlassConHole = new G4Box("PbGlassConHole", 35.309 * cm, 35.275 * cm, PbGlassL / 2.0 + PlateThickness + 1.0 * mm);
    G4SubtractionSolid *solidPbGlassCon = new G4SubtractionSolid("PbGlassModuleContainerS", PbGlassConBox, PbGlassConHole);
    G4LogicalVolume *logicPbGlassCon = new G4LogicalVolume(solidPbGlassCon, HyCalConM, "PbGlassModuleContainerLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, PbGlassCenter - HyCalBoxCenter), logicPbGlassCon, "PbGlass Module Container", logicHyCalCon, false, 0);

    for (int i = 0; i < 576; i++) {
        G4double PbGlassX = lg[i][0] - 0.15 * mm;
        G4double PbGlassY = lg[i][1] - 0.15 * mm;

        // Lead glass module
        G4VSolid *solidPbGlassModule = new G4Box(Form("PbGlassModule%04dS", i), lg[i][0] * mm / 2.0, lg[i][1] * mm / 2.0, PbGlassL / 2.0 + PlateThickness);
        G4LogicalVolume *logicPbGlassModule = new G4LogicalVolume(solidPbGlassModule, HyCalConM, Form("PbGlassModule%04dLV", i));
        new G4PVPlacement(0, G4ThreeVector(lg[i][2] * mm, lg[i][3] * mm, 0), logicPbGlassModule, "PbGlass Module", logicPbGlassCon, false, i);

        G4VSolid *solidPbGlassReflector = new G4Box(Form("PbGlassReflector%04dS", i), PbGlassX / 2.0 + ReflectorT, PbGlassY / 2.0 + ReflectorT, PbGlassL / 2.0);
        G4LogicalVolume *logicPbGlassReflector = new G4LogicalVolume(solidPbGlassReflector, ReflectorM, Form("PbGlassReflector%04dLV", i));
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicPbGlassReflector, "Reflector", logicPbGlassModule, false, 0);

        G4VSolid *solidPbGlassAbsorber = new G4Box(Form("PbGlassAbsorber%04dS", i), PbGlassX / 2.0, PbGlassY / 2.0, PbGlassL / 2.0);
        G4LogicalVolume *logicPbGlassAbsorber = new G4LogicalVolume(solidPbGlassAbsorber, PbGlassModuleM, Form("PbGlassAbsorber%04dLV", i));
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicPbGlassAbsorber, "PbGlass Absorber", logicPbGlassReflector, false, 0);

        G4Box *PbGlassPlateBox = new G4Box("PbGlassPlateBox", PbGlassX / 2.0, PbGlassY / 2.0, PlateThickness / 2.0);
        G4Tubs *PbGlassPlateHole = new G4Tubs("PbGlassPlateHole", 0, PbGlassPlateHoleR, PlateThickness / 2.0 + 1.0 * mm, 0, twopi);
        G4SubtractionSolid *solidPbGlassPlate = new G4SubtractionSolid(Form("PbGlassPlate%04dS", i), PbGlassPlateBox, PbGlassPlateHole);
        G4LogicalVolume *logicPbGlassPlate = new G4LogicalVolume(solidPbGlassPlate, PlateM, Form("PbGlassPlate%04dLV", i));
        new G4PVPlacement(0, G4ThreeVector(0, 0, - PbGlassL / 2.0 - PlateThickness / 2.0), logicPbGlassPlate, "Brass Plate", logicPbGlassModule, false, 0);


        if ((lg[i][2] > -353.09 && lg[i][3] > 352.75) || (lg[i][2] < 353.09 && lg[i][3] < -352.75)) {
            G4double StripWidth = PbGlassX;

            G4VSolid *solidPbGlassStrip = new G4Box(Form("PbGlassStrip%04dS", i), StripWidth / 2.0, StripThickness / 2.0,  PbGlassL / 2.0);
            G4LogicalVolume *logicPbGlassStrip = new G4LogicalVolume(solidPbGlassStrip, PlateM, Form("PbGlassStrip%04dLV", i));
            new G4PVPlacement(0, G4ThreeVector(0, (lg[i][1] * mm - StripThickness) / 2.0, 0), logicPbGlassStrip, "Brass Strip", logicPbGlassModule, false, 0);
            new G4PVPlacement(0, G4ThreeVector(0, (-lg[i][1] * mm + StripThickness) / 2.0, 0), logicPbGlassStrip, "Brass Strip", logicPbGlassModule, false, 1);
        } else {
            G4double StripWidth = PbGlassY;

            G4VSolid *solidPbGlassStrip = new G4Box(Form("PbGlassStrip%04dS", i), StripThickness / 2.0, StripWidth / 2.0, PbGlassL / 2.0);
            G4LogicalVolume *logicPbGlassStrip = new G4LogicalVolume(solidPbGlassStrip, PlateM, Form("PbGlassStrip%04dLV", i));
            new G4PVPlacement(0, G4ThreeVector((lg[i][0] * mm - StripThickness) / 2.0, 0, 0), logicPbGlassStrip, "Brass Strip", logicPbGlassModule, false, 0);
            new G4PVPlacement(0, G4ThreeVector((-lg[i][0] * mm + StripThickness) / 2.0, 0, 0), logicPbGlassStrip, "Brass Strip", logicPbGlassModule, false, 1);
        }
    }

    // PbWO4 module container
    G4double CrystalPlateHoleR = 25.4 * 0.25 * mm; // 0.25 in
    G4double TedlarTapeT = 38.1 * um;
    ReflectorT = 63.0 * um;

    G4Box *PbWO4ConBox = new G4Box("PbWO4ConBox", 35.309 * cm, 35.275 * cm, CrystalL / 2.0 + PlateThickness);
    G4Box *PbWO4ConHole = new G4Box("PbWO4ConHole", 2.0 * cm, 2.0 * cm, CrystalL / 2.0 + PlateThickness + 1.0 * mm);
    G4SubtractionSolid *solidPbWO4Con = new G4SubtractionSolid("PbWO4ModuleContainerS", PbWO4ConBox, PbWO4ConHole);
    G4LogicalVolume *logicPbWO4Con = new G4LogicalVolume(solidPbWO4Con, HyCalConM, "PbWO4ModuleContainerLV");
    new G4PVPlacement(0, G4ThreeVector(0, 0, CrystalCenter - HyCalBoxCenter), logicPbWO4Con, "PbWO4 Module Container", logicHyCalCon, false, 576);

    for (int i = 0; i < 1152; i++) {
        G4double CrystalX = pwo[i][0] - 0.27 * mm;
        G4double CrystalY = pwo[i][1] - 0.25 * mm;
        G4double StripWidth = 25.4 * 0.5 * mm; // 0.5 in

        // PbWO4 module
        G4VSolid *solidPbWO4Module = new G4Box(Form("PbWO4Module%04dS", i), pwo[i][0] * mm / 2.0, pwo[i][1] * mm / 2.0, CrystalL / 2.0 + PlateThickness);
        G4LogicalVolume *logicPbWO4Module = new G4LogicalVolume(solidPbWO4Module, HyCalConM, Form("PbWO4Module%04dLV", i));
        new G4PVPlacement(0, G4ThreeVector(pwo[i][2] * mm, pwo[i][3] * mm, 0), logicPbWO4Module, "PbGlass Module", logicPbWO4Con, false, i);

        G4VSolid *solidPbWO4TedlarTape = new G4Box(Form("PbWO4TedlarTape%04dS", i), CrystalX / 2.0 + TedlarTapeT + ReflectorT, CrystalY / 2.0 + TedlarTapeT + ReflectorT, CrystalL / 2.0);
        G4LogicalVolume *logicPbWO4TedlarTape = new G4LogicalVolume(solidPbWO4TedlarTape, TedlarTapeM, Form("PbWO4TedlarTape%04dLV", i));
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicPbWO4TedlarTape, "Tedlar Tape", logicPbWO4Module, false, 0);

        G4VSolid *solidPbWO4Reflector = new G4Box(Form("PbWO4Reflector%04dS", i), CrystalX / 2.0 + ReflectorT, CrystalY / 2.0 + ReflectorT, CrystalL / 2.0);
        G4LogicalVolume *logicPbWO4Reflector = new G4LogicalVolume(solidPbWO4Reflector, ReflectorM, Form("PbWO4Reflector%04dLV", i));
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicPbWO4Reflector, "Reflector", logicPbWO4TedlarTape, false, 0);

        G4VSolid *solidPbWO4Absorber = new G4Box(Form("PbWO4Absorber%04dS", i), CrystalX / 2.0, CrystalY / 2.0, CrystalL / 2.0);
        G4LogicalVolume *logicPbWO4Absorber = new G4LogicalVolume(solidPbWO4Absorber, PbWO4ModuleM, Form("PbWO4Absorber%04dLV", i));
        new G4PVPlacement(0, G4ThreeVector(0, 0, 0), logicPbWO4Absorber, "PbWO4 Absorber", logicPbWO4Reflector, false, 0);

        G4Box *PbWO4PlateBox = new G4Box("PbWO4PlateBox", CrystalX / 2.0, CrystalY / 2.0, PlateThickness / 2.0);
        G4Tubs *PbWO4PlateHole = new G4Tubs("PbWO4PlateHole", 0, CrystalPlateHoleR, PlateThickness / 2.0 + 1.0 * mm, 0, twopi);
        G4SubtractionSolid *solidPbWO4Plate = new G4SubtractionSolid(Form("PbWO4Plate%04dS", i), PbWO4PlateBox, PbWO4PlateHole);
        G4LogicalVolume *logicPbWO4Plate = new G4LogicalVolume(solidPbWO4Plate, PlateM, Form("PbWO4Plate%04dLV", i));
        new G4PVPlacement(0, G4ThreeVector(0, 0, -(CrystalL + PlateThickness) / 2.0), logicPbWO4Plate, "Brass Plate", logicPbWO4Module, false, 0);

        G4VSolid *solidPbWO4Strip = new G4Box(Form("PbWO4Strip%04dS", i), StripThickness / 2.0, StripWidth / 2.0, CrystalL / 2.0);
        G4LogicalVolume *logicPbWO4Strip = new G4LogicalVolume(solidPbWO4Strip, PlateM, Form("PbWO4Strip%04dLV", i));
        new G4PVPlacement(0, G4ThreeVector((pwo[i][0] * mm - StripThickness) / 2.0, 0, 0), logicPbWO4Strip, "Brass Strip", logicPbWO4Module, false, 0);
        new G4PVPlacement(0, G4ThreeVector((-pwo[i][0] * mm + StripThickness) / 2.0, 0, 0), logicPbWO4Strip, "Brass Strip", logicPbWO4Module, false, 1);
    }

    // Collimator
    G4Box *CollimatorBox = new G4Box("CollimatorBox", 4.07 * cm, 4.07 * cm, 3.02 * cm);
    G4Tubs *CollimatorTube = new G4Tubs("CollimatorTube", 0, 1.95 * cm, 3.1 * cm, 0, twopi);
    G4SubtractionSolid *solidCollimator = new G4SubtractionSolid("CollimatorS", CollimatorBox, CollimatorTube);
    G4LogicalVolume *logicCollimator = new G4LogicalVolume(solidCollimator, CollimatorM, "CollimatorLV");
    G4RotationMatrix rmColl;
    rmColl.rotateZ(-8.8 * deg);
    new G4PVPlacement(G4Transform3D(rmColl, G4ThreeVector(0, 0, fCrystalSurf - PlateThickness - 3.1 * cm - HyCalBoxCenter)), logicCollimator, "Collimator", logicHyCalCon, false, 0);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
