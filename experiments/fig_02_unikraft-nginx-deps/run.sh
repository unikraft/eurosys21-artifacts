# Get dependencies
apt-get install -y cscope git graphviz
pip3 install graphviz

# Modify tmpdir, on my end it would fail if I was using tmpfs
mkdir tmp
export TMPDIR=$PWD/tmp

git clone https://github.com/unikraft/unikraft
git clone https://github.com/unikraft/lib-lwip

mv lib-lwip unikraft/lib/

mv unikraft/lib/nolibc unikraft/lib/lib-newlib

cd unikraft && git checkout 775dc0123e8523008463fe6b28d45b87a836ae89

cd ..

cscope -b -R

./component_symbols.sh

python3 plot.py