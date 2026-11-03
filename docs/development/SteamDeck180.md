# Private Steam Deck test180

Use Windows Shogun 2 with Proton 9.0-4. Enable only the private Deck test.
The public possession mod must remain disabled for this comparison. No extra
community preset is required. D-pad/left stick chooses visible menu controls;
A selects and B goes back. Right trackpad remains a cursor. Native Valve glyphs
and a small bottom legend are retained. Full controller support is unfinished.

This corrects a concrete Test179 UI ownership bug. Its child update/draw callback
used static Component.Adopt on its own group. The native binding resolves the
current Lua component, so that call can detach the group from root and make it
its own parent when a new menu screen changes root child order. The original
native adoption routine has no self-parent guard. Test179's earlier fixture
incorrectly hard-coded static adoption to the root and missed this case.

Test180 keeps all parenting out of frame/draw callbacks. An explicit root:Adopt
runs from the native root adoption event, preserving the stock handler and
its tooltip behavior with a reentrancy guard. Parent geometry updates only when
screen dimensions change. A regression model reproduces179's circular tree and
checks180's repeated frames, nested stock tooltip adoption, failures and cleanup.
These are offline tests, not proof of a working Deck menu.

The input worker reacquires a replaced intro/menu window. A long intro without
UI pulses is no longer reported as a six-second initialization failure. Actual
initialization and Lua errors retain a single bounded failure message.

All four gameplay component binaries and their dependency closures are exact
Test179. Engine admission, hooks, resource preparation, RTS controls, Steam
Input bindings, gyro/pad handling and the public Workshop item are preserved.
No recorder, debugger or profiling capture is included or started.

The owner reported179 black after intro and that the previous build worked.
Before exit, read-only SSH verified the actual loaded bridge/engine hashes and
one menu heartbeat; it did not recover an actual circular UI object. Therefore
the identified bug matches the observed failure but the black-screen correction
still needs a real Deck test. No automatic game launch is performed.

Only private item3829785193 is intended. All new build/diagnostic outputs live
under D:/Shogun2Deck. A staged PC publisher pack is not a Steam upload and does
not by itself update the Deck. Deployment receipts distinguish these states.
