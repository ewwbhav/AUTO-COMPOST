/*
 * SSIP AUTOCOMPOST - FIRMWARE V2 (AI / MACHINE LEARNING MODE)
 * - Uses TensorFlow Lite Micro for decision making
 * - Requires 'model_data.h' generated from training
 */

#include <Arduino.h>
#include <TensorFlowLite_ESP32.h>
#include "model_data.h" // <--- YOU NEED TO GENERATE THIS FILE FIRST
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

// ... [Include all Pin Defines & Setup functions from Code #1 here] ...

// AI Globals
const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;
constexpr int kTensorArenaSize = 2048;
uint8_t tensorArena[kTensorArenaSize];

void setupAI() {
    model = tflite::GetModel(model_data);
    static tflite::MicroErrorReporter micro_error_reporter;
    static tflite::AllOpsResolver resolver;
    static tflite::MicroInterpreter static_interpreter(
        model, resolver, tensorArena, kTensorArenaSize, &micro_error_reporter);
    interpreter = &static_interpreter;
    interpreter->AllocateTensors();
    input = interpreter->input(0);
    output = interpreter->output(0);
}

void loop() {
    // 1. Read Sensors (Same as Code 1)
    SensorData d = readSensors();

    // 2. Feed the Brain
    input->data.f[0] = d.ambTemp / 50.0;
    input->data.f[1] = d.ambHum / 100.0;
    input->data.f[2] = d.compostTemp / 80.0;
    input->data.f[3] = d.soilMoisture / 100.0;
    input->data.f[4] = d.airQualityPPM / 2000.0;

    // 3. Think
    interpreter->Invoke();

    // 4. Act
    float probToxic = output->data.f[2]; // Index 2 = Toxic State
    if (probToxic > 0.6) {
        setFanSpeed(100); // AI decided it's toxic
    } else {
        setFanSpeed(0);
    }
    
    delay(5000);
}
