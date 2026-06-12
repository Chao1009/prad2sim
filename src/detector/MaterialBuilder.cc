//
// MaterialBuilder.cc
// All material definitions, moved verbatim from DetectorConstruction::
// DefineMaterials() and extended with the PRad-II / X17 materials
// (H2Liquid, Aluminized_Kapton, Nomex, Tantalum, PMMA, Viton) ported from
// PRadSim_PRad2 / PRadSim_X17. New materials are appended after the
// original set so the existing configurations see an unchanged table prefix.
//

#include "detector/MaterialBuilder.hh"

#include "G4Element.hh"
#include "G4Isotope.hh"
#include "G4Material.hh"
#include "G4NistManager.hh"

#include "G4Colour.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"

#include "G4ios.hh"

void MaterialBuilder::Build(double targetDensityRatio, double extDensityRatio,
                            std::map<G4String, G4VisAttributes *> &fVisAtts)
{
    G4String symbol;
    G4int z, n;
    G4double a;
    G4double density;
    G4int ncomponents, natoms;
    G4double fractionmass;

    const double fTargetDensityRatio = targetDensityRatio;
    const double fExtDensityRatio = extDensityRatio;

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
    G4Element *Ta = pNM->FindOrBuildElement(z = 73);
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
    fVisAtts[Galaxy->GetName()] = new G4VisAttributes(G4VisAttributes::GetInvisible());

    // Air
    G4Material *Air = new G4Material("Air", density = 1.292 * mg / cm3, ncomponents = 2);
    Air->AddElement(N, fractionmass = 0.7);
    Air->AddElement(O, fractionmass = 0.3);
    fVisAtts[Air->GetName()] = new G4VisAttributes(G4VisAttributes::GetInvisible());

    // Air vacuum of 1.e-6 torr at room temperature, 1 atmosphere = 760 torr
    G4Material *Vacuum = new G4Material("Vacuum", density = 1.0e-6 / 760.0 * 1.292 * mg / cm3, ncomponents = 1, kStateGas, STP_Temperature, 1.0e-6 / 760.0 * atmosphere);
    Vacuum->AddMaterial(Air, fractionmass = 1.0);
    fVisAtts[Vacuum->GetName()] = new G4VisAttributes(G4VisAttributes::GetInvisible());

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
    fVisAtts[Polyester->GetName()] = new G4VisAttributes(G4VisAttributes::GetInvisible());

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

    // ---- PRad-II / X17 additions (ported from PRadSim_PRad2 / PRadSim_X17) ----

    // Liquid Hydrogen (T = 20 K)
    G4Material *H2Liquid = new G4Material("H2Liquid", density = fTargetDensityRatio * 70.80 * mg / cm3, ncomponents = 1, kStateLiquid, 20.0 * kelvin, fTargetDensityRatio * 1.135 * atmosphere);
    H2Liquid->AddElement(H, natoms = 2);
    fVisAtts[H2Liquid->GetName()] = new G4VisAttributes(G4Colour::Cyan());
    fVisAtts[H2Liquid->GetName()]->SetForceSolid(true);

    // Aluminized Kapton (5 um Al + 20 um Kapton, averaged)
    G4Material *Aluminized_Kapton = new G4Material("Aluminized_Kapton", density = fExtDensityRatio * (1.42 * 4. + 2.700) / 5. * g / cm3, ncomponents = 5);
    Aluminized_Kapton->AddElement(H, fractionmass = 0.0273 * (1. - 2.700 / (1.42 * 4. + 2.700)));
    Aluminized_Kapton->AddElement(C, fractionmass = 0.7213 * (1. - 2.700 / (1.42 * 4. + 2.700)));
    Aluminized_Kapton->AddElement(N, fractionmass = 0.0765 * (1. - 2.700 / (1.42 * 4. + 2.700)));
    Aluminized_Kapton->AddElement(O, fractionmass = 0.1749 * (1. - 2.700 / (1.42 * 4. + 2.700)));
    Aluminized_Kapton->AddElement(Al, fractionmass = 2.700 / (1.42 * 4. + 2.700));
    fVisAtts[Aluminized_Kapton->GetName()] = new G4VisAttributes(G4Colour::Blue());

    // Honeycomb Nomex (3 lbs/ft^3)
    G4Material *Nomex = new G4Material("Nomex", density = fExtDensityRatio * 0.04806 * g / cm3, ncomponents = 4);
    Nomex->AddElement(N, natoms = 2);
    Nomex->AddElement(H, natoms = 10);
    Nomex->AddElement(C, natoms = 14);
    Nomex->AddElement(O, natoms = 2);
    fVisAtts[Nomex->GetName()] = new G4VisAttributes(G4Colour::Red());

    // Tantalum (X17 foil target option)
    G4Material *Tantalum = new G4Material("Tantalum", density = fExtDensityRatio * 16.65 * g / cm3, ncomponents = 1);
    Tantalum->AddElement(Ta, natoms = 1);
    fVisAtts[Tantalum->GetName()] = new G4VisAttributes(G4Colour::Red());

    // Light Guide PMMA
    G4Material *PMMA = new G4Material("PMMA", density = 1.19 * g / cm3, ncomponents = 3);
    PMMA->AddElement(C, natoms = 5);
    PMMA->AddElement(O, natoms = 2);
    PMMA->AddElement(H, natoms = 8);
    fVisAtts[PMMA->GetName()] = new G4VisAttributes(G4Colour::Red());

    // Viton (HFP, VF2)
    G4Material *Viton = new G4Material("Viton", density = 2.5 * g / cm3, ncomponents = 3);
    Viton->AddElement(C, natoms = 5);
    Viton->AddElement(H, natoms = 2);
    Viton->AddElement(F, natoms = 8);
    fVisAtts[Viton->GetName()] = new G4VisAttributes(G4Colour::Red());

    // Print out material table
    G4cout << *(G4Material::GetMaterialTable()) << G4endl;
}
