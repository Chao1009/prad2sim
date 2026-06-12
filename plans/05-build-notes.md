# Build & run notes (verified 2026-06-12)

prad2sim now **builds and runs against Geant4 11.4.0** on this machine. This
records the working recipe and the non-obvious gotchas found getting there.

## Installed toolchain (this machine)
- **Geant4 11.4.0** — built from source into `~/Apps/geant4` (batch mode: no
  Qt/OpenGL; datasets included). Build script: `~/Apps/build_geant4.sh`,
  log: `~/Apps/geant4_build.log`. Env: `source ~/Apps/geant4/bin/geant4.sh`.
- **ROOT 6.28/02** — `~/Apps/root` (`ROOTConfig.cmake` in `~/Apps/root/cmake`).
- **prad2 toolkit** — `~/Apps/prad2` (`sim2replay`, `libprad2{ana,dec,det}.a`).

## Gotchas
- **Do NOT `source ~/Apps/root/bin/thisroot.sh` inside a non-interactive shell** —
  this `thisroot.sh` self-locates via `$PWD`/`BASH_SOURCE` and ends up setting
  `ROOTSYS` to the wrong directory. The login profile already sets
  `ROOTSYS=/home/cpeng/Apps/root` correctly; just keep it and add
  `$ROOTSYS/lib` to `LD_LIBRARY_PATH`. Re-assert `ROOTSYS` after sourcing
  `geant4.sh` to be safe.
- prad2sim's `CMakeLists` does `find_package(ROOT ... )` which needs ROOT's
  config dir — pass `-DROOT_DIR=$HOME/Apps/root/cmake` (this ROOT keeps it in
  `cmake/`, not `etc/cmake`).
- The Geant4 build here has **no UI/vis**, so configure with
  `-DWITH_GEANT4_UIVIS=OFF`; and the legacy `digitization/` stage needs EVIO +
  PRadAnalyzer (`$PRAD_PATH`, absent here) so build with `-DBUILD_DIGITIZATION=OFF`.

## Configure + build
```bash
cd /home/cpeng/Projects/prad/prad2sim
export ROOTSYS=$HOME/Apps/root
source ~/Apps/geant4/bin/geant4.sh
export ROOTSYS=$HOME/Apps/root                 # re-assert (geant4.sh / habits)
export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH
rm -rf build && mkdir build && cd build
cmake -DWITH_GEANT4_UIVIS=OFF -DBUILD_DIGITIZATION=OFF \
      -DROOT_DIR=$HOME/Apps/root/cmake \
      -DCMAKE_PREFIX_PATH="$HOME/Apps/geant4;$HOME/Apps/root" ..
make -j$(nproc)
```

## Run (from the source root — needs ./database, ./config, ./output)
```bash
cd /home/cpeng/Projects/prad/prad2sim
export ROOTSYS=$HOME/Apps/root; export LD_LIBRARY_PATH=$ROOTSYS/lib:$LD_LIBRARY_PATH
source ~/Apps/geant4/bin/geant4.sh; export ROOTSYS=$HOME/Apps/root
./build/prad2sim -c config/prad.json run.mac      # run.mac = beamOn 500
# → output/simrun_<N>.root, tree "T"
```
Verified: 500 / 200-event runs exit 0 and write the `T` tree with the expected
`GEM.*` (incl. `DID`, `Xout/Yout/Zout`) and `HC.*` branches.

## The Geant4 10→11.4 source port (what it took)
Three small, faithful changes (X17 fork was the reference) + the CMake guards:
1. `G4VisAttributes::Invisible` → `G4VisAttributes::GetInvisible()`
   (`DetectorConstruction.cc`, 4×) — static member removed in 11.x.
2. `G4String::hash()` removed → `std::hash<std::string>()(name)`
   (`CheckScatteringSD.cc`, `StepRecordSD.cc`, `StandardDetectorSD.cc`).
3. `CalorimeterSD::kInterpolType` `static const` → `static constexpr`
   (link error: ODR-used static const had no out-of-class definition).

`G4VSteppingVerbose::SetInstance`, `G4RunManager`, and the `SteppingVerbose`
signatures were already 11.4-compatible (identical to the X17 fork).

## Not yet done (needs geometry/config + an upstream tweak)
- A HyCal-front **`VD` plane** (enabled) writing `VD.X/Y/Z/P` — the `sim2replay`
  HyCal-cluster source. Today's `VD` is a target-side scattering check.
- GEM `DID` values 0..3 for the PRad-II 4-GEM layout (branch + copy-number sum
  already work for the present 1–2 GEM configs).
- Sim HyCal transverse frame aligned to `hycal_map.json` so `cl_center` resolves.
- `sim2replay` `beamE` is hardcoded 3500 MeV; prad/drad (1.1 GeV) events get cut
  — make it configurable upstream (or run a high-energy config) for a non-empty
  real-data validation. The contract itself is already proven in
  [`../tests/sim2replay`](../tests/sim2replay/).
