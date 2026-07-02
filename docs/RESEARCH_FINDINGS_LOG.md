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

### Milestone 3 reparse findings

- A Milestone 3 generated save from the dummy semantic input reparsed successfully with Save Genie.
- The reparsed Milestone 3 dummy-based output preserved:
  - trainer `RED`
  - rival `BLUE`
  - trainer ID `60066`
  - money `3000`
  - coins `0`
  - no badges
  - empty carried bag
  - one `POTION x1` in PC item storage
  - empty Pokédex
  - empty conservative event subset
- A deliberately different contamination fixture also reparsed successfully with Save Genie.
- The reparsed contamination output confirmed:
  - trainer `ASH`
  - rival `GARY`
  - trainer ID `12345`
  - money `654321`
  - coins `321`
  - badges bitfield `0xA5`
  - Pokédex owned `2`, seen `3`
  - bag items `POTION x5`, `ANTIDOTE x2`
  - PC items `POKE BALL x10`, `ESCAPE ROPE x1`
  - hidden items collected indices `1` and `6`
  - hidden coins collected indices `0` and `11`
  - visited towns indices `0`, `2`, and `10`
- The generator-side `compare-semantics` command reported `PASS` for both the dummy-based reparse and the contamination-fixture reparse.

### Milestone 3 local working-fixture findings

- A private local-only `.red.json` fixture was validated successfully against schema `0.1.0`.
- The fixture contained supported owned fields for:
  - trainer and rival identity
  - trainer ID
  - options
  - playtime
  - money and coins
  - badges
  - Pokedex seen and owned bitfields
  - bag inventory
  - PC item inventory
  - visited towns
  - hidden items
  - hidden coins
- The same fixture also contained unsupported non-empty deferred state, including:
  - a non-empty party
  - occupied permanent PC boxes
  - non-empty Hall of Fame data
  - broad story and world-state progress
- Direct generation from the full fixture was rejected conservatively because the source location was outside the proven Milestone 3 safe baseline.
- A local-only projection fixture was then derived from supported semantic fields only:
  - it omitted the target `physicalImage`
  - it excluded unsupported deferred sections instead of silently claiming support
  - it canonicalized location to the proven Red's house second-floor baseline

### Milestone 3 physical-image isolation and semantic proof

- The projected local fixture generated successfully.
- A variant of the same projected fixture with a replaced `physicalImage` produced a byte-identical `.sav`.
- The generated output SHA-256 was `aec4dc3ea7de1f9de8d1315c38f2216d4b06313ccc79612396fe954762118819`.
- Independent Save Genie reparse of that generated save confirmed:
  - trainer `GOON`
  - rival `KILLUA`
  - trainer ID `257`
  - money `589999`
  - coins `999`
  - badges all set
  - playtime `56:20:07`
  - location `MapID=38`, `X=3`, `Y=6`
  - Pokedex seen `151`, owned `151`
  - bag inventory count `19`
  - PC item inventory count `48`
  - projected visited-towns, hidden-item, and hidden-coin bitfields
- Generator-side `compare-semantics` reported `PASS` for the projected fixture reparse.
- No owned semantic field was observed leaking from the dummy template into the projected output.

### Milestone 3 emulator save-again findings

- The generated projected save loaded successfully in emulator testing.
- `Continue` appeared.
- Trainer information, money, coins, badges, playtime, location, bag contents, and PC item contents all displayed correctly in-game.
- The game saved successfully through the in-game save menu.
- Direct in-game Pokedex UI verification was not performed, because the UI was not accessible consistently from the currently serialized event/progression subset.
- Pokedex verification therefore remains proven by Save Genie reparse and semantic comparison, not by direct UI inspection.
- The emulator wrote its save back to the same file path used for the generated output.
- That post-save file was copied immediately to a stable local validation path and hashed.
- The post-save file remained `32768` bytes with SHA-256 `aec4dc3ea7de1f9de8d1315c38f2216d4b06313ccc79612396fe954762118819`.
- Direct binary comparison against the preserved pristine pre-emulator copy showed no changed bytes.
- Save Genie reparse of the post-save file confirmed:
  - valid main checksum
  - unchanged invalid bank 2 all-box checksum
  - unchanged invalid bank 3 all-box checksum
  - preserved Milestone 3 owned semantic fields
  - unchanged empty current-box cache
  - unchanged suspicious permanent-box decode state under Policy A
- This is sufficient to keep Policy A for the current Milestone 3 load/save-again path without expanding it into a blanket claim about future PC-storage interaction safety.
