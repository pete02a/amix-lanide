# Aktuelle Quellen / AMIX1.1.2

- Offizielle Basis: [amigaux/amix](https://github.com/amigaux/amix/tree/ec9dfd6e57070fddd8a34b8cb83236257ecf0ecf), `ec9dfd6e57070fddd8a34b8cb83236257ecf0ecf`.
- Lokale c57-Erweiterung vollständig als `compat/amigaux-1.1.2/amix-1.1.2-c57.patch`; private Endquelle `2b3a100f27b7dfded7541f9b25de5a48d3c1081d` ist Provenienz, keine versprochene öffentliche Gitabhängigkeit.
- Treiber: stabiler RX-Fix aus der bisherigen c56-Linie, zwei Major-Anpassungen, vollständige aktuelle Quelle unter `src/`; SHA-Bindung `evidence/c57-sources.sha256`.
- Produktionsmodul14855B SHA256 `4cab27b0e9dc24c828f67f3fcbad8c202048009078076fa2569df8bc045e8c29`; Simulator14899B `89c4b54acb3ad1482fd1a4a5f24bbec23810d0b19578ecb515a9e142e24fc96d`.
- Offizieller Kernel67 SHA256 `454d5d652d25a7862601352d64b41164ea5db03975535b3076a9371db1545799`, unverändert. Compiler-/Toolkettenpins stehen unten und im Portbericht.

`evidence/artifacts.sha256`, `stable-sources.sha256` und der folgende ursprüngliche Quellenbericht bleiben c56-Historie. `make check-sources` benutzt jetzt die c57-Liste. Private Paket-/Composerhashes sind Referenzen, keine veröffentlichten offiziellen Binaries. Aktuelle Veröffentlichungsnachweise werden separat erfasst.

---

> Historischer c56-Stand. Für diesen c57-Branch gilt stattdessen der [1.1.2-Portbericht](../compat/amigaux-1.1.2/README.md). Keine alte c56-Installation auf1.1.2 übernehmen.

# Quellen, Pins und Artefakte

## Primärquellen

| Bestandteil | Quelle / geprüfter Pin |
| --- | --- |
| Karten-/ENC-Grundlage | [Zorro-LAN-IDE](https://gitlab.com/MHeinrichs/Zorro-LAN-IDE), `f8dc4383d2eab98d8fa53874e9efba72090c68fe` |
| ENC624J600-Datenblatt | Microchip DS39935B; im Buildarchiv SHA-256 `24debabf51e18dc8022f6b82c13f16451e8e20fdd943269c73f86f8af1e2c110` |
| DLM-/Kernel-Basis | [amigaux/amix](https://github.com/amigaux/amix), `46ef7d1a4d50db04120e26535fcaa86cce8cf042` |
| Integrierte AMIX-1.1-Userland-Rezepte | [amigaux/amix](https://github.com/amigaux/amix), `cdf6c05de9d446e3f75e4b7cf204e6d1100aac5c` |
| Cut66-Rezept | [amigaux/amix-040-060-port](https://github.com/amigaux/amix-040-060-port), `388b93b6dc6404fed5ec041b2005af4da48c1f84`, `cuts/68060-261106-66/cut.sh` (intern Port `512a99f`) |
| Period-Cross-Compiler | [jusii/gcc-cross-amix](https://github.com/jusii/gcc-cross-amix), `33ebadcf6976ee65dbd1e25a586f59da55e5942d` |
| Native CLI-/SSH-Compiler | [amigaux/gcc14-cross-amix](https://github.com/amigaux/gcc14-cross-amix), `dcfd12d79fb970f607a69d72b79e9f7470d1043f` |

Die hier exportierte Treibersnapshot-Quelle entspricht dem lokalen
RX-Fix-Commit `ef82049d78c501cf4381ac07d8d742b8856c9aa1`; dieser lokale
historische Commit ist keine externe Downloadabhängigkeit. Die nötigen
Treiberdateien stehen vollständig unter `src/`.

Die beiden lokalen Registry-Änderungen `8cca5c21...` und `83b03977...`
werden als Patch mitgeliefert, damit nicht auf unveröffentlichte lokale
Gitobjekte verwiesen werden muss. Die alten Buildaufzeichnungen nennen
teilweise noch diese ursprünglichen Commit-IDs.

## Referenzartefakte (nicht mitgeliefert)

| Datei | Bytes | SHA-256 |
| --- | ---: | --- |
| Produktionsmodul `lanide` | 14855 | `e406881216f7ba472fac0b5d32e5c38de25281ba51c2fe1ae1c3d457582d0f47` |
| Simulationsmodul `lanide` | 14899 | `ac02d23673fa59217b4c9aaeb4af3370a4ffd32374209a6a919ba9c50193154f` |
| Eigener Kernel `68060-261114-00` | 2360673 | `d07f36c34d48cec6b5175d776b95087f3339c0ac9b635ef78f977dbb345455bf` |
| Passendes Core-UFS | — | `b99fc24c8dcfad2be68600fb282cab326e59ff855c2a7a9a99a5973707868019` |
| Fixierter Dropbear | 285784 | `c384613b52bd334abe2d12285892d0cddcde62b48dd04352624371a92fb3da39` |

Dropbear-Quellarchiv 2026.94:
`e098034a843699200c8c977a991fff73159735bf795d5f72ef672c41a6b1ae81`.
Lokale GCC14-Binärhashes unterschieden sich vom offiziellen Container;
ABI-/ISA-Gates wurden beibehalten, aber keine offizielle Containeridentität
oder ein offizielles apkg-Paket behauptet.

Nicht übernommen: Performancekandidat mit Modulhash
`8c0d2805e9d680968f1252fb911104405308fc26b7d40e7f9e66e7a2e27a1e96`.
Er war auf echter Hardware nicht funktionsfähig und ist kein Releaseartefakt.
