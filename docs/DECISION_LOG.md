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

- Status: superseded by D-038
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

- Status: superseded by D-041 after completed-playthrough emulator evidence
- Decision: `decoded.pcStorage` plus `decoded.currentBoxCache.selectedBoxNumber` define storage output, and the current-box cache is rewritten from the selected permanent box
- Reason: this prevents stale or dummy cache bytes from surviving in generated output

### D-021: Canonicalize boxed storage level from experience when oracle decode is implausible

- Status: superseded after the boxed-record parser was corrected; stored level at `+0x03` is now preserved
- Decision: boxed storage `level` may be treated as a permitted canonical difference when Save Genie reports implausible source values
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

### D-029: Keep Milestone 6 Red's-house only while enabling extended-state writes

- Decision: Milestone 6 writes Daycare, Hall of Fame, complete named missables, complete named scripts, named event/story/trainer/static flags, hidden objects, visited towns, and the narrow Red's-house runtime subset, but still rejects raw non-baseline locations
- Reason: this expands semantic coverage without reintroducing the Viridian Pokemon Center load-corruption path; emulator testing remains required before declaring Milestone 6 complete

### D-030: Write Daycare's trailing level byte from semantic input

- Decision: the Daycare deposited-mon boxed substructure level is derived from experience, while the Daycare-specific trailing level byte is written from the semantic deposited level
- Reason: Save Genie coverage and tests show the Daycare range extends through `0x2D2C`, so the generator must not omit or template-inherit the extra level byte

## 2026-07-09

### D-031: Close Milestone 6 for the Red's-house safe-location profile

- Decision: accept Milestone 6 extended gameplay-state generation as complete for the emulator-validated Red's-house profile
- Reason: the generated extended-state save passed Save Genie reparse, field-aware comparison, physical-image isolation, determinism, emulator load, movement/menu checks, travel, Hall of Fame viewing, save-again, post-save reparse, and checksum validation

### D-032: Treat dirty selected-box cache as valid post-gameplay state only

- Status: superseded by D-041; valid divergent working-box state can already exist in source semantic input
- Decision: generated saves must synchronize the selected permanent box and current-box cache, but validation of emulator-modified saves may accept a dirty current-box cache when the selected-box dirty flag is set
- Reason: the Milestone 6 post-save artifact showed `PEGGY` / `PIDGEY` in the Box 11 current-box cache while permanent Box 11 remained empty, with all per-box and bank checksums valid; this is coherent Gen I behavior after deposit without box switching

### D-033: Record trainer ID as numeric 257

- Decision: the Milestone 6 fixture's authoritative trainer ID is decimal `257` (`0x0101`), displayed by the game as a five-digit value
- Reason: generated candidate, post-save Save Genie reparse, trainer record, and private screenshot evidence all confirm `257` / `00257`; the earlier `275` / `00275` note was a visual misread rather than a generator mismatch

### D-034: Merge original Milestones 7-9 into one Final Release Milestone

- Decision: treat semantic-equivalence expansion, broad emulator validation, release hardening, CI, versioning, samples, documentation, and publication readiness as one coordinated final release program
- Reason: the core generator is implemented through Milestone 6, and remaining work is primarily proof, hardening, and release discipline rather than new serializer architecture

### D-035: Publish only synthetic public sample inputs

- Decision: public samples are synthetic `.red.json` files derived from the committed dummy reference, stripped of `physicalImage`, and canonicalized into safe semantic examples
- Reason: private saves and screenshots must remain local-only; public CI still needs reproducible generation inputs that exercise supported workflows

### D-036: Do not tag v1.0.0 until the final emulator matrix passes

- Decision: CLI version and release docs may identify the candidate as `1.0.0`, but the Git tag must wait for targeted emulator candidates and private oracle validation
- Reason: the Milestone 5-6 corruption incident proved parser acceptance, checksums, and semantic comparison are insufficient without emulator load/save-again evidence

## 2026-07-13

### D-037: Canonicalize unsupported source locations

- Decision: unsupported source locations are accepted by canonicalizing the generated start location to Red's house second floor and emitting an explicit warning
- Reason: the semantic-sufficiency proof should preserve supported gameplay state without pretending unsupported runtime map clusters are safe; Red's house second floor is the only emulator-proven generated start location
- Consequence: location preservation is not currently part of the supported semantic-equivalence claim for non-baseline source saves

### D-038: Adopt MIT License

- Decision: replace `LICENSE-PENDING.md` with the standard MIT License, credited to `MAQ / BiG MAQ Studios`, and include a clearly non-binding project-stewardship request
- Reason: the owner approved broad MIT licensing while asking users to keep the work focused on education, research, archival preservation, and retro-development rather than merely repackaging it for sale
- Consequence: all MIT permissions remain legally unchanged, including commercial permissions; the stewardship wording is a personal request rather than an additional license restriction. Pokemon-related intellectual property remains owned by its respective rights holders and is not distributed by this project

### D-039: Close the combined Final Release Milestone

- Decision: accept the combined original Milestones 7-9 release milestone after final targeted emulator validation, post-save Save Genie reparses, checksum validation, deterministic-output proof, physical-image isolation proof, public sample validation, and fresh-copy build validation
- Reason: both final emulator candidates loaded and saved normally, post-save SRAM files reparsed with valid checksums, unsupported locations remain fail-closed, and the corrected trainer-ID evidence resolves the final ambiguity

## 2026-07-14

### D-040: Invalidate the first semantic-sufficiency acceptance result

- Decision: retain the first automated reports as historical evidence, but withdraw their success conclusion after manual emulator testing exposed text loss, missing player-visible current-box state, zero-HP withdrawal, and malformed Hall of Fame teams
- Reason: checksum validity, Save Genie reparse, and a comparator built on the same model did not independently prove operational gameplay equivalence

### D-041: Preserve the Bank 1 current working box independently

- Decision: supersede D-020 and the generated-save restriction in D-032; semantic generation now writes the permanent boxes and `decoded.currentBoxCache.cache` as separate structures and preserves the selected-box dirty bit
- Reason: the completed source fixture has selected byte `0x8B`, an empty permanent Box 12, and a populated 20-Pokemon Bank 1 working Box 12. Canonicalizing the cache from permanent Box 12 erased player-visible state

### D-042: Use lossless tokens for ambiguous Generation I text bytes

- Decision: expose a lossless text representation alongside display text and encode tokens such as `<DOT>`, `<PERIOD>`, and `<0xHH>` without fallback substitution
- Reason: source byte `0xF2` renders as a period but was previously decoded as `?` and regenerated as a different question-mark byte

### D-043: Require operational validators independent of parser symmetry

- Decision: generation and comparison must validate box boundaries, withdrawal viability, internal species mapping, complete Hall of Fame slots, text termination, and write provenance in addition to checksums and Save Genie reparse
- Reason: the first parser and generator shared the same boxed-record and Hall of Fame species assumptions, allowing malformed gameplay behavior to compare as equal

### D-044: Keep the corrected proof provisional until a second emulator pass

- Decision: the corrected automated result may be reported only as awaiting manual retest; do not restore the semantic-sufficiency conclusion or publish a new release result until the corrected save passes the focused emulator and post-save gates
- Reason: the four corrected defects are operational and require direct game verification
