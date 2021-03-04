#!/bin/sh

apk add build-base
cat <<EOF > wport.c
#include <stdio.h>
#include <sys/io.h>
int main(int argc, char **argv) {
    int err;

    err = iopl(3);
    if (err < 0) {
        printf("iopl failed\n");
    }

    err = ioperm(0x03f0, 16, 1);
    if (err < 0) {
        printf("ioperm failed\n");
    }

    outb(123, 0x03f0);

    return 0;
}
EOF
gcc -static -O2 -o ./wport wport.c
mv ./wport /sbin/wport
chmod a+rwx /sbin/wport
echo "#!/sbin/openrc-run
command=\"/sbin/wport && echo 'Written!' && poweroff\"" > /etc/init.d/hello.init
chmod a+x /etc/init.d/hello.init
/sbin/rc-update add hello.init default
sed -i "s/MENU TITLE Alpine\/Linux Boot Menu//g" /boot/extlinux.conf
sed -i "s/MENU AUTOBOOT Alpine will be booted automatically in # seconds.//g" /boot/extlinux.conf
sed -i "s/MENU HIDDEN//g" /boot/extlinux.conf
sed -i "s/TIMEOUT 30//g" /boot/extlinux.conf
sed -i "s/DEFAULT menu.c32/DEFAULT virt/g" /boot/extlinux.conf
