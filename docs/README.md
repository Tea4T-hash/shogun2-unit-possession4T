> Current maintained source: v0.34 combines PC and Steam Deck. Use [../BUILD_STEAM_DECK.md](../BUILD_STEAM_DECK.md) and [README](../README.md). Earlier checkpoint and release instructions below are historical.

# Developer documentation

The player guide is the repository's [main README](../README.md). For code work,
start here. The current GitHub release is **v0.33 / Test173**; Test172 has owner
gameplay acceptance and Test173 has offline checks. [Release identity](current-release.json) records its exact
pack and acceptance status; numbered filenames are implementation history, not
product versions.

## Reading path

| Task | Start with |
| --- | --- |
| Understand startup and component boundaries | [Architecture](ARCHITECTURE.md) |
| Find the active implementation | [Source map](../SOURCE_MAP.md), [native source guide](../native/README.md) |
| Build or run offline checks | [Build and test](../BUILD_AND_TEST.md), [dependencies](LOCAL_DEPENDENCIES.md) |
| Find a developer command | [Tool guide](../tools/README.md) |
| Change a hook, protocol or ownership guard | [Native contracts](public-release/NATIVE_CONTRACTS.md) |
| Regenerate tables or resource metadata | [Derived data](public-release/DERIVED_DATA.md), [resource pipeline](public-release/RESOURCES.md) |
| Understand what has actually been tested | [Validation](../VALIDATION.md) |
| Report or investigate a bug | [Known issues](KNOWN_ISSUES.md), [contribution guide](../CONTRIBUTING.md) |
| Prepare a future release | [Release procedure](public-release/RELEASING.md), [provenance](public-release/PROVENANCE.md) |

## Current references and historical evidence

[Status](STATUS.md), [release identity](current-release.json), and the current
[release notes](releases/v0.33.md) describe the shipped behavior. Validation
receipts record finite checks, not blanket stability claims.

[Release engineering](public-release/README.md) separates maintained design
references from earlier plans and test records. [Development history](development/README.md)
explains the Test/Diagnostics notebooks, and [release history](releases/README.md)
indexes earlier product versions. Historical instructions, machine paths,
pending approvals, and statements such as "current build" apply to their
recorded session only. They do not authorize actions or override current guides.

Keep historical paths stable: source includes, fixtures, receipts and recorded
hashes refer to them. The curated source ZIP omits the full development notebook
and older release notes; the history indexes link back to the repository.
