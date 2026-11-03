# Deck campaign setup footer: revision36

The owned Japan/Rise and Boshin Version054 campaign layouts have an authored
1280x1024 minimum. Their Start Game buttons end at968 and1015; Boshin's footer
ends at1024. The established MIT Total War UI decoder supplied this geometry.
Private extracted assets and XML are not part of this repository.

Revision35 failed the owner's actual Deck test. A readiness-authorized, bounded
read-only SSH inspection verified its subscribed pack and loaded bridge. The
outer root was1638x1024, but the native grand_campaign_japan screen remained
1310x819 at164,102. Its main panel was1104x912 at absolute267,56. The footer
started at630,921 and was379x56. The screen's bottom was also921, clipping the
entire footer even though its native visible flag was true. The old fixture
checked viewport height without reproducing this clipping ancestor.

Revision36 retains the shared viewport fit and also resizes the current visible
fullscreen campaign screen to that same logical viewport, then moves it to the
root origin using the original Resize(false,false) and MoveTo methods. The
screen's captured native resize flags already permit this. Native docking
places its fixed-size children. No font, child artwork, text, interactive state,
button callback, campaign rule or controller binding is changed. Both screens
use their authored1280x1024 minimum, never the newly expanded width as feedback.
Leaving setup restores the normal Deck enlargement. Desktop bypass is unchanged.

The updated float-Lua fixture reproduces the old failure, checks both parent
hierarchies, repeated frames, late native adoption, screen exit and the campaign
map exclusion. Native stock and reviewed-Bugfix fixtures execute original
Resize/MoveTo/docking for both layouts at four resolutions:16 parent cases.
They require the complete footer and button to fit every relevant ancestor and
preserve child dimensions, fonts and resize flags. State storage and events are
fixture providers; these checks do not claim actual Deck rendering acceptance.
The owner subsequently accepted the correction ("perf"). This is scoped manual
acceptance of the reported Deck screen, not a new formal loaded-hash check for
revision36, proof of every campaign/DLC layout or long-session stability.
