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
// CosmicsGenerator.cc
// Developer : Chao Gu
// History:
//

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

#include "CosmicsGenerator.hh"

#include "GlobalVars.hh"
#include "RootTree.hh"

#include "TMath.h"
#include "TFoam.h"
#include "TFoamIntegrand.h"
#include "TRandom2.h"
#include "TTree.h"

#include "G4Event.hh"
#include "G4VPrimaryGenerator.hh"

#include "G4ios.hh"
#include "G4ParticleDefinition.hh"
#include "G4ParticleTable.hh"
#include "G4PhysicalConstants.hh"
#include "G4PrimaryParticle.hh"
#include "G4PrimaryVertex.hh"
#include "G4SystemOfUnits.hh"
#include "G4ThreeVector.hh"
#include "G4UnitsTable.hh"
#include "Randomize.hh"

#include <cmath>

static double mmu = 105.6583745 * MeV;

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CosmicsGenerator::CosmicsGenerator() : G4VPrimaryGenerator(), fRegistered(false)
{
    fEMin = 0.5 * GeV;
    fEMax = 100.0 * GeV;
    fZenithMin = 0.0 * deg;
    fZenithMax = 120.0 * deg;

    fPseRan = std::make_unique<TRandom2>();
    fFoamI = std::make_unique<CosmicsIntegrand>(this, 4.28, 854.0, 174.0, 3.09);

    fETGenerator = std::make_unique<TFoam>("ET Generator");
    fETGenerator->SetkDim(2);
    fETGenerator->SetnCells(20000); // Set number of cells
    fETGenerator->SetnSampl(1000); // Set number of samples
    fETGenerator->SetOptRej(1); // Unweighted events in MC generation
    fETGenerator->SetRho(fFoamI.get()); // Set distribution function
    fETGenerator->SetPseRan(fPseRan.get()); // Set random number generator
    fETGenerator->SetChat(0); // Set "chat level" in the standard output
    fETGenerator->Initialize();

    fN = 0;

    for (int i = 0; i < kMaxN; i++) {
        fPID[i] = -9999;
        fX[i] = 1e+38;
        fY[i] = 1e+38;
        fZ[i] = 1e+38;
        fE[i] = 1e+38;
        fMomentum[i] = 1e+38;
        fTheta[i] = 1e+38;
        fPhi[i] = 1e+38;
    }
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CosmicsGenerator::~CosmicsGenerator()
{
    //
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CosmicsGenerator::GeneratePrimaryVertex(G4Event *anEvent)
{
    if (!fRegistered) {
        Register(gRootTree->GetTree());
        fRegistered = true;
    }

    Clear();

    G4ParticleTable *particleTable = G4ParticleTable::GetParticleTable();

    double x, y, z, theta_l, phi_l;
    double p_l, e_l;
    double rvect[2];

    double tempr, tempt, tempp;

    tempr = 2.8 * m;
    tempt = acos(G4UniformRand());
    tempp = twopi * G4UniformRand();

    x = tempr * sin(tempt) * sin(tempp);
    y = -1.4 * m + tempr * cos(tempt);
    z = 2.85 * m + tempr * sin(tempt) * cos(tempp);

    fETGenerator->MakeEvent();
    fETGenerator->GetMCvect(rvect);
    double E = fEMin + (fEMax - fEMin) * rvect[0];
    double zenith = fZenithMin + (fZenithMax - fZenithMin) * rvect[1];

    theta_l = 180.0 * deg - zenith;
    phi_l = twopi * G4UniformRand();

    e_l = E;
    p_l = sqrt(E * E - mmu * mmu);

    G4PrimaryVertex *vertexL = new G4PrimaryVertex(x, y, z, 0);
    G4PrimaryParticle *particleL = new G4PrimaryParticle(particleTable->FindParticle("mu-"));
    double kx_l = sin(theta_l) * sin(phi_l);
    double ky_l = cos(theta_l);
    double kz_l = sin(theta_l) * cos(phi_l);
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
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CosmicsGenerator::Register(TTree *tree)
{
    tree->Branch("GUN.N", &fN, "GUN.N/I");
    tree->Branch("GUN.PID", fPID, "GUN.PID[GUN.N]/I");
    tree->Branch("GUN.X", fX, "GUN.X[GUN.N]/D");
    tree->Branch("GUN.Y", fY, "GUN.Y[GUN.N]/D");
    tree->Branch("GUN.Z", fZ, "GUN.Z[GUN.N]/D");
    tree->Branch("GUN.E", fE, "GUN.E[GUN.N]/D");
    tree->Branch("GUN.P", fMomentum, "GUN.P[GUN.N]/D");
    tree->Branch("GUN.Theta", fTheta, "GUN.Theta[GUN.N]/D");
    tree->Branch("GUN.Phi", fPhi, "GUN.Phi[GUN.N]/D");
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CosmicsGenerator::Print() const
{
    G4int prec = G4cout.precision(3);

    for (int i = 0; i < fN; i++) {
        G4cout << std::setw(10) << fPID[i] << " ";
        G4cout << std::setw(5) << G4BestUnit(fX[i], "Length") << " " << std::setw(5) << G4BestUnit(fY[i], "Length") << " " << std::setw(5) << G4BestUnit(fZ[i], "Length") << " ";
        G4cout << std::setw(5) << G4BestUnit(fE[i], "Energy") << " " << std::setw(8) << G4BestUnit(fMomentum[i], "Energy") << " ";
        G4cout << std::setw(5) << fTheta[i] / pi * 180 << " deg ";
        G4cout << G4endl;
    }

    G4cout.precision(prec);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void CosmicsGenerator::Clear()
{
    for (int i = 0; i < fN; i++) {
        fPID[i] = -9999;
        fX[i] = 1e+38;
        fY[i] = 1e+38;
        fZ[i] = 1e+38;
        fE[i] = 1e+38;
        fMomentum[i] = 1e+38;
        fTheta[i] = 1e+38;
        fPhi[i] = 1e+38;
    }

    fN = 0;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

CosmicsIntegrand::CosmicsIntegrand(CosmicsGenerator *gen, double e0, double eps, double rd, double nn) :  E0(e0), epsilon(eps), Rd(rd), n(nn)
{
    fEMin = gen->fEMin;
    fEMax = gen->fEMax;
    fZenithMin = gen->fZenithMin;
    fZenithMax = gen->fZenithMax;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

double CosmicsIntegrand::Density(int, double *arg)
{
    double E = (fEMin + (fEMax - fEMin) * arg[0]) / GeV;
    double Theta = fZenithMin + (fZenithMax - fZenithMin) * arg[1];

    double cosTheta = cos(Theta);

    return TMath::Power(E0 + E, -n) / (1 + E / epsilon) * TMath::Power(sqrt(Rd * Rd * cosTheta * cosTheta + 2 * Rd + 1) - Rd * cosTheta, -(n - 1));
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......
