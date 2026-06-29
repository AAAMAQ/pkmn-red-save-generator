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

## 2026-06-29

### Milestone 2 template-policy findings

- The committed dummy template remains `32768` bytes with SHA-256 `248bc35328be435b16b47e2bb87c4e9732c2b5c92a95450839ed4619f74eb2e7`.
- The main checksum is valid.
- Bank 2 and bank 3 all-box checksums remain invalid in the committed dummy.
- The permanent box payload ranges begin with `0xFF` and continue to decode as `count=20` in all 12 boxes under Save Genie parsing.
- The current box cache remains empty and the selected current-box byte remains `0x00`.

### Save Genie source findings used for Milestone 2

- `SaveStructure.hpp` confirms the current box cache at `0x30C0-0x3521`.
- `SaveStructure.hpp` confirms a permanent box block length of `0x0462`.
- `SaveStructure.hpp` confirms Bank 2 and Bank 3 checksum locations:
  - `0x5A4C-0x5A52`
  - `0x7A4C-0x7A52`
- Save Genie checksum repair logic confirms checksum calculation as the complemented low byte of the covered sum.

### `pret/pokered` findings used for Milestone 2

- The save/load path validates the main checksum and loads current box data before any PC storage interaction.
- The save flow contains `SaveCurrentBoxData`.
- The save flow also contains `EmptyAllSRAMBoxes`, which indicates SRAM box initialization is an explicit operation rather than an assumed precondition of every early-game save.
- This supports a conservative Milestone 2 policy that preserves untouched permanent storage while core-state-only generation is still in scope.

### Milestone 2 generation findings

- The generator now writes a minimal semantically controlled save from `.red.json` semantic fields without consulting the target `physicalImage`.
- Two generations with identical semantic input but different `physicalImage` values produced byte-identical `.sav` outputs.
- Independent Save Genie reparse of the generated save reported:
  - valid main checksum
  - trainer `RED`
  - rival `BLUE`
  - trainer ID `60066`
  - money `3000`
  - coins `0`
  - no badges
  - playtime `0:00:09`
  - location `MapID=38`, `X=3`, `Y=6`
  - empty party
  - empty Pokédex
  - empty bag
  - empty PC items
  - daycare not in use
- The generated save still preserves the dummy's untouched permanent box banks and invalid bank 2 and 3 all-box checksum state by design under Policy A.

### Milestone 2 emulator-load finding

- The generated Milestone 2 save loaded successfully in emulator testing.
- `Continue` appeared.
- The loaded trainer name was `RED`.
- The loaded money value was `3000`.
- The loaded map state placed Red in his house on the second floor.
- No immediate corruption warning appeared on load.
- This is sufficient to keep Policy A provisionally for Milestone 2 loadability.
- Permanent storage bank behavior is still not fully validated because no post-save `.sav` reparse has been supplied yet.
