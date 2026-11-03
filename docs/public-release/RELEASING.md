> Current maintained source: v0.34 combines PC and Steam Deck. Use [../../BUILD_STEAM_DECK.md](../../BUILD_STEAM_DECK.md) and [README](../../README.md). Earlier checkpoint and release instructions below are historical.

# Releasing

The canonical repository is logsam58/shogun2-unit-possession; main is the only
maintained branch. v0.33 packages the exact Test173 build with matching source.
GitHub publication and Steam Workshop upload are separate tasks. Preserve the
repository's actual visibility and historical tags/releases. No installer
downloads are permitted; obsolete installer assets have been removed with
private backups.

For native changes, build/test the isolated Workshop target and pack with
[BUILD_AND_TEST.md](../../BUILD_AND_TEST.md). Record build checks, Steam delivery
and owner acceptance separately. If only publishing an accepted build, verify
its exact pack hash and matching native source instead of rebuilding gameplay.
Never distribute build/fixture folders or locally extracted game resources.

Update VERSION, README, current-release.json, release notes and validation.
Review and commit the exact source; export/audit the clean source kit and verify
archive hashes and file closure. Tag the same commit. Current assets are a
Workshop ZIP of the exact pack with first-party instructions/notices, a source
ZIP, SHA256SUMS.txt and release-manifest.json. Verify every uploaded asset by
downloading and hashing it before completing the release. Preserve prior
release/tag metadata and repository identity/visibility.

Do not reuse tools/github_release.py's old fixed installer asset list for this
Workshop release. Its authenticated transport can be reused; credentials remain
in memory. A release must never edit Steam settings or install into the game.
Real installs need fresh CLOSED and verified rollback; live checks need fresh
READY/IN BATTLE and exact loaded hashes/hooks. Recording is a separate choice.

Keep naval/multiplayer, intermittent charge/climb, visual/gameplay bug and crash
notices. Resource exclusion does not establish legal clearance for native hooks;
see [provenance](PROVENANCE.md).
