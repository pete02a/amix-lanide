# Bauen: aktueller AMIX-1.1.2-/c57-Stand

## Hostmodell

```sh
make check-sources test sanitize
```

Die aktuelle Hashliste ist `evidence/c57-sources.sha256`. Sie bindet sieben Treiber-/Testdateien; das portable `src/Makefile` ist Buildglue.412 Hardwaremodellfälle sowie dieselben Fälle unter ASan/UBSan prüfen ausgewählte MMIO-/Ring-/Fehlerpfade, nicht komplette STREAMS- oder echte Kartenfunktion. Absichtliche negative RX-/Probe-Meldungen sind Teil der Fehlerfalltests.

## Gepinnte Basis

- AMIX1.1.2: `ec9dfd6e57070fddd8a34b8cb83236257ecf0ecf` aus amigaux/amix.
- Period-Compiler GCC2.7.2.3/binutils2.8.1: gcc-cross-amix `33ebadcf6976ee65dbd1e25a586f59da55e5942d`.
- Passendes generiertes k46-Kit mit privater c57-Registry. Für Kit/Box-Werkzeuge gelten die externen GCA/G14/PORT-Anforderungen des gepinnten DLM-Baums; GCC14 für den Composer muss den AMIX-ABI-Selbsttest bestanden haben.

```sh
# Im isolierten AMIX-Checkout am genannten Pin:
git apply --check /path/to/amix-lanide/compat/amigaux-1.1.2/amix-1.1.2-c57.patch
git apply /path/to/amix-lanide/compat/amigaux-1.1.2/amix-1.1.2-c57.patch
# Kit und privaten Composer nach kernel/dlm-Dokumentation erzeugen.
# Kein Neubau des offiziellen Kernels nur wegen c57.
```

Die Recipepins enthalten lokale Provenienzobjekte und private Paketversionen. Der Patch liefert die Änderungen, keine öffentliche Upstream-Paketzusage. Neue Composerbuilds müssen separat validiert/gepinnt werden; ein historischer Referenzhash ist kein automatisches Ergebnis auf fremden Rechnern. Details stehen im [c57-Portbericht](../compat/amigaux-1.1.2/README.md).

## Modul und Statuswerkzeug

```sh
export PATH=/path/to/period-prefix/bin:$PATH
make module KIT=/absolute/path/to/generated/c57/k46
make simulation KIT=/absolute/path/to/generated/c57/k46
make userland
```

| Ausgabe | Zweck | Referenz-SHA256 |
|---|---|---|
| `build/production/mod.d/lanide` (14855B) | echte Karte | `4cab27b0e9dc24c828f67f3fcbad8c202048009078076fa2569df8bc045e8c29` |
| `build/simulation/mod.d/lanide` (14899B) | No-MMIO-Simulator | `89c4b54acb3ad1482fd1a4a5f24bbec23810d0b19578ecb515a9e142e24fc96d` |
| `build/lanctl` | native Status-/DLPI-Abfrage | frisch gebaute Ausgabe separat erfassen |

Beide Providerdateien heißen `lanide`; die Simulationsausgabe darf nicht ins Hardwareimage. Die Import-Minor ist k46.1.2, der offizielle Kernel67 bietet k46.1.6. Entscheidend sind die importierten ABI-Namen/Kitprüfung, nicht die bloße Gleichheit der Versionszahlen.

Die c57-Produktion unterscheidet sich binär an genau zwei Stellen vom alten c56-Referenzmodul: Offset6879 (0-basiert)56→57, Offset7271 ASCII6→7; Dateigröße unverändert. Kein RX-/TX-Umbau.

Aktuelle Veröffentlichungsprüfungen werden in `evidence/amigaux112-publication-20261121.json` erfasst. Der frische Modulbau benutzt das bereits geprüfte Kit/Compiler-Setup; kein vollständiger Bootstrap aus einem leeren Rechner. Historische82 native DLPI-Checks werden nicht als gerade neu ausgeführt bezeichnet. Auf dem nachweislich geladenen **Simulator** kann `lanctl -t /dev/lan0` diese Schnittstellen prüfen; auf echter Karte verweigert das Werkzeug den Test.

## Legacy

Die alte Kernelneubau-/c56-Anleitung und ihre früheren Binärhashes gelten nur für den [Legacy-Branch](https://github.com/pete02a/amix-lanide/tree/legacy/c56-before-1.1.2). `archive/kernel-build/` und `patches/amix-c56-registry.patch` bleiben historische Aufzeichnungen, sind **nicht der1.1.2-Installationsweg**. Der aktuelle offizielle Kernel67 bleibt unverändert.
