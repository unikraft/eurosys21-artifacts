# Modify tmpdir, on my end it would fail if I was using tmpfs
mkdir tmp
export TMPDIR=$PWD/tmp
git clone https://github.com/unikraft/unikraft

cd unikraft && git checkout 775dc0123e8523008463fe6b28d45b87a836ae89

cd ..


cscope -b -R

./component_symbols.sh

python3 plot.py
