# Summary

The purpose of this file is to provide instructions on how to test latency for the emergency brake feature on the Raspberry Pi and to document its evolution over time.

# Intructions

Start by defining a file to output the log entries derived from timestamps. You can either create your own or use the one already created in this branch:

```shell
Car_control$ find . -name "emergencyBreakLatencyTest.log"
$ ./tests/latencyTest/emergencyBreakLatencyTest.log
```

### Timestamp Code

As stated in the summary, the purpose is to test the latency of the emergency brake. This is a feature that may be triggered multiple times throughout the project. With that in mind, the emergency brake event chosen for this test is when the user presses the START_BUTTOM on the joystick. This ensures that the full execution path is measured to better verify system reliability.

You can start by checking the manual_code.cpp file and locating the START_BUTTON verification:

```shell
if (value == START_BUTTON)
```

After locating it, your task is to initialize timestamps and open the desired file to output the log:

```cpp
auto t_before = std::chrono::high_resolution_clock::now();

CANProtocol::sendEmergencyBrake(*carControl->can, true);
    
auto t_after = std::chrono::high_resolution_clock::now();
auto latency = std::chrono::duration_cast<std::chrono::microseconds>(t_after - t_before).count();

FILE* log = fopen("../tests/latencyTest/emergencyBreakLatencyTest.log", "a");
if (log) {
    fprintf(log, "LATENCY,%lld\n", (long long)latency);
    fclose(log);
}
```

Notice that it is important to first send the emergency brake message and only then calculate the timestamp and write to the file, since file I/O and extra processing also take time. Keep in mind that we are trying to squeeze out every last microsecond to improve efficiency.

Now that we have started the timer, we must continue along the execution path and verify when the message is actually sent.

You can do this by checking the socketCAN.c file. To ensure the timer only counts when the emergency brake is sent, add the following code:

```cpp
if (can_id == 0x100) {  // Only log emergency brake
	FILE* log = fopen("../tests/latencyTest/emergencyBreakLatencyTest.log", "a");
    struct timeval tv;
    gettimeofday(&tv, NULL);
    long long t_write = tv.tv_sec * 1000000LL + tv.tv_usec;

    fprintf(log, "CAN_WRITE,0x%X,%lld\n", can_id, t_write);
	if (log)
		fclose(log);
}
```

After completing all these steps, you are now ready to calculate the average latency. To do so, first check the generated log file (emergencyBreakLatencyTest.log). Then, run the already provided script made just for you to calculate the statistics :3

```shell
Car_control_Raspberry/Car_control$ find . -name "latencyAverageCalculator.sh"
$ ./tests/latencyTest/latencyAverageCalculator.sh
```

Expected Output:
```shell
===========================
Latency Analysis
===========================
Total measurements: x
Minimum latency:   x μs (x,x ms)
Maximum latency:   x μs (x,x ms)
Average latency:   x μs (x,x ms)
```

All set. Just don’t forget to document here any new latency improvements (or regressions :O ) in future updates.

# Practical latency updates

### Test 1
This first test represents the latency after the first stable version of the program, without careful attention to latency efficiency.

```shell
Latency  1:    266 μs (  0,266 ms)
Latency  2:    169 μs (  0,169 ms)
Latency  3:    182 μs (  0,182 ms)
Latency  4:    162 μs (  0,162 ms)
Latency  5:    161 μs (  0,161 ms)
Latency  6:    178 μs (  0,178 ms)
Latency  7:    169 μs (  0,169 ms)
Latency  8:    210 μs (  0,210 ms)
Latency  9:    182 μs (  0,182 ms)
Latency 10:    105 μs (  0,105 ms)
Latency 11:    182 μs (  0,182 ms)
Latency 12:    169 μs (  0,169 ms)
Latency 13:    108 μs (  0,108 ms)

===========================
Latency Analysis
===========================
Total measurements: 13
Minimum latency:   105 μs (0,105 ms)
Maximum latency:   266 μs (0,266 ms)
Average latency:   172 μs (0,173 ms)
===========================
```

### Test2

This test represents a program state where efficiency was a priority.
To achieve the goal of reducing emergency brake latency, the threading model was redesigned to remove responsibilities from the main thread and better distribute them across worker threads. This is the result:

```shell
Latency  1:     69 μs (  0,069 ms)
Latency  2:     50 μs (  0,050 ms)
Latency  3:     64 μs (  0,064 ms)
Latency  4:     72 μs (  0,072 ms)
Latency  5:     48 μs (  0,048 ms)
Latency  6:     52 μs (  0,052 ms)
Latency  7:     71 μs (  0,071 ms)
Latency  8:     59 μs (  0,059 ms)
Latency  9:     80 μs (  0,080 ms)
Latency 10:     62 μs (  0,062 ms)
Latency 11:     74 μs (  0,074 ms)
Latency 12:     47 μs (  0,047 ms)
Latency 13:     61 μs (  0,061 ms)
Latency 14:     85 μs (  0,085 ms)
Latency 15:     48 μs (  0,048 ms)
Latency 16:     52 μs (  0,052 ms)
Latency 17:     48 μs (  0,048 ms)

===========================
Latency Analysis
===========================
Total measurements: 17
Minimum latency:   47 μs (0,047 ms)
Maximum latency:   85 μs (0,085 ms)
Average latency:   60 μs (0,061 ms)
```
