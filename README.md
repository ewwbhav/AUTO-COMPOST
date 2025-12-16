# AUTO-COMPOST
this is my project called AUTCOMPOST - smart composting bin , i was awarded with grant from SSIP cell of our uni

---

#SSIP AutoCompost: Smart AI-Powered Composting Bin ♻️🤖**Current Status:** TRL 3 (Proof of Concept) - Electronics & Algorithms Validated.

##📖 Project OverviewThe **SSIP AutoCompost** is a smart, automated solution for organic waste management. It automates the composting process using a microcontroller-driven system with motorized mixing and environmental monitoring. The system uses **TinyML (Machine Learning on the Edge)** to intelligently classify compost health and automate maintenance (aeration/cooling) with minimal human intervention.

###Key Features* **Automated Operation:** Manages organic waste decomposition with minimal user effort.
* **Smart Monitoring:** Real-time tracking of Compost Temperature, Soil Moisture, Ambient Humidity, and Air Quality (Gas/Odors).
* **Dual-Mode Firmware:** Features a data-collection mode for training and an AI-inference mode for autonomous decision-making.
* **Mechanical Control:** High-torque DC gear motor (via BTS7960) for mixing and high-speed fan for aeration.
* **Energy Efficient:** Designed for low-power operation with support for battery/solar power.

---

##🛠️ Hardware Components| Component | Function |

| --- | --- |
| **ESP32 DevKit V1** | Main Microcontroller (The Brain) |
| **BTS7960** | High-Current H-Bridge Motor Driver (43A) |
| **DC Gear Motor** | High-torque motor for mixing compost |
| **DS18B20** | Waterproof Digital Temperature Sensor (Compost Core Temp) |
| **DHT22** | Digital Sensor (Ambient Temp & Humidity) |
| **Capacitive Soil Moisture** | Analog Sensor (Compost Moisture Content) |
| **MQ-135** | Analog Gas Sensor (Air Quality / Ammonia / CO2) |
| **LCD (16x2 I2C)** | Local Data Display |
| **MOSFET (IRLZ44N)** | Switch for 12V Cooling Fan |
| **Buck Converter** | 12V to 5V Step-down for ESP32 power |

---

##🔌 Pinout Configuration*Based on current PCB soldering configuration.*

| Component | Pin Name | ESP32 GPIO | Notes |
| --- | --- | --- | --- |
| **DS18B20** | Data | **GPIO 13** | *Swapped from standard pinout* |
| **DHT22** | Data | **GPIO 4** |  |
| **Soil Moisture** | Analog Out | **GPIO 34** | Input Only |
| **MQ-135** | Analog Out | **GPIO 35** | **Via Voltage Divider (2.2k/3.3k)** |
| **Fan (MOSFET)** | Gate | **GPIO 25** | PWM Capable |
| **Mixer (BTS7960)** | L_PWM | **GPIO 32** |  |
| **Mixer (BTS7960)** | R_PWM | **GPIO 33** |  |
| **Mixer (BTS7960)** | Enables | **GPIO 26, 27** |  |
| **LCD** | SDA / SCL | **GPIO 21, 22** | I2C Bus |

---

##🚀 Development Process (How It Works)This project follows a "Data-First" Machine Learning workflow. The firmware is split into two stages to enable the system to learn from its specific environment.

###Phase 1: Data Collection & "Standard" Logic**Firmware:** `src/firmware_v1_data_collection.cpp`

* **Goal:** To gather training data while maintaining basic compost health.
* **Logic:** Uses hard-coded thresholds (e.g., `If Temp > 60°C THEN Fan ON`) and timers (Mix every 12 hours).
* **Output:** The system logs sensor readings to the Serial Monitor in CSV format:
`AmbientTemp, Humidity, SoilMoisture, CompostTemp, GasPPM`
* **Action:** We run this phase for 1-2 weeks to capture the behavior of the compost bin in different states (Healthy, Dry, Toxic).

###Phase 2: AI Model Training* **Goal:** To teach the ESP32 to recognize compost states.
* **Workflow:**
1. Export CSV data from Phase 1.
2. Label the data manually (0=Healthy, 1=Dry, 2=Toxic).
3. Train a Neural Network using **TensorFlow Lite (TFLite)** or **Edge Impulse**.
4. Generate a C++ library file (`model_data.h`) containing the trained model weights.



###Phase 3: Sophisticated AI Control**Firmware:** `src/firmware_v2_ai_inference.cpp`

* **Goal:** Autonomous, intelligent control.
* **Logic:** The system feeds live sensor data into the TFLite model. The AI outputs a probability for the current state.
* *Example:* "I am 90% sure the pile is becoming Toxic." -> **Action:** Pre-emptive aeration (Fan ON) before the smell becomes noticeable.


* **Advantage:** Detects complex conditions (e.g., "High Moisture + High Temp + Rising Gas") that simple `IF/ELSE` statements might miss.

---

##📂 Repository Structure```text
/SSIP-AutoCompost
├── /src
│   ├── firmware_v1_data_collection.cpp  # Current running code (TRL 3-4)
│   ├── firmware_v2_ai_inference.cpp     # Future code (TRL 6)
│   └── model_data.h                     # (Placeholder) Trained model file
├── /docs
│   ├── circuit_schematic.png
│   └── pinout_diagram.pdf
├── /data
│   └── training_logs_sample.csv         # Sample data for testing
└── README.md

```

##⚠️ Setup & Calibration Notes1. **MQ-135 Burn-in:** The gas sensor requires 24-48 hours of pre-heating to give stable readings.
2. **Moisture Calibration:**
* `Dry Value` (Air): ~3200 (Raw ADC)
* `Wet Value` (Water): ~1400 (Raw ADC)
* *Update these in `firmware_v1` before uploading.*


3. **Voltage Safety:** Ensure the MQ-135 (5V output) is connected via the voltage divider to protect the ESP32 (3.3V Max).

---

**Developed for SSIP (Student Startup and Innovation Policy).**
