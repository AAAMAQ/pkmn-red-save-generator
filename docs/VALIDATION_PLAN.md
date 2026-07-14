# Validation Plan

Milestone 1 implemented the first validation layers. Generation validation is now implemented through completed Milestone 6 for the currently owned semantic state.

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
- duplicate/cache policy applied without erasing valid divergent working state
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
- Milestone 5 and Milestone 6 code now additionally validate:
  - all 12 permanent boxes
  - independent structural validity of the current-box working copy and all permanent boxes
  - selected-box dirty-state coherence and explicit current/permanent divergence classification
  - boxed-record withdrawal viability using independently calculated Generation I stats
  - per-box checksums and bank 2 and 3 all-box checksums
  - daycare occupancy consistency
  - Hall of Fame record count and entry structure
  - every Hall of Fame slot, internal species ID, level, text boundary, and team stride
  - lossless Generation I text encoding without fallback substitution
  - event-flag source agreement across `events`, `trainerBattles`, `staticBattles`, and `storyProgress`
  - complete 228-entry missable object input
  - complete 97-entry script input with no overlapping script byte ranges
  - named story-evidence/world bits exposed by Save Genie

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

Milestone 6 automated/private-fixture evidence:

- the raw private full semantic fixture is accepted by canonicalizing its non-baseline location to Red's house second floor because the source location is not emulator-proven
- the private Red's-house projection generates successfully without using target `physicalImage`
- Save Genie reparses the generated Red's-house extended-state save successfully
- generator-side semantic comparison reports `PASS with permitted canonical differences`
- corrected boxed-record decoding removes the former boxed-level permitted-difference exception
- all permanent box checksums and bank 2 and 3 all-box checksums validate on generated output
- generation with original, removed, and replaced target `physicalImage` produced byte-identical `.sav` outputs
- the generated save is `32768` bytes and has no overlapping generation-report writes
- Daycare, Hall of Fame, named events, trainer/static/story flags, scripts, missables, hidden items, hidden coins, visited towns, and named story-evidence/world bits are included in the comparison surface
- CLI validation now includes `validate-save`, `inspect`, dry-run generation, write-range display, determinism checks, and physical-image-isolation checks

Milestone 5 and Milestone 6 emulator evidence:

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
- direct raw fixture generation now reports location canonicalization when the source location is not in the emulator-validated location set
- the Milestone 6 Red's-house extended-state candidate passed base-load emulator validation
- movement, menus, travel to a Pokemon Center, and general gameplay behaved normally
- Hall of Fame viewing confirmed 18 completed entries

## Corrective Semantic-Sufficiency Gate

The first completed-playthrough automated comparison was invalidated by manual findings even though checksums and Save Genie reparse passed. The corrected gate therefore requires:

- lossless byte-aware text comparison and fallback detection
- permanent-box and current-working-box comparison as distinct structures
- operational withdrawal viability for every boxed Pokemon
- complete Hall of Fame comparison by record and slot
- strict internal Generation I species-ID validation
- independent structural checks that do not import serializer assumptions
- focused emulator retest followed by post-save reparse

Until the focused retest passes, the correct status is `CORRECTED AUTOMATED PHASE PASSED - AWAITING SECOND MANUAL EMULATOR RETEST`.
- the game saved normally after the validation path
- post-save Save Genie reparse passed
- post-save validation reported valid main, Bank 2, Bank 3, and all 12 per-box checksums
- post-save analysis confirmed Daycare, Hall of Fame, hidden items, hidden coins, missables, visited towns, trainer battle flags, static battle flags, story progress, named events, and scripts survived unchanged
- post-save `worldState` differences were expected gameplay drift from travel to Viridian City Pokemon Center
- current-box cache divergence after deposit was accepted only with an explicit dirty-cache validation mode
- numeric trainer ID remained `257` (`0x0101`), displayed by the game as five digits

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

Milestones 0-6 now have automated generation, checksum regeneration, Save Genie oracle validation where private fixtures are available, deterministic-output proof, field-aware comparison for owned fields, and emulator evidence through the Red's-house safe-location profile.

Remaining validation limitations:

- full map-runtime support for non-baseline locations
- direct Daycare deposit/withdraw interaction for an occupied-Daycare fixture

## Final Release Automated Gates

The release-hardening layer adds:

- public minimal and representative `.red.json` samples
- public unsupported Viridian Pokemon Center canonicalization sample
- CLI `--version`
- CI for CMake build, tests, CLI smoke, public sample generation, checksum validation, determinism, physical-image isolation, unsupported-location canonicalization, whitespace checks, and private-path scanning
- release checklist and validation matrix

These gates do not replace private Save Genie oracle validation or emulator load/save-again validation. They make public release regressions harder to miss.

## Final Release Emulator Result

The final release validation used two targeted emulator candidates:

- a public representative sample covering clean load, movement, menus, trainer/party/Pokedex/Bag/inventory display, normal gameplay, save-again, shutdown, and post-save reparse
- a private full-state Red's-house sample covering clean load, movement, menus, broad travel, doors, stairs, warps, scripts, map transitions, normal save-again, shutdown, and post-save reparse

Both candidates passed. Post-save SRAM files were preserved privately, reparsed through Save Genie, and validated with main, Bank 2, Bank 3, and all per-box checksums valid. Differences from pristine generated saves were classified as expected gameplay drift.

Direct final-test inspection did not repeat every subsystem interaction. PC storage and Hall of Fame direct interaction evidence comes from earlier milestone validation; Daycare remains structurally and oracle-validated but not directly UI-tested in the final emulator run.
