# A collection of benchmarks, tools, scripts, and general banalities

## A common format

The format should be something like this per measurement:

```json
{
    "cmd": "comm.exe arg1 arg2",
    "unit": "seconds",
    "measurement": 42, // Actual value

    // These are optional and there can be more, depending on the tool and benchmark
    "label": "libseff", // to group runs together
    "stddev": 1222,
    "measurements": [1, 2, 3, 776767], // If it was run multiple times (stddev can be calculated from it)
    "parameter": {"what": "arg2", "WOT": 3} // if the run is parametrized
}
```

But we usually work with list of measurements

## Style guidelines

* Make libseff pop and uniform across benchmarks