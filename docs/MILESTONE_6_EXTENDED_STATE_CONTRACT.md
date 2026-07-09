# Milestone 6 Extended State Contract

Status: complete for the controlled Red's-house extended-state surface. The current code writes Daycare, Hall of Fame, named events, trainer/static/story flags, missables, scripts, hidden objects, visited towns, named story-evidence/world bits, and a narrow runtime subset. The Milestone 6 candidate passed Save Genie reparse, field-aware semantic comparison, emulator load, movement/menu checks, Hall of Fame viewing, normal gameplay travel, save-again, post-save reparse, checksum validation, and post-save drift analysis.

## Owned Extended Systems

- daycare occupancy and stored Pokemon
- Hall of Fame record count and entries
- named event flags
- trainer-battle flags
- static-battle flags
- story-progress flags
- persistent script bytes exposed by Save Genie
- missable objects
- hidden items
- hidden coins
- visited towns / Fly destinations
- named story-evidence/world bits exposed by Save Genie
- verified runtime subset required for safe supported locations

## Event-State Authority

The generator writes event-state from named semantic fields, not from opaque raw event-byte copies.

Current merged authority:

1. `decoded.events.flags`
2. `decoded.trainerBattles.records`
3. `decoded.staticBattles.records`
4. `decoded.storyProgress.storyFlags`

If these sources disagree on the same flag index, generation fails conservatively.

## Daycare Rule

- `inUse=false` requires `pokemon=null`
- `inUse=true` requires a valid stored Pokemon
- the deposited boxed substructure level byte is derived from species growth plus experience
- the Daycare-specific trailing level byte is written from the semantic deposited level
- occupied Daycare rejects invalid species, unencodable names, malformed moves, invalid DVs, and levels outside `1..100`

## Hall Of Fame Rule

- entry count must match the number of serialized entries
- each entry may contain at most 6 Pokemon
- party order must remain sequential and 1-based
- unused entry space is zeroed canonically

## Named Event And Script Rule

- missable objects must provide the complete 228-entry named list
- script state must provide the complete 97-entry named script list
- script byte ranges must not overlap
- the event-flag byte range is cleared before named flags are written
- missable and script ranges are cleared before semantic entries are written
- unknown unnamed event bits are canonicalized to clear until a named semantic authority exists
- named story-evidence/world bits currently include rod ownership, Saffron guard state, Lapras gift state, starter state, healing state, and Lorelei room state

## Location Safety Rule

The current location validator is fail-closed and whitelist-based. Enabled supported states are:

- Red's house, second floor

Unsupported map and coordinate combinations fail rather than being guessed.

Viridian City Pokemon Center is disabled after the load-time corruption incident. It may be re-enabled only after the generator owns and synchronizes the full map-runtime cluster required by the game at load time.

## Save-Again Validation Result

The emulator-modified post-save file parsed successfully. The observed gameplay drift was expected:

- location changed from Red's house second floor to Viridian City Pokemon Center
- playtime increased
- normal runtime/cache bytes changed as the game saved after travel

Generated saves must synchronize cache and permanent selected box. Emulator-modified saves may legitimately set the selected-box dirty flag; cache divergence is accepted only when explicitly validated against the game's current-box mechanics.

Post-save checks:

- main checksum valid
- Bank 2 all-box checksum valid
- Bank 3 all-box checksum valid
- all 12 per-box checksums valid
- Daycare unchanged and empty
- Hall of Fame retained exactly 18 entries with the same ordering and contents
- hidden items, hidden coins, missables, visited towns, trainer battles, static battles, story progress, named events, and scripts matched the generated candidate exactly
- `worldState` drift was limited to map-runtime fields expected from travel to Viridian City Pokemon Center
- numeric trainer ID remained `257`, represented by raw bytes `0x0101`; the game displays this as the five-digit value `00257`

## Current Milestone 6 Gaps

- broader safe-location coverage beyond the Red's-house baseline
- stronger duplicate/mirror documentation for every persistent runtime-adjacent field
- complete map-runtime serialization for non-baseline locations
- broader emulator interaction coverage for Daycare deposit/withdraw and multiple supported maps
