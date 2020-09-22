# free-dap

[free-dap](https://github.com/ataradov/free-dap) is a free and open implementation of the CMSIS-DAP debugger firmware.

This the free-dap CMSIS-DAP probe, running as a micropython extension.

free-dap only supports SWD, not JTAG.

## Use

The DAP probe is a USB HID device. To configure micropython as a HID device,  put the following line in boot.py:

```
import dap
pyb.usb_mode('VCP+HID', vid=0x1d50, pid=0x6018, hid=dap.hid_info)
```
To start the DAP probe, type from the micropython prompt:
```
dap.init()
```
Now you can connect openocd to the DAP probe:
```
$ openocd -f interface/cmsis-dap.cfg -f target/stm32f1x.cfg
```
and in another window you can connect gdb to openocd:

```
$ arm-none-eabi-gdb -q -nh
(gdb) target extended-remote localhost:3333
```
and begin debugging.

## Clock frequency calibration
The SWD clock frequency has an accuracy of better than 1%. 

For increased accuracy, the SWD clock frequency can be calibrated.

At the micropython command prompt, type *dap.calibrate()*

 Calibration takes 20 seconds. Calibration output is a tuple of two numbers:
 
```
>>> dap.calibrate()
[=================]
(35985, 8999685)
```
Calibration data are lost when micropython reboots, but there is no need to re-run calibration every time micropython boots. 
Once you know the calibration data of your board, you can simply set calibration values in main.py:

```
>>> dap.calibrate(cal=(35985, 8999685))
```

## Clock frequency measurement 

The calibration tuple consists of two numbers: 

- the first number is the number of delay loops that is needed to produce a clock of 1 kHz
- the second number is the clock frequency that results from using no (0) delay loops.

These two numbers may differ between boards,  micropython versions and compilation runs.

To measure clock frequency using a frequency meter or an oscilloscope, it is possible to output a fixed clock on the SWCLK pin. Simply call calibrate(), with the number of delay cycles wanted. E.g. on the above board, 

```
>>> dap.calibrate(delay=35985)
```
gives a 1kHz square wave, and 
```
>>> dap.calibrate(delay=0)
```
gives a 9 MHz square wave. A call to dap.calibrate(delay=*number*) does not return; type ctrl-c when you are done measuring.

If you attach an oscilloscope to the SWCLK pin  while dap.calibrate() is running, you will see the clock frequency converging to 1 kHz.

## Compilation

On stm32, *mod_dap* requires pins called DAP_SWDIO, _SWCLK, and DAP_SRST in *pins.csv*, eg.:

```
DAP_SWDIO,PB1
DAP_SWCLK,PB0
DAP_SRST,PB11
```

The function *dap_loop()* needs to be called periodically, eg. from the micropython event loop.

To compile, type

```
cd ports/stm32
make USER_C_MODULES=../../../extmod BOARD=PYBD_SF2
```
## To do 
Optimize for high-speed usb

## See also

Original [free-dap](https://github.com/ataradov/free-dap) software, written by A. Taradov.

