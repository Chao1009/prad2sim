# CAD models (STL)

The prad2 / x17 configurations import the PRad-II vacuum window assembly and
the scintillator housing from STL files via CADMesh (see the `cad` section of
`config/prad2.json` / `config/x17.json`).

The STL files (~50 MB total) are **not version-controlled**. Copy them from
the PRadSim_PRad2 / PRadSim_X17 repositories:

```bash
# prad2 configuration (window assembly at -1280 mm offset + housing)
cp ../PRadSim_PRad2/database/CADmodel/*.stl database/CADmodel/

# x17 configuration (window adapter at zero offset — same file NAMES as the
# PRad2 set but different content/positions, hence the subdirectory)
mkdir -p database/CADmodel/x17
cp ../PRadSim_X17/database/CADmodel/Solid008.stl \
   ../PRadSim_X17/database/CADmodel/Solid009.stl \
   ../PRadSim_X17/database/CADmodel/Solid010.stl database/CADmodel/x17/
```

Sources:
- `PRad_Housing.stl` + `Solid*.stl` — [YuanLiSDU/PRadSim_PRad2](https://github.com/YuanLiSDU/PRadSim_PRad2) `database/CADmodel`
- X17-specific window adapters — [YuanLiSDU/PRadSim_X17](https://github.com/YuanLiSDU/PRadSim_X17) `database/CADmodel`

Missing files are reported at startup and skipped (the simulation still runs,
with reduced material-budget fidelity around the vacuum exit window). Builds
configured with `-DPRAD2SIM_USE_CADMESH=OFF` skip all CAD inserts.
