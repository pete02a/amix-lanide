# VERWORFEN: LANIDE Early-TX-Entwicklungsstand vom 15.11.2026

**Dieser Archiv-Branch enthält einen auf echter Hardware gescheiterten Kandidaten.
Nicht als funktionierenden Treiber installieren.** Der stabile Treiber bleibt auf
[main](https://github.com/pete02a/amix-lanide/tree/main).

Veröffentlicht am 16.11.2026 auf ausdrücklichen Wunsch des Eigentümers zur Analyse,
nicht als neue Version oder Freigabe. Kein Pull Request und kein Merge nach main.

## Was wurde geändert?

- `src/hw.c`: `lanide_hw_txpoll()` erkennt einen bereits abgeschlossenen TX ohne
  Warteschleife; Statusauswertung und Freigabe auch vor dem nächsten Timertick.
- `src/lanide.c`: `wsrv()` ruft die frühe Abschlussprüfung bei belegtem TX-Puffer
  auf; zusätzlich maximal vier Queue-Einträge pro Aufruf.
- Der Timer verwendet dieselbe Abschlussauswertung. Nur echte Timerticks altern
  die TX-Timeouts; RX-Budget vier bleibt unverändert.
- Kein neuer Kernel, keine Interruptaktivierung, kein PHY-/Resetumbau.

## Messbefund und Hypothese

Auf dem A3000 wurden vor der Änderung zweimal 1 MiB in etwa 9,4 s heruntergeladen,
auch nach `/dev/null`; Upload etwa 12 s. Erfasste CPU-Proben zeigten ca. 98 % idle,
aber keine vollständige Lastkurve. Am FTP-Gegenrechner lagen die recv-Abstände
im Median bei 16,65 ms, häufig 1460 Bytes. Das sind Anwendungsereignisse, kein
Wire-Mitschnitt; Scheduling und Coalescing sind möglich.

Ein ausstehender TX, Freigabe nur im 60-Hz-Timer und 1460 * 60 = 87.600 Bytes/s
sprechen stark für ein timerbedingtes Uploadlimit. Der Downloadengpass ist damit
nicht vollständig erklärt. Die Optimierung war opportunistisch: ausschließlich
wartender Queue-Verkehr konnte weiterhin vom Timer abhängen.

## Hardware-Ergebnis: FAIL

Generation 3 bootete, aber Routerping und SSH funktionierten nicht.
`SIM=0 RUNNING=1 ERROR=0 OPENS=3 TX=3 RX=1152 TXERR=0 RXERR=0 TIMEOUT=0
DROPPED=0 DELIVERED=36`. Eine gesetzte IP und RUNNING/ERROR allein belegen
keinen funktionierenden Datenpfad. Kein nachträglicher FTP-Benchmark möglich;
**kein Geschwindigkeitsgewinn nachgewiesen**.

Die konkrete Ausfallursache bleibt offen. Ein Fehler im geänderten
TX-Abschluss-/Queue-Ablauf ist eine Hypothese, kein nachgewiesener Befund.
Frühe Freigabe und Queuebudget wurden gleichzeitig geändert, nicht unabhängig
auf echter Hardware isoliert. Generation 2 wurde wiederhergestellt und SSH plus
Routerping direkt geprüft; Generation 3 danach verworfen.

## Testgrenzen und Provenienz

Vor dem Hardwaretest: 417 Hostfälle, ASan/UBSan, Produktions-/SIM-Modulprüfung
und 82 native DLPI-Simulationsfälle bestanden. Die Produktions-Abwesenheitsprobe
belegte ENXIO ohne MMIO auf einem System ohne Karte, keine echte Kartenfunktion.
Der reale Hardwarefehler hat Vorrang vor den bestandenen Modelltests.

Die sechs Dateien `hw.c`, `hw.h`, `lanide.c`, `status.h`, `test-hw.c`, `Master`
sind byteidentisch zum archivierten Kandidaten. `lanctl.c` bleibt aus der stabilen
Basis. `src/Makefile` bleibt der veröffentlichte KIT-Adapter; die ursprüngliche
Kandidaten-Makefile liegt unter `archive/rejected-early-tx-original.Makefile`.
Quellhashes: `evidence/rejected-early-tx-sources.sha256`.

Das damals getestete Produktionsmodul hatte 15.220 Bytes und SHA256
`8c0d2805e9d680968f1252fb911104405308fc26b7d40e7f9e66e7a2e27a1e96`.
Es wird hier **nicht** als Binary veröffentlicht.

Die übrigen Dokumente und bisherigen Belege beschreiben die **stabile Basis**,
nicht eine Abnahme dieses Kandidaten. Insbesondere `evidence/verification.json`,
`evidence/host-tests.txt` und `evidence/stable-sources.sha256` sind historische
Gen2-Belege. Die Hashprüfung dieses Branches nutzt das separate Kandidatenmanifest.

## Prüfung auf dem Host

```sh
make check-sources
make test
make sanitize
```

Diese Prüfungen sind kein Hardware-Funktionsnachweis. Cross-Build-Voraussetzungen:
[Buildanleitung der stabilen Basis](docs/BUILD.md). Dieser Branch enthält keine
Images, Binaries, ROMs, privaten Rohlogs oder Zugangsdaten. Es wurden für die
Veröffentlichung keine Änderungen am A3000 durchgeführt.
