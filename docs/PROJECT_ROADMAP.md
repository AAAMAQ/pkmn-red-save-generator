# Project Roadmap

## Milestone 0: Boundary, Audit, And Repository Setup

- confirm repository boundaries
- initialize Git at generator root
- preserve existing Xcode project
- audit dummy resources
- inspect prerequisite code, tests, schema, and docs
- define scope, architecture, equivalence, canonicalization, validation, and roadmap

## Milestone 1: Generator Foundation

- establish `src/` and `tests/`
- add target `.red.json` reader and validator
- add supported-profile gate
- add physical-image isolation guardrails
- add primitive encoders and low-level writers
- add template loader and validator
- add CMake build and doctest test target
- keep Xcode as a first-class build path
- status: implemented

## Milestone 2: Minimal Valid Generated Save

- copy validated template to working buffer
- prove loadable untouched working copy
- write trainer identity and minimal safe core state
- regenerate main checksum
- confirm Save Genie reparse and emulator load
- resolve suspicious permanent-box/current-box inheritance in the working buffer before acceptance
- status: implemented

## Milestone 3: Core Gameplay State

- money
- coins
- badges
- Pokedex
- bag inventory
- PC item storage
- core semantic comparison output
- conservative persistent event subset
- contamination and determinism tests
- status: implemented

## Milestone 4: Party Serialization

- full party count and species list
- complete party Pokemon records
- OT names and nicknames
- party-derived validation
- Save Genie reparse and emulator validation
- status: implemented

## Milestone 5: PC Storage

- all 12 permanent boxes
- current selected box semantics
- current-box cache synchronization
- bank and per-box checksum regeneration
- status: implemented and emulator-validated through load, Bill's PC access, deposit, box switching, game-triggered save, withdrawal, normal save, post-save reparse, current-box cache validation, and storage checksum validation

## Milestone 6: Extended Systems

- daycare
- Hall of Fame
- story progress
- trainer battles
- static encounters
- missables
- hidden items
- hidden coins
- visited towns and supported world-state values
- status: implemented, Save Genie reparsed, semantic-compared, emulator-loaded, gameplay-tested, save-again reparsed, and validated for the Red's-house safe-location profile
- emulator validation confirmed normal Continue/load, movement, menus, travel, Hall of Fame viewing with 18 records, normal save-again, and no corruption
- post-save analysis confirmed valid main, Bank 2, Bank 3, and all 12 per-box checksums
- post-save analysis confirmed Daycare, Hall of Fame, hidden items, hidden coins, missables, visited towns, trainer battle flags, static battle flags, story progress, named event flags, and scripts survived unchanged
- expected gameplay drift included travel/location changes, playtime increase, and normal runtime/cache updates from gameplay
- non-baseline location support remains disabled until full map-runtime serialization is implemented and emulator-proven

## Combined Final Release Milestone: Original Milestones 7-9

- status: corrective automated phase and focused second emulator retest passed; save-again durability and post-save analysis remain required
- scope: broad semantic validation, regression hardening, emulator matrix validation, CLI hardening, public samples, CI, documentation, versioning, and release preparation
- current baseline: Milestones 0-6 complete and pushed through `bf275ad`
- release candidate version: `v1.0.0`
- release status: the first broad pass proved general playability but invalidated the original semantic-sufficiency conclusion by exposing four operational defects; the focused corrected-save retest passed, but no post-emulator battery save was produced

### Original Milestone 7: Full Semantic Comparison

- field-by-field equivalence reporting
- contamination fixtures
- unsupported-field reporting
- deterministic output tests
- status: reopened and strengthened after the first manual pass; corrected structural comparison received focused emulator confirmation

### Original Milestone 8: Emulator And Save-Again Validation

- targeted emulator candidates for core load, PC storage, Daycare, Hall of Fame, event/world state, and unsupported-location canonicalization
- save-again and shutdown verification
- post-save Save Genie reparse and semantic comparison
- status: focused corrected-save checks passed for punctuation, selected Box 12, boxed-Pokemon withdrawal HP, and Hall of Fame teams; normal save/shutdown/reload and post-save reparse remain pending

### Original Milestone 9: Release Readiness

- finalize release version and CLI `--version`
- publish supported profile and limitations
- finalize validation matrix
- document provenance reporting
- public samples and CI
- release checklist and release notes
- status: historical `v1.0.0` remains published, but the final semantic-sufficiency proof is provisional and no corrective release should be published before save-again durability and post-save analysis pass
