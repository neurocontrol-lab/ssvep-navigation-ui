"""Configure and launch a Windows maze run."""

import json
import math
import os
from pathlib import Path
import subprocess
import sys
from tempfile import TemporaryDirectory
import tkinter as tk
from tkinter import filedialog, messagebox, ttk


ROOT = Path(__file__).resolve().parents[1]
START_DECODER = "Start decoder"
MODES = {START_DECODER: ["--start-decoder"], "Use running decoder": ["--attach-decoder"],
         "Keyboard only": ["--keyboard-only"]}
PRESETS = ("4 s / 3 harmonics", "2 s / 2 harmonics")
CUSTOM = "Custom JSON"
TARGETS = {"forward": "Forward", "backward": "Backward",
           "turn_left": "Turn left", "turn_right": "Turn right"}

# Use the chosen checkout and interpreter to resolve profiles before opening the maze.
RESOLVE_PROFILES = """
import json
import sys
from ssvep_decoding.config import read_profile, reference_frequency, validate

preset, custom, stimulus_file = sys.argv[1:]
decoder = read_profile("decoder", custom or None)
if not custom:
    from ssvep_decoding.experiment import PRESETS
    decoder["window_seconds"], decoder["harmonics"] = PRESETS[preset]
stimulus = read_profile("stimulus", stimulus_file or None)
control = read_profile("control")
validate(read_profile("acquisition"), stimulus, decoder, control)
frequencies = {control["target_actions"][target["id"]]: reference_frequency(stimulus, target)
               for target in stimulus["targets"]}
print(json.dumps({"decoder": decoder, "stimulus": stimulus, "frequencies": frequencies}))
"""


def path_from_root(value):
    return (ROOT / Path(os.path.expandvars(value.strip())).expanduser()).resolve()


def require_file(value, label):
    if not value.strip() or not (path := path_from_root(value)).is_file():
        raise ValueError(f"Choose a valid {label}.")
    return path


def decoder_python(folder, value):
    if value.strip():
        return require_file(value, "decoder Python executable")
    for path in (folder / ".venv/Scripts/python.exe",
                 Path.home() / ".conda/envs/venv/python.exe", Path(sys.executable)):
        if path.is_file():
            return path
    raise ValueError("Choose a valid decoder Python executable.")


def read_json_command(command, folder, timeout=15):
    startup = subprocess.STARTUPINFO(dwFlags=subprocess.STARTF_USESHOWWINDOW,
                                     wShowWindow=subprocess.SW_HIDE) if os.name == "nt" else None
    result = subprocess.run(
        command, cwd=folder, capture_output=True, encoding="utf-8", errors="replace",
        env={**os.environ, "PYTHONIOENCODING": "utf-8"},
        startupinfo=startup, creationflags=getattr(subprocess, "CREATE_NO_WINDOW", 0), timeout=timeout,
    )
    if result.returncode:
        details = (result.stderr or result.stdout).strip().splitlines()
        raise ValueError(details[-1] if details else f"Process exited with code {result.returncode}.")
    return json.loads(result.stdout)


def validate_frequencies(info, profiles):
    refresh = profiles["stimulus"]["refresh_hz"]
    for action, period in info["periods"].items():
        expected = refresh / period
        actual = profiles["frequencies"].get(action)
        if actual is None or abs(actual - expected) > info["tolerance_hz"]:
            received = f"{actual:.3f} Hz" if actual is not None else "no reference"
            raise ValueError(
                f"{TARGETS[action]} expects {expected:.3f} Hz at {refresh:g} Hz, "
                f"but the stimulus profile provides {received}."
            )


class NavigationLauncher:
    def __init__(self, root):
        self.root = root
        self.maze_process = None
        self.profile_directory = None
        self.closing = False
        root.title("SSVEP Navigation Launcher")
        root.minsize(660, 400)
        root.protocol("WM_DELETE_WINDOW", self.close)

        self.executable = tk.StringVar(value=str(ROOT / "build/windows/x64/Release/openmaze.exe"))
        self.display = tk.StringVar(value="Fullscreen")
        self.route_hint = tk.BooleanVar(value=True)
        self.accepted_command = tk.BooleanVar(value=True)
        self.mode = tk.StringVar(value=START_DECODER)
        self.output = tk.StringVar(value="EEG steering")
        self.folder = tk.StringVar(value=os.getenv("SSVEP_DECODER_DIR") or str(ROOT.parent / "ssvep-decoding"))
        self.python = tk.StringVar(value=os.getenv("SSVEP_DECODER_PYTHON", ""))
        self.port = tk.StringVar(value=os.getenv("SSVEP_DECODER_PORT") or "COM3")
        self.decoder_profile = tk.StringVar(value=os.getenv("SSVEP_DECODER_PROFILE", ""))
        self.preset = tk.StringVar(value=CUSTOM if self.decoder_profile.get() else PRESETS[0])
        self.stimulus_profile = tk.StringVar(value=os.getenv("SSVEP_STIMULUS_PROFILE", ""))
        self.status = tk.StringVar(value="Ready")
        self.stimulus_status = tk.StringVar()
        self.decoder_fields = []

        body = ttk.Frame(root, padding=12)
        body.pack(fill="both", expand=True)
        tabs = ttk.Notebook(body)
        tabs.pack(fill="both", expand=True)
        maze, decoder, stimulus = [ttk.Frame(tabs, padding=12) for _ in range(3)]
        for tab, title in zip((maze, decoder, stimulus), ("Maze", "Decoder", "Stimulus")):
            tabs.add(tab, text=title)
            tab.columnconfigure(1, weight=1)

        self.field(maze, 0, "Maze executable", self.executable, browse="exe")
        self.field(maze, 1, "Display", self.display, choices=("Fullscreen", "Windowed"))
        ttk.Checkbutton(maze, text="Show route hint", variable=self.route_hint).grid(row=2, column=1, sticky="w")
        ttk.Checkbutton(maze, text="Show accepted command", variable=self.accepted_command).grid(
            row=3, column=1, sticky="w")

        self.field(decoder, 0, "Decoder mode", self.mode, choices=tuple(MODES))
        for row, label, variable, options in (
            (1, "Decoder folder", self.folder, {"browse": "folder"}),
            (2, "Python executable", self.python, {"browse": "exe"}),
            (3, "Serial port", self.port, {}),
            (4, "Decoder output", self.output, {"choices": ("EEG steering", "Monitor only")}),
            (5, "Decoder preset", self.preset, {"choices": (*PRESETS, CUSTOM)}),
        ):
            self.decoder_fields.extend(self.field(decoder, row, label, variable, **options))
        self.custom_fields = self.field(decoder, 6, "Custom decoder profile", self.decoder_profile, browse="json")
        ttk.Label(decoder, text="Leave Python blank to look in the decoder checkout, the Conda venv, "
                               "and the launcher's Python environment.", wraplength=540).grid(
            row=7, column=0, columnspan=3, sticky="w", pady=(10, 0))
        ttk.Label(decoder, text="A running decoder uses its own settings and must use --maze-hud or --maze-control. "
                               "Press F3 in the maze to arm a steering stream.", wraplength=540).grid(
            row=8, column=0, columnspan=3, sticky="w", pady=(6, 0))

        self.stimulus_fields = self.field(stimulus, 0, "Stimulus profile", self.stimulus_profile, browse="json")
        ttk.Label(stimulus, text="Leave blank to use the decoder's default profile. "
                                "The maze fixes the frame periods and checks the active display at runtime.",
                  wraplength=540).grid(row=1, column=0, columnspan=3, sticky="w", pady=(2, 8))
        self.frequencies = ttk.Treeview(stimulus, columns=("target", "period", "frequency"),
                                        show="headings", height=4, selectmode="none")
        for key, title in (("target", "Target"), ("period", "Period (frames)"), ("frequency", "Maze frequency (Hz)")):
            self.frequencies.heading(key, text=title)
            self.frequencies.column(key, width=150, anchor="center")
        self.frequencies.grid(row=2, column=0, columnspan=3, sticky="ew")
        ttk.Label(stimulus, textvariable=self.stimulus_status, wraplength=540).grid(
            row=3, column=0, columnspan=3, sticky="w", pady=(8, 0))
        ttk.Button(stimulus, text="Refresh preview", command=self.refresh_preview).grid(row=4, column=0, sticky="w")

        actions = ttk.Frame(body)
        actions.pack(fill="x", pady=(12, 0))
        self.launch_button = ttk.Button(actions, text="Launch Maze", command=self.launch)
        self.launch_button.pack(side="left")
        ttk.Label(actions, textvariable=self.status).pack(side="left", padx=12)
        for variable in (self.mode, self.preset):
            variable.trace_add("write", self.update_fields)
        tabs.bind("<<NotebookTabChanged>>",
                  lambda _: self.refresh_preview() if tabs.select() == str(stimulus) else None)
        self.update_fields()

    def field(self, parent, row, label, variable, *, choices=None, browse=None):
        ttk.Label(parent, text=label).grid(row=row, column=0, sticky="w", padx=(0, 10))
        widget = (ttk.Combobox(parent, textvariable=variable, values=choices, state="readonly")
                  if choices else ttk.Entry(parent, textvariable=variable))
        widget.grid(row=row, column=1, sticky="ew", pady=3)
        widgets = [widget]
        if browse:
            button = ttk.Button(parent, text="Browse…", command=lambda: self.browse(variable, browse))
            button.grid(row=row, column=2, padx=(8, 0))
            widgets.append(button)
        return widgets

    def browse(self, variable, kind):
        if kind == "folder":
            value = filedialog.askdirectory(parent=self.root)
        else:
            extension = "*.exe" if kind == "exe" else "*.json"
            value = filedialog.askopenfilename(parent=self.root, filetypes=[("Executable" if kind == "exe" else "JSON", extension)])
        if value:
            variable.set(value)
            if variable is self.stimulus_profile:
                self.refresh_preview()

    def update_fields(self, *_):
        starts_decoder = self.mode.get() == START_DECODER
        groups = ((self.decoder_fields + self.stimulus_fields, starts_decoder),
                  (self.custom_fields, starts_decoder and self.preset.get() == CUSTOM))
        for widgets, enabled in groups:
            for widget in widgets:
                widget.configure(state="disabled" if not enabled else
                                 "readonly" if isinstance(widget, ttk.Combobox) else "normal")

    def stimulus_info(self, executable):
        try:
            # Older builds understand --manual and leave decoder startup to the operator.
            info = read_json_command([executable, "--stimulus-info", "--manual", "--windowed"], ROOT, timeout=5)
            if set(info["periods"]) == set(TARGETS):
                return info
        except (ValueError, KeyError, TypeError, subprocess.TimeoutExpired) as error:
            raise ValueError("Build the current maze executable for launcher support.") from error
        raise ValueError("The maze executable returned an unsupported stimulus description.")

    def refresh_preview(self):
        self.frequencies.delete(*self.frequencies.get_children())
        try:
            executable = require_file(self.executable.get(), "maze executable")
            info = self.stimulus_info(executable)
            refresh = 60.0
            if self.mode.get() == START_DECODER:
                value = self.stimulus_profile.get()
                profile = (require_file(value, "stimulus profile") if value.strip()
                           else path_from_root(self.folder.get()) / "ssvep_decoding/profiles/stimulus_60hz.json")
                if profile.is_file():
                    refresh = float(json.loads(profile.read_text(encoding="utf-8"))["refresh_hz"])
            if not math.isfinite(refresh) or refresh <= 0:
                raise ValueError("The profile needs a positive refresh rate.")
            for action, period in info["periods"].items():
                self.frequencies.insert("", "end", values=(TARGETS[action], period, f"{refresh / period:.3f}"))
            self.stimulus_status.set(f"Expected maze frequencies at {refresh:g} Hz.")
        except (OSError, ValueError, KeyError, TypeError, subprocess.SubprocessError) as error:
            self.stimulus_status.set(f"Preview unavailable. {error}")

    def decoder_settings(self, info):
        folder = path_from_root(self.folder.get())
        if not (folder / "ssvep_decoding/__main__.py").is_file():
            raise ValueError("Choose an ssvep-decoding checkout.")
        python = decoder_python(folder, self.python.get())
        port = self.port.get().strip()
        if not port:
            raise ValueError("Enter the decoder serial port.")
        preset = self.preset.get()
        custom = require_file(self.decoder_profile.get(), "decoder profile") if preset == CUSTOM else ""
        stimulus = self.stimulus_profile.get()
        stimulus = require_file(stimulus, "stimulus profile") if stimulus.strip() else ""
        profiles = read_json_command([python, "-c", RESOLVE_PROFILES, preset, custom, stimulus], folder)
        validate_frequencies(info, profiles)
        self.profile_directory = TemporaryDirectory(prefix="ssvep-maze-")
        directory = Path(self.profile_directory.name)
        for name in ("decoder", "stimulus"):
            (directory / f"{name}.json").write_text(json.dumps(profiles[name], indent=2), encoding="utf-8")
        return {"SSVEP_DECODER_DIR": str(folder), "SSVEP_DECODER_PYTHON": str(python),
                "SSVEP_DECODER_PORT": port, "SSVEP_DECODER_PROFILE": str(directory / "decoder.json"),
                "SSVEP_STIMULUS_PROFILE": str(directory / "stimulus.json")}

    def launch(self):
        try:
            executable = require_file(self.executable.get(), "maze executable")
            self.status.set("Checking launch settings…")
            self.root.update_idletasks()
            info = self.stimulus_info(executable)
            mode = self.mode.get()
            flags = list(MODES[mode])
            env = os.environ.copy()
            for enabled, flag in ((self.display.get() == "Windowed", "--windowed"),
                                  (not self.route_hint.get(), "--no-route-hint"),
                                  (not self.accepted_command.get(), "--no-accepted-command")):
                if enabled:
                    flags.append(flag)
            if mode == START_DECODER:
                env.update(self.decoder_settings(info))
                if self.output.get() == "Monitor only":
                    flags.append("--monitor-only")
            self.maze_process = subprocess.Popen([executable, *flags], cwd=ROOT, env=env)
        except (OSError, ValueError, KeyError, TypeError, subprocess.SubprocessError) as error:
            self.cleanup_profiles()
            self.status.set("Launch failed")
            messagebox.showerror("Could not launch the maze", str(error), parent=self.root)
            return
        self.launch_button.configure(state="disabled")
        self.status.set("Maze running")
        self.root.after(250, self.poll_maze)

    def cleanup_profiles(self):
        if self.profile_directory is not None:
            self.profile_directory.cleanup()
            self.profile_directory = None

    def poll_maze(self):
        code = self.maze_process.poll()
        if code is None:
            self.root.after(250, self.poll_maze)
            return
        self.maze_process = None
        self.cleanup_profiles()
        if self.closing:
            self.root.destroy()
            return
        self.launch_button.configure(state="normal")
        self.status.set(f"Maze exited (code {code})" if code else "Maze closed")

    def close(self):
        if self.maze_process is None:
            self.root.destroy()
        else:
            self.closing = True
            self.root.withdraw()


if __name__ == "__main__":
    root = tk.Tk()
    NavigationLauncher(root)
    root.mainloop()
