# Historische Kernel-Buildaufzeichnung

Zusätzliche Schritte nach einem vollständig aufgebauten Cut66. Dies sind
Aufzeichnungen des tatsächlichen Buildwegs, **kein eigenständig ausführbarer
Installer**. Setzen die ursprüngliche `drivers/lanide/stage6`-Baumstruktur,
Cut-Artefakte, lokale Commitobjekte und gepinnte Compiler voraus. Der lokale
Rechnerpfad wird über `AMIX_WORK_ROOT` angegeben; der ursprüngliche private
Build-Absender wurde durch einen neutralen Platzhalter ersetzt.

Die gemeinsame Registry-Änderung wird separat als öffentlicher Patch unter
`patches/` mitgeliefert. Für neue Builds den Patch im eigenen Checkout
anwenden und die dortige Commit-/DLM-Auswahl explizit aktualisieren. Die
historischen lokalen SHA-IDs dürfen nicht als öffentlich vorhanden gelten.

Siehe ../../docs/BUILD.md für die aktuell geprüften Modul-Buildziele und
für die Grenzen des Kernel-Reproduktionsnachweises.
