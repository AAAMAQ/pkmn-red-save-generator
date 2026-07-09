# Release Checklist

Candidate version: `v1.0.0`.

Do not tag or publish a release until every item below passes.

This checklist defines the required gates. The v1.0.0 pass/fail evidence is recorded in `docs/FINAL_RELEASE_VALIDATION_MATRIX.md`.

## Repository

- [ ] `main` synchronized with `origin/main`.
- [ ] Working tree clean except intended release changes.
- [ ] Committed dummy save SHA-256 is `248bc35328be435b16b47e2bb87c4e9732c2b5c92a95450839ed4619f74eb2e7`.
- [ ] Pokemon Red Save Genie has no tracked source changes.
- [ ] Private fixtures, screenshots, generated saves, reports, ROMs, PDFs, and emulator outputs are ignored and untracked.
- [ ] Tracked files contain no private absolute paths, private fixture names, usernames, secrets, or machine-specific assumptions.

## Build And Test

- [ ] Clean CMake configure.
- [ ] Clean CMake build.
- [ ] CTest passes.
- [ ] Xcode build passes with local signing disabled where required.
- [ ] CLI help and version work.
- [ ] CLI invalid-input behavior returns nonzero with actionable errors.
- [ ] `git diff --check` passes.

## Samples And CI

- [ ] `samples/minimal.red.json` generates and validates.
- [ ] `samples/representative.red.json` generates and validates.
- [ ] `samples/unsupported-viridian-pokemon-center.red.json` is rejected.
- [ ] Determinism check passes.
- [ ] Physical-image isolation check passes.
- [ ] GitHub Actions CI passes.

## Semantic Validation

- [ ] Save Genie reparses private validation outputs.
- [ ] Field-aware semantic comparison has no unexpected mismatch.
- [ ] Unsupported non-empty state is rejected or explicitly reported.
- [ ] All generated checksums are valid.
- [ ] Write provenance contains no undeclared overlaps.

## Emulator Validation

- [ ] Core load/save-again candidate passes.
- [ ] PC storage candidate passes.
- [ ] Daycare candidate passes.
- [ ] Hall of Fame candidate passes.
- [ ] Event/world candidate passes.
- [ ] Unsupported-location rejection candidate passes.
- [ ] Post-save SRAM files reparse and compare with only expected gameplay drift.

## Release

- [ ] `CHANGELOG.md` contains the release entry.
- [ ] README and docs reflect current support and limitations.
- [ ] Release notes prepared.
- [ ] Tag `v1.0.0` created only after all gates pass.
- [ ] Tag pushed only after explicit final approval.
- [ ] Release contains no ROMs, private saves, screenshots, private fixtures, or emulator binaries.
