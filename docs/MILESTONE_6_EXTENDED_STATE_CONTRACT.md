# Milestone 6 Extended State Contract

Status: paused. The current extended-state surface passed private-fixture Save Genie reparse plus semantic comparison, but full Milestone 6 progression is stopped until the Milestone 5-6 load-time corruption incident is resolved through emulator validation.

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
- stored daycare level is currently canonicalized from species growth plus experience, matching the boxed-Pokemon policy

## Hall Of Fame Rule

- entry count must match the number of serialized entries
- each entry may contain at most 6 Pokemon
- party order must remain sequential and 1-based
- unused entry space is zeroed canonically

## Location Safety Rule

The current location validator is fail-closed and whitelist-based. Enabled supported states are:

- Red's house, second floor

Unsupported map and coordinate combinations fail rather than being guessed.

Viridian City Pokemon Center is disabled after the load-time corruption incident. It may be re-enabled only after the generator owns and synchronizes the full map-runtime cluster required by the game at load time.

## Current Milestone 6 Gaps

- broader safe-location coverage beyond the Red's-house baseline
- emulator validation for Daycare interaction, Hall of Fame viewing, and multi-map navigation
- stronger duplicate/mirror documentation for every persistent runtime-adjacent field
- complete map-runtime serialization for non-baseline locations
