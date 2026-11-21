# Abnahme – aktueller c57-Stand und getrennte Historie

## AMIX1.1.2 / c57

Der aktuelle Hauptzweig entspricht dem stabilen RX-Fix mit c57-Anpassung, nicht dem Early-TX-Kandidaten. Identität: Produktionsmodul14855B, SHA256 `4cab27b0e9dc24c828f67f3fcbad8c202048009078076fa2569df8bc045e8c29`, offizieller Kernel67 bytegleich. Sieben Quelldateien wurden gegen den letzten funktionierenden privaten1.1.2-Stand verglichen; nur Major56→57 änderte den Treiber gegenüber Legacy.

Historische c57-Softwareintegration am17.11.:892 Composer-Hostprüfungen,82 native No-MMIO-DLPI-Checks, Kartenabwesenheit vor MMIO, native Paket-/Katalogprüfungen und Gast-Rücklesen. Sie sind nicht als gerade neu ausgeführte Tests oder reale Karte ausgegeben. Aktuelle frische Host-/Sanitizer-/Produktions-/Simulationsbuilds stehen separat in `evidence/amigaux112-publication-20261121.json`.

Reale spätere Beobachtungen:

- Am18.11. meldete der Eigentümer funktionierende Routerverbindung nach Kalt-/Warmstarts mit AMIX1.1.2/c57 ohne ZZ9000. Im ZZ9000-Z2-Aufbau wurden ebenfalls erfolgreicher Betrieb und CDE-Nutzung berichtet.
- Am21.11. wurde LANIDE vor dem Ausbau vorübergehend gezielt benutzt: Route überlan0, Router-Ping/SSH, realer LANIDE-Status `SIM=0,RUNNING=1,ERROR=0` sowie begrenzter FTP-Verkehr. Das bekannte c57-Modul war im eingesetzten Vollaufbau SHA-gebunden. Am Ende TX3269/RX4597/DELIVERED3993, TXERR/RXERR/TIMEOUT/DROPPED0.
- Ein vollständiger Geschwindigkeitsvergleich wurde vom Benutzer abgebrochen und gestrichen. Zweiter Upload421/Brokenpipe entstand durch den gewollten Serverstopp, kein neuer Hardwarefehler. Daraus wird weder ein vollständig hashgeprüfter c57-Rundtransfer noch ein abgeschlossener Benchmark behauptet.
- Die frühere Kombination mit ZZ9000/ZorroIII zeigte weiter LANIDE-Ausfälle; Ursache unbekannt. c57 löst die Nummernkollision mit X-Surf, nicht einen bewiesenen Bus-/Firmwarekonflikt. Karte inzwischen entfernt, keine neue Hardwareabnahme bei dieser Repository-Aktualisierung.

Keine lange Dauerlast-/Neustartserie und kein vollständig neu ausgeführtes APKG-OS-Upgrade. Öffentlicher kuratierter Beleg: `evidence/amigaux112-hardware-20261121.json`; private Fotos, IP-/MAC-Daten und Rohlogs bleiben außerhalb des Repositorys.

---

## Historischer c56-Bericht (15.11.2026, unveränderte Abgrenzung)

Die folgenden ursprünglichen Tests gehören zur älteren c56-Basis, nicht automatisch zur c57-Neuabnahme.

> Historischer c56-Stand. Für diesen c57-Branch gilt stattdessen der [1.1.2-Portbericht](../compat/amigaux-1.1.2/README.md). Keine alte c56-Installation auf1.1.2 übernehmen.

# Ursprünglicher Bericht – Stand 15. November 2026

## 1. Hostmodell

Stabiler RX-Fix: 412 Fälle bestanden, außerdem AddressSanitizer und
UndefinedBehaviorSanitizer. Vor der Veröffentlichung aus den exportierten,
SHA-geprüften Quellen erneut ausgeführt. Der Test injiziert unter anderem
Kartenabwesenheit, Mapping-/Probe-Fehler, TX-Abbruch und Timeout,
RX-Ringwrap, fehlerhafte Statuswerte und sichere/unsichere Empfangsmetadaten.
Die dabei absichtlich ausgegebenen `bad RX header ... stopped`-Meldungen
gehören zu Negativtests; maßgeblich sind Exitcode und abschließendes PASS.

## 2. Native AMIX-/Emulatorprüfungen

- Produktions- und Simulationsmodul: k46-Modulprüfung ohne COMMON-Warnungen.
- 82 native No-MMIO-DLPI-Prüfungen: acht Streams, Mehrfachbindung, Daten,
  Padding, Fehlerantworten, Schließen und Wiederöffnen.
- IP-/ARP-Streams und Interfacekonfiguration geprüft.
- Produktionsmodul ohne Karte: ENXIO vor MMIO-Zugriff, kein positiver
  Hardwarebefund daraus abgeleitet.
- Fixierter SSH-Server: Shell, PTY/controlling TTY, SCP und Kaltstart geprüft.
- Vollimage: normale Generation ohne Entwicklungsfreigabe, Modul-/Kernel-
  Prüfsummen und unveränderte Rückfallgeneration geprüft.

## 3. Echte A3000-Hardware

Nacheinander durch reale Konsole, SSH und Benutzerfotos bestätigt:

- Stage5: echte ARP-Antwort und bidirektionaler Ethernet-Grundverkehr.
- Persistenter Provider: DHCP-Adresse, drei Ping-Antworten, 0% Verlust;
  danach Router und Internetziel per DNS-Namen erreichbar.
- RX-Fix: beschädigte Frames werden gezählt und verworfen, ohne den
  Empfang global anzuhalten. SSH vom Mac und vom Entwicklungsrechner.
- FTP Binärmodus: 1 KiB, 64 KiB, 1 MiB, 8 MiB jeweils hin und zurück;
  alle zurückgeladenen Dateien SHA-256-identisch. Während dieses Tests
  keine neuen TX-/RX-Fehler oder Timeouts. Bei 8 MiB ungefähr 110 KB/s
  Download und 82 KB/s Upload.
- Großes MkII-/68040-Image: LAN und SSH sofort nach dem Hochfahren ohne
  manuelles `run-net`, Router/DNS/Internet-Ping und `RUNNING=1`, `ERROR=0`.
- Natives ECS-X mit xterm/xclock/xeyes funktioniert auch im Vollimage.

Die Originalfotos und privaten Rohlogs bleiben im Projektarchiv, nicht in
GitHub. Der öffentliche Nachweis ist ein bereinigter Abnahmebericht mit
Quell- und Artefakthashes, keine unabhängige Zertifizierung.

## 4. Bekannte Grenzen und negative Abnahme

Keine lange Dauerlast-/Neustartserie. Steigende RXERR/DROPPED allein sind
kein globaler Fehler; RUNNING/ERROR allein sind aber auch kein Netznachweis.

Ein späterer TX-Frühfreigabe-Versuch hatte 417 Hostfälle und 82 native
DLPI-Prüfungen bestanden, lieferte auf echter Hardware jedoch keine
funktionierende Verbindung zum Router, obwohl RUNNING=1/ERROR=0 angezeigt
wurde. Ursache ungeklärt. Die Fassung wurde zurückgenommen und aus der
Bootauswahl entfernt. Die stabile Generation 2 wurde danach selbst per SSH
mit Router-Ping überprüft. **Kein belegter Geschwindigkeitsgewinn.**

Die zwei bereits bekannten Vollimage-Metadatenmeldungen betreffen die alte
Rückfallgeneration ohne c56 und den gegenüber dem alten Paketdatensatz
abweichenden Entwicklungskernel. Sie sind keine neuen Laufzeitfehler,
aber auch kein vollständig bereinigtes offizielles Paketrelease.
