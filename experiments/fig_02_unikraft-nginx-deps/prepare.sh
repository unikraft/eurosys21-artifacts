#!/bin/bash
apt-get install -y cscope git graphviz python3-pip
pip3 install graphviz

git clone https://github.com/unikraft/unikraft
git clone https://github.com/unikraft/lib-lwip

mv lib-lwip unikraft/lib/


cd unikraft && git checkout 775dc0123e8523008463fe6b28d45b87a836ae89

cd ..
mv unikraft/lib/nolibc unikraft/lib/lib-newlib
