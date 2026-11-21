# Integration in AMIX1.1.2 mit c57

Dies ist eine koordinierte Test-/Integrationsreferenz, kein universeller Installer. Die frühere c56-Integration ist im [Legacy-Branch](https://github.com/pete02a/amix-lanide/tree/legacy/c56-before-1.1.2) erhalten.

## Komponenten

1. Offizieller k46-Kernel `68060-261115-67` unverändert, mit passendem Core-UFS. Kernel-SHA256 `454d5d652d25a7862601352d64b41164ea5db03975535b3076a9371db1545799`.
2. Privater c57-Composer/Registry und passendes Kit. Corepatch0051 prüft die Majors gegen das **gebootete Manifest**, nicht nur die einkompilierte Fallbacktabelle. Keine Kernel-Binäränderung für diesen Port nötig.
3. Produktionsmodul im normalen Bootmanifest: Klasseboot, Match `zorro:0a1c:7b`, `majors=c:57`, `sim=0`, `unload never`.
4. `/dev/lan0`: Character57,Minor0, root-eigener geschützter Node; `/etc/inet/link.d/lanide` aus `config/lanide.link`.

c56 bleibt X-Surf, c68 VA2000. Der Composer muss die Ergänzung tatsächlich enthalten; die Textregistry allein ändert kein bereits installiertes `kreg.c`-Binary. Die Versionsfloors und Paketrezepte stehen unter [compat/amigaux-1.1.2](../compat/amigaux-1.1.2/README.md). Bei Paketinstallation muss der APKG-Client den Modul-kbootset-Floor unterstützen (im dokumentierten Aufbau0.3.13); private Versionen sind keine offiziellen Releases.

`config/bootmanifest.example` enthält nur das Modulfragment. Generation, Rootcontroller/-unit/-slice und alle Referenzdateien werden vom tatsächlichen Ziel übernommen. Nicht eine Emulator-Rootspec übernehmen. Den geladenen Provider nicht in place austauschen; neue Generation nativ transaktional komponieren, SHA/Registry/Node prüfen, danach kontrolliert booten. CLI immer gegen die **installierte** Composerfassung prüfen. Die alte c56-CLI ohne `default` ist keine allgemeine Beschreibung des neueren privaten1.1.2-Composers.

## Automatischer Netzstart

```text
# /etc/inet/link.d/lanide
lan /dev/lan 0 57
```

Die Schnittstelle kann explizit in `/etc/inet/interfaces` gewählt werden:

```text
lan0 dhcp name amix040
```

Der vorhandene `inetinit`/S69inet-Ablauf ruft `ifup -a` auf; kein eigener LANIDE-Startdaemon. Keine Wildcard-DHCP-Zeile als Beweis dafür verwenden, dass gerade die richtige NIC aktiv ist. Einen bestehenden Node mit falscher Major nicht allein wegen seiner Existenz akzeptieren.

Nach normalem Boot ohne manuelles `run-net`: `lanctl`, tatsächliche DHCP-Adresse/Gateway/DNS/Hosteintrag, Route und Router-Ping/SSH prüfen. `SIM=0,RUNNING=1,ERROR=0` allein beweist keinen funktionierenden Datenpfad. RXERR/DROPPED können beim gezielten Verwerfen fehlerhafter Frames steigen; Zeitverlauf und tatsächlicher Verkehr sind maßgeblich.

## Abgrenzung

Keine Karte wird durch diese Veröffentlichung neu eingebaut oder getestet. Die frühere LANIDE-/ZZ9000-ZorroIII-Störung bleibt ungeklärt; keine neue Z3-Unterstützung. Der Eigentümer betreibt inzwischen ZZ9000-LAN und hat LANIDE entfernt.

`patches/dropbear-grantpt.patch` und die alten SSH-/PTY-Abnahmen sind historische, separate Komponenten. Nicht blind einen aktuellen1.1.2-SSH-Server durch den alten privaten Build ersetzen. Bestehende Konten/Hostschlüssel, Grafik, Locale und CDE werden vom LANIDE-Port nicht geändert.
