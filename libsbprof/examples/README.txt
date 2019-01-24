$Id: README.txt,v 1.1 2003/05/16 06:18:35 cgd Exp $

This directory contains examples which demonstrate use of the
libsbprof profiling library.


Example perfex1
---------------

This example starts up profiling, then spin-loops for a second
reading the ZBBus Count register.

When run, it will produce output like:

| CFE> boot -elf server:/dir/perfex1
| Loader:elf Filesys:tftp Dev:eth0 File:server:/dir/perfex1 Options:(null)
| Loading: 0xffffffff80020000/52416 0xffffffff8002ccc0/3904 Entry at 0xffffffff80020004
| Closing network.
| Starting program at 0xffffffff80020004
| *** libsbprof: to save cpu profiling data, use a command like:
|  ifconfig eth0 -auto; save server:/dir/cpu-samples 0xffffffff8002e080 0x6f64
| *** libsbprof: to save zbbus profiling data, use a command like:
|  ifconfig eth0 -auto; save server:/dir/zb-samples 0xffffffff8182e0a0 0xea000
| 
| *** program exit status = 0
| CFE>

Use commands like the ones mentioned in the output to save the CPU
and ZBBus profiling data to the server.  (If the directory from
which the program is run is writable via TFTP, it's quite likely
that you can just paste the commands back to CFE.  If not, adjust
them as appropriate, but use the addresses and lengths output
by the library.)

Once the data has been saved to the server, you can use the performance
tools on the data files.

To analyze the ZBbus profiling data, you can use 'tbanal' directly on
the zb-samples file.

To analyze the CPU profiling data, first use 'pgather' to create
a profile database from the data:

| % mkdir db-dir
| % pgather -file cpu-samples db-dir -kf perfex1 -rm

Then, use pshow to to analyze the program as normal:

| % pshow db-dir -e cycles -f perfex1
|             Event      Count     Period
|             -----      -----     ------
|            cycles       3007     131072
| 
| All samples are from image perfex1
| 
|   cycles     %  cum% Proc               
|   ------  ----  ---- ----               
|     2749  91.4  91.4 main               
|      258   8.6 100.0 sbprof_zbbus_count 

(In this example, some amount of time is spent in the routine that
reads the ZBBus Count register, but most of time piles up on the use
of the value that is read.)

For more information about using the performance analysis tools to
analyze program behaviour, please see the HTML documentation and
manual pages that comes with them.

