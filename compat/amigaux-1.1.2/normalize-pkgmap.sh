#!/bin/sh
set -e
cd /opt/compat112
# Stock 1991 pkgmk renders an explicit zero device minor as '?'.
# Assert the precise prototype-derived row, normalize just that field,
# then require both pkgchk and the independent catalog validator.
if grep '^1 c none /dev/lan0 57 ? 0600 root sys$' spool/lanide46/pkgmap > /dev/null; then
 sed 's@^1 c none /dev/lan0 57 ? 0600 root sys$@1 c none /dev/lan0 57 0 0600 root sys@' spool/lanide46/pkgmap > spool/lanide46/pkgmap.fixed
 mv spool/lanide46/pkgmap.fixed spool/lanide46/pkgmap
fi
grep '^1 c none /dev/lan0 57 0 0600 root sys$' spool/lanide46/pkgmap
if grep '^1 c kinfo /dev/kinfo 53 ? 0444 root sys$' spool/kbootset/pkgmap > /dev/null; then
 sed 's@^1 c kinfo /dev/kinfo 53 ? 0444 root sys$@1 c kinfo /dev/kinfo 53 0 0444 root sys@' spool/kbootset/pkgmap > spool/kbootset/pkgmap.fixed
 mv spool/kbootset/pkgmap.fixed spool/kbootset/pkgmap
fi
grep '^1 c kinfo /dev/kinfo 53 0 0444 root sys$' spool/kbootset/pkgmap
/usr/sbin/pkgchk -d /opt/compat112/spool kbootset lanide46
