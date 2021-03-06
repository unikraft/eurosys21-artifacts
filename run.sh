#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# Authors: Alexander Jung <a.jung@lancs.ac.uk>
WORKDIR="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
. $WORKDIR/support/common.sh

# Influential environmental variables
EXPERIMENTS_DIR=${EXPERIMENTS_DIR:-$(pwd)/experiments}
DOCKER_IMAGE_PLOT=${DOCKER_IMAGE_PLOT:-unikraft/eurosys21-artifacts-plot:latest}
PLOT_FORMAT=${PLOT_FORMAT:-svg}
DOCKER_FORCE_BUILD=${DOCKER_FORCE_BUILD:-y}

# Program arguments
SHOW_HELP=n
LIST_ALL=n
DRY_RUN=n
DOCKER_PLOT=y
NO_DEPS=n
VERBOSE=n

_help() {
    cat <<EOF
$0 - Run all or a specific experiment.

Usage:
  $0 [OPTIONS] [FIGURE_ID|TEST_NAME] [ACTION]

If no figure ID or test name is provided, the action is run for all
experiments.  If no action is provided, all actions are performed.

Examples:
  $0 fig_01      Runs prepare, run and plot for fig_01.
  $0 prepare     Prepares all experiments.

Actions:
  prepare              Prepares the host and/or builds dependent tools
                         and images before the test is run.
  run                  Runs the given experiment and saves the results.
  plot                 Uses the data from the experiment to generate
                         the plot.
  clean                Clean intermediate build files from an experiment.

Options:
     --no-deps         Do not try to install dependencies.
     --no-docker-plot  Do not use Docker for plotting.
  -l --list            List all tests and exit.
  -v --verbose         Be verbose.
  -h --help            Show this help menu.

Influential Environmental Variables
  EXPERIMENTS_DIR      Directory of all the experiments
                         (default: ./experiments).
  DOCKER_FORCE_BUILD   Force build of Docker containers
                         (default: $DOCKER_FORCE_BUILD).
  DOCKER_IMAGE_PLOT    Docker environment for generating plots
                         (default: $DOCKER_IMAGE_PLOT).
  PLOT_FORMAT          File format for the plot
                         (default: $PLOT_FORMAT).
EOF
}

# Parse flag arguments
for i in "$@"; do
  case $i in
    -v|--verbose)
      VERBOSE=y; shift;;
    -l|--list)
      LIST_ALL=y; shift;;
    --no-docker-plot)
      DOCKER_PLOT=n; shift;;
    --no-deps)
      NO_DEPS=y; shift;;
    -h|--help)
      _help; exit 0;;
    *)
      ;;
  esac
done


# Dependency management
function install_dependencies() {
  local DISTRO=$(lsb_release -c -s)

  case $DISTRO in
    buster)
      apt-get update
      apt-get install -y \
        git \
        clang \
        bsdcpio \
        doxygen \
        libhugetlbfs-bin \
        build-essential \
        qemu-system-x86 \
        redis-tools \
        socat \
        meson \
        cscope \
        uuid-runtime \
        uuid-dev \
        libuuid1 \
        bridge-utils \
        net-tools \
        ifupdown \
        bison \
        curl \
        flex \
        wget \
        sudo \
        libmhash-dev \
        libmhash2 \
        linux-cpupower \
        gawk \
        musl-tools \
        qemu-utils
      
      # Plot script requirements
      if [[ $DOCKER_PLOT == 'n' ]]; then
        apt-get install -y \
          python3 \
          python3-pip \
          python3-click \
          python3-tabulate \
          python3-numpy \
          python3-graphviz \
          python3-matplotlib \
          musl-tools \
          texlive-full \
          texlive-fonts-recommended \
          texlive-fonts-extra \
          dvipng

        pip3 install -r $WORKDIR/requirements.txt
      fi
      ;;
    *)
      log_err "This script is not yet compatible with $DISTRO"
      exit 1
      ;;
  esac
}

# Perform an action on an experiment
function perform() {
  local BASENAME=$1
  local ACTION=$2

  log_inf "Running $ACTION on $BASENAME..."

  export PATH=$PATH:$WORKDIR/tools

  case $ACTION in
    plot)
      if [[ $DOCKER_PLOT == 'y' ]]; then
        docker run -it --rm \
          -v $(pwd):/root/workspace -w /root/workspace \
          $DOCKER_IMAGE_PLOT \
          make -C /root/workspace/experiments/$BASENAME \
            PLOT=/root/workspace/plots/$BASENAME.$PLOT_FORMAT \
            plot
      else
        make -C $WORKDIR/experiments/$BASENAME \
          PLOT=$WORKDIR/plots/$BASENAME.$PLOT_FORMAT \
          plot
      fi
      ;;
    prepare|run|clean)
      make -C $WORKDIR/experiments/$BASENAME $ACTION
      ;;
    *)
      log_err "Unknown action: $ACTION"
      ;;
  esac
}

# Save positional arguments
REQUEST=$1
ACTION=$2

# Are we outputting the list?
if [[ $LIST_ALL == 'y' ]]; then
  printf "FIGURE_ID  TEST_NAME\n"

elif [[ $NO_DEPS != 'y' && $ACTION != "clean" ]]; then
  log_inf "Installing dependencies"
  install_dependencies
fi

# Perform the same action for every experiment?
case $REQUEST in
  prepare|run|plot|clean)
    ACTION=$REQUEST
    REQUEST=
    ;;
esac

# Should we start building dependencies?
if [[ $LIST_ALL != 'y' && $NO_DEPS == 'n' && $ACTION != "clean" ]]; then
  if  [[ $DOCKER_PLOT == 'y' ]]; then
    log_inf "Building docker..."
    DOCKER_FORCE_BUILD=$DOCKER_FORCE_BUILD make -C $WORKDIR docker
  else
    log_inf "Building utilities..."
    DOCKER_FORCE_BUILD=$DOCKER_FORCE_BUILD make -C $WORKDIR
  fi
fi

# Gather list of experiments
for E in $EXPERIMENTS_DIR/*; do
  BASENAME=$(basename $E)
  FIGURE_ID=${BASENAME:0:6}
  EXPERIMENT=${BASENAME:7}

  # Skip folders not well-formatted
  if [[ $EXPERIMENT == '' ]]; then continue; fi

  # Use opportunity of iteration to list
  if [[ $LIST_ALL == 'y' ]]; then
    if [[ $VERBOSE == 'y' || -f $E/Makefile ]]; then
      printf "%9s  %s\n" $FIGURE_ID $EXPERIMENT
    fi

    # Exit here for listing
    continue
  fi

  # We can only run experiments with a Makefile!
  if [[ ! -f $E/Makefile ]]; then
    log_dbg "No Makefile in $E"
    continue;
  fi

  # Run all experiments?
  if [[ -z "$REQUEST" || $FIGURE_ID == $REQUEST || $EXPERIMENT == $REQUEST ]]; then
    if [[ -z "$ACTION" ]]; then
      perform $BASENAME prepare
      perform $BASENAME run
      perform $BASENAME plot
    else
      perform $BASENAME $ACTION
    fi
  fi
done
