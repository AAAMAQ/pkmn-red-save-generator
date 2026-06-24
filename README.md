# Pkmn Red Save Generator

Status: Milestone 0 repository initialization and audit only.

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
- Existing Xcode project preserved in place.
- No generator serializer has been implemented yet.
- Milestone 0 documentation, audit notes, and safety boundaries are tracked under `docs/`.

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
  - Existing Xcode source folder, currently only a placeholder `main.cpp`
- `Pkmn Red Save Generator.xcodeproj/`
  - Existing Xcode project metadata
- `rescource/`
  - Preserved reference material from earlier research; left untouched during Milestone 0
- `docs/`
  - Milestone 0 scope, audit, policy, and roadmap documents

## Next Milestones

- Milestone 1: input model, physical-image isolation, primitive encoders, template loader
- Milestone 2: minimal valid generated save
- Milestone 3+: progressive semantic coverage, independent reparse, emulator validation, save-again validation

See `docs/PROJECT_ROADMAP.md` for the detailed roadmap.
