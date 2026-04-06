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
// DRadPrimaryGenerator.cc
// Developer : Chao Gu
// History:
//   Mar 2017, C. Gu, Add for DRad configuration.
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "DRadPrimaryGenerator.hh"

#include "GlobalVars.hh"
#include "RootTree.hh"

#include "TTree.h"

#include "G4Event.hh"
#include "G4LogicalVolume.hh"
#include "G4LogicalVolumeStore.hh"
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
#include "Randomize.hh"

#include <cmath>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DRadPrimaryGenerator::DRadPrimaryGenerator(G4String type, G4bool rec, G4String par, G4String path) : PRadPrimaryGenerator(type, rec, par)
{
    if (path.empty()) {
        if (fEventType == "elastic")
            path = "edelastic.dat";
        else if (fEventType == "moller")
            path = "moller.dat";
        else if (fEventType == "disintegration")
            path = "edepn.dat";
    }

    // OpenFile doesn't read the whole file into memory
    if (!fParser.OpenFile(path)) {
        G4cout << "ERROR: failed to read event file " << "\"" << path << "\"" << G4endl;
        exit(1);
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DRadPrimaryGenerator::~DRadPrimaryGenerator()
{
    //
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DRadPrimaryGenerator::GeneratePrimaryVertex(G4Event *anEvent)
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

    if (fRecoilOn || fEventType == "moller" || fEventType == "disintegration") {
        G4PrimaryVertex *vertexH = new G4PrimaryVertex(x, y, z, 0);
        G4PrimaryParticle *particleH = nullptr;

        if (fEventType == "moller")
            particleH = new G4PrimaryParticle(particleTable->FindParticle("e-"));
        else if (fEventType == "disintegration")
            particleH = new G4PrimaryParticle(particleTable->FindParticle("proton"));
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
        G4PrimaryParticle *particleP = nullptr;

        if (fEventType == "disintegration")
            //particleP = new G4PrimaryParticle(particleTable->FindParticle("neutron"));
            return;
        else
            particleP = new G4PrimaryParticle(particleTable->FindParticle("gamma"));

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
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

double DRadPrimaryGenerator::GenerateZ()
{
    return fTargetCenter + fTargetHalfL * 2 * (0.5 - G4UniformRand());
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
