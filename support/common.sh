#!/usr/bin/env bash
# SPDX-License-Identifier: BSD-3-Clause
# Authors: Alexander Jung <a.jung@lancs.ac.uk>
if [[ ! -n $WORKDIR ]]; then echo "Don't execute me!"; exit 1; fi

# Logger tools
NC='\033[0m'
RED='\033[0;31m'
log_err() { echo -e "[${RED}ERR${NC}] $1"; }
GRN='\033[0;32m'
log_inf() { echo -e "[${GRN}INF${NC}] $1"; }
YLO='\033[0;33m'
log_dbg() { if [[ $VERBOSE == "y" ]]; then echo -e "[${YLO}DBG${NC}] $1"; fi; }

# Sanity checks
# Check if all necessary tools to run ukbench are installed
function check_tools {
  local MISSING=
  local COMMANDS=(
  )

  log_dbg "Checking for missing tools..."

  for COMMAND in "${COMMANDS[@]}"; do
    if [[ $VERBOSE == 'y' ]]; then
      printf '      * %-15s' "$COMMAND"
    fi
    if hash "$COMMAND" 2>/dev/null; then
      if [[ $VERBOSE == 'y' ]]; then
        echo OK
      fi
    else
      MISSING="$COMMAND ${MISSING}"
      if [[ $VERBOSE == 'y' ]]; then
        echo -e "\033[0;31mmissing\033[0m"
      fi
    fi
  done

  if [[ $MISSING != "" ]]; then
    log_err "Missing programs: $MISSING"
    exit 1
  fi
}

# Prompts
function maybe {
  local CMD="$@"

  log_inf "${CMD}"
  if [[ $DRY_RUN != 'y' ]]; then
    bash -c "${CMD}"
  fi
}

function confirm_or_die() {
  local YES_IM_SURE=$1

  if [[ $YES_IM_SURE != 'y' ]]; then
    read -p "Are you sure wish to continue? [Ny]: " -n 1 -r
    echo    # (optional) move to a new line
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
      log_err "Cancelling operation!"
      exit 1
    fi
  fi
}

# trace ERR through pipes
set -o pipefail

# trace ERR through 'time command' and other functions
set -o errtrace

# exit the script if any statement returns a non-true return val
set -o errexit
