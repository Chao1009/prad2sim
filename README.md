# prad2sim

Geant4 simulation package for the Jefferson Lab PRad / DRad / PRad-II / X17
experiments.

`prad2sim` propagates events from a beam-target interaction through the full
experimental setup (target, GEM trackers, scintillators, HyCal calorimeter,
and the recoil detector for DRad), records truth hits to a ROOT tree, and
interfaces with the PRad-II analysis chain: the `prad2` toolkit's
`prad2ana_sim2replay` converts the truth tree into the same `recon` tree
format as replayed data (see `tests/sim2replay/`), so simulated events are
consumed by `prad2evviewer` and the analysis tools identically to real data.
A legacy digitization stage producing EVIO files for
[PRadAnalyzer](https://github.com/JeffersonLab/PRadAnalyzer) is retained as
an optional component.

Geometry is organized as composable `DetectorModule` subsystems
(`src/detector/`); each experiment configuration assembles its module list
from JSON parameters, so adding a configuration does not mean editing a
monolithic geometry function. Requires Geant4 ≥ 11 (developed against 11.4).

## Dependencies

- [Geant4](http://geant4.cern.ch/) (built with UI/Vis drivers for interactive mode)
- [ROOT](https://root.cern.ch/) with `MathMore` and `Foam` components
- [EVIO](https://coda.jlab.org/drupal/content/event-io-evio) — digitization stage only
- [PRadAnalyzer](https://github.com/JeffersonLab/PRadAnalyzer) — digitization stage only

The digitization stage looks for EVIO via `$ET_INC`/`$ET_LIB` or `$EVIODIR`,
and PRadAnalyzer via `$PRAD_PATH` or `$PRADANADIR`.

## Building

```bash
mkdir build && cd build
cmake ..
make -j
```

This produces the `prad2sim` executable in the build directory along with the
macro and config files needed to run it.

CMake options:

| Option | Default | Description |
|---|---|---|
| `WITH_GEANT4_UIVIS` | `ON` | Require Geant4 UI/Vis drivers (set `OFF` for batch-only Geant4 builds) |
| `BUILD_DIGITIZATION` | `ON` | Build the legacy `PRadDig`/`PRadRec` EVIO stage (needs EVIO + PRadAnalyzer) |
| `PRAD2SIM_USE_CADMESH` | `ON` | CAD (STL) geometry import for prad2/x17 via the vendored CADMesh header |

The prad2/x17 CAD models (~50 MB of STL files) are not version-controlled —
see `database/CADmodel/README.md` for where to copy them from. Missing models
are reported and skipped at startup.

## Running

```bash
./prad2sim [options] [macro]
```

Options:

| Flag | Default | Description |
|---|---|---|
| `-c, --conf=FILE` | `config/prad.json` | JSON configuration file |
| `-p, --physics=LIST` | `FTFP_BERT` | Geant4 reference physics list (suffix `_LOCAL` to use the modified EM list, prefix `EM` for pure EM, optional `_EXTRA`) |
| `-s, --seed=N` | `random` | Random seed (`random` uses the system clock) |
| `-t, --nthreads=N` | `1` | Number of worker threads (Geant4 MT build only) |
| `-h, --help` | | Print usage |

If a macro file is given, the simulation runs in batch mode; otherwise it
opens an interactive UI session using `init_vis.mac` / `gui.mac`.

Example batch run:

```bash
./prad2sim -c config/drad.json -p FTFP_BERT_LOCAL run.mac
```

Output ROOT files are written to `output/` and the run number is taken from
(and incremented in) `output/file.output`.

### Multi-threading

When built against a multi-threaded Geant4 install (`G4MULTITHREADED` defined
by the Geant4 installation itself — no extra CMake flags needed), pass `-t N`
to run with `N` worker threads:

```bash
./prad2sim -t 8 run.mac
```

Each worker thread writes its own ROOT file (`output/simrun_N_t0.root` …
`_t7.root` for `-t 8`) so there is no cross-thread contention on the output
tree. Merge them into a single file afterward:

```bash
./merge.sh          # merges all unmerged runs found in output/
./merge.sh -f       # force re-merge (overwrites existing merged files)
# or manually for a specific run number:
hadd output/simrun_N.root output/simrun_N_t*.root
```

Without `-t` (or in a single-threaded Geant4 build), `prad2sim` behaves
exactly as before and writes a single `output/simrun_N.root`.

## Configuration

JSON configs live in `config/` and support inheritance via a `_base` key
(followed recursively), so each experiment file only needs to override what
differs from its base:

- `defaults.json` — base parameters (geometry, target, detector positions)
- `prad.json` — PRad hydrogen-target setup (1 GEM station, HyCal at 273.5 cm)
- `drad.json` — DRad deuterium-target setup with recoil detector and He bag
- `prad2.json` — PRad-II: two GEM stations (3 mm drift gas, detector IDs 0..3),
  HyCal geometry generated from the prad2 reconstruction `hycal_map.json`,
  four-paddle veto scintillator + housing, CAD vacuum-window assembly, and
  the `VD` virtual plane consumed by `prad2ana_sim2replay`
- `x17.json` — X17 search (based on `prad2.json`): target at −455 cm, long
  flight path (GEMs at +6.7/7.1 m, HyCal at +7.5 m), He-bag vacuum exit,
  beam-pipe shielding; LH2 target by default with a Ta-foil option
- `test.json` — minimal target + virtual-detector setup

All lengths in JSON are in cm (except keys suffixed `_mm`), energies in MeV,
angles in degrees. Geant4 unit conversion happens in the loader. Positions
can be absolute or target-relative (`gem.center_ref` / `hycal.surface_ref` =
`"target"`). Geometry parameters can also be overridden at runtime through
the `/prad2sim/...` UI commands defined in the messenger classes.

## Data interface (PRad-II)

The prad2/x17 truth tree carries everything `prad2ana_sim2replay` (from the
`prad2` toolkit) needs to produce a `recon` tree in the replayed-data format
(`prad2evviewer/docs/REPLAYED_DATA.md`): the `VD` plane hits become HyCal
clusters (smeared, `cl_center` resolved against `hycal_map.json` — the sim
HyCal is built from the same map), and `GEM` hits with entry/exit positions
and detector IDs 0..3 feed the HyCal↔GEM matching:

```bash
prad2ana_sim2replay <ep dir> <ee dir> <ep_lumi> <ee_lumi> -o sim_recon.root
```

`tests/sim2replay/run_e2e.sh` runs the whole chain and validates the output;
`tests/regression/` holds the fixed-seed regression harness used to verify
geometry refactors.

## Event generators

Stand-alone generators live under `evgen/`, each with its own `Makefile`:

- `esepp` — elastic e-p scattering with radiative corrections (A. Gramolin)
- `moller` — Møller scattering (A. Gramolin)
- `newep`, `newee`, `norc` — additional ep/ee generators
- `aao`, `eds`, `mitee`, `qesed` — inelastic / quasi-elastic channels

Generators write event files that `prad2sim` reads via
`/prad2sim/gun/type file` and `/prad2sim/gun/path <file>`.

## Digitization

`digitization/` builds two helpers that turn the simulated ROOT hits into
data products comparable to real PRad data:

- `PRadDig` — applies HyCal/GEM digitization and writes EVIO output
- `PRadRec` — runs the PRadAnalyzer reconstruction over the digitized output

## Repository layout

```
prad2sim.cc          main program (Geant4 RunManager + CLI)
src/, include/       physics, generator, messenger, SD and I/O classes
src/detector/        composable DetectorModule subsystems (target, beamline,
                     vacuum system, GEM, scintillator, HyCal, virtual planes,
                     CAD inserts) + MaterialBuilder
include/external/    vendored CADMesh single header (STL import)
config/              JSON configurations (prad, drad, prad2, x17, test)
database/            calibration, mapping and geometry tables
  CADmodel/          CAD STL models (not version-controlled; see README there)
evgen/               stand-alone physics event generators
digitization/        legacy EVIO digitization stage (optional)
tests/regression/    fixed-seed regression harness (tree-stats diff)
tests/sim2replay/    data-interface contract + end-to-end validation
plans/               restructuring plans and findings
output/              ROOT output files (run number tracked in file.output)
*.mac                Geant4 macro files (run.mac, vis.mac, gui.mac, init_vis.mac)
merge.sh             merges per-thread ROOT files from a multi-threaded run
```

## Authors

Chao Peng, Maxime Levillain, Chao Gu

## Acknowledgements

- [A. Gramolin](https://github.com/gramolin) for [ESEPP](https://github.com/gramolin/esepp) and the Møller event generator
- M. Meziane for the radiative correction codes
- The [ROOT](https://root.cern.ch/), [Geant4](http://geant4.cern.ch/) and [EVIO](https://coda.jlab.org/drupal/content/event-io-evio) collaborations
