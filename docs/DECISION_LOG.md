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

### D-009: Keep Xcode and CMake as equal Milestone 1 build paths

- Decision: preserve the Xcode CLI target and add a top-level CMake build
- Reason: the project needs a native Apple development path plus an auditable command-line test path

### D-010: Strip `physicalImage` before semantic-state construction

- Decision: remove the target JSON `physicalImage` from the document before building `RedSemanticState`
- Reason: complete isolation is easier to prove when low-level generator code never receives the raw image field at all

### D-011: Treat `0xFF`-filled permanent box leading bytes as suspicious template state

- Decision: baseline analysis flags all-`0xFF` permanent box leading bytes as suspicious, alongside the previously documented decode-level concerns
- Reason: the committed dummy must not be silently trusted when permanent storage appears stale, uninitialized, or semantically inconsistent

### D-012: Milestone 2 preserves permanent box banks as canonical unused state

- Decision: Milestone 2 uses Policy A for the committed 9-second dummy and preserves the permanent PC box banks and bank 2 and 3 all-box checksum bytes unchanged in generated working copies
- Reason: Save Genie source, `pret/pokered` save flow research, and direct template inspection support treating these ranges as canonical unused early-game state while storage serialization remains out of scope

### D-013: Milestone 2 supports one verified safe baseline location only

- Decision: Milestone 2 generation accepts only the verified baseline Red's house second-floor location from semantic input
- Reason: minimal generation must stay within locations proven safe by the committed dummy, Save Genie decode, and planned emulator validation rather than guessing arbitrary coordinates
