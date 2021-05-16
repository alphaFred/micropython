.. _mimxrt_general:

General information about the i.MX RT port
==========================================

The i.MX RT is a family of powerfull controllers. MicroPython tries to provide a 
generic port which runs on as many members of that family as well as boards.
Currently only controllers of the i.MX RT family featuring an ARM Cortex M7 
controller are supported.

The port is tested and developed on NXP evaluation baords, as well as then PRJC Teensy 4.0.
For any other board you are using please make sure you have a datasheet, 
schematics and other reference materials so you can look up any board-specific 
functions. The following boards are supported by the port out of the box:

- MIMXRT1010-EVK
- MIMXRT1020-EVK
- MIMXRT1050-EVK
- MIMXRT1060-EVK
- MIMXRT1064-EVK
- Teensy 4.0

Supported Processors
--------------------

+-------------+--------------------+-------------------------+
| Product     | CPU                | Memory                  |
+=============+====================+=========================+
| i.MX RT1064 | Cortex-M7 @600 MHz | 1 MB SRAM, 4 MB Flash   |
+-------------+--------------------+-------------------------+
| i.MX RT1060 | Cortex-M7 @600 MHz | 1 MB SRAM               |
+-------------+--------------------+-------------------------+
| i.MX RT1050 | Cortex-M7 @600 MHz | 512 kB SRAM             |
+-------------+--------------------+-------------------------+
| i.MX RT1020 | Cortex-M7 @500 MHz | 256 kB SRAM             |
+-------------+--------------------+-------------------------+
| i.MX RT1010 | Cortex-M7 @500 MHz | 128 kB SRAM             |
+-------------+--------------------+-------------------------+

Note
    Most of the controllers do not have internal flash memory. Therefore
    their flash capacity is dependant on an external flash chip!
