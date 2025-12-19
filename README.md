LIS2DE12 Accelerometer application for detecting a car crash.
The application starts in a sleep state where the accelerometer is setup with a slow sample rate and a motion detection interrupt on INT pin 2.
When a motion is detected (car is running) it enters the active state where the sample rate is increased and the g-force threshold is increased.
If the g-force threshold is reached, the application enters crash state and the sample rate is increase to maximum (5376 Hz) and 200 samples is checked.
If at any time, one of the 200 samples (37 ms real time) changes direction or fall below <tbd> g, the crash is considered false and the detection is aborted.
The application then falls back to active state again.
Each time the application enters the active state, a 1 minute timer is started. When the timer has reached 0 the application goes to sleep state.
