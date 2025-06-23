# Changelog

## 2.6.0

- Added markers for each n-th tick in the pitch graph.
- Renamed _Hold First Jump_ to _Initial Jump Duration_.
- Added _Total Jump Duration_ metric, which is _Initial Jump Duration + Time to Double Jump_.
  This metric is helpful when practicing fast-jumping mechanics like speed-flips or wall-dashes, where you don't care about the two separate components.
- Don't clear the double-jump metrics when timed-out to allow to see how long the jump would have been available.
- Default success color is now green (since the blue color confused some users).
- Visual improvement: Draw borders on top of the graphs to avoid visible overlap.

## 2.5.0

- Allow configuring timings
- Show pitch down in history (as an option)
- Also record inputs after dodging (to also see timings/inputs when backflipping)
- Calculate pitch up amount only between jumps
- Allow toggling different parts of the UI
- Draw a backdrop behind to whole UI (fully opaque by default)
- Show cvars in console

## 2.4.0

- Split pitch and boost history
- Ignore input for non-player cars
- Show warning in custom training when jump is not first input

## 2.3.0

- Record pitch input some configurable time after double jumping.
- Remove 100 ms tick lines and add jump input line.
- Disable recording in replay.
- Add border and text color setting.
- Draw UI relative to screen.
- Improve resetting values and hooks.

## 2.2.0

- Persistent storage for cvars

## 2.1.0

- Use cvars to remember settings

## 2.0.0

- More settings. Especially color settings.
- Pitch history: A graph showing your pitch inputs.
- Better total tilt calculation: Considers aerial sensitivity and input amount.
- Use in-game time to fix slow-motion.
