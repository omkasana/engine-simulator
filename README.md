# Open Engine Simulator

https://github.com/user-attachments/assets/5894bb7e-eee5-42f8-ab42-29088f6f7b51

Open Engine Simulator is a community-driven fork of
[AngeTheGreat's Engine Simulator](https://github.com/ange-yaghi/engine-sim):
a real-time internal combustion engine simulation designed specifically to
produce engine audio and simulate engine response characteristics. It is not a
scientific tool and cannot be expected to provide accurate figures for
engineering or engine tuning.

## Download and play

Get the latest build from this repository's [Releases](../../releases) page.
Choose the archive that matches your computer:

| Your computer | Download | Run |
| --- | --- | --- |
| Apple Silicon Mac (M1/M2/M3/M4) | macOS arm64 `.dmg` | Drag `engine-sim.app` to Applications, then open it |
| Windows PC | Windows x86_64 installer `.exe` | Run Setup, then open **Open Engine Simulator** from Start |
| Linux x86_64 PC | Debian/Ubuntu `.deb` | Install the package, then open **Open Engine Simulator** from the app launcher |

Portable `.zip` (macOS/Windows) and `.tar.gz` (Linux) builds are also attached
for users who do not want an installer.

The community macOS build is ad-hoc signed but not Apple-notarized. macOS may
require you to approve it in **System Settings > Privacy & Security** before
its first launch.

## Using the simulator

| Input | Action |
| --- | --- |
| A / S / D / H | Ignition / starter (hold) / dyno / RPM hold |
| G + scroll | Change RPM-hold speed |
| F / Tab / Escape | Fullscreen / screen / exit |
| I | Show dyno information |
| Shift + Space | Clutch |
| Up / Down | Change gear |
| Z / X / C / V / B + scroll | Volume / convolution / HF gain / LF noise / HF noise |
| N + scroll | Simulation frequency |
| M / , | Increase / decrease view layer |
| Enter | Reload the engine script |
| Q / W / E / R | Throttle position |
| Space + scroll | Fine throttle adjustment |
| 1–5 | Simulation time warp |
| F1 / F2 / F4 | Controls / engine picker / display and fuel settings |

RPM hold requires both `H` and the dyno (`D`).

Open **Settings** (or press F4) to choose between two saved interface modes.
**Legacy** is the default and preserves the original pixel-style dashboard,
original script units, compact engine picker, and original fuel display.
**Modern** uses a cleaner scalable typeface and roomier controls, and adds an
imperial/metric selector plus USD, EUR, GBP, INR, JPY, AUD, or CAD fuel-cost
configuration. Enter your local price per litre; the dashboard shows the
matching currency symbol and code. Currency starting values are examples, not
live pump prices. All choices are stored for the current user and remain active
when another engine is loaded.

On touch or mouse-driven hosts, the dashboard keeps the same displays while
adding direct controls: tap the Ignition, Dyno., or Hold row to toggle it; hold
the Starter row while cranking; tap the up/down arrows in the Gear panel; and
drag the slim slider beside the Throttle display. Releasing the slider returns
the throttle to closed.

## Why this fork exists

Open Engine Simulator is a community-driven, cross-platform fork of
[Engine Simulator](https://github.com/ange-yaghi/engine-sim), created by
[Ange Yaghi (AngeTheGreat)](https://github.com/ange-yaghi). The original
project's simulation work, scripts, visual language, and contributors made
this project possible. This is independently maintained and not an official
upstream release.

The SDL desktop application is an important reference host, but it is not the
fork's sole focus. The larger goal is a stable, portable engine-sound simulator
core that other applications can host. The simulator, scripting, rendering,
and desktop/audio-device adapters are therefore kept as distinct layers. The
audio synthesis path is being shaped into a reusable device-independent
library, so it can eventually serve applications beyond this desktop program.

Contributions are welcome; see [CONTRIBUTING.md](CONTRIBUTING.md).
The [upstream FAQ](https://github.com/ange-yaghi/engine-sim/wiki/Frequently-Asked-Questions)
is useful historical background, but may not describe this fork's changes.

## Develop

Clone with the required submodules:

```sh
git clone --recurse-submodules <this-fork-url>
cd engine-sim
```

Install CMake 3.24+, Ninja, and a C++17 compiler. Script-enabled desktop builds
also require Flex and Bison 3.2+. SDL3 is found or fetched by CMake. Shader
artifacts are included; SDL_shadercross is only required to regenerate them.

### Build and run

The Makefile wraps the CMake presets. Set `PLATFORM` to one of
`macos-arm64`, `linux-x86_64`, or `windows-x86_64`:

```sh
# Apple Silicon macOS
make PLATFORM=macos-arm64 portable-run

# Linux x86_64
make PLATFORM=linux-x86_64 portable-run

# Windows x86_64 (GNU Make/MSYS2)
make PLATFORM=windows-x86_64 portable-run
```

On Windows without GNU Make, run the equivalent CMake commands:

```powershell
cmake --preset windows-x86_64-desktop
cmake --build --preset windows-x86_64-desktop
.\build\windows-x86_64-desktop\engine-sim-desktop.exe
```

To configure, build, and test only the portable core:

```sh
make PLATFORM=macos-arm64 portable-test
```

To reproduce CI's complete desktop, scripting, and SDL dummy-audio validation,
use `make PLATFORM=<target> portable-validate`.

### Browser build

The browser host targets WebAssembly and WebGL 2. Install and activate the
[Emscripten SDK](https://emscripten.org/docs/getting_started/downloads.html),
then build and serve it locally:

```sh
make web
make web-serve
```

Open <http://localhost:8080/>. Click **Enable audio** once the page loads;
browsers require that interaction before they allow sound playback.

On Apple Silicon with Homebrew, install the local prerequisites with:

```sh
brew install bison flex cmake ninja
```

If Flex/Bison are keg-only, pass their executable paths while configuring,
for example `-DBISON_EXECUTABLE=/opt/homebrew/opt/bison/bin/bison` and
`-DFLEX_EXECUTABLE=/opt/homebrew/opt/flex/bin/flex`.

### Package

Build packages on their target platform:

```sh
make PLATFORM=macos-arm64 portable-package
make PLATFORM=linux-x86_64 portable-package
make PLATFORM=windows-x86_64 portable-package
```

The macOS preset creates a DMG and app ZIP, Windows creates an NSIS Setup EXE
and portable ZIP, and Linux creates a DEB and portable tarball. Cross-compiling
is not required or recommended; build each package on its target operating
system. See [CMakePresets.json](CMakePresets.json) for the underlying presets.

### Publish a GitHub release

The `Release packages and web app` GitHub Actions workflow builds every desktop
package when a version tag is pushed. After committing and pushing a release:

```sh
git tag v0.3.0
git push origin v0.3.0
```

The workflow creates the matching GitHub Release and attaches the DMG, app
ZIP, Windows installer, Windows portable ZIP, Linux DEB, and Linux tarball.
It can also be run manually from **Actions** using an existing tag. A normal
branch push does not create release assets.

## Attribution and license

Open Engine Simulator preserves the upstream MIT license and original
copyright notice; see [LICENSE](LICENSE). Historical Patreon supporter
acknowledgements belong to the original project and remain in its
[upstream repository](https://github.com/ange-yaghi/engine-sim).
