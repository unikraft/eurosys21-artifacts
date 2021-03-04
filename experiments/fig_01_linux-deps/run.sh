# Modify tmpdir, on my end it would fail if I was using tmpfs
mkdir tmp
export TMPDIR=$PWD/tmp

git clone https://github.com/torvalds/linux

cd linux && git checkout c85fb28b6f999db9928b841f63f1beeb3074eeca

cd ..

./create_cscope_db.sh

./component_symbols.sh
