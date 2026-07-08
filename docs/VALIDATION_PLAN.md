# Validation Plan

Milestone 1 implements the first validation layers. Generation validation is now implemented through the current Milestone 5 and Milestone 6 code surface for the currently owned semantic state.

## Stage 1: Input Validation

- JSON syntax
- schema format and version
- supported profile gate
- required semantic sections present
- value ranges
- array capacities
- name encodability policy
- contradictory-state rejection where known
- implemented in Milestone 1 via `RedJsonReader`, `RedJsonValidator`, and `RedSemanticStateBuilder`

## Stage 2: Template Validation

- file exists
- expected size
- expected hash or approved hash list
- supported target profile
- no output-path collision with template path
- template immutability checks before and after generation
- suspicious baseline analysis for current-box cache and permanent-box state
- implemented in Milestone 1 via `TemplateProfile`, `CanonicalTemplateLoader`, `TemplateBaselineAnalyzer`, and `TemplateValidator`

## Stage 3: Generation Validation

- no target `physicalImage` reads
- no out-of-range writes
- supported sections written deterministically
- duplicate/cache synchronization completed
- checksum regeneration completed
- partially implemented in Milestone 1:
  - `physicalImage` is stripped before semantic-state construction
  - low-level bounded writers exist
  - deterministic template-copy initialization exists

## Stage 4: Structural Validation

- output length correct
- counts correct
- terminators present
- strings encodable
- party and storage structures internally consistent
- checksums valid
- Milestone 2 now validates:
  - output size `32768`
  - regenerated main checksum
  - unchanged permanent box banks under Policy A
  - unchanged bank 2 and bank 3 all-box checksum state under Policy A
- Milestone 4 now additionally validates:
  - full active-party count and species-list structure
  - party record field bounds, move packing, name encodability, and HP/status invariants
  - deterministic party serialization independent of target `physicalImage`
- Experimental Milestone 5 and paused Milestone 6 code now additionally validate:
  - all 12 permanent boxes
  - current-box cache agreement with the selected permanent box
  - per-box checksums and bank 2 and 3 all-box checksums
  - daycare occupancy consistency
  - Hall of Fame record count and entry structure
  - event-flag source agreement across `events`, `trainerBattles`, `staticBattles`, and `storyProgress`

## Stage 5: Independent Reparse

Use the completed Save Genie as an external oracle:

```text
generated .sav
-> Save Genie parse
-> generated .red.json semantics
-> semantic comparison report
```

## Stage 6: Emulator Validation

Eventually confirm:

- save is recognized
- Continue appears
- trainer identity displays correctly
- party, boxes, inventory, Pokedex, and location load safely
- no corruption warning appears
- game can save again

Mandatory base-load gate for every storage or extended-state save:

1. title screen renders normally
2. Continue appears
3. selecting Continue loads without corruption
4. player appears on expected map
5. screen tiles and text are normal
6. movement works
7. menu opens
8. trainer page opens
9. party page opens
10. save can be performed normally

Milestone 5 PC-storage gate after base load:

- open Bill's PC
- open Pokemon storage
- inspect current box
- inspect at least one occupied box
- withdraw one Pokemon
- deposit one Pokemon
- switch boxes
- accept any game-triggered box save
- return to gameplay
- save normally
- close emulator
- reparse post-save output
- compare party, all boxes, selected box, and current-box cache
- validate all storage checksums

Milestone 2 emulator-load evidence now recorded:

- `Continue` appeared
- the save loaded
- trainer `RED` displayed correctly
- money `3000` displayed correctly
- Red loaded in his house on the second floor
- no immediate corruption warning appeared

Milestone 2 and Milestone 3 emulator save-again evidence now recorded for the approved local validation path:

- the Milestone 3 projected save loaded successfully
- trainer identity, money, coins, badges, playtime, location, bag contents, and PC item contents displayed correctly
- the game saved successfully through the in-game menu
- the post-save file was byte-identical to the preserved pre-emulator copy
- Save Genie reparsed the post-save file successfully
- generator-side semantic comparison still reported `PASS`

Milestone 4 emulator save-again evidence is now also recorded for the approved local projected-party validation path:

- `Continue` appeared
- the save loaded without corruption warnings
- trainer/core state displayed correctly
- the six-party lineup displayed correctly
- inspected levels, names, HP, status, and moves displayed correctly
- bag contents and PC item storage displayed correctly
- the game saved successfully through the in-game menu
- the emulator-written file was byte-identical to the preserved pre-emulator save
- Save Genie reparsed the post-save file successfully
- generator-side semantic comparison still reported `PASS`

Current Milestone 5 and Milestone 6 automated/private-fixture evidence before emulator testing:

- the private full semantic fixture now generates successfully without using target `physicalImage`
- Save Genie reparses the generated full-fixture save successfully
- generator-side semantic comparison reports `PASS with permitted canonical differences`
- the only reported permitted differences are a small set of boxed `level` fields whose source values are implausible oracle decodes
- all permanent box checksums and bank 2 and 3 all-box checksums validate on generated output
- generating from the original fixture, a version with `physicalImage` removed, and a version with replaced `physicalImage` produced byte-identical `.sav` outputs

Current Milestone 5 and Milestone 6 emulator evidence:

- title screen and Continue appeared normally
- the original full Milestone 5-6 save corrupted immediately after selecting Continue
- no movement, menu access, PC interaction, box switching, or save occurred before the original failure
- the corrected Red's-house storage diagnostic now passes the base-load gate
- PC storage viewing passed provisionally after gameplay progression was used to reach Bill's PC
- visual box counts matched `[20, 18, 17, 19, 20, 17, 0, 0, 0, 0, 0, 5]`
- a cache-aware Save Genie comparison found no storage mismatches in the post-gameplay save
- a controlled-interaction candidate validated deposit into Box 11, selected-box cache behavior, Box 3 write-back, all per-box checksums, and Bank 2/3 all-box checksums
- the final post-withdrawal artifact validated selected/current Box 12, withdrawn `RED` in party slot 6, deposited `PEGGY` in permanent Box 11, active Box 12 current-box cache, all per-box checksums, and Bank 2/3 all-box checksums
- controlled deposit, withdraw, box-switch, save-again, and post-save comparison passed for Milestone 5
- the failure is documented in `docs/MILESTONE_5_6_LOAD_CORRUPTION_INCIDENT.md`
- direct full-fixture generation now fails closed because the source location is not in the emulator-validated location set
- Milestone 6 remains incomplete until Daycare, Hall of Fame, broader event/world-state, safe-location, and emulator save-again validation pass

Pokedex verification wording for Milestone 3 must remain precise:

- Pokedex semantic verification passed through Save Genie reparse and semantic comparison
- direct in-game Pokedex UI verification was not performed
- broader event and world-state serialization may still be needed to make the Pokedex UI accessible consistently

## Stage 7: Save-Again Validation

```text
generated .sav
-> emulator load
-> in-game save
-> reparsed save
-> semantic comparison
```

## Physical-Image Isolation Test Strategy

These tests are mandatory once generation begins:

1. Generate from a normal target `.red.json`.
2. Remove the target `physicalImage`.
3. Corrupt `physicalImage.standardSramHex`.
4. Replace target raw image with a different save's raw image.
5. Confirm generated output is identical in all cases where semantic fields are unchanged, or confirm deterministic rejection under a documented strict-input mode.

The standalone dummy template is not part of this prohibition. The isolation rule applies to the raw image embedded in the target `.red.json`.

Milestone 1 proof:

- the semantic-state builder receives only a sanitized JSON document
- tests confirm identical semantic state when `physicalImage` is removed, corrupted, or replaced

## Dummy Immutability Test Strategy

- hash dummy template before generation
- run generation and tests
- hash dummy template after generation
- assert the original template hash is unchanged

## Dummy Contamination Test Strategy

- use a target semantic fixture that differs sharply from the dummy in:
  - trainer identity
  - money
  - location
  - Pokedex
  - bag and PC items
  - party
  - all 12 boxes
  - current box
  - daycare
  - Hall of Fame
  - events and visited towns
- reparse generated output with Save Genie
- assert no dummy-only state remains in covered fields

## Current Milestone Limitation

Milestone 2 now has an automated minimal generator, main checksum regeneration, Save Genie reparse, deterministic-output proof, and field-aware comparison for owned minimal fields.

Current open validation gap:

- emulator PC interaction for Milestone 5
- emulator Daycare, Hall of Fame, and broader navigation interaction for Milestone 6
