#!/bin/sh
echo "init alsa"
cp /var/lib/alsa/asound.state_org /var/lib/alsa/asound.state
sndCardStr=$(cat "/sys/class/sound/card0/id")
result=$(echo $sndCardStr | grep "x9core01ak7738")
if [ x"${result}" != x ];
then
  echo "set to ms board"
  amixer sset 'DSP Firmware PRAM' 'dsp1_data5'
  amixer sset 'DSP Firmware CRAM' 'dsp1_data5'
  amixer sset 'DSP Firmware PRAM' 'dsp2_data5'
  amixer sset 'DSP Firmware CRAM' 'dsp2_data5'
  amixer sset 'DSP Firmware PRAM' 'dsp3_data5'
  amixer sset 'DSP Firmware CRAM' 'dsp3_data5'
fi
result=$(echo $sndCardStr | grep "x9refak7738")
if [ x"${result}" != x ];
then
  echo "set default for ref03 "
  amixer sset 'DSP Firmware PRAM' 'dsp1_data2'
  amixer sset 'DSP Firmware CRAM' 'dsp1_data2'
  amixer sset 'DSP Firmware PRAM' 'dsp2_data2'
  amixer sset 'DSP Firmware CRAM' 'dsp2_data2'
fi
result=$(echo $sndCardStr | grep "x9ref04ak7738")
if [ x"${result}" != x ];
then
  echo "set default for ref04 "
fi
result=$(echo $sndCardStr | grep "x9refmach")
if [ x"${result}" != x ];
then
  echo "audiomanager enabled, do nothing "
fi
echo "dsp init done, snd: $sndCardStr"
alsactl restore
