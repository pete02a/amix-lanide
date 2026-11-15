#!/bin/bash
set -euo pipefail
P=${AMIX_WORK_ROOT:?set AMIX_WORK_ROOT to the historical build workspace}
S=$P/drivers/lanide/stage6
W=$S/kernel-baseline
export LAN_DLM=$S/amix-custom/kernel/dlm
export LAN_REV=8cca5c21a82c0d1d74403bb3afdd928d18f51e82
. "$GCA/env.sh"
CROSS=$AMIX_CROSS_PREFIX
PATH="$P/tools/cde-host-bin:$P/tools/host-root/usr/bin:$W/gnu-bin:$PATH"
export PATH GCA AMIX_ALLOW_UNKNOWN_STOCK=1 STAMP_BUILDID_CPU=60
export SOURCE_DATE_EPOCH=1794614400
export GIT_CONFIG_COUNT=1 GIT_CONFIG_KEY_0=user.email GIT_CONFIG_VALUE_0=builder@example.invalid
cd "$W"
sha256sum -c SHA256SUMS-68060-261106-66 > "$S/baseline-before-custom.log"
[ "$(python3 "$W/port/tools/blocks.py" buildseq)" = '0 0' ]
cd "$W/port"
python3 src/check_dlm_splice.py --free-rows 56 build/c-kinfo > "$S/major56-free.log"
DLM_REPO=$LAN_DLM DLM_REV=$LAN_REV sh relink-040-dlm.sh build/c-kinfo build/l-dlm > "$S/custom-dlm.log" 2>&1
# A frozen experimental cut, not another official66. Its reserved local stamp is00.
STAMP_BUILDID=stamp sh relink-040-fpe.sh build/l-dlm build/l-fpe > "$S/custom-fpe.log" 2>&1
mkdir -p "$W/kernels/l" "$W/log/l"
cp build/l-fpe "$W/kernels/l/unix-fpe"
W=$W CONTROL= bash "$S/post-lanide.sh" l > "$S/custom-post.log" 2>&1
# Generate an ABI kit from the same explicit registry source used by the kernel.
PORT=$W/amix-040-060-port make -C "$LAN_DLM" kit GCA="$GCA" > "$S/custom-kit.log" 2>&1
python3 - "$S" <<'PY2'
from pathlib import Path
import sys,shutil
s=Path(sys.argv[1]);w=s/'kernel-baseline';d=s/'ufs-lanide'
assert not d.exists()
shutil.copytree(w/'ufsmod/mod',d,ignore=shutil.ignore_patterns('build','make.log'))
f=d/'Master';v=f.read_text();assert '$build 68060-261106-66' in v
f.write_text(v.replace('$build 68060-261106-66','$build 68060-261114-00'))
PY2
make -C "$S/ufs-lanide" KIT="$LAN_DLM/build/kit/k46" > "$S/custom-ufs.log" 2>&1
cp "$S/ufs-lanide/build/k46/mod.d/ufs" "$W/out/l/ufs.mod"
cd "$W/out/l"
sha256sum unix kabi abi-k46.1.4.txt ufs.mod > "$S/custom-kernel.sha256"
cd "$W"
sha256sum -c SHA256SUMS-68060-261106-66 > "$S/baseline-after-custom.log"
echo 'CUSTOM KERNEL BUILT; baseline files unchanged; native boot still required'
