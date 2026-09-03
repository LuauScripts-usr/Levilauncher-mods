# LeviLauncher Controller Overdrive

A high-performance, fully customizable `.so` modification for LeviLauncher designed to bypass default Bedrock controller limitations, optimize raw input transmission, and maximize hit registration/CPS.

## Features
- **Zero-Deadzone Raw Input:** Absolute 0.00f deadzone for instant micro-adjustment registration.
- **Custom Response Curves:** Sub-linear mathematical remapping for precise target acquisition.
- **CPS Optimization:** Input polling acceleration for maximum clicks-per-second registration.
- **10000x Performance:** Compiled with `-O3 -ffast-math -flto` and zero-allocation hot paths.

## Compilation
This project utilizes GitHub Actions for automated cloud compilation. Pushing to the `main` branch will automatically compile the ARM64 `.so` binary.
