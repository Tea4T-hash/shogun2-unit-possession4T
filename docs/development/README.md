# Development history

This directory preserves the chronological Test and Diagnostics notebooks and
their validation receipts. It is historical evidence, not a setup guide or a
list of outstanding tasks. Version claims, local paths and readiness requests
describe the session in which each note was written.

Start new work with the [developer guide](../README.md), [current release identity](../current-release.json),
and [validation summary](../../VALIDATION.md). Product v0.32 uses Test166; its
Workshop design and receipts are in `docs/public-release`, not a new `Test166.md`
in this directory.

| Records | Subject |
| --- | --- |
| `Checkpoint2`–`Checkpoint4` | Early implementation checkpoints |
| `Test100`–`Test109` | Orders, presentation, animation, context and friendly-fire work |
| `Diagnostics110`–`Diagnostics112` | External recorder, startup recovery and diagnostic hardening |
| `Test113`–`Test124` | QTE input, crowd admission, participant identity and preparation |
| `Test125`–`Test144` | Possession, movement, combat, equipment and ability work |
| `Test146`–`Test157` | Charge, HUD, camera/lifetime and final-human role work |

Numbers are historical identifiers; gaps do not imply missing required build
inputs. Read each note alongside its available `*-validation.json` receipt and
the relevant source regression tests. A proposed check in a notebook is not
evidence that it ran.

The full notebooks are kept in the [repository history directory](https://github.com/logsam58/shogun2-unit-possession/tree/main/docs/development)
and omitted from the curated source ZIP. Do not execute historical install,
deployment, capture or publication commands as part of routine development.

The Deck campaign setup footer correction and its captured clipping regression
are documented in [CampaignSetup209.md](CampaignSetup209.md).
