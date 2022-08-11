#!/bin/sh

PULSE_DIR="/tmp/$( whoami )-pulse"
mkdir -p $PULSE_DIR && chmod 755 $PULSE_DIR
#export PULSE_CONFIG_PATH=$PULSE_DIR
#export PULSE_STATE_PATH=$PULSE_DIR
#export PULSE_RUNTIME_PATH=$PULSE_DIR
export XDG_CONFIG_HOME=$PULSE_DIR
export XDG_RUNTIME_DIR=$PULSE_DIR
#export XDG_RUNTIME_DIR=$PULSE_DIR
pulseaudio --start --log-targ=auto
