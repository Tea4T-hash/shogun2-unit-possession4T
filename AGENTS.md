# Shogun 2 Unit Possession — maintained main

The combined PC/Steam Deck source is the maintained implementation. Read README.md,
BUILD_STEAM_DECK.md and docs/current-release.json first. Native/deck contains the
combined runtime and build-time adaptations; older numerical suffixes are historical.
Use owned game resources and isolated output directories. Never commit extracted
assets, engine DLLs, compiled builds, credentials, local connection profiles or dumps.

Offline work and tests may run while the owner plays. Actual installation or staging
requires explicit approval, exact rollback and a closed receiving game and launcher.
A live SSH/memory check needs readiness and verified process/build identity; keep it
read-only and bounded. Publishing GitHub source does not upload Steam Workshop.
Preserve repository identity, visibility, prior tags/releases and the existing
Workshop item 3825625050. No installer downloads. Keep the MIT and vendor notices.

Use evidence from native decoding and original instructions for engine changes.
Keep build/fixture results separate from real gameplay acceptance. The current
revision34 enemy-target acquisition still fails the owner's live trial; do not
claim it fixed. Charge, climbing, camera and infantry/cavalry animation acceptance
are separate, scoped results. The latest source includes those targeting attempts.
Do not resume targeting development or live testing just because source is published.
No subagents unless the owner explicitly requests them.
