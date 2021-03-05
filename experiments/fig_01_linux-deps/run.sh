# Modify tmpdir, on my end it would fail if I was using tmpfs
mkdir tmp
export TMPDIR=$PWD/tmp

./create_cscope_db.sh

./component_symbols.sh
