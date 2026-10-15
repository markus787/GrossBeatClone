# GrossBeatClone – Anleitung (Windows)

Eigener Zeit-/Lautstärke-Effekt im Stil von Gross Beat (VST3).

## Einmalig installieren
1. **Visual Studio 2022 Community** (kostenlos) – beim Installieren "Desktopentwicklung mit C++" anhaken.
2. **CMake** (cmake.org, beim Installieren "Add to PATH" wählen).
3. **Git** (git-scm.com).

## Bauen
Im Ordner `GrossBeatClone` ein Terminal öffnen (PowerShell):

    cmake -B build -G "Visual Studio 17 2022" -A x64
    cmake --build build --config Release

Beim ersten Mal lädt CMake JUCE herunter (dauert ein paar Minuten, Internet nötig).

## In FL Studio laden
1. Den Ordner `GrossBeatClone.vst3` suchen:
   `build\GrossBeatClone_artefacts\Release\VST3\`
2. Ihn nach `C:\Program Files\Common Files\VST3\` kopieren.
3. FL Studio: *Options > Manage plugins > Find installed plugins*.
4. Plugin auf einen Mixer-Slot legen (es ist ein Effekt).

## Bedienung
- **Zyklus**: Länge des Musters (Beats/Takte)
- **Zeit-Kurve**: Stutter, Tape Stop, Reverse, Scratch ...
- **Lautstärke-Kurve**: Gates, Fade, Pump
- **Mix** und **Ausgang** wie üblich

Neue Kurven hinzufügen: `Source/Curves.h`.
