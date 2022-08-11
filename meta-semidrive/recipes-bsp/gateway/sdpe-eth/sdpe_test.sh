#!/bin/sh

# Ignore SIGTTIN signal when running background
trap "" SIGTTIN

echo 10 > /sys/devices/system/cpu/cpufreq/ondemand/up_threshold
echo 100 > /sys/devices/system/cpu/cpufreq/ondemand/sampling_down_factor

sdpe_test_app $@
