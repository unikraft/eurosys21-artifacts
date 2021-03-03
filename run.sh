#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# Authors: Alexander Jung <a.jung@lancs.ac.uk>
WORKDIR=$(dirname $0)
. $WORKDIR/support/common.sh

# Influential environmental variables
EXPERIMENTS_DIR=${EXPERIMENTS_DIR:-$(pwd)/experiments}
DOCKER_IMAGE_PLOT=${DOCKER_IMAGE_PLOT:-unikraft/eurosys21-artifacts-plot:latest}
PLOT_FORMAT=${PLOT_FORMAT:-svg}

# Program arguments
SHOW_HELP=n
LIST_ALL=n
DRY_RUN=n
NO_DOCKER=n
VERBOSE=n

_help() {
    cat <<EOF
$0 - Run all or a specific experiment.

Usage:
  $0 [OPTIONS] [FIGURE_ID|TEST_NAME] [ACTION]

If no figure ID or test name is provided, the action is run for all
experiments.

Example:
  $0 fig_01

Actions:
  prepare            Prepares the host and/or builds dependent tools
                       and images before the test is run.
  run                Runs the given experiment and saves the results.
  plot               Uses the data from the experiment to generate
                       the plot.
  clean              Clean intermediate build files from an experiment.

Options:
  -D --no-docker     Do not use Docker for plotting.
  -l --list          List all tests and exit.
  -v --verbose       Be verbose.
  -h --help          Show this help menu.

Influential Environmental Variables
  EXPERIMENTS_DIR    Directory of all the experiments
                       (default: ./experiments).
  DOCKER_IMAGE_PLOT  Docker environment for generating plots
                       (default: $DOCKER_IMAGE_PLOT).
  PLOT_FORMAT        File format for the plot
                       (default: $PLOT_FORMAT).
EOF
}

# Perform an action on an experiment
_perform() {
  local BASENAME=$1
  local ACTION=$2

  log_dbg "Running $ACTION on $BASENAME..."

  case $ACTION in
    plot)
      if [[ $NO_DOCKER == 'n' ]]; then
        docker run -it --rm \
          -v $(pwd):/root/workspace -w /root/workspace \
          $DOCKER_IMAGE_PLOT \
          make -C /root/workspace/experiments/$BASENAME \
            PLOT=/root/workspace/plots/$BASENAME.$PLOT_FORMAT \
            plot
      else
        make -C $WORKDIR/experiments/$BASENAME plot
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


# Parse flag arguments
for i in "$@"; do
  case $i in
    -v|--verbose)
      VERBOSE=y; shift;;
    -l|--list)
      LIST_ALL=y; shift;;
    -D|--no-docker)
      NO_DOCKER=y; shift;;
    -h|--help)
      _help; exit 0;;
    *)
      ;;
  esac
done

# Save positional arguments
REQUEST=$1
ACTION=$2

# Are we outputting the list?
if [[ $LIST_ALL == 'y' ]]; then
  printf "FIGURE_ID  TEST_NAME\n"

# Do we need Docker?
elif [[ $NO_DOCKER == 'n' ]]; then
  log_dbg "Building utility containers..."
  make -C $WORKDIR docker
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

  # Perform the same action for every experiment?
  case $REQUEST in
    prepare|run|plot|clean)
      ACTION=$REQUEST
      REQUEST=$EXPERIMENT
      ;;
  esac

  # Run all experiments?
  if [[ -z "$REQUEST" ]]; then
    _perform $BASENAME $ACTION
  elif [[ $FIGURE_ID == $REQUEST || $EXPERIMENT == $REQUEST ]]; then
    _perform $BASENAME $ACTION
  fi
done
