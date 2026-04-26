# Power Quality Waveform Analyser

**Module:** UGMFGT-15-1 Programming for Engineers  
**Academic Year:** 2025–26  
**University:** University of the West of England, Bristol  
**GitHub:** https://github.com/salem102/power_quality_analyser

---

## What This Program Does

This command-line C application reads a power quality sensor log file (`power_quality_log.csv`), analyses the three-phase voltage waveforms, and produces a structured report (`results.txt`). It computes:

- RMS voltage per phase (with EN 50160 compliance check)
- Peak-to-peak amplitude per phase
- DC offset per phase
- Standard deviation and variance per phase (Merit/Distinction extension)
- Clipping detection per phase (|V| >= 324.9 V)
- Bitwise status flags per phase (Distinction extension)
- System-wide frequency, power factor, and THD statistics

---

## File Structure

```
power_quality_analyser/
├── main.c            Entry point; command-line argument handling and orchestration
├── waveform.c        All analysis functions (RMS, peak-to-peak, DC offset, etc.)
├── waveform.h        WaveformSample struct, PhaseMetrics struct, function prototypes
├── io.c              CSV loading and results file writing
├── io.h              I/O function prototypes
├── CMakeLists.txt    CMake build configuration
├── README.md         This file
└── logbook.docx      Development logbook (submitted separately)
```

---

## How to Compile and Run

### Option 1: CLion (Recommended IDE)

1. Open CLion and select **File -> Open**.
2. Navigate to the `power_quality_analyser/` folder and click **Open**.
3. CLion will detect `CMakeLists.txt` automatically.
4. Click the **green Run button** or press **Shift+F10**.
5. To pass the CSV argument: go to **Run -> Edit Configurations**, and in the **Program arguments** field enter:
   ```
   power_quality_log.csv
   ```
6. Ensure `power_quality_log.csv` is in the **working directory** (set in the same Run Configuration dialog).

### Option 2: Command-Line with CMake

```bash
cd power_quality_analyser
mkdir build && cd build
cmake ..
make
cp ../power_quality_log.csv .
./power_analyser power_quality_log.csv
```

### Option 3: Direct GCC (single command)

```bash
gcc -std=c99 -Wall -Wextra main.c waveform.c io.c -o power_analyser -lm
./power_analyser power_quality_log.csv
```

> **Note:** The `-lm` flag is required to link the math library (`sqrt()` in `waveform.c`).

---

## Expected Output

```
Loaded 1000 samples from 'power_quality_log.csv'.
Analysing Phase A...
Analysing Phase B...
Analysing Phase C...

Power Quality Analysis - Results

Phase A:
  RMS Voltage  : 230.0101 V  [COMPLIANT]
  Peak-to-Peak : 649.8000 V
  DC Offset    : +0.000000 V
  Std Dev      : 162.6347 V
  Clipped      : 3 samples
  Status Flags : 0x01

Total clipped samples across all phases: 9

Writing results to results.txt...
Results file written ok.
Done.
```

A full report will be saved to `results.txt` in the working directory.

---

## Dependencies

- C99-compliant compiler (GCC >= 9 or MinGW on Windows)
- CMake >= 3.15 (for CLion builds)
- Standard C library (`stdio.h`, `stdlib.h`, `math.h`, `stdint.h`, `stddef.h`)
