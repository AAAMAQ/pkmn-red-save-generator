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

## 2026-07-06

### Milestone 4 party-serialization findings

- The generator now serializes the full active party in the main save area:
  - party count
  - species list and terminator
  - all six party records
  - OT names
  - nicknames

## 2026-07-07

### Milestone 5 storage findings

- The generator now writes all 12 permanent PC boxes from semantic input instead of inheriting dummy storage banks.
- The selected permanent box is copied into the Bank 1 current-box cache.
- Generated outputs now have:
  - valid main checksum
  - valid bank 2 all-box checksum
  - valid bank 3 all-box checksum
  - valid per-box checksums for all 12 boxes
  - byte-level current-box cache synchronization
- The private full fixture reparsed in Save Genie with:
  - 7 occupied permanent boxes
  - Box 12 containing 5 Pokemon
  - an empty selected box 11 cache matching the selected permanent box

### Milestone 6 extended-state findings

- The generator now serializes:
  - daycare occupancy
  - Hall of Fame entries
  - missable objects
  - hidden items
  - hidden coins
  - visited towns
  - named event flags
  - trainer-battle flags
  - static-battle flags
  - story-progress flags
  - persistent script bytes
  - a verified runtime subset for safe supported locations
- The private full fixture generated successfully at the real Viridian City Pokemon Center location during automated validation, before emulator testing exposed the load-time defect documented below.
- Save Genie reparse plus generator semantic comparison reported `PASS with permitted canonical differences`.
- The only permitted differences were five boxed `level` fields where the source fixture already carried implausible boxed-level values such as `139`, `154`, and `255`.

### Milestone 5-6 physical-image independence findings

- Generating from the original private fixture, a version with `physicalImage` removed, and a version with replaced `physicalImage` produced identical output bytes.
- All three generated outputs hashed to `77ba8ffd186e69e67a3119f58dba41f60a1a205e734e6bf49c014916d0cce7a5`.
- Party comparison is now field-aware down to indexed paths such as move PP, nickname, current HP, DVs, and Stat Experience.
- The party generation path still ignores the target `physicalImage` entirely.

### Party-stat policy finding

- The private six-Pokemon working fixture exposed a mismatch between an inferred stat formula and actual stored party stats from a real save.
- Observed Save Genie-decoded party live stats should be treated as authoritative stored state for Milestone 4 generation.
- The validator now enforces structural and range invariants for stored live stats rather than rejecting real saves over small calculator-policy differences.
- `PokemonStatCalculator` remains useful for helper construction, PP packing, DV derivation, and future analysis, but Milestone 4 generation preserves the decoded stored party stats supplied by semantic input.

### Milestone 4 local working-fixture findings

- A private local-only six-Pokemon `.red.json` fixture was validated successfully against schema `0.1.0`.
- The raw fixture still exceeded Milestone 4 scope because it contained:
  - non-empty permanent PC storage with `116` boxed Pokemon across `7` occupied boxes
  - non-empty Hall of Fame with `18` entries
  - broad story, trainer-battle, static-battle, event-flag, script, and world-state progress
  - a live source location outside the currently proven safe baseline
- Direct generation from the raw fixture was rejected conservatively because the source location was `mapId=41` instead of the proven `mapId=38`.
- A local-only Milestone 4 projection fixture was derived:
  - removed the target `physicalImage`
  - preserved supported Milestone 4 owned semantics
  - canonicalized location to Red's house second floor
  - kept daycare empty
  - kept Hall of Fame empty

### Milestone 4 physical-image isolation and semantic proof

- The projected local fixture generated successfully.
- A variant of the same projected fixture with replaced `physicalImage` produced a byte-identical `.sav`.
- Both projected-party outputs had SHA-256 `d40757bbd58effa0ba73b44aa2d3352f01e6ca2dd79aab539d1e9bee9dd13c0d`.
- Independent Save Genie reparse of the generated Milestone 4 save confirmed:
  - trainer `GOON`
  - rival `KILLUA`
  - trainer ID `257`
  - money `589999`
  - coins `999`
  - badges all set
  - playtime `56:20:07`
  - location `Red's house (second floor)`
  - Pokedex seen `151`, owned `151`
  - bag inventory count `19`
  - PC item inventory count `48`
  - six-party lineup with expected species, levels, names, HP, status, and moves
- Generator-side `compare-semantics` reported `PASS` for both projected-party reparses.
- Direct raw-fixture-to-generated comparison found only one owned-field difference:
  - `core.mapId` remained canonicalized to the currently proven safe baseline

### Milestone 4 emulator save-again findings

- The generated projected-party save loaded successfully in emulator testing.
- `Continue` appeared.
- The save loaded without corruption warnings.
- Trainer/core state displayed correctly.
- The full six-Pokemon lineup displayed correctly in-game.
- Inspected party levels, names, HP, status, and moves were correct.
- Bag contents and PC item storage displayed correctly.
- The game saved successfully through the in-game menu.
- The emulator-written file was copied to a stable local validation path before further analysis.
- The post-save file remained `32768` bytes with SHA-256 `d40757bbd58effa0ba73b44aa2d3352f01e6ca2dd79aab539d1e9bee9dd13c0d`.
- Direct binary comparison against preserved pristine pre-emulator outputs showed no changed bytes.
- Save Genie reparse of the post-save file confirmed:
  - valid main checksum
  - unchanged invalid bank 2 all-box checksum
  - unchanged invalid bank 3 all-box checksum
  - preserved Milestone 4 owned semantic fields
  - unchanged empty current-box cache
  - unchanged suspicious permanent-box decode state under Policy A
- This keeps Policy A for Milestone 4 load/save-again validation while leaving PC-storage semantics and checksum repair to Milestone 5.

## 2026-07-07 - Milestone 5-6 Load-Time Corruption Incident

- Emulator validation exposed a blocking load-time defect in the full Milestone 5-6 generated save.
- Confirmed facts:
  - the generated save was `32768` bytes
  - automated generator validation passed
  - main, per-box, bank 2, and bank 3 checksum validation passed
  - Save Genie reparsed the generated save
  - semantic comparison passed under the current comparator rules
  - physical-image independence passed
  - title screen and Continue appeared normally
  - severe corruption appeared immediately after selecting Continue
  - no walking, menu access, PC interaction, box switching, or in-game save occurred before the failure
- The incident demonstrates that parser acceptance, semantic comparison, and checksum validity are insufficient emulator-safety evidence.
- The four additional Save Genie diagnostic outputs produced from the exact failing save decoded:
  - valid main, Bank 2, and Bank 3 checksums
  - trainer `GOON` and rival `KILLUA`
  - the six-party lineup
  - all 12 PC boxes with expected counts `[20, 18, 17, 19, 20, 17, 0, 0, 0, 0, 0, 5]`
  - Viridian Pokemon Center location `map=41`, `x=3`, `y=6`, `xBlock=1`, `yBlock=0`
  - event/story/script counts matching the private fixture surface
- These Save Genie diagnostics are diagnostic evidence only; they strengthen the finding that a save can be semantically readable while remaining unsafe for the game.
- Binary comparison found that the failing save combined Viridian Pokemon Center location bytes with Red's-house/dummy map-runtime bytes around the map pointer, dimension, warp/object, and trainer-header regions.
- Specific mismatched map/runtime fields include `0x2613`, `0x2615`, `0x2616`, `0x2618`, `0x261A`, `0x265A`, `0x278D`, `0x27D1`, `0x27D2`, `0x29E8`, and `0x2CDC`.
- The strongest current finding is unsafe non-baseline location admission without a complete map-runtime serialization contract.
- Corrective action:
  - the location validator now fails closed to the emulator-validated Red's-house baseline only
  - direct generation from the full private Viridian Pokemon Center fixture is rejected until full map-runtime serialization is implemented and emulator-proven
  - a byte-provenance ledger now annotates generated report ranges with before/after evidence
  - generation now rejects undeclared overlapping non-template write ranges
  - broad runtime report ranges were split into exact subranges
- Storage serialization remains experimental until the corrected Red's-house storage diagnostic save passes base-load and PC interaction tests in the emulator.
- Milestone 6 progression is paused until Milestone 5 storage passes emulator validation.

### Corrected Red's-house storage diagnostic emulator result

- The corrected Red's-house storage diagnostic passed the base-load gate:
  - Continue appeared
  - loading reached Red's house second floor without corruption
  - movement worked
  - the menu opened
  - trainer/core, money, coins, badges, and party looked correct
- The tester could not access Bill's PC directly from the clean diagnostic state and therefore progressed gameplay before PC inspection.
- Because gameplay occurred first, the resulting save is a gameplay-modified post-test save rather than a pristine generated baseline.
- Visual PC storage verification passed provisionally:
  - in-game box counts matched `[20, 18, 17, 19, 20, 17, 0, 0, 0, 0, 0, 5]`
  - occupied/empty indicators matched the expected generated layout
  - no visible box-list corruption was observed
- Save Genie reparse of the gameplay-modified save reported valid main, Bank 2, and Bank 3 checksums.
- The post-gameplay selected box state used current-box byte `0x82`, selected box `3`, and a dirty current-box cache containing `17` Pokemon.
- Permanent box 3 decoded as empty, but a cache-aware comparison matched expected box 3 contents against the active current-box cache with no storage mismatches.
- This supports the interpretation that the original load corruption fix is effective and that storage is viewable in game, while leaving controlled deposit, withdraw, box-switch, and post-save comparison as required Milestone 5 evidence.

### Controlled storage interaction artifact analysis

- The tester reported successful deposit, box switching, withdrawal, game-triggered save, normal save, and emulator close with no visible storage corruption.
- The initially supplied local artifact `controlled-storage-interaction-start copy.sav` was not usable as post-save evidence:
  - size `32768`
  - SHA-256 `c1005fd8ad32468e748a1e7a2fda8c1fc36dad708d39e70fb9baaf7852c62ce3`
  - Save Genie parsed it as invalid erased/mostly-`0xFF` SRAM
  - main, Bank 2, and Bank 3 checksums were invalid
- A sibling local artifact `controlled-storage-interaction-start.sav` parsed as the likely valid post-interaction candidate:
  - size `32768`
  - SHA-256 `b0b86a6581c1ce1199d1dc4860c452ea6efa6959e60e1ba910bcbcb69950db03`
  - main checksum valid
  - Bank 2 and Bank 3 all-box checksums valid
  - all 12 per-box checksums valid
  - Box 3 cache from the previous test was flushed to permanent Box 3
  - `PEGGY` / `PIDGEY` moved from the party to the active Box 11 cache
- This is strong evidence for deposit, selected-box cache, and box write-back behavior.
- This intermediate artifact was superseded by the final post-withdrawal artifact.

### Final Milestone 5 storage interaction evidence

- The latest valid post-withdrawal save was preserved privately:
  - size `32768`
  - SHA-256 `3da8ec51484f2aa707c32dde36dc70712cb29163ef992bdb672a79c2bcfa89fe`
- Save Genie reparsed it successfully.
- Checksum validation passed:
  - main checksum stored/calculated `0x02`
  - Bank 2 all-box checksum stored/calculated `0x92`
  - Bank 3 all-box checksum stored/calculated `0x2B`
  - all 12 per-box checksums valid
- Storage interaction results:
  - selected/current box decoded as Box `12`
  - raw current-box byte `0x8B`
  - current-box cache count `4`
  - current-box cache contains `SLAYER`, `PARASECT`, `PIDGEOT`, and `RHYHORN`
  - permanent Box 11 contains deposited `PEGGY` / `PIDGEY`
  - permanent Box 12 count is `0`, consistent with Box 12 being represented by the active current-box cache
  - party count is `6`
  - party slot 6 contains withdrawn `RED` / `CHARIZARD`
  - `RED` is fainted with HP `0/165`
- No Pokemon loss, duplication, name corruption, checksum corruption, graphical corruption, or text corruption was reported or found in the post-save parse.
- Milestone 5 storage validation is complete.
