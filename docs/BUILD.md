> Historischer c56-Stand. Für diesen c57-Branch gilt stattdessen der [1.1.2-Portbericht](../compat/amigaux-1.1.2/README.md). Keine alte c56-Installation auf1.1.2 übernehmen.

# Bauen und Reproduzieren

## Hosttests (Linux)

Benötigt: GNU make, C-Compiler, SHA-256-Werkzeug; für `sanitize` ASan/UBSan.
Keine AMIX-Bibliotheken und keine Hardware erforderlich.

```sh
make check-sources test sanitize
```

`src/` ist bis auf das portable Modul-Makefile ein bytegleicher Export des
abgenommenen stabilen RX-Fix-Quellstands. Die Hashdatei bindet die sieben
Originaldateien. Das Testprogramm inkludiert `hw.c` und ersetzt DDI/MMIO
mit einem Fehler injizierenden Modell. STREAMS bleibt ein eigener nativer Test.

## Cross-Toolchain und k46-Kit

Geprüfte Basis (Links und vollständige Pins: [SOURCES.md](SOURCES.md)):

- Period-Compiler GCC 2.7.2.3 / binutils 2.8.1 aus `jusii/gcc-cross-amix`,
  Pin `33ebadcf6976ee65dbd1e25a586f59da55e5942d`.
- AMIX-DLM-Quellen auf `46ef7d1a4d50db04120e26535fcaa86cce8cf042`,
  ergänzt um die c56-Registry in `patches/amix-c56-registry.patch`.
- Generiertes Kit `kernel/dlm/build/kit/k46` aus genau diesem Stand, mit
  passendem 040/060-Port. `make kit` benötigt explizit `GCA` und `PORT`.
- Native Box-Werkzeuge wie `kbootset` benötigen separat GCC14 mit bestandenem
  AMIX-ABI-Selbsttest. Ein beliebiger vorhandener m68k-GCC14 genügt nicht.

Patch anwenden, **bevor** Kit und nativen Composer erzeugt werden:

```sh
# Im separaten AMIX-Quellcheckout am genannten Pin:
git apply --check /path/to/amix-lanide/patches/amix-c56-registry.patch
git apply /path/to/amix-lanide/patches/amix-c56-registry.patch
# Danach nach AMIX-DLM-Dokumentation Kit, Kernel und Box-Tools erzeugen.
```

Major 56 ist eine lokale Registrierung dieses Entwicklungsstands, keine
Behauptung einer dauerhaften Upstream-Zuteilung. Ein anderes Kernelregister
muss auf Konflikte geprüft werden.

## Treiber und Statuswerkzeug

```sh
# Installiertes Prefix des gepinnten Period-Compilers auf PATH:
export PATH=/path/to/gcc-cross-amix/build/prefix/bin:$PATH
make module KIT=/absolute/path/to/amix/kernel/dlm/build/kit/k46
make simulation KIT=/absolute/path/to/amix/kernel/dlm/build/kit/k46
make userland
```

Ausgaben:

- `build/production/mod.d/lanide`: echte Hardware, **14855 Bytes**,
  SHA-256 `e406881216f7ba472fac0b5d32e5c38de25281ba51c2fe1ae1c3d457582d0f47`.
- `build/simulation/mod.d/lanide`: No-MMIO-Simulation, **14899 Bytes**,
  SHA-256 `ac02d23673fa59217b4c9aaeb4af3370a4ffd32374209a6a919ba9c50193154f`.
- `build/lanctl`: natives Status-/DLPI-Testwerkzeug.

Beide Moduldateien heißen `lanide`; niemals die Simulationsausgabe an den
Produktionspfad kopieren. Das Kit führt `modfix`/`modcheck` aus. Die geprüfte
Import-Minor-Version ist k46.1.2, der verwendete Kernel bietet k46.1.4.
Unterschiedliche Nummern sind hier kein Widerspruch: maßgeblich sind die
importierten ABI-Namen und die Kitprüfung.

Vor Veröffentlichung wurden beide Module aus diesem Export in einem neuen
Buildverzeichnis erneut gebaut: **beide Dateihashes stimmen bytegenau mit
den vorher abgenommenen Artefakten überein**. Dafür wurde das bereits
geprüfte bestehende Kit verwendet; das ist kein frischer Komplettbau von
Compiler, Kernel und Kit aus einem leeren Rechner.

## Kernel-Rezept und Grenzen der Reproduktion

Der ursprüngliche Cut `68060-261106-66` wurde zuerst vollständig gebaut;
alle acht Referenzhashes stimmten. Darauf wurde die zusätzliche Registry
in einen getrennten Kernel `68060-261114-00` eingebaut, passendes UFS
gebaut und die Baseline danach erneut unverändert geprüft.

`archive/kernel-build/` enthält die damaligen zusätzlichen Relink-/Post-
Schritte als technische Aufzeichnung. Das gemeinsame Arbeitsverzeichnis
wird über `AMIX_WORK_ROOT` angegeben. **Diese Dateien sind kein fertiger
Bootstrap- oder Installationsbefehl**: Sie erwarten den vorher aufgebauten
Cut, dessen Verzeichnisstruktur und lokale Quellen/Commitobjekte. Einige
Prüfschritte beziehen sich auf diese eingefrorene Buildumgebung. Nicht
unverändert auf ein produktives System ausführen.

Der öffentliche Einstieg zur Basis ist `cuts/68060-261106-66/cut.sh` am
gepinnten Port-Checkout. Dessen Anforderungen (AMIX_SYSTEM, NetBSD-FPSP-
Archiv, GCA, G14/binutils 2.43.1, PAULA) gelten unverändert. Die eigene
Build-ID `00` ist nur eine isolierte Entwicklungszuordnung, kein offizieller
Release. Dieses Repository verteilt keinen vorgebauten Kernel oder UFS.

## Native Prüfungen

In einem isolierten AMIX-Testimage **ohne echte Karte** kann der getrennte
Simulationsbuild benutzt werden. Die reguläre Boot-/Modulladefreigabe des
Testimages muss dazu passen; nicht zusätzlich auf dem laufenden A3000 laden.

```sh
# Nur auf dem nachweislich geladenen No-MMIO-Simulationsprovider:
/path/to/lanctl -t /dev/lan0
```

Erwartet: 82 DLPI-Prüfungen. `lanctl -t` prüft `SIM=1` und verweigert den
Test auf dem echten Provider. Danach das Testimage wieder mit dem echten
Modul kalt starten; Kartenabwesenheit muss vor MMIO mit ENXIO enden.
