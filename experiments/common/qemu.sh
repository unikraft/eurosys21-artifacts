#!/bin/bash

function kill_qemu {
	killall -9 qemu-system-x86
	pkill -9 qemu-system-x86
}
