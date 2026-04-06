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
// DetectorConstruction.cc
// Developer : Chao Peng, Chao Gu
// History:
//   Aug 2012, C. Peng, Original version.
//   Jan 2017, C. Gu, Rewrite with ROOT support.
//   Mar 2017, C. Gu, Add DRad configuration.
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "DetectorConstruction.hh"

#include "DetectorMessenger.hh"
#include "SimConfig.hh"

#include "G4Element.hh"
#include "G4Isotope.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"

#include "G4VPhysicalVolume.hh"
#include "G4VUserDetectorConstruction.hh"

#include "G4Colour.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4String.hh"
#include "G4VisAttributes.hh"

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::DetectorConstruction(G4String conf, const SimConfig &config) : G4VUserDetectorConstruction(), fConfig(conf)
{
    if (fConfig != "prad" && fConfig != "drad" && fConfig != "test")
        fConfig = "prad";

    fVisAtts.clear();

    // World geometry
    fWorldSizeXY = config.GetDouble("world", "size_xy", 150.0) * cm;
    fWorldSizeZ = config.GetDouble("world", "size_z", 600.0) * cm;

    // Target
    fTargetCenter = config.GetDouble("target", "center", -291.1) * cm;
    fTargetR = config.GetDouble("target", "radius", 14.5) * cm;
    fTargetHalfL = config.GetDouble("target", "half_length", 2.75) * cm;
    fTargetMat = config.GetString("target", "material", "D2Gas");
    fTargetDensityRatio = config.GetDouble("target", "density_ratio", 1.0);

    // Recoil detector
    fRecoilDetNSeg = config.GetInt("recoil_detector", "n_segments", 20);
    fRecoilDetCenter = fTargetCenter;
    fRecoilDetR = config.GetDouble("recoil_detector", "radius", 13.5) * cm;
    fRecoilDetHalfL = config.GetDouble("recoil_detector", "half_length", 2.6) * cm;
    fRecoilDetL1Thickness = config.GetDouble("recoil_detector", "l1_thickness_um", 200) * um;
    fRecoilDetL2Thickness = config.GetDouble("recoil_detector", "l2_thickness_um", 300) * um;

    // Derived positions (vacuum system)
    fDownChamberCenter = fTargetCenter + 74.0 * mm + 71.0 * cm / 2.0;
    fVacBoxCenter = fTargetCenter + 74.0 * mm + 71.0 * cm + 425.17 * cm / 2.0;

    // GEM detectors
    std::vector<double> gemCenters = config.GetDoubleArray("gem", "center", {217.5, 257.5});

    for (size_t i = 0; i < gemCenters.size() && i < 10; i++)
        fGEMCenter[i] = gemCenters[i] * cm;

    // Scintillator plane
    fSciPlaneCenter = config.GetDouble("scintillator_plane", "center", 262.5) * cm;

    // HyCal
    fCrystalSurf = config.GetDouble("hycal", "crystal_surface", 295.0) * cm;

    fExtDensityRatio = config.GetDouble("ext_density_ratio", 1.0);

    // Sensitive detector flags
    fTargetSDOn = config.GetBool("sensitive_detectors", "target", false);
    fRecoilDetSDOn = config.GetBool("sensitive_detectors", "recoil", false);
    fGEMSDOn = config.GetBool("sensitive_detectors", "gem", true);
    fSciPlaneSDOn = config.GetBool("sensitive_detectors", "scintillator_plane", false);
    fHyCalSDOn = config.GetBool("sensitive_detectors", "hycal", true);
    fVirtualSDOn = config.GetBool("sensitive_detectors", "virtual", false);

    // HyCal optical properties
    fAttenuationLG = config.GetDouble("hycal", "attenuation_lg", 0.0);
    fReflectanceLG = config.GetDouble("hycal", "reflectance_lg", 1.0);

    detectorMessenger = std::make_unique<DetectorMessenger>(this);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DetectorConstruction::~DetectorConstruction()
{
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

G4VPhysicalVolume *DetectorConstruction::Construct()
{
    // Define materials
    DefineMaterials();

    // Define volumes
    if (fConfig == "drad")
        return DefineDRadVolumes();
    else if (fConfig == "test")
        return DefineTestVolumes();
    else
        return DefinePRadVolumes();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::ConstructSDandField()
{
    if (fConfig == "drad")
        DefineDRadSDs();
    else if (fConfig == "test")
        DefineTestSDs();
    else
        DefinePRadSDs();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DetectorConstruction::DefineMaterials()
{
    G4String symbol;
    G4int z, n;
    G4double a;
    G4double density;
    G4int ncomponents, natoms;
    G4double fractionmass;

    G4NistManager *pNM = G4NistManager::Instance();

    // Define elements from NIST material table
    G4Element *H  = pNM->FindOrBuildElement(z = 1);
    G4Element *He = pNM->FindOrBuildElement(z = 2);
    G4Element *C  = pNM->FindOrBuildElement(z = 6);
    G4Element *N  = pNM->FindOrBuildElement(z = 7);
    G4Element *O  = pNM->FindOrBuildElement(z = 8);
    G4Element *F  = pNM->FindOrBuildElement(z = 9);
    G4Element *Na = pNM->FindOrBuildElement(z = 11);
    G4Element *Al = pNM->FindOrBuildElement(z = 13);
    G4Element *Si = pNM->FindOrBuildElement(z = 14);
    G4Element *P  = pNM->FindOrBuildElement(z = 15);
    G4Element *S  = pNM->FindOrBuildElement(z = 16);
    G4Element *Ar = pNM->FindOrBuildElement(z = 18);
    G4Element *K  = pNM->FindOrBuildElement(z = 19);
    G4Element *Cr = pNM->FindOrBuildElement(z = 24);
    G4Element *Mn = pNM->FindOrBuildElement(z = 25);
    G4Element *Fe = pNM->FindOrBuildElement(z = 26);
    G4Element *Ni = pNM->FindOrBuildElement(z = 28);
    G4Element *Cu = pNM->FindOrBuildElement(z = 29);
    G4Element *Zn = pNM->FindOrBuildElement(z = 30);
    G4Element *As = pNM->FindOrBuildElement(z = 33);
    G4Element *W  = pNM->FindOrBuildElement(z = 74);
    G4Element *Pb = pNM->FindOrBuildElement(z = 82);

    // Define isotopes
    G4Isotope *H2 = new G4Isotope("H2", z = 1, n = 2, a = 2.0141 * g / mole);
    G4Element *D = new G4Element("Deuterium", symbol = "D", ncomponents = 1);
    D->AddIsotope(H2, 1.0);

    // Define materials

    // Space Vacuum
    G4Material *Galaxy = new G4Material("Galaxy", density = universe_mean_density, ncomponents = 1, kStateGas, 0.1 * kelvin, 1.0e-19 * pascal);
    Galaxy->AddElement(H, fractionmass = 1.0);
    fVisAtts[Galaxy->GetName()] = new G4VisAttributes(G4VisAttributes::Invisible);

    // Air
    G4Material *Air = new G4Material("Air", density = 1.292 * mg / cm3, ncomponents = 2);
    Air->AddElement(N, fractionmass = 0.7);
    Air->AddElement(O, fractionmass = 0.3);
    fVisAtts[Air->GetName()] = new G4VisAttributes(G4VisAttributes::Invisible);

    // Air vacuum of 1.e-6 torr at room temperature, 1 atmosphere = 760 torr
    G4Material *Vacuum = new G4Material("Vacuum", density = 1.0e-6 / 760.0 * 1.292 * mg / cm3, ncomponents = 1, kStateGas, STP_Temperature, 1.0e-6 / 760.0 * atmosphere);
    Vacuum->AddMaterial(Air, fractionmass = 1.0);
    fVisAtts[Vacuum->GetName()] = new G4VisAttributes(G4VisAttributes::Invisible);

    // Hydrogen Gas (T = 19.5 K, P = 470 mTorr)
    G4Material *H2Gas = new G4Material("H2Gas", density = fTargetDensityRatio * 0.47 / 760.0 * 273.15 / 19.5 * 0.08988 * mg / cm3, ncomponents = 1, kStateGas, 19.5 * kelvin, fTargetDensityRatio * 0.47 / 760.0 * atmosphere);
    H2Gas->AddElement(H, natoms = 2);
    fVisAtts[H2Gas->GetName()] = new G4VisAttributes(G4Colour::Cyan());

    // Deuteron Gas
    G4Material *D2Gas = new G4Material("D2Gas", density = fTargetDensityRatio * 0.47 / 760.0 * 273.15 / 19.5 * 0.1796 * mg / cm3, ncomponents = 1, kStateGas, 19.5 * kelvin, fTargetDensityRatio * 0.47 / 760.0 * atmosphere);
    D2Gas->AddElement(D, natoms = 2);
    fVisAtts[D2Gas->GetName()] = new G4VisAttributes(G4Colour::Cyan());

    // Copper C101
    G4Material *Copper = new G4Material("Copper", density = fExtDensityRatio * 8.92 * g / cm3, ncomponents = 1);
    Copper->AddElement(Cu, natoms = 1);
    fVisAtts[Copper->GetName()] = new G4VisAttributes(G4Colour::Brown());
    G4Material *Copper0d2 = new G4Material("Copper0.2", Copper->GetDensity() * 0.2, Copper);
    fVisAtts[Copper0d2->GetName()] = new G4VisAttributes(G4Colour::Brown());
    G4Material *Copper0d75 = new G4Material("Copper0.75", Copper->GetDensity() * 0.75, Copper);
    fVisAtts[Copper0d75->GetName()] = new G4VisAttributes(G4Colour::Brown());
    G4Material *Copper0d8 = new G4Material("Copper0.8", Copper->GetDensity() * 0.8, Copper);
    fVisAtts[Copper0d8->GetName()] = new G4VisAttributes(G4Colour::Brown());

    // Kapton
    G4Material *Kapton = new G4Material("Kapton", density = fExtDensityRatio * 1.42 * g / cm3, ncomponents = 4);
    Kapton->AddElement(H, fractionmass = 0.0273);
    Kapton->AddElement(C, fractionmass = 0.7213);
    Kapton->AddElement(N, fractionmass = 0.0765);
    Kapton->AddElement(O, fractionmass = 0.1749);
    fVisAtts[Kapton->GetName()] = new G4VisAttributes(G4Colour::Brown());
    G4Material *Kapton0d2 = new G4Material("Kapton0.2", Kapton->GetDensity() * 0.2, Kapton);
    fVisAtts[Kapton0d2->GetName()] = new G4VisAttributes(G4Colour::Brown());
    G4Material *Kapton0d8 = new G4Material("Kapton0.8", Kapton->GetDensity() * 0.8, Kapton);
    fVisAtts[Kapton0d8->GetName()] = new G4VisAttributes(G4Colour::Brown());

    // Silicon
    G4Material *Silicon = new G4Material("Silicon", density = 2.329 * g / cm3, ncomponents = 1);
    Silicon->AddElement(Si, natoms = 1);
    fVisAtts[Silicon->GetName()] = new G4VisAttributes(G4Colour::Green());

    // Aluminum
    G4Material *Aluminum = new G4Material("Aluminum", density = fExtDensityRatio * 2.700 * g / cm3, ncomponents = 1);
    Aluminum->AddElement(Al, natoms = 1);
    fVisAtts[Aluminum->GetName()] = new G4VisAttributes(G4Colour::Grey());

    // Tedlar
    G4Material *Tedlar = new G4Material("Tedlar", density = 1.545 * g / cm3, ncomponents = 3);
    Tedlar->AddElement(H, natoms = 3);
    Tedlar->AddElement(C, natoms = 2);
    Tedlar->AddElement(F, natoms = 1);
    fVisAtts[Tedlar->GetName()] = new G4VisAttributes(G4Colour::Grey());

    // Stainless Steel
    G4Material *SSteel = new G4Material("SSteel", density = fExtDensityRatio * 7.9 * g / cm3, ncomponents = 9);
    SSteel->AddElement(C, fractionmass = 0.0007);
    SSteel->AddElement(Si, fractionmass = 0.01);
    SSteel->AddElement(Mn, fractionmass = 0.02);
    SSteel->AddElement(Ni, fractionmass = 0.09);
    SSteel->AddElement(P, fractionmass = 0.00045);
    SSteel->AddElement(S, fractionmass = 0.00015);
    SSteel->AddElement(Cr, fractionmass = 0.18);
    SSteel->AddElement(N, fractionmass = 0.0011);
    SSteel->AddElement(Fe, fractionmass = 0.6976);
    fVisAtts[SSteel->GetName()] = new G4VisAttributes(G4Colour::Grey());

    // Nickel
    G4Material *Nickel = new G4Material("Nickel", density = fExtDensityRatio * 8.908 * g / cm3, ncomponents = 1);
    Nickel->AddElement(Ni, natoms = 1);
    fVisAtts[Nickel->GetName()] = new G4VisAttributes(G4Colour::Black());

    // GEM Frame G10
    G4Material *NemaG10 = new G4Material("NemaG10", density = fExtDensityRatio * 1.700 * g / cm3, ncomponents = 4);
    NemaG10->AddElement(Si, natoms = 1);
    NemaG10->AddElement(O, natoms = 2);
    NemaG10->AddElement(C, natoms = 3);
    NemaG10->AddElement(H, natoms = 3);
    fVisAtts[NemaG10->GetName()] = new G4VisAttributes(G4Colour::Brown());

    // Ar/CO2 Gas
    G4Material *CO2 = new G4Material("CO2", density = fExtDensityRatio * 1.842e-3 * g / cm3, ncomponents = 2);
    CO2->AddElement(C, natoms = 1);
    CO2->AddElement(O, natoms = 2);
    G4Material *ArCO2 = new G4Material("ArCO2", density = fExtDensityRatio * 1.715e-3 * g / cm3, ncomponents = 2);
    ArCO2->AddElement(Ar, fractionmass = 0.7);
    ArCO2->AddMaterial(CO2, fractionmass = 0.3);
    fVisAtts[ArCO2->GetName()] = new G4VisAttributes(G4Colour::Yellow());

    // He Gas
    G4Material *HeGas = new G4Material("HeGas", density = fExtDensityRatio * 0.1786e-3 * g / cm3, ncomponents = 1);
    HeGas->AddElement(He, natoms = 1);
    fVisAtts[HeGas->GetName()] = new G4VisAttributes(G4Colour::Cyan());

    // Scintillator EJ204
    G4Material *EJ204 = new G4Material("EJ204", density = fExtDensityRatio * 1.032 * g / cm3, ncomponents = 2);
    EJ204->AddElement(H, natoms = 521);
    EJ204->AddElement(C, natoms = 474);
    fVisAtts[EJ204->GetName()] = new G4VisAttributes(G4Colour::Green());

    // Rohacell 31 IG
    G4Material *Rohacell = new G4Material("Rohacell", density = fExtDensityRatio * 0.023 * g / cm3, ncomponents = 3);
    Rohacell->AddElement(C, natoms = 5);
    Rohacell->AddElement(H, natoms = 8);
    Rohacell->AddElement(O, natoms = 2);
    fVisAtts[Rohacell->GetName()] = new G4VisAttributes(G4Colour::Grey());

    // Tungsten
    G4Material *Tungsten = new G4Material("Tungsten", density = 19.25 * g / cm3, ncomponents = 1);
    Tungsten->AddElement(W, natoms = 1);
    fVisAtts[Tungsten->GetName()] = new G4VisAttributes(G4Colour::Black());

    // Polyester (3M VM-2000 reflector)
    G4Material *Polyester = new G4Material("Polyester", density = 1.37 * g / cm3, ncomponents = 3);
    Polyester->AddElement(C, natoms = 10);
    Polyester->AddElement(H, natoms = 8);
    Polyester->AddElement(O, natoms = 4);
    fVisAtts[Polyester->GetName()] = new G4VisAttributes(G4VisAttributes::Invisible);

    // Brass
    G4Material *Brass = new G4Material("Brass", density = 8.53 * g / cm3, ncomponents = 2);
    Brass->AddElement(Cu, fractionmass = 0.7);
    Brass->AddElement(Zn, fractionmass = 0.3);
    fVisAtts[Brass->GetName()] = new G4VisAttributes(G4Color::Brown());

    // PbWO4 Crystal
    G4Material *PbWO4 = new G4Material("PbWO4", density = 8.280 * g / cm3, ncomponents = 3);
    PbWO4->AddElement(Pb, natoms = 1);
    PbWO4->AddElement(W, natoms = 1);
    PbWO4->AddElement(O, natoms = 4);
    fVisAtts[PbWO4->GetName()] = new G4VisAttributes(G4Colour::Blue());

    // Silica
    G4Material *SiO2 = new G4Material("SiO2", density = 2.200 * g / cm3, ncomponents = 2);
    SiO2->AddElement(Si, natoms = 1);
    SiO2->AddElement(O, natoms = 2);
    fVisAtts[SiO2->GetName()] = new G4VisAttributes(G4Colour::Green());

    // Lead Glass
    G4Material *PbO = new G4Material("PbO", density = 9.530 * g / cm3, ncomponents = 2);
    PbO->AddElement(Pb, natoms = 1);
    PbO->AddElement(O, natoms = 1);

    G4Material *K2O = new G4Material("K2O", density = 2.320 * g / cm3, ncomponents = 2);
    K2O->AddElement(K, natoms = 2);
    K2O->AddElement(O, natoms = 1);

    G4Material *Na2O = new G4Material("Na2O", density = 2.270 * g / cm3, ncomponents = 2);
    Na2O->AddElement(Na, natoms = 2);
    Na2O->AddElement(O, natoms = 1);

    G4Material *As2O3 = new G4Material("As2O3", density = 3.738 * g / cm3, ncomponents = 2);
    As2O3->AddElement(As, natoms = 2);
    As2O3->AddElement(O, natoms = 3);

    G4Material *PbGlass = new G4Material("PbGlass", density = 3.86 * g / cm3, ncomponents = 5);
    PbGlass->AddMaterial(PbO, fractionmass = 0.5080);
    PbGlass->AddMaterial(SiO2, fractionmass = 0.4170);
    PbGlass->AddMaterial(K2O, fractionmass = 0.0422);
    PbGlass->AddMaterial(Na2O, fractionmass = 0.0278);
    PbGlass->AddMaterial(As2O3, fractionmass = 0.0050);
    fVisAtts[PbGlass->GetName()] = new G4VisAttributes(G4Colour::Blue());

    // Virtual Detector Material
    G4Material *VirtualDetM = new G4Material("VirtualDetM", density = universe_mean_density, ncomponents = 1, kStateGas, 0.1 * kelvin, 1.0e-19 * pascal);
    VirtualDetM->AddElement(H, fractionmass = 1.0);
    fVisAtts[VirtualDetM->GetName()] = new G4VisAttributes(G4Colour::Cyan());

    // Print out material table
    G4cout << *(G4Material::GetMaterialTable()) << G4endl;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
