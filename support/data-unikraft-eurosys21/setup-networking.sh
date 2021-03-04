#!/bin/bash

set -x

brctl addbr br0
ifconfig br0 172.133.1.1
