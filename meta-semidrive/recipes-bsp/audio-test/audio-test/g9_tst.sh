#!/bin/sh
echo "parameter: "$1
if [ $# -eq 0 ];
then
    echo "err: param numbers, pls use ./g9_tst.sh p1,p2,p3,p4,p5,c1,c2,c3,c4,c5"
    exit
fi

if [ $1 = "p1" ];
then
    echo "tst playback 1: aplay cov_stereo_48k_16bits.wav"
    amixer sset "PCM" 107
    amixer sset "Output Driver Power-On time" 200ms
    aplay cov_stereo_48k_16bits.wav
    exit
elif [ $1 = "p2" ];
then
    echo "tst playback 2: tinyplay cov_stereo_8k_16bits.wav -D 0"
    amixer sset "PCM" 107
    amixer sset "Output Driver Power-On time" 200ms
    aplay cov_stereo_8k_16bits.wav
    exit
elif [ $1 = "p3" ];
then
    echo "tst playback 3: tinyplay cov_stereo_16k_16bits.wav -D 0"
    amixer sset "PCM" 107
    amixer sset "Output Driver Power-On time" 200ms
    aplay cov_stereo_16k_16bits.wav
    exit
elif [ $1 = "p4" ];
then
    echo "tst playback 4: tinyplay cov_stereo_32k_16bits.wav -D 0"
    amixer sset "PCM" 107
    amixer sset "Output Driver Power-On time" 200ms
    aplay cov_stereo_32k_16bits.wav
    exit
elif [ $1 = "p5" ];
then
    echo "tst playback 5: tinyplay cov_stereo_44.1k_16bits.wav -D 0"
    amixer sset "PCM" 107
    amixer sset "Output Driver Power-On time" 200ms
    aplay cov_stereo_44.1k_16bits.wav
    exit
elif [ $1 = "c1" ];
then
    echo "tst capture 1: arecord -f dat 48k_stereo_16.wav"
    amixer sset "Left PGA Mixer Line1L" off
    amixer sset "Left PGA Mixer Line1R" on
    amixer sset "PGA" 80
    arecord -f dat 48k_stereo_16.wav
    exit
elif [ $1 = "c2" ];
then
    echo "tst capture 2: arecord -t wav -r 44100 -c 2 -f S16_LE 44.1k_stereo_16.wav"
    amixer sset "Left PGA Mixer Line1L" off
    amixer sset "Left PGA Mixer Line1R" on
    amixer sset "PGA" 80
    arecord -t wav -r 44100 -c 2 -f S16_LE 44.1k_stereo_16.wav
    exit
elif [ $1 = "c3" ];
then
    echo "tst capture 3: arecord -t wav -r 32000 -c 2 -f S16_LE 32k_stereo_16.wav"
    amixer sset "Left PGA Mixer Line1L" off
    amixer sset "Left PGA Mixer Line1R" on
    amixer sset "PGA" 80
    arecord -t wav -r 32000 -c 2 -f S16_LE 32k_stereo_16.wav
    exit
elif [ $1 = "c4" ];
then
    echo "tst capture 4: arecord -t wav -r 16000 -c 2 -f S16_LE 16k_stereo_16.wav"
    amixer sset "Left PGA Mixer Line1L" off
    amixer sset "Left PGA Mixer Line1R" on
    amixer sset "PGA" 80
    arecord -t wav -r 16000 -c 2 -f S16_LE 16k_stereo_16.wav
    exit
elif [ $1 = "c5" ];
then
    echo "tst capture 5: arecord -t wav -r 8000 -c 2 -f S16_LE 8k_stereo_16.wav"
    amixer sset "Left PGA Mixer Line1L" off
    amixer sset "Left PGA Mixer Line1R" on
    amixer sset "PGA" 80
    arecord -t wav -r 8000 -c 2 -f S16_LE 8k_stereo_16.wav
    exit
else
    echo "cann't find test case with id "$1
    exit
fi
