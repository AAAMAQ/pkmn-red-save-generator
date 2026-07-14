# Prerequisite Source Concordance

This document records which Save Genie sources informed Milestone 0 planning for the generator. The prerequisite repository was inspected read-only only.

## License And Reuse Status

- Save Genie is our trusted prerequisite codebase.
- Save Genie has a top-level standard MIT License credited to `MAQ / BiG MAQ Studios`, with the same non-binding stewardship request used by this repository.
- Reuse inside this repository should remain traceable even when code is ported or rewritten rather than copied directly.
- Third-party references remain supporting evidence, not automatic authority, and retain their own licenses.

## Reviewed Prerequisite Sources

| Source | Type | Key finding | Generator use | Reuse mode |
|---|---|---|---|---|
| `Pkmn Red Save Genie/README.md` | overview | Confirms project purpose, architecture, coverage philosophy, and current-box/checksum support. | Boundary setting, terminology, high-level architecture. | Reference |
| `Pkmn Red Save Genie/HPP Files/SaveStructure.hpp` | source header | Central offset truth for SRAM layout, party, current box cache, permanent boxes, checksums, text, BCD. | Primary structural reference for generator-owned serializers. | Port or rewrite |
| `Pkmn Red Save Genie/CPP Files/SaveStructure.cpp` | source impl | Safe byte access, low-level codecs, checksum routines, layout helpers. | Algorithm reference for primitive encoding and integrity logic. | Port or rewrite |
| `Pkmn Red Save Genie/HPP Files/ReadOnlyData.hpp` | source header | Reader-facing semantic model and high-level summaries. | Helps define semantic comparison targets and audit vocabulary. | Reference and selective port |
| `Pkmn Red Save Genie/CPP Files/ReadOnlyData.cpp` | source impl | Decode behavior for trainer, location, party, boxes, events, world state, daycare, Hall of Fame. | Independent reparse oracle and serializer target reference. | Reference |
| `Pkmn Red Save Genie/HPP Files/WriteOnlyData.hpp` | source header | Safe mutation boundaries, validation model, checksum repair expectations. | Safety and validation design reference. | Reference |
| `Pkmn Red Save Genie/CPP Files/WriteOnlyData.cpp` | source impl | Demonstrates strict text validation, BCD writes, badge mirror handling, item-list mutation patterns. | Serializer safety and field validation reference. | Selective port or rewrite |
| `Pkmn Red Save Genie/HPP Files/RedMasterJson.hpp` | source header | Defines the lossless `.red.json` system and preservation-focused coverage model. | Clarifies what must not be reused for semantic generation. | Reference |
| `tests/savegenie_core_tests.cpp` | tests | Covers BCD, text codec, checksum repair, party decode, world state, daycare, validation patterns. | Test design inspiration for primitive and structural checks. | Reference |
| `tests/red_master_json_tests.cpp` | tests | Confirms `.red.json` hierarchy, deterministic export mode, and reconstruction flow. | Helps define schema assumptions and forbidden raw-image path. | Reference |
| `docs/schema/pokemon-red-master-save.schema.json` | schema | Enumerates required top-level sections and decoded hierarchy for schema `0.1.0`. | Input compatibility and field inventory. | Reference |
| `docs/RED_SAVE_COVERAGE.md` | doc | Full `0x0000-0x7FFF` coverage model, including runtime and unknown classifications. | Basis for inheritance-map structure and canonicalization policy. | Reference |
| `docs/research/PKMN_RED_SAVE_GENIE_RESEARCH_DOCUMENT.md` | doc | Consolidates offsets, checksum facts, coverage milestones, remaining questions, and safety principles. | Evidence for supported profile, risks, and roadmap. | Reference |
| `docs/external_projects_deep_analysis.md` | doc | Summarizes third-party reference value and specific current-box/checksum findings. | Secondary evidence and cautionary notes. | Reference |
| `docs/research/junebug_save_editors_analysis_findings.md` | doc | Highlights current-box semantics, bank checksum notes, strict text-policy lessons, and reuse caveats. | Helps avoid known third-party pitfalls. | Reference |

## External Third-Party Sources Noted By Save Genie

These were not treated as primary authority during Milestone 0, but they remain useful supporting evidence:

- `pret/pokered`
- `junebug12851/pokered-save-editor`
- `junebug12851/pokered-save-editor-2`

Any later direct code reuse from those sources must be preceded by license review and explicit traceability.
