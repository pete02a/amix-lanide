# AMIX LANIDE Ethernet — AMIX 1.1.2 / c57

Aktueller Stand auf `main`: **stabiler RX-Fix mit der AMIX-1.1.2-Kompatibilitätsanpassung auf Character-Major57**. Schnittstelle `lan0`, Karte Zorro-LAN-IDE/ENC624J600. Kein Early-TX-/Performancekandidat und kein Zorro-III-Kompatibilitätsfix.

Der Datenpfad ist unverändert zum funktionierenden RX-Fix vom15.11.2026. Die beiden Treiberänderungen betreffen ausschließlich die Major-Nummer in `Master` und im DLM-Wrapper. Unter AMIX1.1.2 bleibt c56 für X-Surf; der offizielle Kernel67 bleibt bytegleich.

## Nachweisstand

- Reale c57-Funktion auf A3000/CyberStorm MkII68040/64MiB wurde nach der 1.1.2-Integration beobachtet: ohne ZZ9000 Router-Ping nach Starts; mit ZZ9000/ZorroII funktionierender Betrieb. Am21.11. waren Route über lan0, Router-Ping, SSH und begrenzter FTP-Verkehr direkt nachweisbar.
- **Nicht gelöst:** LANIDE-Ausfälle in der früheren Kombination mit ZZ9000/ZorroIII. Der c57-Port beseitigt keinen nachgewiesenen Bus-/Firmwarefehler. LANIDE ist inzwischen aus dem Zielrechner entfernt; diese Veröffentlichung löst keine neuen Hardwaretests aus.
- Der alte c56-Stand hatte ausführliche DHCP-/Autostart-/Hashtransfer-/ECS-X-Belege. Diese werden **nicht rückwirkend als neue c57-Tests** ausgegeben.
- Aktuelle Softwareprüfungen:412 Hostmodellfälle und ASan/UBSan; getrennte Produktions-/Simulationsbuilds. Historische c57-Integration:892 Composer-Hostchecks und82 native No-MMIO-DLPI-Prüfungen. Kein unbegrenzter Dauerlast- oder vollständiger OS-Upgrade-Nachweis.

Details: [Abnahme](docs/ACCEPTANCE.md), [c57-Portbericht](compat/amigaux-1.1.2/README.md), [aktueller bereinigter Hardwarebeleg](evidence/amigaux112-hardware-20261121.json).

## Entwicklereinstieg

```sh
make check-sources test sanitize
# Passende period-Cross-Toolchain und generiertes c57-k46-Kit:
make module KIT=/absolute/path/to/c57/k46-kit
make simulation KIT=/absolute/path/to/c57/k46-kit
make userland
```

Produktionsmodul:14855Bytes, SHA256 `4cab27b0e9dc24c828f67f3fcbad8c202048009078076fa2569df8bc045e8c29`. Simulator und echte Hardware sind strikt getrennt; niemals den Simulator ins Hardwareimage übernehmen.

[Build](docs/BUILD.md) · [Integration](docs/INTEGRATION.md) · [Architektur](docs/ARCHITECTURE.md) · [Entwicklung](docs/DEVELOPMENT.md) · [Quellen](docs/SOURCES.md)

## Zusammengehörige Integration

Privater c57-Composer/Registry, passendes Kit, Modul, **Bootmanifest-Majortabelle**, `/dev/lan0`57,0 und globale `link.d/lanide` müssen übereinstimmen. Ein neuer Kernel nur für die Majornummer ist bei der gepinnten1.1.2-Basis nicht nötig. Die lokale Nummer ist keine dauerhafte Upstream-Reservierung; die privaten Paketversionen sind keine offiziellen Releases.

Der reguläre AMIX-Netzstart kann über `inetinit`/`ifup`/`dhcpc` erfolgen; keine neue Startschleife. Root-/Controller-/Generationswerte aus Beispielen nie unverändert auf einen anderen Rechner übernehmen.

## Historie und Abgrenzung

- [Legacy c56 vor1.1.2](https://github.com/pete02a/amix-lanide/tree/legacy/c56-before-1.1.2): alter funktionierender Stand mit seinen damaligen Build-/Abnahmebelegen.
- [Kompatibilitätsvorbereitung](https://github.com/pete02a/amix-lanide/tree/compat/amigaux-1.1.2-c57): unveränderter historischer Vorbereitungsbranch; `main` enthält nun c57 plus aktualisierte Dokumentation.
- [Verworfener Early-TX-Versuch](https://github.com/pete02a/amix-lanide/tree/archive/rejected-early-tx-20261115): ausdrücklich kein funktionierendes Update, nicht nach `main` übernommen.
- [ZZ9000-LAN-Entwicklung](https://github.com/pete02a/amix-zz9000-lan) ist ein getrenntes Projekt.

Keine SD-Images, Kernel-/UFS-/SSH-Binaries, ROMs, privaten Rohlogs oder Zugangsdaten in diesem Repository. Polling und ein einzelner TX-Puffer bleiben; kein Interrupt-, PHY-, Reset- oder Leistungsoptimierungsauftrag ist hier enthalten.
