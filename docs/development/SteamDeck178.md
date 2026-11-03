# Private Steam Deck test178

This update tests menu navigation first. Use Windows Shogun2 under Proton9.0-4.
Enable only the private Deck test; leave the normal possession mod disabled.
No special community layout or manual Steam configuration is required. The mod
reads native Steam Input actions; do not add keyboard/mouse bindings for this test.

Menus: left stick or D-pad moves the cursor; right pad is relative cursor.
A, left trigger or right-pad click selects; B goes back. Left pad scrolls.

RTS core follows Allyn's published Shogun T.W Setup (Workshop582338806): left
stick moves camera, stick click speeds it up, shoulder buttons turn, right
stick tilts/turns, right pad points, left trigger selects/drags, right trigger
orders/drags formation, left pad zooms. L4 or R5 adds to selection. X halts.
The original reference VDF is not shipped. Native game functions and native
mouse-message handling implement these actions, rather than a keyboard preset.

Group shortcuts, full possession/QTE control and campaign controls remain work
in progress. This is not the finished Steam Deck release. Existing Windows
keyboard/mouse possession remains available. Default menus have an owned
bottom prompt strip; basic input no longer depends on drawing the strip.

If initialization or prompt callbacks fail, a single private error message may
appear. Photograph its exact wording. There is no timed diagnostic, recorder,
debugger or recurring history. There is no remote connection to your Deck.

Offline checks cover fractional cursor drift, input policy, actual native
camera routines with synthetic providers, real compiled Lua/export loading and
mock UI lifecycle. Real menu rendering, focus, widget behavior and Steam Deck
performance still require your test. Update existing PRIVATE item3829785193;
never upload this candidate to the public item3825625050.
