#!/bin/sh

# for MCAL coding styles.
astyle -A8 --align-pointer=name --indent=spaces=4 --keep-one-line-blocks --pad-header --convert-tabs --break-blocks --pad-oper --pad-comma --max-code-length=100 --suffix=none --lineend=linux $@
