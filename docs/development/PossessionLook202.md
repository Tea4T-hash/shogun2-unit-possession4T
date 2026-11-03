# Deck possession look timing (202)

Revision28 was verified over paired SSH: its installed pack, loaded bridge,
adapter, QTE and HUD matched. The owner reports jagged ordinary right-stick
looking in both first and third person, without LT. Fractional input handling
alone did not resolve it. The600-second10Hz read-only capture contains5992
samples,232 possession/stick-active samples, all with aiming off. It does not
record the rendered camera, so it cannot by itself establish the jitter cause.

## Reproduction and change

Revision28 sends time-scaled stick displacement from the8ms worker to the UI
queue, then back to the worker, before a rendered camera can use it. An isolated
fixture with the actual queue and camera consumer reproduces uneven rotation
for a constant stick under a deterministic plausible UI/worker/render schedule.
At60fps it produces0.240–0.360rad/s between frames for a requested0.250rad/s;
at90fps,0.180–0.360rad/s. This is a scheduling reproduction, not a recording of
the actual Deck render schedule or proof that it explains every visible hitch.

The candidate publishes the latest validated analog rate and integrates it at
the already-owned camera callback. The stick's curve, deadzone, sensitivity,
ADS blend, mounted constraints and QTE eye limits remain unchanged. The rate
has no accumulated backlog or smoothing tail; zero input stops immediately.
A stale sample, focus/menu loss, changed lease or device epoch resets elapsed
time. The previous50ms pause bound and configured sample-staleness limit apply.
Native actor serial0 remains valid. PC bypasses the path. Pad/gyro displacement
retains its fractional queue; stick displacement is subtracted there to avoid
double application. A zero-input Deck worker no longer rewrites unchanged camera
angles while the camera callback integrates the stick.

Focused fixtures exercise4296 actual camera advances over30/40/60/90Hz, both
perspectives and hip/half/full ADS, plus release, reversal, stale input, focus,
identity/epoch changes, wheel suppression, cancellation and20000 concurrent rate
samples. These establish offline behavior, not live acceptance.

## Reported crash

The owner subsequently reported a crash during a melee/QTE animation at12:40:03
on2026-11-02. The10-minute capture had finished at approximately12:32:55. Steam
confirms the recorded PID5331 exited; the checked system/user journals and dump
locations supplied no fault address, module or core dump. Miles was suggested
by the owner but is unconfirmed. The camera candidate was still offline and
was not running. No new debugger, exception handler or game settings were enabled.

Evidence on D: is in Shogun2Deck/diagnostics/revision28-battle-capture-1,
Shogun2Deck/diagnostics/revision28-crash-1 and builds/Test181/look202-review-1.
The existing audio guard, accepted charge and upward-wall implementation remain.
The crash and broader lone-soldier enemy acquisition remain unresolved.

## Reference

Valve's [Steam Input API](https://partner.steamgames.com/doc/api/ISteamInput)
provides floating-point analog action values and separately describes input
updates. Our scheduling conclusion comes from the mod source and actual-code
fixture; the documentation does not diagnose this Shogun runtime.

## Candidate validation

Revision29 completed the full clean build: 100 adapter, 50 QTE and 8 HUD test
executables, camera/controller fixtures, compiled bootstrap and native Lua
checks. The Python suite passed 52 tests. The seven native package-reader cases
passed for local and subscribed pack paths. Exact revision28 restoration and
foreign-file protection passed in isolated fixtures. Twenty protected PC paths
remain unchanged.

The prepared pack is `959b099b956cfe66ceed33126d0ceedd44d8e759c5da186c6678e781e353d935`
(4,246,640 bytes). On 2026-11-02 the owner approved staging and transfer; both
PC and Deck copies were independently verified against this hash, with exact
revision28 backups and tested rollback. Nothing was uploaded to Workshop.
Live Deck smoothness and crash causation remain unverified.
