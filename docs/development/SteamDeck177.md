# Private Deck test177: menus first

Test175/176 did not provide working menu or battle controls on the owner's Deck.
The earlier hardware diagnostic was successful; gameplay integration was not.
This is another private test, not verified Steam Deck support or a public update.

The frontend now starts in the Workshop menu bootstrap instead of waiting for
the possession adapter to load in battle. It uses Valve Steam Input006 and the
game's own UI components, hovered-component selection and native Back action.
Prompts use Shogun's native tooltip layout and real Steam controller glyphs,
prepared locally. There is no external menu window or keyboard layout.

The input gate now permits resting thumbs on capacitive touch surfaces while
still requiring held buttons/triggers to be released on reacquisition. The old
gate blocked every confirm event in the controlled resting-thumb cases.
Asynchronous controller discovery retains its owned Steam initialization.
A process-local lease prevents the menu and battle readers from competing.
The cache-lifetime correction from176 is retained.

## Manual Deck test

1. On PC, **Update the existing private Deck item3829785193**, keeping its
   visibility Private/Hidden. Do not create a new item or update the public mod.
2. Close the game on the Deck, let Steam download the update, then restart the
   Deck to end any previous native-action manifest session. Use official
   **Proton10.0-4** (10.0-4b maintenance build is suitable), Steam Input enabled,
   and only the private Deck test enabled. No special controller layout.
3. Test the **main menu first**. The in-game bottom prompts should show Select,
   Back, Move cursor and Cursor, with actual Steam Deck button/stick/pad icons.
   Move the cursor with the right trackpad or left stick. A or right-pad click
   activates the game's hovered control. B uses the native frontend Back action.
   D-pad provides slower cursor movement. No READY message is needed: there is
   no remote connection to the Deck.
4. Try Single Player and Custom Battle, then Back. If controls do nothing, send
   a photo of the bottom prompts/status before changing settings. Connecting,
   waiting-for-bindings and initialization-failure states are displayed in game.
5. Only if menus respond, try RTS battle camera/selection/orders below.

This increment uses cursor navigation with native hover feedback, rather than
inventing a directional focus order for every menu. Editable text, specialized
widgets, campaign controls and all menu variants still need hardware validation.

| RTS control | Action |
| --- | --- |
| Left stick; hold L3 | Camera pan; faster pan |
| Right stick horizontal/vertical | Rotate/native wheel zoom |
| Hold R3 + right stick vertical | Camera pitch |
| Right pad swipe | Native cursor |
| Right pad click/hold + swipe | Selection/selection box |
| A; hold R5 | Selection; add to selection |
| R2 press, swipe, release | Native contextual order/formation drag |
| B during order drag | Cancel |
| X | Native Halt |

Controller possession/QTE dispatch, groups/wheels, remaining RTS shortcuts,
campaign/naval support and suspend/resume are unfinished. The possession mod
itself still loads; complete controller play is not being claimed.

## Verification and limits

`tools/deck/build177.py` builds an isolated private source snapshot. The normal
Windows runtime and public engine guards are preserved. Regression checks
include native controller ABI/origins/asynchronous discovery, RTS policies,
native engine guards, local PNG-to-TGA glyph conversion/storage/lifetime, and
float-Lua menu behavior. UI policy fixtures use an explicit mocked game UI;
they cannot prove actual Shogun rendering, Steam delivery or Deck gameplay.
Private artifacts contain owned-game test resources and must not be uploaded.

No recorder, debugger, profiler or90-second diagnostic starts. No Steam personal
configuration, CPU settings or public Workshop/GitHub release is changed. Safe
publisher rollback preserves the previous176 pack. When returning to ordinary
play, disable the private test and restart the Deck to end the manifest session.
