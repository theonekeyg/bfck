bfck=../bfck
TMP=$(mktemp)

trap "rm -f $TMP" 0 1
cat <<EOF >$TMP
++++++++++
[>++++++++++<-]
>++++. (h)
---. (e)
+++++++. (l)
. (l)
+++. (o)
EOF

if [ "$($bfck $TMP)" != "hello" ]; then
  exit 1
fi
