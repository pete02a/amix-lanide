# AMIX LANIDE Ethernet-Treiber

Persistenter **STREAMS/DLPI-Treiber für die Zorro-LAN-IDE-Netzwerkkarte mit
ENC624J600**. Der hier veröffentlichte Stand ist der funktionierende RX-Fix vom
15. November 2026, nicht der später verworfene Performanceversuch.

## Vorbereitung auf amigaux 1.1.2

Der separate [Kompatibilitätsbranch `compat/amigaux-1.1.2-c57`](https://github.com/pete02a/amix-lanide/tree/compat/amigaux-1.1.2-c57) enthält den c57-Port, Registry-/Composer-Patch, Paketrezepte und Testnachweise. **Noch keine c57-Hardwarefreigabe und kein vollständiges 1.1.2-Upgrade.** Der offizielle Zielkernel bleibt unverändert; c56 gehört dort X-Surf. Der hier auf `main` verbleibende c56-Treiber ist für die bisherige eigene Basis bestimmt, nicht unverändert für 1.1.2.

## Geprüfter Stand

Auf einem **echten Amiga 3000 mit CyberStorm Mk II, 68040 und 64 MiB RAM**
unter AMIX/Amiga UNIX 2.1c mit dem AMIX-1.1-k46-DLM-Unterbau bestätigt:

- Ethernet in beide Richtungen, IP/ARP und DHCP.
- Automatischer Start von LAN und SSH nach dem Einschalten, ohne `run-net`.
- Router-, DNS- und Internet-Ping sowie SSH-Anmeldung mit Terminal.
- FTP-Dateien mit 1 KiB, 64 KiB, 1 MiB und 8 MiB herunter- und zurückgeladen;
  alle Rückübertragungen SHA-256-identisch.
- Etwa **110 KB/s Download und 82 KB/s Upload** bei 8 MiB. Das sind
  beobachtete FTP-Clientwerte, keine zugesicherte Geschwindigkeit.
- Natives ECS-X (`xstart card=native`) funktioniert im integrierten Vollimage
  weiterhin; die Grafikkonfiguration wurde dafür nicht verändert.

**Kein Langzeit-Stabilitätsnachweis:** Die erfolgreichen Abnahmen und die
Grenzen sind in [Hardwareabnahme](docs/ACCEPTANCE.md) getrennt aufgeführt.

## Schnellstart für Entwickler

Ohne AMIX oder Karte auf einem Linux-Host:

```sh
make check-sources
make test
make sanitize
```

Dies prüft **412 Hardwaremodell-Fälle**, nicht die komplette STREAMS-Schicht
und nicht die echte Karte. Für Kernelmodul und `lanctl` werden eine passende
AMIX-Cross-Toolchain und ein generiertes k46-DLM-Kit benötigt:
[Buildanleitung](docs/BUILD.md).

## Inhalt

| Pfad | Inhalt |
| --- | --- |
| `src/lanide.c` | STREAMS/DLPI-Provider, acht Streams und SAP-Demultiplexing |
| `src/hw.c`, `src/hw.h` | MMIO/SRAM, TX/RX, Ringvalidierung und Lebenszyklus |
| `src/status.h`, `src/lanctl.c` | Status-IOCTL und native Abfrage/Simulationstests |
| `src/test-hw.c` | Host-Hardwaremodell mit Fehlerfallprüfungen |
| `src/Master` | Bootklasse, Kartenkennung, Major 56, kein Entladen |
| `patches/` | AMIX-Major-/Composer-Registrierung und separater SSH-PTY-Fix |
| `config/` | Globale Linkdefinition und Bootmanifest-Fragment |
| `archive/kernel-build/` | Historische Kernel-Buildschritte, kein Installer |
| `evidence/` | Quell-/Artefakthashes und bereinigter Prüfstand |

## Installation und Betrieb

Ein Modul allein reicht nicht: **Character-Major 56 muss im Kernel und im
Boot-Composer registriert sein.** Passender Kernel, UFS-Modul, Registry und
Bootmanifest gehören zusammen. Siehe [Integration](docs/INTEGRATION.md).

Im integrierten System genügt zum Prüfen:

```sh
/opt/lanide/bin/lanctl
/usr/sbin/ifconfig lan0
```

`SIM=0`, `RUNNING=1`, `ERROR=0` und erfolgreicher Datenverkehr sind gemeinsam
maßgeblich. `RXERR`/`DROPPED` können beim Verwerfen fehlerhafter Frames steigen.
Die Zähler allein beweisen keine funktionierende Verbindung.

## Grenzen

- Polling, **kein Interruptbetrieb**, vier verbrauchte RX-Frames pro Systemtick.
- Ein TX-Puffer; keine in diesem Stand abgenommene Leistungsoptimierung.
- Kein Controller-Reset und keine PHY-/EIE-/Board-IRQ-Schreibzugriffe.
- `unload never`: kein Austausch des aktiven Moduls im laufenden System.
- Die angegebene Karte/68040-Konfiguration ist geprüft; daraus folgt keine
  Abnahme aller LAN-IDE-Revisionen, Clockportvarianten oder 68060-Systeme.
- Die experimentelle eigene Kernel-Build-ID ist kein offizielles AMIX-Release.
- SD-Images, Kernel-/UFS-/SSH-Binaries, ROMs, private Logs und Zugangsdaten
  sind nicht Bestandteil dieses Quellpakets.

[Architektur](docs/ARCHITECTURE.md) · [Entwicklungsverlauf](docs/DEVELOPMENT.md)
· [Quellen und Pins](docs/SOURCES.md)
