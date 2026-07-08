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

### D-014: Milestone 3 keeps Policy A provisionally for permanent box banks

- Decision: Milestone 3 continues preserving the committed dummy permanent storage banks and invalid bank 2 and 3 all-box checksums unchanged
- Reason: emulator load validation passed, but there is still no post-save reparse evidence proving how those untouched storage banks behave after an in-game save cycle

### D-015: Milestone 3 rejects unsupported non-empty deferred states

- Decision: non-empty party, occupied daycare, and non-empty Hall of Fame inputs now fail generation instead of being silently cleared
- Reason: rejecting unsupported semantic state is safer than hiding mismatches behind canonical empty defaults

### D-016: Milestone 3 owns badges, Pokedex, item inventories, and a conservative event subset

- Decision: expand the generator to serialize badges, the badge mirror, Pokedex owned and seen bitfields, bag inventory, PC item inventory, and the visited-towns and hidden-item and hidden-coin bitfields
- Reason: these fields are well mapped by Save Genie research, fit inside the checksum-covered bank 1 semantic region, and materially reduce dummy-state contamination risk without forcing premature party or box serialization

### D-017: Keep Policy A only for the validated no-storage-interaction flow

- Decision: retain Policy A after Milestone 3 for generated saves that preserve permanent PC storage unchanged and have been validated through load and save-again without using PC storage
- Reason: the post-save emulator file remained byte-identical, reparsed cleanly, and preserved owned semantics, but this evidence still does not prove that the untouched permanent boxes are semantically safe once storage interaction itself enters scope

### D-018: Record Pokedex verification as semantic, not direct UI, proof

- Decision: claim Milestone 3 Pokedex verification through Save Genie reparse and semantic comparison only
- Reason: the generated save preserved the owned and seen bitfields, but the in-game Pokedex UI was not directly inspected because broader progression gating remains outside the current event subset

## 2026-07-07

### D-019: Retire Policy A for generated permanent storage

- Decision: generated saves now fully rewrite all 12 permanent boxes, the selected-box cache, and all storage checksums
- Reason: the committed dummy permanent storage is too suspicious to inherit once storage support exists

### D-020: Make the selected permanent box the only storage write authority

- Decision: `decoded.pcStorage` plus `decoded.currentBoxCache.selectedBoxNumber` define storage output, and the current-box cache is rewritten from the selected permanent box
- Reason: this prevents stale or dummy cache bytes from surviving in generated output

### D-021: Canonicalize boxed and daycare stored level from experience

- Decision: boxed and daycare stored `level` are currently derived from species growth rate plus experience during generation
- Reason: the current Save Genie boxed-level decode is not yet trustworthy for every real fixture record, while `experience` remains stable and semantically authoritative

### D-022: Treat emulator load-time corruption as authoritative blocker

- Decision: Milestone 5 is blocked and Milestone 6 is paused after the full generated save corrupted immediately after Continue
- Reason: Save Genie reparse, checksum validation, semantic comparison, and deterministic output did not prove game-load safety

### D-023: Fail closed to the Red's-house baseline location

- Decision: remove the Viridian City Pokemon Center safe-location profile until the full map-runtime cluster is serialized and emulator-proven
- Reason: binary analysis showed the failing save admitted Viridian location bytes while retaining Red's-house/dummy map-runtime pointers, dimensions, warp/object data, and adjacent runtime state

### D-024: Require byte-provenance and overlap diagnostics for generation reports

- Decision: generated reports now include per-range byte provenance and generation fails on undeclared non-template range overlaps
- Reason: the preserved failing report contained broad overlapping ranges that could hide clobbers even when the final checksum was valid

### D-025: Treat Save Genie incident outputs as diagnostic evidence only

- Decision: Save Genie outputs generated from the failing Milestone 5-6 save may be used to investigate decoded structure and map/runtime coherence, but they are not acceptance evidence for emulator safety
- Reason: the exact failing save decoded with valid checksums, valid party/storage summaries, and Viridian location semantics while still corrupting immediately after Continue

### D-026: Treat corrected storage viewing as provisional Milestone 5 evidence

- Decision: the corrected Red's-house storage diagnostic base-load result and PC box-list visual verification are positive Milestone 5 evidence, but not final completion evidence
- Reason: gameplay progression occurred before Bill's PC was reached, so the post-test save includes expected gameplay changes and active current-box-cache state that must be separated from storage defects

### D-027: Compare selected storage through the active current-box cache after gameplay

- Decision: when a post-gameplay save has the current-box changed flag set, selected-box storage comparison must use the active current-box cache as the semantic authority for that selected box until the game-triggered box save or normal save synchronizes it
- Reason: the post-gameplay save decoded permanent box 3 as empty while current-box cache contained the expected 17 Pokemon; a cache-aware comparison found no storage mismatches and matches Gen I's current-box-cache model

## 2026-07-08

### D-028: Close Milestone 5 after final post-withdrawal storage evidence

- Decision: Milestone 5 PC storage generation is accepted as complete after the final emulator-controlled post-withdrawal save reparsed with valid main, Bank 2, Bank 3, and per-box checksums
- Reason: the final artifact proved current Box 12 cache behavior, deposited `PEGGY` in permanent Box 11, withdrawn fainted `RED` in the party, no observed storage corruption, and successful game-triggered plus normal save flows
