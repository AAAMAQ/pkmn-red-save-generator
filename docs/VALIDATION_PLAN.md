# Validation Plan

Milestone 0 defines the validation pipeline; it does not yet implement the full generator.

## Stage 1: Input Validation

- JSON syntax
- schema format and version
- supported profile gate
- required semantic sections present
- value ranges
- array capacities
- name encodability policy
- contradictory-state rejection where known

## Stage 2: Template Validation

- file exists
- expected size
- expected hash or approved hash list
- supported target profile
- no output-path collision with template path
- template immutability checks before and after generation

## Stage 3: Generation Validation

- no target `physicalImage` reads
- no out-of-range writes
- supported sections written deterministically
- duplicate/cache synchronization completed
- checksum regeneration completed

## Stage 4: Structural Validation

- output length correct
- counts correct
- terminators present
- strings encodable
- party and storage structures internally consistent
- checksums valid

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

Milestone 0 stops at audit, contracts, and planning. No generator implementation has been validated yet.
