# prad2sim restructuring plans

Planning and findings for reworking the PRad/DRad/PRad2/X17 Geant4 simulation
into one modular, readable, performant package whose data products match the
PRad-II replayed-data interface.

## Documents

| File | Contents |
|---|---|
| [00-current-state.md](00-current-state.md) | Inventory of the existing `prad2sim` rework, the three reference sims, and a gap analysis. |
| [01-architecture.md](01-architecture.md) | Target modular architecture: detector modules, SD layer, generators, config, build. |
| [02-configurations.md](02-configurations.md) | How `prad` / `drad` / `prad2` / `x17` differ and how to express all four from one codebase. |
| [03-data-interface.md](03-data-interface.md) | Mapping simulation output to `../prad2evviewer/docs/REPLAYED_DATA.md` (`events` / `recon` trees) and the digitization plan. |
| [04-roadmap.md](04-roadmap.md) | Phased implementation roadmap with concrete, ordered tasks. |
| [05-build-notes.md](05-build-notes.md) | Verified Geant4 11.4 build/run recipe, env gotchas, and the 10→11.4 source port. |

## One-paragraph summary

`prad2sim` is already a partial rewrite of `../PRadSim`: it has a JSON config
loader with inheritance (`SimConfig`), a detector split
(`DetectorConstruction` dispatcher + `DetectorCommon` / `DetectorPRad` /
`DetectorDRad`), and `prad`/`drad`/`test` configs. The two newest reference
forks (`../PRadSim_PRad2`, `../PRadSim_X17`) carry the physics/geometry we
still need to fold in: CAD-imported geometry, in/out hit positions, 3 mm GEM
drift gas, the 4-plane scintillator, beam-pipe shielding, the X17 window
adapter, and Geant4 11.4 / Qt6 compatibility.

The **data interface** turned out to be largely solved on the `prad2` side: the
installed `prad2ana_sim2replay` converts sim truth into the `recon` tree the
`prad2*` viewer/analysis read, and that contract is now **validated end to end**
(`../tests/sim2replay/`). prad2sim's job is to emit a truth tree `T` carrying a
HyCal-front `VD` plane and GEM in/out positions — the SD now writes the required
`.DID`/`.Xout/.Yout/.Zout` branches; the remaining pieces are Geant4 geometry.

> Environment: ROOT, the `prad2` toolkit, **and Geant4 11.4** are now installed
> (`~/Apps/geant4`); prad2sim builds and runs locally — see
> [05-build-notes.md](05-build-notes.md). The whole pipeline is testable here.
