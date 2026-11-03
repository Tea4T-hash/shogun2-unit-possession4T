# Private Steam Deck test179

Menu navigation comes first. Run Windows Shogun2 using Proton9.0-4. Enable the
private Deck test only. No additional community controller preset is required.

D-pad or left stick moves between visible menu choices; A selects. B goes back.
The right trackpad also controls the native pointer; LT or pad-click selects.
The left pad scrolls. A bottom legend uses Valve's own shaped/coloured glyphs.
Directional navigation reads current native widget visibility, enabled state and
geometry, then uses the game's ordinary pointer/highlight and click handling.
It never directly forces a disabled or obscured button's callback. Specialized
widgets and full campaign control remain unfinished; this is a private test.

Test178's worker started but its menu callback never fired. Native research
shows FrontEndRoot owns a no-op native pulse handler; tooltip also owns a native
pulse handler. Native controllers bypass the registered Lua pulse. This build
owns a plain pixel component and its callbacks; stock root callbacks stay intact.
Native worker input remains independent of painting, with focus-loss cleanup.
The prior cursor drift and native camera-rate corrections are retained exactly.
RTS core retains the Allyn layout conventions described in SteamDeck178.md.
Full possession/QTE controls, groups and game-specific widgets are pending.

Only existing PRIVATE Workshop item3829785193 is updated. The public mod is
unchanged. No recorder/debugger or timed test is included. If a single failure
message appears, photograph it. There is no remote connection to the Deck.
Real glyph rendering, focus and directional behavior require your hardware test;
offline checks use explicit UI/backend substitutes, not a running game.
