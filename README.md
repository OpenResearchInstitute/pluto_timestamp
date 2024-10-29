# Pluto Timestamp Routine

Implements a simple precise timestamping facility for ADALM Pluto
SDR platform, using a timer register built into the ARM Cortex-A9
processor core.

The timer register runs on the CPU clock, at 666,666,687 Hz
divided by 2, and constantly counts up from zero unless reset
by a write the timer register. This program never writes to the
timer register. The register is 64 bits wide, so it will roll
over after about 877 years.

## API Calls
If you include this in a larger program there are just two APIs
you need to worry about.

### void initialize_timer_register_access_block(void)
This function must be called once before any other API call.
If you're already using `/dev/mem` and `mmap` to access registers,
and you probably are, you will probably want to share the open
`/dev/mem` you already have and copy the `mmap` code from this
API instead of calling it.

### uint64_t get_timestamp(void);
Call this function to read the timestamp from hardware.

## Building the Code
This program can be built as a standalone demonstration program
to run on the Pluto. For example:
```
/usr/bin/arm-linux-gnueabihf-gcc --sysroot=/home/abraxas3d/pluto-0.38.sysroot -std=gnu99 -g -o pluto_timestamp pluto_timestamp.c -Wall -Wextra -DMAIN -DLOCAL_DMA_ROUTINES
scp pluto_timestamp root@pluto.local:/tmp
ssh root@pluto.local /tmp/pluto_timestamp
```

`-DMAIN` enables the main program. If you're integrating this code
with a larger program, leave this out.

`-DLOCAL_DMA_ROUTINES` enables the program to implement the
`read_dma()` and `write_dma()` functions. If you're integrating
this code with a program that already has these functions, leave
this out.

## Using the Timestamps
There are a number of ways you can use the timestamps provided
by this code. The example main program uses them to measure the
duration of a specific section of code, by taking a timestamp
before and another timestamp after, and simply subtracting and
scaling by COUNTS_PER_SECOND.

Another obvious application would be to control a loop so as to
terminate after a specified time. Take a timestamp at the start,
and add the desired duration in units of 1/COUNTS_PER_SECOND
seconds. Then each time through the loop, take a timestamp and
compare it to that computed sum. When the timestamp value exceeds
the sum, it's time to end the loop.

The timestamps could also be used for logging purposes, and for
any other purpose you like.

## About Timestamping Precision
The hardware counter updates every 3ns, but that doesn't mean
your timestamps are that accurate or that precise.

As a point of reference, two consecutive calls to
`get_timestamp()` with nothing in between takes 0.29 microseconds
on my Pluto. Any function calls you make between timestamps
will add additional overhead, of course.

Another issue is that the CPU clock is probably not locked to
whatever you're doing in hardware, such as a DSP sample clock.
The oscillators on a stock Pluto are not very accurate in
frequency. They are reasonably stable at room temperature,
but you can expect them to drift with ambient temperature.
As a result, you can't expect these timestamps to work for
synchronous signal processing on the Pluto. But they should
be pretty good for general timing and debugging purposes.