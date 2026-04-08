# prad2sim

Geant4 simulation package for the Jefferson Lab PRad / DRad experiments.

`prad2sim` propagates events from a beam-target interaction through the full
PRad/DRad setup (target, GEM trackers, scintillator plane, HyCal calorimeter
and the recoil detector for DRad), records hits to a ROOT tree, and feeds the
result into a digitization stage that produces EVIO files compatible with
[PRadAnalyzer](https://github.com/JeffersonLab/PRadAnalyzer).

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
macro and config files needed to run it. The digitization sub-project builds
`PRadDig` and `PRadRec` and copies them to the parent directory.

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
| `-h, --help` | | Print usage |

If a macro file is given, the simulation runs in batch mode; otherwise it
opens an interactive UI session using `init_vis.mac` / `gui.mac`.

Example batch run:

```bash
./prad2sim -c config/drad.json -p FTFP_BERT_LOCAL run.mac
```

Output ROOT files are written to `output/` and the run number is taken from
(and incremented in) `output/file.output`.

## Configuration

JSON configs live in `config/` and support inheritance via a `_base` key, so
each experiment file only needs to override what differs from
`defaults.json`:

- `defaults.json` — base parameters (geometry, target, detector positions)
- `prad.json` — PRad hydrogen-target setup
- `drad.json` — DRad deuterium-target setup with recoil detector
- `test.json` — test configuration

All lengths in JSON are in cm, energies in MeV, angles in degrees. Geant4
unit conversion happens in the loader. Geometry parameters can also be
overridden at runtime through the `/prad2sim/...` UI commands defined in the
messenger classes.

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
src/, include/       detector, physics, generator, messenger, I/O classes
config/              JSON configurations and detector parameter files
database/            calibration, mapping and pedestal tables
evgen/               stand-alone physics event generators
digitization/        EVIO digitization and reconstruction stage
output/              ROOT output files (run number tracked in file.output)
*.mac                Geant4 macro files (run.mac, vis.mac, gui.mac, init_vis.mac)
```

## Authors

Chao Peng, Maxime Levillain, Chao Gu

## Acknowledgements

- [A. Gramolin](https://github.com/gramolin) for [ESEPP](https://github.com/gramolin/esepp) and the Møller event generator
- M. Meziane for the radiative correction codes
- The [ROOT](https://root.cern.ch/), [Geant4](http://geant4.cern.ch/) and [EVIO](https://coda.jlab.org/drupal/content/event-io-evio) collaborations
