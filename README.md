# Nexys A7 Etch-A-Sketch

A digital Etch-A-Sketch implemented on the **Nexys A7 FPGA**, built atop **Chu’s MMIO framework**.  
This project uses SystemVerilog for the hardware side and a C++ application (via Vitis) for input control.  
It supports real-time drawing, erase/reset, and is structured for modular enhancements (colors, brush sizes, save/load, etc.).

---

## Repository Structure
├── hardware/ ← SystemVerilog modules (VGA, pixel buffer, cursor logic, etc.) \n
├── software/ ← Vitis C++ source (main app, drivers, headers)\n
└── README.md


---

## Features

- Real-time drawing on VGA (640×480)
- Keyboard control via PS/2 (arrow keys / WASD)
- Draw / erase toggle
- Screen clear / reset function (shake-like)
- Modular architecture for future extension

---

## Tools & Requirements

- **FPGA Board**: Nexys A7 (Xilinx Artix-7)
- **HDL**: SystemVerilog, Vivado (specify version, e.g., 2022.2)
- **Software**: C++ via Vitis (specify version)
- **Interfaces**: VGA output, PS/2 keyboard

---

## Building & Usage

### Hardware

1. Open the `hardware/` folder in Vivado.  
2. Synthesize, implement, generate bitstream.  
3. Program the Nexys A7 with the bitstream.

### Software

1. Import the `software/` into Vitis workspace using the exported hardware platform.  
2. Build the application.  
3. Run it on the board; the software reads PS/2 key inputs and updates the hardware via MMIO.
