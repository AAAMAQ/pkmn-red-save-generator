# Project Scope

## Purpose

Pkmn Red Save Generator is a separate repository whose job is to synthesize a new, valid Pokemon Red `.sav` from semantic `.red.json` input.

The generator is not an archival reconstructor. It must not use the raw `physicalImage` embedded in the target `.red.json`.

## Repository Boundary

- Active writable repository:
  - this repository root
- Read-only prerequisite:
  - sibling `Pkmn Red Save Genie` repository

Milestone 0 rule:

- inspect Save Genie freely as a technical reference
- do not modify, stage, or commit anything in Save Genie
- do not modify unrelated folders outside this repository

## In Scope

- repository initialization
- dummy-save audit
- prerequisite audit
- architecture proposal
- input-field contract
- canonicalization policy
- semantic equivalence proposal
- validation strategy
- implementation roadmap

## Explicitly Out Of Scope For Milestone 0

- full save serialization
- template mutation engine
- emulator automation
- save-again workflow
- FireRed implementation
- changes to Save Genie

## Generator Success Criteria

The finished generator must eventually satisfy:

1. Read target `.red.json`.
2. Ignore target `physicalImage`.
3. Build output from semantic data and approved deterministic rules.
4. Regenerate duplicated/cache/checksum structures.
5. Reparse output through Save Genie.
6. Verify semantic equivalence for covered fields.
7. Load in emulator.
8. Save again in-game.
9. Reparse and verify semantic equivalence again.

## Initial Risk Register

| Risk | Why it matters | Initial mitigation |
|---|---|---|
| Dummy-state contamination | Template values may leak into generated output. | Track every inherited range, add deliberately-different contamination fixtures, compare reparsed output against target semantics. |
| Target `physicalImage` leakage | Hidden archival reconstruction would violate the project contract. | Treat target `physicalImage` as forbidden input, add output-invariance tests with missing/corrupted/replaced raw image. |
| Checksum mistakes | Saves may appear to generate but fail to load or save. | Centralize checksum logic, add unit tests, independent reparse, and emulator validation. |
| Current-box cache divergence | Permanent boxes and cache can disagree, causing stale or unsafe output. | Define one canonical semantic source, always synchronize both representations, reparse both after generation. |
| Incomplete overwrite coverage | Template inheritance can hide unsupported serialization. | Maintain byte-range inheritance map and report overwritten versus inherited ranges for every generation. |
| Unsupported event combinations | Some semantic combinations may be impossible or unsafe in-game. | Validate target combinations, reject or warn on contradictory states, keep unsupported combinations explicit. |
| Unsafe locations | A formally decoded map/location can still be unsafe with mismatched world state. | Start with conservative supported-location policy and add emulator checks before widening support. |
| Unknown-byte defaults | Zeroing or inventing unknown bytes can corrupt saves. | Prefer audited template inheritance or verified fixed constants over guessed defaults. |
| Template compatibility | A wrong template profile can generate structurally wrong saves. | Validate template size, hash, profile, and checksum status before use. |
| Accidental dummy modification | Testing could overwrite the canonical template. | Treat `Dummy Save/` as read-only, test immutability with SHA-256 checks before and after generation runs. |
