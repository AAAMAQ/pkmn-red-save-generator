# Decision Log

## 2026-06-24

### D-001: Initialize a dedicated generator repository

- Decision: initialize Git at the generator repository root
- Reason: the generator must be a separate project from Save Genie

### D-002: Preserve the Xcode skeleton during Milestone 0

- Decision: keep the existing Xcode project and placeholder `main.cpp` unchanged for now
- Reason: repository setup and audit work take priority over build-system reorganization

### D-003: Treat target `.red.json` `physicalImage` as forbidden

- Decision: generation authority will come from semantic fields only
- Reason: this project must prove semantic reconstruction, not raw archival reconstruction

### D-004: Treat the standalone dummy `.sav` as an allowed template candidate, not as forbidden `physicalImage`

- Decision: the dummy `.sav` may be used as a template resource after validation
- Reason: the prohibition applies to the raw image embedded in the target `.red.json`, not to an audited standalone template file

### D-005: Treat the dummy `.red.json` and companion summaries as reference-only

- Decision: the dummy `.red.json`, `PokemonSummary.json`, `PokemonBoxes.json`, and `SaveGenieSummary.txt` are audit material only
- Reason: the generator must not reconstruct template bytes indirectly from those files

### D-006: Do not reorganize `rescource/` during Milestone 0

- Decision: leave the existing reference-material folder untouched for now
- Reason: safe reorganization can happen later once the build layout is formalized

### D-007: Mark license status as pending

- Decision: add `LICENSE-PENDING.md` instead of selecting a final license prematurely
- Reason: neither repository currently exposes a clear top-level license file, and reuse policy should stay explicit

### D-008: Record that the current dummy is conditionally usable at best

- Decision: document the dummy as unsafe for unqualified full-save inheritance
- Reason: invalid box-bank checksums and full invalid-looking permanent boxes create high contamination risk
