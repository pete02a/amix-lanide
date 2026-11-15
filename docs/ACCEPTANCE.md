# Abnahme – Stand 15. November 2026

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
