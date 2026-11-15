# Architektur und Sicherheitsgrenzen des Treibers

## Hardwarevertrag

AutoConfig-Kennung `0x0a1c007b` (Hersteller 2588, Produkt 123), erwartete
Basis `0x40000000`, Ausdehnung `0x10000`. Die Clockportvariante mit anderer
Produktkennung wird dadurch nicht unterstützt. Register und SRAM werden
über `dev_kvmap` uncached abgebildet, nicht als rohe physische C-Zeiger.
Die Registerseite beginnt bei Basis+`0x7000`, SRAM umfasst `0x6000` Bytes.
Vor dem Wortzugriff prüft der Backendcode die beiden Byteadressen per
`bprobe`; dies ist kein zusätzlicher vollständiger Word-Fault-Wrapper.

TX belegt den unteren SRAM-Bereich; RX nutzt den Ring `0x1000..0x5fff`.
Ethernetdaten und die Little-Endian-RX-Metadaten werden getrennt behandelt.
SRAM und Engines gehören während des Betriebs der Schnittstelle. Anders als
bei den frühen Diagnosemodulen wird kein alter RAM-Snapshot zurückkopiert.

## STREAMS und Lebenszyklus

Style-1-DLPI-Provider mit acht Endpunkten. IP und ARP binden getrennte SAPs;
auch mehrere Streams derselben SAP werden berücksichtigt. Neunter Stream,
ungültige Primitive, DLSAP-Formate und Paketgrößen haben explizite Fehlerwege.
TX läuft asynchron, Empfang über einen begrenzten Timer-Callback. Der
Hardwarevertrag serialisiert Zugriffe auf IPL4; es gibt dort keine
Warteschleife auf Hardware oder Zeitfortschritt.

Letztes Schließen stoppt die Engines und beendet das Polling; erneutes
Öffnen startet kontrolliert neu. Halt/Fehlerpfade quieszieren die Hardware.
Der persistente Produktionsprovider ist nicht entladbar. Ein noch geladenes
Modul erneut zu laden ist kein Update auf eine neue Binärdatei.

## RX-Fix

Der reale Problemfall hatte `cursor=0x247a`, `next=0x24c2`, `bytes=0x40`,
`status=0x3a8`. Der Folgeverweis passt zur Ringrechnung, aber der Frame trägt
einen Längenprüffehler. Die alte Sammelprüfung stoppte dafür fälschlich RX.

Der stabile Fix trennt nun:

1. Sicher begrenzter, aber fehlerhafter Frame: CRC-/Längen-/RXOK-Fehler oder
   nicht unterstützte Größe → einen Frame konsumieren, Ringzeiger und
   Paketanzahl fortschreiben, Fehler-/Dropzähler erhöhen und weiterarbeiten.
2. Unsichere Ringmetadaten: ungültiger Bereich, Alignment, Folgeverweis oder
   Ringbelegung → weiterhin harter Fehler und Stop; keine blinde Fortsetzung.

Auch ein verworfener Frame zählt zum Budget von vier Frames pro Tick. Das
verhindert eine unbegrenzte Schleife unter Fehlerlast. Hosttests prüfen alle
256 betrachteten Statuskombinationen, Randgrößen, Wrap, Fehlerketten und
weiteres TX nach einem Drop.

## Produktions- und Simulationstrennung

`LANIDE_NATIVE_TEST` erzeugt ausschließlich einen No-MMIO-Prüfprovider.
`lanctl -t` verweigert Selbsttests auf einer echten NIC. Die Simulation
prüft DLPI/STREAMS-Verträge; sie ersetzt keine Hardwareabnahme. Beide Builds
haben denselben Modulnamen und müssen strikt getrennt gehalten werden.

## Nicht übernommene Optimierung

Der Versuch, TX-Abschlüsse zusätzlich bei neuer Sendeaktivität früher zu
verarbeiten, bestand Modell-/Emulatortests, fiel aber auf der echten Karte
durch. Dieser Code ist **nicht** in `src/` enthalten; siehe Entwicklungsverlauf.
