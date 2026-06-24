# Research Findings Log

## 2026-06-24

### Repository boundary confirmation

- Confirmed writable project root: this repository root
- Confirmed read-only prerequisite: sibling `Pkmn Red Save Genie` repository
- Initialized new Git repository at the generator root.
- Confirmed Save Genie remained untouched during the audit.

### Generator repository state

- The generator root was not previously a Git repository.
- The repository currently contains:
  - `Dummy Save/`
  - `Pkmn Red Save Generator/` with placeholder `main.cpp`
  - `Pkmn Red Save Generator.xcodeproj`
  - `rescource/`
- No generator architecture is implemented yet.

### Dummy resource findings

- Dummy `.sav` size is `32768` bytes.
- Dummy `.sav` hash is `248bc35328be435b16b47e2bb87c4e9732c2b5c92a95450839ed4619f74eb2e7`.
- Dummy `.red.json` reports schema `0.1.0`, game `Pokemon Red`, region assumption `USA-Europe`, and matching source hash.
- Save Genie-backed validation confirmed:
  - valid main checksum
  - invalid bank 2 all-box checksum
  - invalid bank 3 all-box checksum
  - empty party
  - empty current-box cache
  - permanent boxes 1-12 each decoding as `count=20`
- Companion summaries consistently report one PC item: `POTION x1`.

### Suitability finding

- The dummy is not a harmless blank save.
- It is unsafe to treat the permanent-box regions as trusted inherited defaults.
- Any template-based generator must document and minimize inherited ranges explicitly.

### Prerequisite findings

- Save Genie documents complete Red-side preservation coverage and lossless `.red.json` export.
- `SaveStructure.hpp` is the key structural reference for offsets, layout, current-box cache, and checksum locations.
- `WriteOnlyData` shows strict text validation and checksum-repair discipline that should inform generator safety rules.
- Save Genie schema `pokemon-red-master-save.schema.json` is the primary input contract for target `.red.json` compatibility.

### License finding

- No top-level `LICENSE` file was found at either repository root.
- Final license selection remains pending for the generator.
