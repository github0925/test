@echo off
echo V1.20.1701_20200427
set BIN=%~dp0%fw\ddr_squeezer_loader_debug.elf.bin
set BIN_LOAD=%~dp0%ddr_squeezer_loader_usb.elf.bin
set TOOLS_HOME=%~dp0%\tools\windows
set PATH=%TOOLS_HOME%;%PATH%
set ShowHelp=0
if "%1" == "" (set ShowHelp=1 
goto PrintHelp
)

if "%1" == "help" (set ShowHelp=1 
goto PrintHelp
)

if exist %BIN_LOAD%.signed.rsa1024 del %BIN_LOAD%.signed.rsa1024

if "%1" neq "" ( copy /y /b %BIN% %BIN_LOAD% 
ddr_seq_parser.exe if=%1 of=%1.bin
bin_injector.exe if=%1.bin of=%BIN_LOAD% seek=0x5c000

if "%2" neq "" (bin_injector of=%BIN_LOAD% inj=0x5bfe0:0x01 inj=0x5bfe1:0x50 inj=0x5bfe2:0x41 inj=0x5bfe3:0x50
  bin_injector of=%BIN_LOAD% inj=0x5bfe4:%2 inj=0x5bfe5:0x00 inj=0x5bfe6:0x00 inj=0x5bfe7:0x00
  if "%3" neq "" (bin_injector of=%BIN_LOAD% inj=0x5bfe4:%2 inj=0x5bfe5:%3 inj=0x5bfe6:0x00 inj=0x5bfe7:0x00
  )
)
    
atb_signer.exe sign --sec_ver 1234 --pkg_ver 1 --rcp key=.\key\TestRSA1024_ossl.pem rot=1 --iib img=%BIN_LOAD% to=0x140000 entry=0x140000 uuid=12345678ab --dgst sha256 --of %BIN_LOAD%.signed.rsa1024
del %1.o %1.bin

echo;
echo TestSetup Done.
echo Please put your board into USB boot mode [boot pin=4'b1000] then load this image using USB
echo To run DDR_Squeezer, please type "memtester start size [loop=m] [core=n]" into uart terminal.
echo For example: memtester 0x40000000 0x100000000 loop=4 core=4  
echo;

del %BIN_LOAD%
)

:PrintHelp
if %ShowHelp% == 1 (echo;
echo Usage: %0 ddr_init_seq [security_uart_num ap_uart_num]
echo e.g.   Take x9_evb board for example, the command is as followed
echo        %0 .\ddr_script\x9_evb_lpddr4x_4266.c 10 9
echo feature list : 
echo       .\ddr_script\g9_ref_lpddr4x_2133.c 
echo       .\ddr_script\x9_evb_ddr4_1600.c
echo       .\ddr_script\x9_evb_ddr4_3200.c
echo       .\ddr_script\x9_evb_lpddr4x_2133.c
echo       .\ddr_script\x9_evb_lpddr4x_4266.c
echo       .\ddr_script\x9_evb_lpddr4x_ecc_2133.c
echo       .\ddr_script\x9_evb_lpddr4x_ecc_4266.c
echo;
)

