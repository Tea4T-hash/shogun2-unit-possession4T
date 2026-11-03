> Historical development/checkpoint notes. The combined implementation is now
> maintained in public `main`; use the root README and BUILD_STEAM_DECK.md for
> current play/build instructions. Earlier private-item state below is historical.

# Deliver Deck support through the existing public Workshop item

The private test3829785193 is owner-only. Other players subscribe to the original
public Unit Possession item3825625050, which currently has no Deck support.
The accepted candidate selects its PC or Deck profile automatically at startup.

Promote the combined build by updating item3825625050, retaining the public
publisher stem shogun2_unit_possession_workshop165 and existing subscription.
Do not replace the public link with the private test link or create a new item.
Steam updates installed subscribed items when needed before launching the game.
Players need to let Steam finish downloading and restart the game; the running
native mod cannot be replaced mid-session. No manual copy or installer is needed.
[Valve Workshop implementation guide](https://partner.steamgames.com/doc/features/workshop/implementation#ItemInstallation).

Before public promotion, prepare an audited package with the public publisher
identity and player notes; preserve the accepted native payload and local-resource
method. Check the exact promotion package on PC and Deck, retain the previous
public pack for rollback, then use Update on the existing public item and verify
its uploaded/downloaded hashes. Preserve the existing visibility, preview and
subscribers. Keep private testing available separately and never enable both packs.

This file records the delivery plan only. No public package was staged or uploaded
by saving private v0.34-deck.2. Multiplayer remains outside this release.
