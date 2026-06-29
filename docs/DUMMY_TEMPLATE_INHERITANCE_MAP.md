# Dummy Template Inheritance Map

Status: Milestone 2 has refined the minimal-generation ranges below. Storage, party serialization beyond the empty canonical state, and broader event coverage remain pending future refinement.

Purpose:

- answer which output ranges should come from target semantics
- answer which ranges may remain inherited from the canonical dummy
- make hidden template dependence visible

## Category Meanings

- overwritten from target semantic input
- derived from target semantic input
- synchronized duplicate/cache data
- regenerated checksum
- retained canonical dummy default
- intentionally cleared
- unsupported
- unknown but inherited
- runtime/scratch inherited
- fixed format constant

## Provisional Range Map

| Offset range | Size | Purpose | Generation category | Source of value | Evidence | Risk | Validation method |
|---|---:|---|---|---|---|---|---|
| `0x0000-0x0497` | 1176 | Bank 0 sprite/runtime scratch buffers | runtime/scratch inherited | dummy template | Save Genie coverage map classifies as runtime scratch. | Medium; behavior not fully decoded. | Reparse plus emulator save-again observation. |
| `0x0498-0x0597` | 256 | Bank 0 unused pre-Hall-of-Fame block | retained canonical dummy default | dummy template | Save Genie coverage classifies as unused. | Low to medium. | Hash-stable template audit and emulator sanity. |
| `0x0598-0x1857` | 4800 | Hall of Fame records | overwritten from target semantic input | target `decoded.hallOfFame` or intentional clear policy | Save Genie layout and schema expose Hall of Fame directly. | High if inherited because dummy may contain meaningful history in other templates. | Reparse and semantic comparison. |
| `0x1858-0x1FFF` | 1960 | Bank 0 unused post-Hall-of-Fame block | retained canonical dummy default | dummy template | Save Genie coverage classifies as unused. | Low to medium. | Emulator save-again and difference reporting. |
| `0x2000-0x2597` | 1432 | Bank 1 lead-in runtime region | runtime/scratch inherited | dummy template unless a subrange is proven necessary | Save Genie coverage classifies as runtime-heavy. | Medium to high if later proven load-sensitive. | Emulator load/save-again and targeted diff study. |
| `0x2598-0x29F2` | 1115 | Core Bank 1 semantic region before event flags split | overwritten from target semantic input | target `decoded.trainer`, `rival`, `moneyAndCoins`, `options`, `location`, `badges`, `pokedex`, `inventory`, and related fields | Save Genie layout places these fields in checksum-covered Bank 1. | High; dummy identity and PC item state already present. | Reparse and semantic comparison. |
| `0x29F3-0x2CF3` | 769 | Event/story/world-state region | overwritten from target semantic input | target `decoded.events`, `storyProgress`, `trainerBattles`, `staticBattles`, `visitedTowns`, `hiddenItems`, `hiddenCoins`, `scripts`, `worldState` | Save Genie docs and schema expose these as decoded semantic regions. | High; contradictions can create impossible states. | Semantic comparison and emulator safety tests. |
| `0x2CF4-0x300B` | 792 | Daycare and adjacent persistent gameplay state | overwritten from target semantic input | target `decoded.daycare` plus explicit clear policy | Save Genie layout exposes daycare offsets and tests validate decode. | High if inherited or half-written. | Reparse and focused daycare tests. |
| `0x300C-0x30BF` | 180 | Party data lead-in and party structures | overwritten from target semantic input | target `decoded.party` | Save Genie layout, tests, and schema expose party fields. | High; malformed party data can corrupt saves. | Reparse and structural validation. |
| `0x30C0-0x3521` | 1122 | Current box cache | synchronized duplicate/cache data | derived from target storage semantics and selected current box | Save Genie layout exposes current-box cache as distinct from permanent boxes. | Very high; stale cache can disagree with permanent boxes. | Reparse current-box cache and compare against permanent storage. |
| `0x3522-0x3522` | 1 | Final byte inside main checksum-covered range | synchronized duplicate/cache data | depends on finalized subrange audit | Save Genie main checksum end is `0x3522`. | Medium until subrange audit completes. | Byte-range audit and reparse. |
| `0x3523-0x3523` | 1 | Main checksum byte | regenerated checksum | computed from working buffer | Save Genie checksum rules and tests. | Critical if wrong. | Independent checksum validator and reparse. |
| `0x3524-0x3FFF` | 2780 | Bank 1 post-checksum unknown/raw-preserved region | unknown but inherited | dummy template | Save Genie coverage marks this as unknown/raw-preserved. | Medium to high; meaning not fully known. | Emulator save-again diff and targeted future research. |
| `0x4000-0x5A4B` | 6732 | Permanent PC Boxes 1-6 payload | overwritten from target semantic input | target `decoded.pcStorage` | Save Genie layout and schema expose all 12 boxes. | Critical; dummy box payload is currently invalid-looking. | Reparse box contents and checksum validation. |
| `0x5A4C-0x5A52` | 7 | Bank 2 all-box and per-box checksums | regenerated checksum | computed from generated box payload | Save Genie layout and docs list Bank 2 checksum locations. | Critical. | Independent checksum validator and reparse. |
| `0x5A53-0x5FFF` | 1453 | Bank 2 unknown tail | unknown but inherited | dummy template | Save Genie coverage classifies as unknown remainder. | Medium. | Future audit plus emulator save-again. |
| `0x6000-0x7A4B` | 6732 | Permanent PC Boxes 7-12 payload | overwritten from target semantic input | target `decoded.pcStorage` | Save Genie layout and schema expose all 12 boxes. | Critical; dummy box payload is currently invalid-looking. | Reparse box contents and checksum validation. |
| `0x7A4C-0x7A52` | 7 | Bank 3 all-box and per-box checksums | regenerated checksum | computed from generated box payload | Save Genie layout and docs list Bank 3 checksum locations. | Critical. | Independent checksum validator and reparse. |
| `0x7A53-0x7FFF` | 1453 | Bank 3 unknown tail | unknown but inherited | dummy template | Save Genie coverage classifies as unknown remainder. | Medium. | Future audit plus emulator save-again. |

## Notes

- Milestone 2 exact write and clear ranges are now:
  - overwritten from target semantic input:
    - `0x2598-0x25A2` player name
    - `0x25F3-0x25F5` money
    - `0x25F6-0x2600` rival name
    - `0x2601` options
    - `0x2604` letter delay
    - `0x2605-0x2606` trainer ID
    - `0x2609` contrast
    - `0x260A-0x2610` safe location and coordinates
    - `0x2850-0x2851` coins
    - `0x2CED-0x2CF1` playtime
  - intentionally cleared to canonical minimal defaults:
    - `0x0598-0x1857` Hall of Fame block
    - `0x25A3-0x25B5` Pokédex owned bitfield
    - `0x25B6-0x25C8` Pokédex seen bitfield
    - `0x25C9-0x25F2` bag inventory
    - `0x2602` badges
    - `0x27E6-0x284B` PC item inventory
    - `0x284E` Hall of Fame entry count
    - `0x29D6` badge mirror
    - `0x2CF4-0x2D2B` daycare block
    - `0x2F2C-0x30BF` party block
  - regenerated checksum:
    - `0x3523` main checksum
  - preserved canonical inherited state under Policy A:
    - `0x30C0-0x3521` current box cache
    - `0x4000-0x5A52` permanent box banks 1-6 plus stored checksum bytes
    - `0x6000-0x7A52` permanent box banks 7-12 plus stored checksum bytes

- The current dummy's invalid permanent box contents make the box-related rows the highest-risk inherited areas in the entire template.
- No final generator should ship without making the overwritten, derived, synchronized, regenerated, and inherited ranges explicit in its generation report.
