# Command-line guide

## Build the maze

Open `build/windows/openmaze.sln` in Visual Studio 2022 with the **Desktop development with C++** workload. Restore the solution's NuGet packages, select **Release** and **x64**, then build. The executable is `build/windows/x64/Release/openmaze.exe`.

From a Visual Studio Developer PowerShell in the navigation repository root, build with:

```powershell
msbuild .\build\windows\openmaze.sln /p:Configuration=Release /p:Platform=x64
```

## Start the launcher

Run the launcher from the navigation repository root. Use the batch file or the active Python environment with Tk.

```powershell
.\run-navigation.cmd
python .\launcher\navigation_launcher.py
```

The batch file checks `SSVEP_NAVIGATION_PYTHON`, the navigation checkout's `.venv`, the user's Conda environment named `venv`, then Python on `PATH`. `SSVEP_NAVIGATION_PYTHON` selects the launcher interpreter. `SSVEP_DECODER_PYTHON` selects the interpreter for the decoder.

## Run the executable directly

Run from the navigation repository root so the executable can load `resources/`. By default, the maze opens fullscreen and starts the decoder from the sibling `ssvep-decoding` checkout when it has no active stream.

```powershell
.\build\windows\x64\Release\openmaze.exe
.\build\windows\x64\Release\openmaze.exe --windowed --keyboard-only
```

| Option | Effect |
|---|---|
| `--windowed` | Open in a resizable window. |
| `--keyboard-only` | Use keyboard controls without opening the UDP decoder link. |
| `--start-decoder` | Use a decoder started by the maze. If a stream is already active, use attach mode or stop that stream and press F3 to retry. |
| `--attach-decoder` | Listen to a running decoder. Press F3 after valid steering data arrives. |
| `--monitor-only` | Prevent EEG steering. A decoder started by the maze sends scores and decisions for display. |
| `--no-route-hint` | Start with the route hint hidden. F2 can show it. |
| `--no-accepted-command` | Hide the centered accepted-command overlay. |
| `--manual` | Skip decoder startup. F3 can start a decoder or arm an existing stream. |
| `--stimulus-info` | Print the maze's frame periods and frequency tolerance as JSON, then exit. |

For a decoder started by the maze, set these optional environment variables before running the executable.

| Variable | Default |
|---|---|
| `SSVEP_DECODER_DIR` | Sibling `ssvep-decoding` checkout |
| `SSVEP_DECODER_PYTHON` | The decoder checkout's `.venv` or the user's Conda environment named `venv` |
| `SSVEP_DECODER_PORT` | `COM3` |
| `SSVEP_DECODER_PROFILE` | `ssvep_decoding/profiles/decoder_4s_3h.json` in the decoder checkout |
| `SSVEP_STIMULUS_PROFILE` | The decoder's default 60 Hz stimulus profile |

```powershell
$env:SSVEP_DECODER_DIR = 'C:\path\to\ssvep-decoding'
$env:SSVEP_DECODER_PYTHON = 'C:\path\to\python.exe'
$env:SSVEP_DECODER_PORT = 'COM3'
$env:SSVEP_DECODER_PROFILE = 'C:\path\to\decoder-profile.json'
.\build\windows\x64\Release\openmaze.exe --windowed --start-decoder --monitor-only
```

## Attach a decoder started from a terminal

Upload the MG24 EEG streamer and close Arduino Serial Monitor or Plotter. Use a 60 Hz display with the default stimulus profile. Launch the maze in attach mode from the navigation checkout.

```powershell
.\build\windows\x64\Release\openmaze.exe --attach-decoder
```

From the decoder checkout, run a live session with your actual serial port and a new session path. This example displays live scores while you navigate manually.

```powershell
python -m ssvep_decoding live --port COM3 --seconds 165 --decoder ssvep_decoding/profiles/decoder_4s_3h.json --maze-hud --session sessions/maze_monitor_01
```

For steering, use `--maze-control` in place of `--maze-hud`, give the run a new session name, and press **F3** in the focused maze when the HUD shows valid EEG. Pass `--stimulus` to the decoder if you use a different matching stimulus profile. [Navigation behavior and recording details](navigation.md)
