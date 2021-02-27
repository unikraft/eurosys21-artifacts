#!/bin/sh

set -eu

die() {
    >&2 echo "$1"
    exit 1
}

obtain() {
    # `obtain user repository commit|tag|hash`
    git clone "https://github.com/$1"/"$2".git
    cd "$2"
    if git branch -a | grep "$3" >/dev/null; then
        git checkout "$3"
    else
        git checkout -b eurosys21 "$3"
    fi
    cd ..
}

[ -s "$(command -v musl-gcc)" ] \
    || die "musl-gcc is needed, install musl-tools first."

[ -s "$(command -v curl)" ] \
    || die "curl is needed, install curl first."

[ -s "$(command -v qemu-system-x86_64)" ] \
    || die "qemu-system-x86_64 is needed, install qemu-system-x86 first."

root_folder="$(mktemp --tmpdir --directory sqlite_experiments.XXX)"
echo "Experiments are going to be built and run in $root_folder."
echo "This will require about 2 GB of disk space."
sleep 5
cd "$root_folder"

echo "\n======  Obtaining Unikraft Core  ======\n"

obtain cffs unikraft eurosys21

echo "\n======  Obtaining Unikraft Libraries  ======\n"

mkdir libs
cd libs
obtain cffs lib-musl eurosys21
obtain unikraft lib-newlib 0fe194147867c54c012cdd5f25a5ac088505e289
obtain unikraft lib-pthread-embedded RELEASE-0.5
obtain unikraft lib-sqlite RELEASE-0.5
obtain cffs lib-tlsf eurosys21
cd ..

echo "\n======  Obtaining SQLite Upstream Code  ======\n"

curl -O "https://www.sqlite.org/2019/sqlite-amalgamation-3300100.zip"
unzip sqlite-amalgamation-3300100.zip
rm sqlite-amalgamation-3300100.zip
sqlite_dir="$PWD/sqlite-amalgamation-3300100"

echo "\n======  Building SQLite for GNU\Linux with musl  ======\n"

cd sqlite-amalgamation-3300100
musl-gcc -c -fno-omit-frame-pointer -fno-stack-protector -fno-tree-sra \
    -fno-split-stack -O2 -fno-PIC -I. -g3 sqlite3.c
ar -crs libsqlite.a sqlite3.o
cd ..

echo "\n======  Obtaining SQLite Test Applications  ======\n"

mkdir apps
cd apps
obtain cffs app-sqlite-test newlib-native
mv app-sqlite-test app-sqlite-newlib-native
obtain cffs app-sqlite-test musl-native
mv app-sqlite-test app-sqlite-musl-native
obtain cffs app-sqlite-test musl-compat
mv app-sqlite-test app-sqlite-musl-compat
obtain cffs app-sqlite-test linux-native
mv app-sqlite-test app-sqlite-linux-native

echo "\n======  Preparing SQLite Test Applications  ======\n"

# Turn .config templates into real .config
for app in app-sqlite-*; do
    if [ -f "$app/.config" ]; then
        sed -i -e "s+{{ROOT_FOLDER}}+$root_folder+" \
            -e "s+{{APP_NAME}}+$app+" "$app/.config"
    fi
done

# Copy SQLite include file to app folder
for app in app-sqlite-*; do
    mkdir "$app/include"
    cp "$sqlite_dir/sqlite3.h" "$app/include"
done

# Copy SQLite library for apps that need it
for app in app-sqlite-musl-compat app-sqlite-linux-native; do
    cp "$sqlite_dir/libsqlite.a" "$app"
done

echo "\n======  Building SQLite Test Applications  ======\n"

for app in app-sqlite-*; do
    echo "\n==  Building $app  ==\n"
    cd "$app"
    make
    cd ..
done

echo "\n======  Running SQLite Test Applications  ======\n"

mkdir ../results
for app in app-sqlite-*; do
    echo "\n===  Running $app Experiment  ===\n"
    cd "$app"
    ./run.sh | tee perf.dat
    cp perf.dat "../../results/$app.dat"
    cd ..
done

echo "\n======  Experiments Finished!  ======\n
You can now:

* see the experiments results in $root_folder/results;
* inspect or re-run the individual experiments from
  $root_folder/apps;
"
