# CLI Reference

The executable is `pkmn-red-save-generator`. The CMake build places it at `build/pkmn-red-save-generator`.

## General

```sh
build/pkmn-red-save-generator --help
build/pkmn-red-save-generator --version
```

`--version` currently reports `pkmn-red-save-generator 1.0.0`.

## Generate

```sh
build/pkmn-red-save-generator generate <input.red.json> <output.sav> \
  --report <output.report.json> \
  --summary \
  --show-ranges
```

Equivalent explicit form:

```sh
build/pkmn-red-save-generator generate \
  --input <input.red.json> \
  --template "Dummy Save/Pokemon - Red Version (USA, Europe) (SGB Enhanced).sav" \
  --profile profiles/pokemon-red-usa-europe-v1.json \
  --output <output.sav> \
  --report <output.report.json>
```

Safety behavior:

- output save paths must not collide with the input JSON or template;
- report paths must not collide with the output save or template;
- existing output and report files are not overwritten;
- target `physicalImage` is ignored;
- unsupported locations and inconsistent mirrored state are rejected.

## Dry Run

```sh
build/pkmn-red-save-generator generate samples/representative.red.json /tmp/representative.sav \
  --dry-run \
  --summary \
  --show-ranges
```

Dry-run generation builds and validates bytes but does not write the `.sav`.

## Validate

```sh
build/pkmn-red-save-generator validate samples/minimal.red.json
build/pkmn-red-save-generator inspect samples/representative.red.json
build/pkmn-red-save-generator validate-template \
  --template "Dummy Save/Pokemon - Red Version (USA, Europe) (SGB Enhanced).sav" \
  --profile profiles/pokemon-red-usa-europe-v1.json
build/pkmn-red-save-generator validate-save --input-save /tmp/generated.sav
```

Use `--allow-dirty-current-box` only for emulator-modified post-save files where Gen I has intentionally written a dirty current-box cache that differs from the permanent selected box.

## Determinism And Physical-Image Isolation

```sh
build/pkmn-red-save-generator check-determinism \
  --input samples/representative.red.json \
  --work-dir /tmp/savegen-determinism

build/pkmn-red-save-generator check-physical-image-isolation \
  --input samples/representative.red.json \
  --work-dir /tmp/savegen-physical-image
```

Expected invariant:

```text
same semantic input + same template + same profile => identical generated .sav bytes
```

Changing, removing, or corrupting the target JSON `physicalImage` must not alter output bytes.

## Semantic Comparison

```sh
build/pkmn-red-save-generator compare-semantics \
  --target-json <input.red.json> \
  --reparsed-json <generated.red.json>
```

The `reparsed-json` should come from the read-only Pokemon Red Save Genie oracle. Public CI does not require the sibling Save Genie repository.

## Public Samples

```sh
build/pkmn-red-save-generator generate samples/minimal.red.json /tmp/minimal.sav \
  --report /tmp/minimal.report.json \
  --summary
build/pkmn-red-save-generator validate-save --input-save /tmp/minimal.sav

build/pkmn-red-save-generator generate samples/representative.red.json /tmp/representative.sav \
  --report /tmp/representative.report.json \
  --summary
build/pkmn-red-save-generator validate-save --input-save /tmp/representative.sav
```

The negative sample below is expected to fail:

```sh
build/pkmn-red-save-generator inspect samples/unsupported-viridian-pokemon-center.red.json
```

## Exit Codes

- `0`: command succeeded, or help/version was displayed.
- nonzero: invalid arguments, missing file, invalid JSON, unsupported semantic input, unsafe location, checksum failure, write-overlap failure, output collision, or semantic mismatch.
