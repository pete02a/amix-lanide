> Historischer c56-Stand. Für diesen c57-Branch gilt stattdessen der [1.1.2-Portbericht](../compat/amigaux-1.1.2/README.md). Keine alte c56-Installation auf1.1.2 übernehmen.

# Integration in AMIX und automatischer LAN-Start

Dies beschreibt die erfolgreich umgesetzte Vollimage-Integration. Es ist
kein universeller Installer und setzt einen zur Registry passenden Kernel,
UFS, Loader/Composer und ein separat gesichertes Testmedium voraus. Der
laufende stabile A3000 wird durch dieses Quellpaket nicht verändert.

## Zusammengehörige Komponenten

1. Eigener k46-Kernel mit freier, tatsächlich registrierter Character-Major
   56 (`lanide`), dazugehöriges UFS-Modul und korrekte ABI-/Symboltabellen.
2. Native `kbootset`-Fassung aus derselben Registry. Nur die Registrydatei
   zu ändern genügt nicht, wenn der installierte Composer noch das alte
   generierte `kreg.c` verwendet. Der Patch enthält beide Änderungen.
3. Produktionsmodul, SHA-gebunden im normalen Bootmanifest:
   `class=boot`, Match `zorro:0a1c:7b`, `majors=c:56`, kein devload nötig.
4. `/dev/lan0`: Character 56, Minor 0; Linkdefinition und DHCP-Auswahl.
5. Separat reparierter SSH-Server, falls die ältere PTY-Störung vorliegt.

`config/bootmanifest.example` ist nur das LANIDE-Fragment. Generation,
Kernel, Rootcontroller/-unit/-slice und alle anderen Module/Parameter müssen
vom tatsächlichen Zielsystem übernommen werden. Keine fremde Build-Spec
mitsamt fremdem Rootgerät verwenden. Die Reihenfolge der vorhandenen Major-
Einträge beibehalten und LANIDE ergänzen. Den Inhalt der alten guten
Rückfallgeneration nicht überschreiben.

Über den nativen Composer wird die neue Generation transaktional gespeichert
(`commit --spec ...`), danach kontrolliert gebootet und geprüft. Die genaue
CLI mit der **installierten Fassung** abgleichen. Der hier eingesetzte
ältere Composer hat `next`, `rollback`, `mark-bad`, aber **kein `default`**.
Ein normales Boot ohne Entwicklungsfreigabe war Teil der Abnahme.

## Globale Linkdefinition und DHCP

`config/lanide.link` muss unter `/etc/inet/link.d/lanide` liegen:

```text
lan /dev/lan 0 56
```

Der AMIX-DHCP-Client liest dieses Verzeichnis fest. Ein privates `IF_CONF`
für `ifup` allein ersetzt diese globale Linkzuordnung nicht.

Die vorhandene `/etc/inet/interfaces`-DHCP-Regel soll `lan0` erfassen. In
einem passenden privaten Testprofil lautete die explizite Zeile:

```text
lan0 dhcp
```

Im geprüften Vollimage blieb die bestehende Wildcard-DHCP-Konfiguration
unverändert. `inetinit`/`S69inet` ruft `ifup -a` auf; kein zusätzlicher
LANIDE-Startdaemon und kein manuelles `run-net` sind nötig.

Bei der Anlage von Verzeichnissen beachten: Das native alte `mkdir -p`
war bei existierendem Verzeichnis kein zuverlässiger Startpfad. Die geprüfte
Form ist eine explizite Existenzprüfung und nur dann einfaches `mkdir`.

## SSH ist eine separate Komponente

Der abgenommene Dropbear-Build enthält den Signal-/`grantpt`-Fix aus
`patches/dropbear-grantpt.patch`. Grundlage ist das AMIX-1.1-Dropbear-Rezept
am in [SOURCES.md](SOURCES.md) genannten Pin, nicht ein unmodifiziertes
Upstream-Tarball. Das Buildrezept kompiliert die zusätzliche Datei und
verlinkt sie; vorhandene Paketpatches bleiben bestehen. Der lokale Build
war kein offizielles Paketrelease.

Bestehende Accounts und Hostschlüssel bleiben erhalten. Der Server wurde
über den vorhandenen `S76dropbear`-Dienst auf Port 22 gestartet. Bei einem
Austausch erst eine **neue Datei** übertragen und atomisch umbenennen;
ein laufendes Dropbear-Binary nicht in place überschreiben. Die zum
Multicall-Binary gehörenden Alias-Hardlinks konsistent aktualisieren.
Das alte native init-Skript unterstützt `start`/`stop`, kein `restart`.

## Abnahme auf Hardware

Nach normalem Start ohne `run-net`:

```sh
/opt/lanide/bin/lanctl
/usr/sbin/ifconfig lan0
# Router-IP des eigenen LANs einsetzen:
ping ROUTER_IP
```

DHCP-Adresse, Routerverbindung, DNS-Ziel, SSH-Anmeldung und längere binäre
Übertragung mit Prüfsummen prüfen. Vorher/nachher Zähler vergleichen.
`SIM=0`, `RUNNING=1`, `ERROR=0` sind nötig, aber nicht allein hinreichend.

## Was nicht als Teil des Images behauptet wird

Deutsche Konsolenkeymap und SSH-Korn-Shell-Tasten wurden später direkt auf
der physischen SD eingerichtet und vom Benutzer bestätigt. Sie sind nicht
in den früher exportierten Vollimagebytes enthalten und sind unabhängig
vom LANIDE-Treiber. Grafik, X-Start und Benutzerpasswort wurden für die
LANIDE-Integration nicht umkonfiguriert.
