# Validation Plan

Milestone 1 implements the first validation layers. Full generation validation remains incomplete until Milestones 2 and 3.

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

Remaining Milestone 2 limitation:

- emulator load and save-again reparse evidence have now been confirmed for the approved Milestone 3 local validation path
- permanent PC storage remains preserved canonical inherited state under Policy A and is still not serialized semantically
- broader event-state serialization remains deferred to Milestone 3 and later
