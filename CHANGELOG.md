# Changelog

## v1.0.0 - 2026-07-09

First public Pokemon Red Save Generator release.

Implemented:

- semantic `.red.json` to Pokemon Red `.sav` generation using the committed canonical dummy template;
- physical-image isolation;
- deterministic generation;
- trainer/core state, Pokedex, inventory, party, PC storage, Daycare, Hall of Fame, events, scripts, missables, hidden objects, visited towns, and Red's-house safe runtime subset;
- generator-owned permanent boxes, current-box cache synchronization, per-box checksums, Bank 2 checksum, and Bank 3 checksum;
- write provenance and undeclared-overlap detection;
- CLI workflows for generation, validation, inspection, determinism, physical-image isolation, checksum validation, and semantic comparison;
- public samples and CI smoke coverage;
- final public/private emulator validation candidates, post-save Save Genie reparses, and checksum validation.

Known limitations:

- generated saves target gameplay-equivalent semantic reconstruction, not byte identity;
- generated start locations currently fail closed to the emulator-validated Red's-house profile;
- public CI does not include emulator execution or private Save Genie oracle fixtures.
