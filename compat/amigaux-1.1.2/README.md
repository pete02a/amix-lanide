# LANIDE c57: Vorbereitung für amigaux 1.1.2

Stand 17.11.2026. **Historischer Vorbereitungsstand vom17.11.; Fortschreibung21.11.: c57-Funktion auf echter Hardware später beobachtet, siehe aktuelle Abnahme. Kein vollständiger neuer OS-Upgrade-/Dauerlastnachweis.**

## Zweck und Abgrenzung

Upstream 1.1.2 verwendet Character-Major **56 für X-Surf**. Dieser Branch portiert den stabilen RXfix unverändert im Datenpfad auf lokal **c57**. `main` enthält inzwischen diesen c57-Port; der alte c56-Stand bleibt im Branch `legacy/c56-before-1.1.2`. c57 ist nur auf dem gepinnten Ziel als frei geprüft, keine dauerhafte Upstream-Zuteilung. Keine A–D-/Early-TX-Optimierung, keine ZZ9000-Netzwerk- oder Blitterentwicklung.

**Der offizielle Kernel wird nicht neu gebaut oder gepatcht.** Der Core-Patch `kernel/dlm/core/patches/0051-manifest-major-table.patch` nutzt die Major-Tabelle des Bootmanifests; die einkompilierte Tabelle ist Fallback ohne Manifest. Unsere frühere Forderung nach einem Kernelneubau war falsch. Ein bloßer Treiberneubau reicht trotzdem nicht: privater kbootset, passendes Kit, Modul, Manifest, Gerätenummer und link.d müssen zusammenpassen. c56 bleibt xsurf, c68 bleibt va2000.

## Gepinnte Quellen und Build

- [amigaux/amix 1.1.2](https://github.com/amigaux/amix/tree/ec9dfd6e57070fddd8a34b8cb83236257ecf0ecf), Commit `ec9dfd6e57070fddd8a34b8cb83236257ecf0ecf`.
- Period-Compiler [jusii/gcc-cross-amix](https://github.com/jusii/gcc-cross-amix/tree/33ebadcf6976ee65dbd1e25a586f59da55e5942d), GCC2.7.2.3/binutils2.8.1.
- Native Composer: projektkonformer GCC14/AMIX-ABI-Build nach den DLM-Anforderungen; kein beliebiger m68k-Compiler.

In einem separaten AMIX-Checkout am genannten Pin:

```sh
git apply --check /path/to/amix-lanide/compat/amigaux-1.1.2/amix-1.1.2-c57.patch
git apply /path/to/amix-lanide/compat/amigaux-1.1.2/amix-1.1.2-c57.patch
```

Danach gemäß `kernel/dlm`-Builddokumentation Kit und nativen kbootset mit den dort verlangten GCA/G14/PORT-Variablen erzeugen; keine Kernel-Neukompilation. Kit-Registry und Composer müssen c57 kennen. Freie cdevsw-Zeile57 gegen den tatsächlichen Kernel prüfen, nicht nur gegen Textquellen. Der gepinnte Kernel bietet k46.1.6, Modulimport-Minor k46.1.2; ABI-Digest `18c0a1c98b2c9284fb10a6b6386403ed5e13193e3ec7aa4f113ff2a174285378` bleibt unverändert.

Im LANIDE-Checkout:

```sh
make check-sources test sanitize
export PATH=/path/to/gcc-cross-amix/build/prefix/bin:$PATH
make module KIT=/absolute/path/to/generated/k46
make simulation KIT=/absolute/path/to/generated/k46
make userland
```

`src/Makefile` ist der portable öffentliche Buildwrapper; `src/lanide.c` und `src/Master` sind der c57-Kandidat. `hw.c`, `hw.h`, `status.h`, `lanctl.c`, `test-hw.c` bleiben bytegleich zum stabilen main1079538. Die zwei Sourceänderungen erzeugten im bisherigen Produktionsbuild exakt zwei Byteänderungen: Offset6879 (0-basiert)56→57 und Offset7271 ASCII6→7, Größe14855B. Kein TX-/RX-Umbau.

Die Patch-Recipe enthält lokale Provenienz-ID94d71328 und den damals erzeugten Composerhash; das lokale Gitobjekt ist keine öffentliche Downloadabhängigkeit. Alle notwendigen Registryänderungen stehen im Patch. Ein neuer Checkout/Build kann wegen eingebetteter Build-/Commitidentität einen anderen Composerhash ergeben: Recipepin für den eigenen validierten Build setzen, niemals den Referenzhash ungeprüft behaupten. Die geprüfte lokale Endquelle war2b3a100f27b7dfded7541f9b25de5a48d3c1081d. Kein Bootstrap von Compiler/Kit aus einem leeren Rechner wurde hier nachgewiesen.

## Integration und Pakete

`config/bootmanifest.example` ist **nur ein Fragment**, keine auf einen echten Rechner anwendbare Root-/Bootspec. Der richtige offizielle Kernel67 und dessen zugehöriges Core-UFS kommen gemeinsam aus dem Kernelpaket. Keine Testmaschinen-Gerätewerte aus Emulatorlogs übernehmen.

- Privater Composer `kbootset 1.1.0.261117-0` vor der c57-Generation.
- `lanide46 1.0.0-20261117`, `/dev/lan0` Character57,Minor0 und `config/lanide.link` gemeinsam umstellen. Bestehende falsche Gerätenodes werden nicht durch bloße Existenz richtig.
- Kein gleichzeitiger Austausch eines geladenen `unload never`-Moduls.
- **APKG0.3.13** muss den kbootset-Floor im Modul unterstützen. Älterer Client kann ihn ablehnen.
- Die private Versionsnummer ist kein offizielles Release. Höhere zukünftige Upstream-kbootset-Versionen garantieren nicht, dass unser c57-Eintrag erhalten bleibt; Registry vor jedem Update erneut prüfen.

Die Dateien `lan.pkginfo`, `lan.proto`, `lan.modreg`, `kb.pkginfo`, `kb.proto`, `copyright` sind die tatsächlichen Paket-Baureferenzen mit dem Gast-Stagingpfad `/opt/compat112`. Sie sind keine fertigen Installationsskripte. Für `kbroot` werden der eigene gebaute Composer sowie Handbuch/RC-Dateien aus dem gepinnten Upstream-Paket benötigt, nicht aus diesem Repo. Die benötigten Pins stehen im Registry-/Recipepatch. `lanctl` und Produktionsmodul aus den Builds bereitstellen; Simulator niemals paketieren.

Stock-pkgmk1991 schrieb beim expliziten Minor0 im pkgmap teilweise `?`. `normalize-pkgmap.sh` normalisiert ausschließlich die beiden exakt erwarteten Zeilen (lan0=57,0 und kinfo=53,0) und verlangt danach pkgchk. Anschließend pkgtrans und **strengen** Repositorykatalog-/Digestcheck erneut ausführen. Keine gelockerte Validierung. Ein Zweipaket-Zusatzkatalog ist kein Ersatz für den vollständigen offiziellen Repositorykatalog.

## Vorhandene Nachweise und offene Grenzen

Die folgenden Ergebnisse stammen aus der dokumentierten Integration am17.11., nicht aus einem erneuten Hardwaretest bei dieser Veröffentlichung:

| Ebene | Nachweis | Grenze |
| --- | --- | --- |
| Host-Composer | 892 Basisprüfungen,0Fehler; weitere Registry-/Import-/Manifest-/Upgrade-Suiten | optionale Tests ohne Inputs ausdrücklich übersprungen |
| Nativer Simulator | 82 DLPI-Prüfungen,8Streams,Multibind,Padding,Fehlerfälle,Close/Reopen | keine Karte, keine Durchsatzmessung |
| Produktion ohne Karte | offizieller Kernel67,ENXIO,NO MMIO,kein Modulrest | keine Hardwarefunktion bewiesen |
| Pakete | native pkgmk/pkgtrans/pkgchk und strenger Katalog-/Digestcheck | kein vollständiges APKG-OS-Upgrade |
| Integrität | Kernel unverändert, Gast-Rücklesen,Manifest-/Member-CRC/SHA,Originalgeneration erhalten | Emulator ist nur Integrationsträger, kein neues freigegebenes Vollimage |

Originalkernel SHA256 `454d5d652d25a7862601352d64b41164ea5db03975535b3076a9371db1545799`.
Produktionsmodul SHA256 `4cab27b0e9dc24c828f67f3fcbad8c202048009078076fa2569df8bc045e8c29`.
Altes c56-Modul SHA256 `e406881216f7ba472fac0b5d32e5c38de25281ba51c2fe1ae1c3d457582d0f47`.
Weitere Referenzhashes: [reference-artifacts.sha256](reference-artifacts.sha256). Keine Binärdateien, Kits, Images, ROMs oder privaten Rohlogs veröffentlicht.

Frühe fehlgeschlagene Proben wurden nicht als PASS gezählt: Testspezifikation ohne LANIDE-Member (korrigiert), fehlendes Helper-Ausführungsbit (korrigiert), Minor0/pkgmap (siehe oben), nicht benötigter Kernel-Rebuild am FPE-Assembler gescheitert/abgebrochen. Der private Composer wurde aus derselben lokalen Quelle zweimal bytegleich gebaut, nicht als identischer offizieller Upstream-Composer ausgegeben.

## Fortschreibung21.11.2026

Der stabile c57-Port ist nun Hauptstand; aktuelle [Abnahme](../../docs/ACCEPTANCE.md) und `evidence/amigaux112-hardware-20261121.json` dokumentieren die späteren realen Funktionsbeobachtungen. Es wurde bei dieser Veröffentlichung kein neues Hardware-/OS-Upgrade ausgeführt. C56-Legacy und verworfener Early-TX-Zweig bleiben getrennt erhalten. LANIDE/ZZ9000-Z3 bleibt ungeklärt, keine neue Optimierung. Der Eigentümer nutzt inzwischen ZZ9000-LAN und hat LANIDE entfernt.
