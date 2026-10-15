# PIGEON screen-reader fork

This is an experimental fork of [PIGEON](https://github.com/TheShrimpBlimp/pigeon),
based on its local v0.3 candidate and unified UI. It adds support for
[Charles Vestal's FM-1 Screen Reader](https://github.com/charlesvestal/fm1-reader).
It does not change the primary PIGEON repository, installer or published emulator.

## Use it

On hardware, this fork must be installed before it can announce screens. Connect
USB to a desktop computer and open https://charlesvestal.github.io/fm1-reader/
in Chrome or Edge. Connect the matching FM1 MIDI port. Choose the reader's voice
or your own screen reader, and use its WHERE action to repeat the current screen.
No announcements are sent before a reader requests them. Listening expires after
five seconds without a keepalive; the browser reader sends one every two seconds.

The local emulator has an **Enable screen announcements** checkbox, an output
choice (**My screen reader** or **Browser voice**), and **Read current screen**.
It uses the same firmware protocol and pronunciation file as hardware. Browser
voice depends on the browser's installed speech synthesis support. Speech is off
by default. The normal performance and sample-bank flows remain available.

## Pronunciation file and publication

`firmware/words.tsv` owns PIGEON's abbreviations and musical labels. The build
runs `tools/gen_words.py`, producing an immutable, fingerprint-named JSON file
in `web/words/` and its URL in `build/gen/ann_words.h`. All generated JSON files
must stay in the repository; never replace a file used by a released device.

The intended URL is `https://theshrimpblimp.github.io/pigeon-reader/words/`.
The site generator copies these files to `words/`. Publishing that Pages site is
a separate step; this work does not deploy a website or release firmware.
Before publication, drop the matching JSON file into the official reader page.
The emulator loads the matching local JSON automatically, so it can be tested
without publishing anything.

## Implementation

- `firmware/src/announce.c` is an unmodified copy of fm1-reader's protocol and
  debounce implementation. Update it with `../fm1-reader/tools/sync_announce.sh .`.
- `firmware/src/pigeon_reader.c` observes the selected page, track, controls,
  menus, dialogs, project names, recording and manually edited sequence steps.
  WHERE includes control values, visible active steps, pattern occupancy and
  navigation. Calibration prompts and transient messages are also announced.
- Reader frames are consumed before editor/update SysEx. A 128-packet queue holds
  a complete announcement; the adapter checks capacity before writing and never
  waits for USB. WHERE sends one chunk per UI frame and retries a full queue.
- Knob readouts settle for 120 ms and page changes for 200 ms. Playback steps and
  count-in beats are not spoken on every tick. Ordinary firmware operation sends
  no reader traffic. Text buffers live in the main-loop pool, not the audio ISR.
- Errors and warnings carry the protocol's protected flag. Browser speech lets
  protected warnings finish before a later announcement.
- `web/emu/reader-words.js` is an unmodified copy of fm1-reader's word expansion
  module. PIGEON vocabulary stays in `words.tsv`, not in the generic reader code.

## Build and validate

Clone fm1-reader beside this repository (`../fm1-reader`). Install the normal
PIGEON build dependencies from [BUILDING.md](../BUILDING.md). Then:

```sh
PYTHON=.venv/bin/python ./build-sampler.sh
PYTHON=.venv/bin/python sh tests/run_reader_tests.sh
PYTHON=.venv/bin/python sh tests/run_pigeon_tests.sh
PATH="$PWD/build/emsdk/upstream/emscripten:$PATH" sh web/emu/build.sh
node tests/reader-wasm-test.mjs
```

`tests/pigeon_reader_host.c` runs scripts against the real firmware UI and the
protocol. Its commands include `listen`, `where`, `said TEXT`, `page`, `track`,
`turn`, `press`, `wait`, `disconnect`, queue `block` and `speakdump`.
`PIGEON_SAID_LOG=FILE` records actual announcements. The dump visits the three
tracks, applicable pages, supported overlays, every visible settings row and
value, confirmation types, and all timing enums. The runner audits both the dump
and the scenario log with upstream's actual word expansion module.

## Release checks still required

- Flash only when explicitly authorized, retaining a complete recovery archive.
- Confirm the physical USB handshake, words URL and one announcement per action.
- Check sample-loader transfers and USB audio alongside an attached reader.
- Listen with VoiceOver or NVDA, plus the official reader's voice. Automated
  protocol and ARIA tests cannot replace that listening check.
- Publish the fork's versioned words only when website publication is authorized.

## Credits

PIGEON: The Shrimp Blimp. FM-1 Screen Reader protocol, generator, pronunciation
engine and integration instructions: Charles Vestal, GPL-3.0. The upstream
implementation is at https://github.com/charlesvestal/fm1-reader. PIGEON retains
Felucca's author and licensing notices. The browser adapter uses the browser's
speech synthesis; it does not embed eSpeak or claim to be the upstream reader.

## Integration record

Shared upstream files are pinned to fm1-reader commit
`541ea332ebecd84aeea6a45b52a5fe99801c8570` and verified byte-for-byte.

- [x] Isolated development copy of the current PIGEON candidate.
- [x] Non-blocking USB protocol, messages and settled semantic observer.
- [x] Versioned words generated by hardware, host-test and emulator builds.
- [x] Scripted simulator, logged announcements, WHERE and queue-pressure checks.
- [x] Word audit: 963 messages, zero errors and zero warnings.
- [x] Upstream protocol and pronunciation tests.
- [x] Native reader scenario under address/undefined-behavior sanitizers.
- [x] Browser adapter and compiled WebAssembly protocol tests.
- [x] Real Chromium/AudioWorklet test of announcements, local words, live regions and WHERE.
- [x] Hardware compilation and flash/RAM budget checks.
- [ ] Physical-device and VoiceOver/NVDA listening checks.
- [ ] Publish the fork's words URL when authorized.

The checked hardware build uses 348,808 bytes for the application, 55,688 of
65,536 bytes in data/BSS, and 356,880 of 376,832 bytes in the pool. It leaves
9,848 bytes in data/BSS and 19,952 bytes in the pool. This is a build check, not
a claim of verified hardware operation.
