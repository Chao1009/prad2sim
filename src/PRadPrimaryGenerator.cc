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
// PRadPrimaryGenerator.cc
// Developer : Chao Gu, Weizhi Xiong
// History:
//   Apr 2017, W. Xiong, Add target thickness profile.
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "PRadPrimaryGenerator.hh"

#include "ConfigParser.h"
#include "GlobalVars.hh"
#include "RootTree.hh"

#include "TROOT.h"
#include "TError.h"
#include "TObject.h"
#include "TMath.h"
#include "TFile.h"
#include "TFoam.h"
#include "TFoamIntegrand.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TRandom2.h"
#include "TTree.h"
#include "Math/InterpolationTypes.h"
#include "Math/Interpolator.h"

#include "G4Event.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
#include "G4Material.hh"
#include "G4PhysicalVolumeStore.hh"
#include "G4Tubs.hh"
#include "G4VPhysicalVolume.hh"
#include "G4VPrimaryGenerator.hh"

#include "G4ios.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"
#include "G4PhysicalConstants.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4SystemOfUnits.hh"
#include "G4String.hh"
#include "G4ThreeVector.hh"
#include "G4UnitsTable.hh"
#include "Randomize.hh"

#include <cmath>
#include <fstream>
#include <vector>

static double me = 0.510998928 * MeV;
static double mmu = 105.6583745 * MeV;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PRadPrimaryGenerator::PRadPrimaryGenerator(G4String type, G4bool rec, G4String par) : PrimaryGenerator(), fEventType(type), fRecoilOn(rec), fRecoilParticle(par)
{
    //
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PRadPrimaryGenerator::PRadPrimaryGenerator(G4String type, G4bool rec, G4String par, G4String path, G4String pile_up_profile, G4String target_profile): PrimaryGenerator(), fEventType(type), fRecoilOn(rec), fRecoilParticle(par), fPileUpProfile(nullptr)
{
    if (!pile_up_profile.empty()) {
        fPileUpProfile = new TFile(pile_up_profile.c_str(), "READ");
        fClusterNumber = dynamic_cast<TH1D *>(fPileUpProfile->Get("cluster_number"));
        fClusterEvsTheta = dynamic_cast<TH2D *>(fPileUpProfile->Get("signal_cluster_E_theta"));
    }

    if (!target_profile.empty())
        LoadTargetProfile(target_profile);

    if (path.empty()) {
        if (fEventType == "elastic")
            path = "epelastic.dat";
        else if (fEventType == "moller")
            path = "moller.dat";
    }

    // only open file, do not read the whole file into memory
    if (!fParser.OpenFile(path)) {
        G4cout << "ERROR: failed to read event file " << "\"" << path << "\"" << G4endl;
        exit(1);
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PRadPrimaryGenerator::~PRadPrimaryGenerator()
{
    if (fPileUpProfile)
        fPileUpProfile->Close();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PRadPrimaryGenerator::GeneratePrimaryVertex(G4Event *anEvent)
{
    if (!fRegistered) {
        Register(gRootTree->GetTree());
        fRegistered = true;
    }

    Clear();

    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();

    if (!fTargetInfo) {
        G4VPhysicalVolume *physiTargetCon = G4PhysicalVolumeStore::GetInstance()->GetVolume("Target Container");
        G4LogicalVolume *logicTarget = G4LogicalVolumeStore::GetInstance()->GetVolume("TargetLV");

        if (physiTargetCon) fTargetCenter = physiTargetCon->GetObjectTranslation().z();

        G4Tubs *solidTarget = nullptr;

        if (logicTarget)
            solidTarget = dynamic_cast<G4Tubs *>(logicTarget->GetSolid());

        if (solidTarget)
            fTargetHalfL = solidTarget->GetZHalfLength();
        else
            G4cout << "WARNING: target volume not found" << G4endl;

        fTargetInfo = true;
    }

    if (fEventType == "inelastic") {
        int pid[4];
        double p[4][3];

        while (fParser.ParseLine()) {
            if (!fParser.CheckElements(16))  continue;
            else {
                fParser >> pid[0] >> p[0][0] >> p[0][1] >> p[0][2] >> pid[1] >> p[1][0] >> p[1][1] >> p[1][2] >> pid[2] >> p[2][0] >> p[2][1] >> p[2][2] >> pid[3] >> p[3][0] >> p[3][1] >> p[3][2];
                break;
            }
        }

        double x = G4RandGauss::shoot(0, 0.08) * mm;
        double y = G4RandGauss::shoot(0, 0.08) * mm;
        double z = GenerateZ();

        for (int i = 0; i < 4; i++) {
            if (p[i][2] <= 0.) continue;

            G4PrimaryParticle *particleL = new G4PrimaryParticle(pid[i], p[i][0], p[i][1], p[i][2]);
            G4PrimaryVertex *vertexL = new G4PrimaryVertex(x, y, z, 0);
            vertexL->SetPrimary(particleL);
            anEvent->AddPrimaryVertex(vertexL);

            fPID[fN] = pid[i];
            fX[fN] = x;
            fY[fN] = y;
            fZ[fN] = z;
            fE[fN] = particleL->GetTotalEnergy();
            fMomentum[fN] = particleL->GetTotalMomentum();
            fTheta[fN] = particleL->GetMomentum().theta();
            fPhi[fN] = particleL->GetMomentum().phi();
            fN++;
        }

        return;
    }

    double e_l = 0, theta_l = 0, phi_l = 0;
    double e_h = 0, theta_h = 0, phi_h = 0;
    double e_p = 0, theta_p = 0, phi_p = 0;

    while (fParser.ParseLine()) {
        if (!fParser.CheckElements(9))
            continue;
        else {
            fParser >> e_l >> theta_l >> phi_l >> e_h >> theta_h >> phi_h >> e_p >> theta_p >> phi_p;
            break;
        }
    }

    double x = G4RandGauss::shoot(0, 0.08) * mm;
    double y = G4RandGauss::shoot(0, 0.08) * mm;
    double z = GenerateZ();

    G4PrimaryVertex *vertexL = new G4PrimaryVertex(x, y, z, 0);
    G4PrimaryParticle *particleL = new G4PrimaryParticle(particleTable->FindParticle("e-"));
    double m_l = particleL->GetParticleDefinition()->GetPDGMass();
    double p_l = sqrt(e_l * e_l - m_l * m_l);
    double kx_l = sin(theta_l) * cos(phi_l);
    double ky_l = sin(theta_l) * sin(phi_l);
    double kz_l = cos(theta_l);
    particleL->SetMomentumDirection(G4ThreeVector(kx_l, ky_l, kz_l));
    particleL->SetTotalEnergy(e_l);
    vertexL->SetPrimary(particleL);

    anEvent->AddPrimaryVertex(vertexL);

    fPID[fN] = particleL->GetPDGcode();
    fX[fN] = x;
    fY[fN] = y;
    fZ[fN] = z;
    fE[fN] = e_l;
    fMomentum[fN] = p_l;
    fTheta[fN] = theta_l;
    fPhi[fN] = phi_l;
    fN++;

    if (fRecoilOn || fEventType == "moller") {
        G4PrimaryVertex *vertexH = new G4PrimaryVertex(x, y, z, 0);
        G4PrimaryParticle *particleH = nullptr;

        if (fEventType == "moller")
            particleH = new G4PrimaryParticle(particleTable->FindParticle("e-"));
        else
            particleH = new G4PrimaryParticle(particleTable->FindParticle(fRecoilParticle));

        double m_h = particleH->GetParticleDefinition()->GetPDGMass();
        double p_h = sqrt(e_h * e_h - m_h * m_h);
        double kx_h = sin(theta_h) * cos(phi_h);
        double ky_h = sin(theta_h) * sin(phi_h);
        double kz_h = cos(theta_h);
        particleH->SetMomentumDirection(G4ThreeVector(kx_h, ky_h, kz_h));
        particleH->SetTotalEnergy(e_h);
        vertexH->SetPrimary(particleH);

        anEvent->AddPrimaryVertex(vertexH);

        fPID[fN] = particleH->GetPDGcode();
        fX[fN] = x;
        fY[fN] = y;
        fZ[fN] = z;
        fE[fN] = e_h;
        fMomentum[fN] = p_h;
        fTheta[fN] = theta_h;
        fPhi[fN] = phi_h;
        fN++;
    }

    if (e_p > 0) {
        G4PrimaryVertex *vertexP = new G4PrimaryVertex(x, y, z, 0);
        G4PrimaryParticle *particleP = new G4PrimaryParticle(particleTable->FindParticle("gamma"));
        double kx_p = sin(theta_p) * cos(phi_p);
        double ky_p = sin(theta_p) * sin(phi_p);
        double kz_p = cos(theta_p);
        particleP->SetMomentumDirection(G4ThreeVector(kx_p, ky_p, kz_p));
        particleP->SetTotalEnergy(e_p);
        vertexP->SetPrimary(particleP);

        anEvent->AddPrimaryVertex(vertexP);

        fPID[fN] = particleP->GetPDGcode();
        fX[fN] = x;
        fY[fN] = y;
        fZ[fN] = z;
        fE[fN] = e_p;
        fMomentum[fN] = e_p;
        fTheta[fN] = theta_p;
        fPhi[fN] = phi_p;
        fN++;
    }

    if (fPileUpProfile && G4UniformRand() < 0.0202) {
        int n_pile_up = int(fClusterNumber->GetRandom());

        if (fN + n_pile_up > kMaxN)
            n_pile_up = kMaxN - fN;

        for (int i = 0; i < n_pile_up; i++) {
            double e_add, theta_add;
            fClusterEvsTheta->GetRandom2(theta_add, e_add);

            theta_add = theta_add / 180.0 * pi;
            e_add = e_add * MeV;
            double phi_add = twopi * (0.5 - G4UniformRand());

            G4PrimaryVertex *vertexA = new G4PrimaryVertex(x, y, z, 0);
            G4PrimaryParticle *particleA = new G4PrimaryParticle(particleTable->FindParticle("gamma"));
            double kx_add = sin(theta_add) * cos(phi_add);
            double ky_add = sin(theta_add) * sin(phi_add);
            double kz_add = cos(theta_add);
            particleA->SetMomentumDirection(G4ThreeVector(kx_add, ky_add, kz_add));
            particleA->SetTotalEnergy(e_add);
            vertexA->SetPrimary(particleA);

            anEvent->AddPrimaryVertex(vertexA);

            fPID[fN] = particleA->GetPDGcode();
            fX[fN] = x;
            fY[fN] = y;
            fZ[fN] = z;
            fE[fN] = e_add;
            fMomentum[fN] = e_add;
            fTheta[fN] = theta_add;
            fPhi[fN] = phi_add;
            fN++;
        }
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PRadPrimaryGenerator::LoadTargetProfile(const std::string &path)
{
    if (path.empty()) return;

    ConfigParser c_parser;
    c_parser.ReadFile(path);

    std::vector<double> z, density;
    z.clear();
    density.clear();
    double tempz, tempd;

    while (c_parser.ParseLine()) {
        if (!c_parser.CheckElements(2))
            continue;

        c_parser >> tempz >> tempd;

        if (!z.empty() && tempz * 10.0 <= z.back()) continue;

        // convert z to mm, and convert density to H_atom/cm^3
        z.push_back(tempz * 10.0);
        density.push_back(tempd * 2.0);
    }

    fTargetProfile = std::make_unique<ROOT::Math::Interpolator>(z, density, ROOT::Math::Interpolation::kLINEAR);
    fZMin = z[0];
    fZMax = z.back();

    fPseRan = std::make_unique<TRandom2>(0);
    fFoamI = std::make_unique<TargetProfileIntegrand>(this);

    fZGenerator = std::make_unique<TFoam>("Z Generator");
    fZGenerator->SetkDim(1);
    fZGenerator->SetnCells(10000); // Set number of cells
    fZGenerator->SetnSampl(500); // Set number of samples
    fZGenerator->SetOptRej(1); // Unweighted events in MC generation
    fZGenerator->SetRho(fFoamI.get()); // Set distribution function
    fZGenerator->SetPseRan(fPseRan.get()); // Set random number generator
    fZGenerator->SetChat(0); // Set "chat level" in the standard output
    fZGenerator->Initialize();
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

double PRadPrimaryGenerator::GenerateZ()
{
    if (fZGenerator) {
        double rvect[1];
        fZGenerator->MakeEvent();
        fZGenerator->GetMCvect(rvect);
        return fTargetCenter + fZMin + (fZMax - fZMin) * rvect[0];
    } else
        return fTargetCenter + fTargetHalfL * 2 * (0.5 - G4UniformRand());
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

TargetProfileIntegrand::TargetProfileIntegrand(PRadPrimaryGenerator *gen)
{
    fTargetProfile = gen->fTargetProfile.get();
    fZMin = gen->fZMin;
    fZMax = gen->fZMax;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

double TargetProfileIntegrand::Density(int, double *arg)
{
    double z = fZMin + (fZMax - fZMin) * arg[0];

    return fTargetProfile->Eval(z);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
