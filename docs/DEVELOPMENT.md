# Entwicklungsstand AMIX1.1.2 / c57

`main` übernimmt den am17.11. vorbereiteten c57-Port und die nachfolgenden Funktionsbeobachtungen. `hw.c`/`hw.h`, Ring-/RX-Fix und Polling bleiben unverändert; nur `Master` und DLM-Wrapper verwenden57 statt56. X-Surf behält56, der offizielle Kernel67 bleibt bytegleich. Composer/Kit/Manifest/Node/link.d werden gemeinsam angepasst, kein Kernelneubau und keine neue Leistungsoptimierung.

Die bisherige c56-Fassung ist auf `legacy/c56-before-1.1.2` erhalten. `compat/amigaux-1.1.2-c57` bleibt der historische Vorbereitungsbranch. Der verworfene Early-TX-Branch bleibt getrennt und wird nicht nachmain übernommen. Die frühere Z3/LANIDE-Störung ist nicht gelöst; der Eigentümer hat LANIDE inzwischen ausgebaut und nutzt das separate ZZ9000-Ethernetprojekt.

Die nachfolgende Entwicklungshistorie beschreibt die ursprüngliche c56-Linie und damalige CLI-/Kernelentscheidungen. Für aktuelle Installation gelten BUILD/INTEGRATION und der1.1.2-Portbericht, nicht alte Rebuildanweisungen.

---

# Entwicklungsverlauf

Die Entwicklung erfolgte in kleinen getrennten Prüfständen; dies ist eine
Zusammenfassung, nicht die unveränderte private Git-Historie.

| Stufe | Ergebnis |
| --- | --- |
| 0 | AutoConfig-Erkennung und ABI-/Kernelkit-Prüfung, noch kein MMIO |
| 1 | ESTAT/MAC/Ready/Link nur lesen |
| 2 | Kontrollierter Testregister-Schreibzugriff mit Wiederherstellung |
| 3 | SRAM-/Endian-/Ringvertrag prüfen |
| 4 | Vorbedingungen/Controllerstatus untersuchen, POR-EIE korrekt behandeln |
| 5 | Echte ARP-Anfrage/-Antwort mit Rücksetzen des Diagnosezustands |
| 6 | Persistenter STREAMS/DLPI-Provider, c56, IP/ARP, DHCP und SSH |
| RX-Fix | Fehlerhafte Frames sicher verwerfen statt RX global zu stoppen |
| Vollimage | Normales Bootmanifest, LAN/DHCP/SSH automatisch, Rückfall erhalten |

## Integrationsfehler und ihre Korrekturen

- Privater `IF_CONF`-Pfad allein genügt nicht: `dhcpc` liest Linkdefinitionen
  fest aus `/etc/inet/link.d`. Daher wird `lanide` dort installiert.
- Das native alte `mkdir -p` verhielt sich beim bestehenden Verzeichnis nicht
  wie erwartet. Vor Anlage explizit auf Verzeichnisexistenz prüfen.
- `_KERNEL` um `net/if.h` erzeugte private COMMON-Definitionen von Kernel-
  Variablen. Der Provider liest für das öffentliche `ifstats`-Layout diesen
  Header ohne `_KERNEL`; Modulprüfung danach ohne diese Warnungen.
- Dropbear-PTY-Problem: SIGCHLD unterbrach den libc-`grantpt`-Hilfsprozess.
  Separater Patch maskiert das Signal begrenzt um den Aufruf, stellt Maske
  und errno wieder her; native Kaltstart-SSH-Prüfung folgte.
- Aktives Dropbear niemals in place überschreiben. Über eine neue Datei
  und atomisches Umbenennen installieren; Hostkeys/Konten unverändert halten.
- Major 56 im Kernel **und** in der nativen Composer-Registry nachtragen.
  Reihenfolge der vorhandenen Major-Datensätze nicht beliebig verändern.

## Verworfener Performancezweig

Upload-Daten trafen am Linux-FTP-Server im Median etwa alle 16,65 ms ein.
Das passt zum AMIX-Tick von 60 Hz und dem einen, erst beim Timer freigegebenen
TX-Puffer. Datei- und `/dev/null`-Downloads waren ähnlich schnell; begrenzte
CPU-Proben zeigten überwiegend Leerlauf. Dies stützt eine Timerbegrenzung,
ist aber keine vollständige TCP-/Wire-Level-Analyse.

Zusätzliche TX-Abschlussprüfung beim Senden brachte in den Softwaretests
keinen Fehler, auf echter Hardware jedoch keinen funktionierenden LAN-Weg.
Die Ursache ist offen. Der Stand wurde verworfen; dieses Repository liefert
nur den wiederhergestellten stabilen RX-Fix. Kein Kandidat wird als Update
für das aktive System angeboten.

Die tatsächlich benutzte ältere `kbootset`-CLI kennt **kein `default ID`**.
Für den einmaligen Rückweg wurde `next ID` verwendet, danach mit der zu
dieser Fassung passenden `rollback`-/`mark-bad`-Logik dauerhaft bereinigt.
Ein neuerer Quellbaum darf nicht als CLI-Dokumentation eines älteren Binarys
herangezogen werden. Bootauswahländerungen müssen zur installierten Fassung
und zum tatsächlichen Storezustand passen.
