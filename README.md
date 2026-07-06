# Pkmn Red Save Generator

Status: Milestone 4 party serialization implemented.

This project generates a new Pokemon Red save from semantic `.red.json` data. It does not use the target `.red.json` `physicalImage` to reconstruct the original binary. Output is intended to be gameplay-equivalent and structurally valid, not byte-identical to the source save.

## Project Boundary

- Writable project: this repository root
- Read-only prerequisite: sibling `Pkmn Red Save Genie` repository

The prerequisite Save Genie remains the trusted Red-side research and parsing codebase. This generator is a separate repository with a different goal:

- Save Genie: archival export and byte-identical reconstruction via preserved raw bytes
- Save Generator: deterministic semantic save synthesis without using the target `physicalImage`

## Core Rules

- The target `.red.json` supplies semantic state only.
- The raw `physicalImage` embedded in the target `.red.json` is forbidden as generation input.
- The standalone dummy `.sav` under `Dummy Save/` is an explicitly permitted canonical template candidate.
- The dummy `.red.json`, `PokemonSummary.json`, `PokemonBoxes.json`, and `SaveGenieSummary.txt` are audit and validation references only.
- Output must represent the target semantic state, not the dummy player.
- Success means semantic equivalence plus emulator validity, not byte identity.

## Current Repository State

- Git repository initialized at the generator root on 2026-06-24.
- Existing Xcode project preserved and now builds against the shared source tree.
- A first-class CMake build and doctest-based test target now exist alongside Xcode.
- Milestones 1-4 are now implemented for the currently owned semantic surface:
  - foundation, validation, and template safety
  - minimal valid save generation
  - core trainer, item, Pokedex, and conservative event generation
  - active party Pokemon serialization

## Key Milestone 0 Findings

- The generator folder began as an Xcode skeleton and was not previously a Git repository.
- The supplied dummy save is a real `0x8000` Pokemon Red save with a valid main checksum.
- The dummy is not an empty baseline:
  - trainer name `RED`
  - rival name `BLUE`
  - trainer ID `60066`
  - money `3000`
  - playtime `0:00:09`
  - location `Red's house (second floor)`
  - PC item storage contains `POTION x1`
  - current box cache is empty
  - permanent boxes 1-12 all decode as `count = 20`
  - bank 2 and bank 3 all-box checksums are invalid

This means the dummy cannot be treated as a harmless blank save. Any template-based generator must explicitly overwrite or clear all covered semantic regions and regenerate all affected checksums before output.

## Repository Layout

- `Dummy Save/`
  - Canonical template candidate and companion audit material
- `Pkmn Red Save Generator/`
  - Xcode entrypoint folder with the CLI `main.cpp`
- `Pkmn Red Save Generator.xcodeproj/`
  - Existing Xcode project metadata
- `src/`
  - Milestone 1 source modules for CLI, input, model, template, encoding, integrity, generation, comparison, and reporting
- `tests/`
  - doctest-based unit and integration coverage for Milestone 1
- `profiles/`
  - machine-readable supported target profiles
- `third_party/`
  - vendored `nlohmann/json` and `doctest`
- `docs/`
  - scope, audit, policy, roadmap, and execution-plan documents

## Milestone 1 Capabilities

- `validate-input --input <target.red.json>`
  - validates supported schema and semantic sections
  - strips the target `physicalImage` before semantic-state construction
- `validate-template --template <dummy.sav> --profile <profile.json>`
  - validates size, hash, checksum expectations, and suspicious baseline traits
- `show-profile --profile <profile.json>`
  - prints the active supported target profile

## Current Generator Capabilities

- `generate --input <target.red.json> --template <dummy.sav> --profile <profile.json> --output <generated.sav> --report <generated.generation-report.json>`
  - generates a deterministic Pokemon Red save from semantic `.red.json` data
  - ignores the target `physicalImage`
  - preserves the committed dummy template unchanged
- `compare-semantics --target-json <target.red.json> --reparsed-json <savegenie-output.red.json>`
  - compares currently owned semantic fields after Save Genie reparse

Currently owned through Milestone 4:

- trainer and rival identity
- trainer ID
- options
- playtime
- money and coins
- badges
- conservative safe baseline location
- Pokedex seen and owned bitfields
- bag inventory
- PC item inventory
- conservative visited-town, hidden-item, and hidden-coin subset
- full active party serialization:
  - party count
  - species list and terminator
  - party records
  - OT names
  - nicknames
  - moves, PP, DVs, Stat Experience, current HP, status, and stored live stats

## Build Paths

- CMake:
  - `cmake -S . -B build`
  - `cmake --build build`
  - `ctest --test-dir build --output-on-failure`
- Xcode:
  - open `Pkmn Red Save Generator.xcodeproj`
  - or build from the command line with code signing disabled for local CLI builds

## Next Milestones

- Milestone 5: PC storage and current-box cache synchronization
- Milestone 6: daycare, Hall of Fame, extended story/event/world state
- Milestone 7: broader semantic-equivalence coverage
- Milestone 8: expanded emulator and save-again validation
- Milestone 9: release hardening

See `docs/PROJECT_ROADMAP.md` for the detailed roadmap.
