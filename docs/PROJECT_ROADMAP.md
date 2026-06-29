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

## Milestone 4: Party Serialization

- full party count and species list
- complete party Pokemon records
- OT names and nicknames
- party-derived validation

## Milestone 5: PC Storage

- all 12 permanent boxes
- current selected box semantics
- current-box cache synchronization
- bank and per-box checksum regeneration

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

## Milestone 7: Full Semantic Comparison

- field-by-field equivalence reporting
- contamination fixtures
- unsupported-field reporting
- deterministic output tests

## Milestone 8: Emulator And Save-Again Validation

- emulator load verification
- menu and continue verification
- in-game save
- second reparse and semantic comparison

## Milestone 9: Release Readiness

- finalize license
- publish supported profile and limitations
- finalize validation matrix
- document provenance reporting
- produce release checklist
