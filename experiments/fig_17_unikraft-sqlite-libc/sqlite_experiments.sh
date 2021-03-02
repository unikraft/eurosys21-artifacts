#!/bin/sh

set -eu

script_folder="$(dirname "$(readlink -f "$0")")"
root_folder="/tmp/sqlite_experiments_build"
results_folder="$script_folder/results"

usage="Usage: $0 [--prepare|--run|--plot|--clean]

Options:
    --prepare
        Download and build applications in $root_folder
    --run
        Run experiments (prepare step must have been run before) and store
        results in $results_folder
    --plot
        Converts results to a figure
    --clean
        Remove built applications, but keep results and figure (if present)

    Without argument, the script will run the prepare, run and plot steps.
"

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

prepare() {
    [ -s "$(command -v musl-gcc)" ] \
        || die "musl-gcc is needed, install musl-tools first."

    [ -s "$(command -v curl)" ] \
        || die "curl is needed, install curl first."

    if [ -e "$root_folder" ]; then
        >&2 echo "$root_folder already exists. Skipping the prepare step."
        return
    else
        mkdir "$root_folder"
    fi

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

    echo "Applications have been built and should be ready to run."
}

run() {
    [ -s "$(command -v qemu-system-x86_64)" ] \
        || die "qemu-system-x86_64 is needed, install qemu-system-x86 first."

    [ -d "$root_folder/apps" ] \
        || die "Missing $root_folder/apps.
Run the prepare step first."
    cd "$root_folder/apps"

    if [ -e "$results_folder" ]; then
        old_results_folder="$results_folder.$(date +%Y%m%d%H%M%S)"
        >&2 echo "
$results_folder already exists.
Moving it to $old_results_folder.
"
        mv "$results_folder" "$old_results_folder"
    fi
    mkdir "$results_folder"

    echo "\n======  Running SQLite Test Applications  ======\n"

    for app in app-sqlite-*; do
        echo "\n===  Running $app Experiment  ===\n"
        cd "$app"
        ./run.sh | tee perf.dat
        cp perf.dat "$results_folder/$app.dat"
        cd ..
    done

    echo "Measurements have been saved to $results_folder."
}

plot() {
    [ -d "$results_folder" ] \
        || die "Missing $results_folder. Run the prepare and run steps first."

    echo "\n======  Generating Figure  ======\n"

    "$script_folder/gen_fig.py" "$results_folder" \
        "$script_folder/unikraft_sqlite_libc.pdf"

    echo "Generated figure is available at
$script_folder/unikraft_sqlite_libc.pdf"
}

if [ $# -gt 1 ]; then
    die "Too many arguments.
$usage"
elif [ $# -eq 0 ]; then
    prepare
    run
    plot
else
    case "$1" in
        --clean)
            echo "Removing $root_folder..."
            rm -rf "$root_folder"
            ;;
        --plot)
            plot
            ;;
        --prepare)
            prepare
            ;;
        --run)
            run
            ;;
        *)
            die "'$1': unsupported argument.
$usage"
            ;;
    esac
fi
