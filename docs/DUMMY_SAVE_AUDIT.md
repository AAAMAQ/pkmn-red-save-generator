# Dummy Save Audit

Audit date: 2026-06-24

Repository-local resource directory:

- `Dummy Save/`

Read-only policy:

- no files in `Dummy Save/` were modified during this audit

## File Inventory

| File | Size | SHA-256 |
|---|---:|---|
| `Dummy Save/Pokemon - Red Version (USA, Europe) (SGB Enhanced).sav` | 32768 bytes | `248bc35328be435b16b47e2bb87c4e9732c2b5c92a95450839ed4619f74eb2e7` |
| `Dummy Save/Pokemon - Red Version (USA, Europe) (SGB Enhanced).red.json` | 1825847 bytes | `78edaed6f9c7fab1b6ff5cad67df5722df730b3529b8d7f7ba7b60a4ba7f3fd1` |
| `Dummy Save/PokemonSummary.json` | 146042 bytes | `c4a6e351053a1504a2761734b3fafe0ec08910bf02fd7b46ddc6573ec7ad793e` |
| `Dummy Save/PokemonBoxes.json` | 231721 bytes | `dd41cd7b9bec702da3f3ed0a5f0c5af6e54cf4131492172b89d7b17d4669c9c6` |
| `Dummy Save/SaveGenieSummary.txt` | 101706 bytes | `689916d0b84c04ae199bd256b92d2f150d11d01918322c57b5d62251e6bb4b6e` |

## Dummy `.sav` Audit

Exact path:

- `Dummy Save/Pokemon - Red Version (USA, Europe) (SGB Enhanced).sav`

Observed facts:

- expected game from companion metadata: `Pokemon Red`
- expected region from companion metadata: `USA-Europe`
- revision: unknown from save structure alone
- SRAM size: `0x8000` / 32768 bytes
- trailing bytes: none
- main checksum: valid
- bank 2 all-box checksum: invalid
- bank 3 all-box checksum: invalid

Save Genie-backed validation result:

- A temporary read-only validator was compiled against Save Genie source in `/tmp`.
- The validator loaded the dummy `.sav` through Save Genie classes and reported:
  - size `32768`
  - `main_valid=true`
  - `bank2_valid=false`
  - `bank3_valid=false`
  - trainer `RED`
  - rival `BLUE`
  - trainer ID `60066`
  - money `3000`
  - coins `0`
  - map ID `38`
  - coordinates `x=3`, `y=6`
  - playtime `0:0:9`
  - party count `0`
  - current box cache count `0`
  - permanent boxes 1-12 each decode as `count=20`

Semantic baseline:

- trainer name: `RED`
- rival name: `BLUE`
- trainer ID: `60066`
- money: `3000`
- coins: `0`
- badges owned: none
- playtime: `0h 0m 9s`
- map: `Red's house (second floor)` (`MapID 38`, `0x26`)
- party count: `0`
- current box cache: empty
- occupied permanent boxes: all 12 reported occupied
- Pokedex owned/seen: `0 / 0`
- bag inventory: empty
- PC item storage: `POTION x1`
- daycare: empty
- Hall of Fame entries: `0`
- visited towns count: `11`
- event flags set: `0`
- story flags completed: `0`

Minimal-baseline assessment:

- This is not a truly minimal blank save.
- It contains meaningful semantic state and suspicious storage state.
- It should not be described as semantically empty.

## Dummy `.red.json` Audit

Exact path:

- `Dummy Save/Pokemon - Red Version (USA, Europe) (SGB Enhanced).red.json`

Observed facts:

- schema format: `pkmn-red-master-save`
- schema version: `0.1.0`
- game: `Pokemon Red`
- region assumption: `USA-Europe`
- physical image encoding: `hex_uppercase_continuous`
- physical image standard SRAM length: `32768`
- physical image total length: `32768`
- conversion model present: yes
- source whole-file SHA-256: `248BC35328BE435B16B47E2BB87C4E9732C2B5C92A95450839ED4619F74EB2E7`
- coverage summary:
  - total standard SRAM bytes: `32768`
  - uncovered bytes: `0`
  - overlapping primary bytes: `0`
- decoded sections present:
  - trainer, rival, options, playtime, moneyAndCoins, badges, location, runtimeState, pokedex, inventory, party, pcStorage, currentBoxCache, daycare, hallOfFame, events, trainerBattles, staticBattles, storyProgress, scripts, missableObjects, hiddenItems, hiddenCoins, visitedTowns, worldState, checksums, semanticConsistency

Consistency with dummy `.sav`:

- source hash matches the actual dummy `.sav`
- trainer, rival, money, coins, location, playtime, party count, and checksum state match the Save Genie text summary and temporary validator results
- permanent storage also matches the companion summaries: all 12 boxes report `count=20`

Important generation rule:

- this `.red.json` is reference material only
- its `physicalImage` must not be used as a hidden byte source for template loading

## `PokemonSummary.json` Audit

Purpose:

- compact save-level summary for trainer, playtime, location, inventory, party, Pokedex, daycare, checksums, and PC box counts

Observed facts:

- trainer: `RED`
- rival: `BLUE`
- trainer ID: `60066`
- money: `3000`
- coins: `0`
- playtime: `0:0:9`
- map: `Red's house (second floor)`
- bag count: `0`
- PC item count: `1`
- party count: `0`
- PC box counts: `[20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20]`
- current box cache count: `0`
- main checksum valid: `true`
- bank 2 all valid: `false`
- bank 3 all valid: `false`

Consistency:

- matches the dummy `.red.json` on the audited core values
- agrees with `PokemonBoxes.json` about the box counts and current box cache count

## `PokemonBoxes.json` Audit

Purpose:

- detailed storage-focused summary for party, permanent boxes, and current box cache

Observed facts:

- party (`boxNumber 0`) count: `0`
- current box cache count: `0`
- permanent boxes 1-12 count: `20` each
- first visible entries decode as obviously invalid-looking filler:
  - species ID `255`
  - nickname `99999999999`
  - OT name `99999999999`
  - OT ID `65535`
  - level `255`
  - EXP `16777215`

Consistency:

- matches the dummy `.red.json` and temporary validator on box occupancy counts
- strongly suggests the dummy permanent-box regions are not a safe inherited player-state baseline

## `SaveGenieSummary.txt` Audit

Purpose:

- verbose human-readable save summary generated by Save Genie

Observed facts:

- confirms trainer identity, location, playtime, badges, Pokedex, and checksum state
- confirms current box cache count `0`
- confirms PC item storage contains `POTION x1`
- notes bank 2 and bank 3 all-box checksum invalidity
- reports `wFossilItem = INVALID (0x00)` and `wFossilMon = INVALID (0x00)` in the runtime/world-state section

Consistency:

- broadly consistent with the dummy `.red.json`, `PokemonSummary.json`, and `PokemonBoxes.json`

## Supported Game/Version Profile

From the audited materials, the dummy appears tied to:

- game: `Pokemon Red`
- region assumption: `USA-Europe`
- size: standard `0x8000` SRAM

Revision remains unknown. The filename mentions `SGB Enhanced`, but that alone is not treated as sufficient proof of target profile.

## Known Limitations

- bank 2 and bank 3 all-box checksums are invalid
- permanent box storage decodes as full of invalid-looking entries
- the dummy is not semantically blank
- some runtime fields remain only partially understood

## Suitability Assessment

Milestone 0 conclusion:

- The dummy is not suitable as an unqualified full-save canonical template for final generation output.
- It may be conditionally usable as a tightly controlled initialization base only if:
  - permanent box data is fully overwritten or intentionally cleared before claiming storage support
  - current-box cache is regenerated deliberately
  - all affected bank and per-box checksums are regenerated
  - inherited unknown/runtime ranges are documented explicitly

If a cleaner canonical template becomes available later, it should be evaluated openly rather than swapped silently.

## Sensitive Or Personal Data

No obvious personal real-world data was found.

The file still contains meaningful gameplay identity and progression-like values that must not leak into generated output:

- player name
- rival name
- trainer ID
- money
- location
- PC item storage
- visited towns
- permanent box contents

## Mandatory Overwrite Or Clear Targets

At minimum, any generator using this template must not allow the following dummy state to leak into covered output:

- trainer identity
- money and coins
- badges
- location and safe coordinate state
- Pokedex seen and owned state
- bag inventory
- PC item inventory
- party count and party data
- all 12 permanent boxes
- current box cache
- daycare
- Hall of Fame
- visited towns
- supported event, story, trainer-battle, static-battle, and world-state values
