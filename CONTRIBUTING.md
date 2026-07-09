# Contributing

This project is intentionally conservative because malformed saves can load far enough to corrupt gameplay.

Before proposing changes:

- keep Pokemon Red Save Genie read-only unless a separate task explicitly targets that repository;
- do not commit ROM files, private saves, screenshots, emulator outputs, PDFs, or generated validation artifacts;
- do not use target `.red.json` `physicalImage` as generation authority;
- add tests for every serializer range or semantic contract change;
- run CMake tests and CLI sample workflows;
- document any new location or runtime-state support with emulator evidence.

Location support is fail-closed. Do not whitelist a map from map ID and coordinates alone.
