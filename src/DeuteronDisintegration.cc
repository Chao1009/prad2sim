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
// DeuteronDisintegration.cc
// Developer : Chao Gu
// History:
//   May 2017, C. Gu, Add Deuteron disintegration.
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "DeuteronDisintegration.hh"

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

static double me = 0.510998928 * MeV;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DeuteronDisintegration::DeuteronDisintegration(G4double e, G4double eflo, G4double efhi, G4double thlo, G4double thhi) : PrimaryGenerator(), fEBeam(e), fEnpLo(eflo), fEnpHi(efhi), fReactThetaLo(thlo), fReactThetaHi(thhi)
{
    //
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

DeuteronDisintegration::~DeuteronDisintegration()
{
    //
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void DeuteronDisintegration::GeneratePrimaryVertex(G4Event *anEvent)
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

        if (logicTarget) solidTarget = dynamic_cast<G4Tubs *>(logicTarget->GetSolid());

        if (solidTarget)
            fTargetHalfL = solidTarget->GetZHalfLength();
        else
            G4cout << "WARNING: target volume not found" << G4endl;

        fTargetInfo = true;
    }

    double Md = particleTable->FindParticle("deuteron")->GetPDGMass();
    double Mp = particleTable->FindParticle("proton")->GetPDGMass();
    double Mn = particleTable->FindParticle("neutron")->GetPDGMass();

    double x, y, z, theta_l, phi_l;

    x = G4RandGauss::shoot(0, 0.08) * mm;
    y = G4RandGauss::shoot(0, 0.08) * mm;
    z = fTargetCenter + fTargetHalfL * 2 * (0.5 - G4UniformRand());
    theta_l = fReactThetaLo + (fReactThetaHi - fReactThetaLo) * G4UniformRand();
    phi_l = twopi * G4UniformRand();

    double E = fEBeam;
    double P = sqrt(E * E - me * me);

    double enp = fEnpLo + (fEnpHi - fEnpLo) * G4UniformRand();

    double w = enp + Mn + Mp;
    double w2 = w * w;
    double sinth22 = sin(theta_l / 2.0) * sin(theta_l / 2.0);
    double e_l = (2 * Md * E + Md * Md - w2) / (2 * Md + 4 * E * sinth22);
    double p_l = sqrt(e_l * e_l - me * me);

    G4PrimaryVertex *vertexL = new G4PrimaryVertex(x, y, z, 0);
    G4PrimaryParticle *particleL = new G4PrimaryParticle(particleTable->FindParticle("e-"));
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

    G4ThreeVector vi_l(0, 0, P);
    G4ThreeVector vf_l(p_l * sin(theta_l), 0, p_l * cos(theta_l));
    G4ThreeVector vq = vi_l - vf_l;
    double vq2 = vq.mag2();

    double Mp2 = Mp * Mp;
    double Mn2 = Mn * Mn;
    double ecm_p = (w2 + Mp2 - Mn2) / (2 * w);
    double pcm_p = sqrt(ecm_p * ecm_p - Mp2);

    // Boost variables
    double ee = sqrt(w2 + vq2);
    double pp = sqrt(vq2);
    double gamma = ee / w;
    double v = pp / ee;

    double theta_cm = acos(-1.0 + 2.0 * G4UniformRand());
    //double theta_cm = pi * G4UniformRand();

    G4ThreeVector vf_p(pcm_p * sin(theta_cm), 0, gamma * (pcm_p * cos(theta_cm) + v * ecm_p)); // NOTE: not lab system
    double dtheta = vf_p.theta(); // angle between proton momentum and virtual photon momentum

    if (vq.theta() - dtheta > 0) {
        vf_p.setTheta(vq.theta() - dtheta);
        vf_p.setPhi(vq.phi());
    } else {
        vf_p.setTheta(dtheta - vq.theta());
        vf_p.setPhi(vq.phi() + pi);
    }

    double phi = twopi * G4UniformRand(); // angle between scattering plane and interaction plane

    vf_p.rotate(vq, phi);
    vf_p.rotateZ(phi_l);

    double p_p = vf_p.mag();
    double theta_p = vf_p.theta();
    double phi_p = vf_p.phi();
    double e_p = sqrt(Mp * Mp + vf_p.mag2());

    G4PrimaryVertex *vertexP = new G4PrimaryVertex(x, y, z, 0);
    G4PrimaryParticle *particleP = new G4PrimaryParticle(particleTable->FindParticle("proton"));
    particleP->SetMomentumDirection(vf_p.unit());
    particleP->SetTotalEnergy(e_p);
    vertexP->SetPrimary(particleP);

    anEvent->AddPrimaryVertex(vertexP);

    fPID[fN] = particleP->GetPDGcode();
    fX[fN] = x;
    fY[fN] = y;
    fZ[fN] = z;
    fE[fN] = e_p;
    fMomentum[fN] = p_p;
    fTheta[fN] = theta_p;
    fPhi[fN] = phi_p;
    fN++;

    /* Neutron
    G4ThreeVector vf_n = vq - vf_p;

    double p_n = vf_n.mag();
    double theta_n = vf_n.theta();
    double phi_n = vf_n.phi();
    double e_n = sqrt(Mn * Mn + vf_n.mag2());

    G4PrimaryVertex *vertexN = new G4PrimaryVertex(x, y, z, 0);
    G4PrimaryParticle *particleN = new G4PrimaryParticle(particleTable->FindParticle("neutron"));
    particleN->SetMomentumDirection(vf_n.unit());
    particleN->SetTotalEnergy(e_n);
    vertexN->SetPrimary(particleN);

    anEvent->AddPrimaryVertex(vertexN);

    fPID[fN] = particleN->GetPDGcode();
    fX[fN] = x;
    fY[fN] = y;
    fZ[fN] = z;
    fE[fN] = e_n;
    fMomentum[fN] = p_n;
    fTheta[fN] = theta_n;
    fPhi[fN] = phi_n;
    fN++;
    */
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
