# Multi-Level Authentication School Locker Using Arduino 📚🚨

This repository contains the code made for a <b> multi-level security locker system </b> using an <b> Arduino microcontroller </b> and its system implemented with <b>C++</b>.

[![Demo Video](https://img.shields.io/badge/Demo%20Video-Watch-red?logo=youtube)]()
[![Report](https://img.shields.io/badge/Report-Read-blue?logo=microsoft-word)](https://drive.google.com/file/d/1GR87IhDaqO7N5YS1hmfOAu4GgrUsVi7z/view?usp=sharing)

<img src="Arduino Poster.png" alt="x" style="border-radius: 15px; margin-bottom: 20px; width: 250px;">

## System Description & Workflow

This multi-level security locker system allows users to access their lockers through multi-layer authentication or enables school officials to override the lock in specific scenarios.

The system operates as follows:

1. Authentication Options:
    - Users start by selecting an option on the keypad module:
    - Press 1 for normal authentication.
    - Press 2 to use an override code for school officials.

4. Normal Authentication:
    - After selecting 1, the user places their finger on the fingerprint reader.
    - The system allows up to 5 attempts for successful fingerprint authentication. If all attempts fail, an alarm system is triggered.
    - Upon successful fingerprint verification, the system prompts the user for face recognition.
    - A successful match will grant the user access to the locker.

5. Override Code:
    - School officials or trusted members with access to the override code can bypass authentication via the keypad in case of emergencies or other authorized needs.

6. Alarm for Unauthorized Access Detection:
    - If unauthorized users fail multiple attempts, the system activates the alarm. The alarm can only be disabled using the override code by a school security official.


## Modules Used

This project utilizes the following hardware components:

1. 4x4 Membrane Keypad: Allows user input for authentication selection and override code entry.
2. Liquid Crystal I2C LCD: Displays user prompts and system messages.
3. Fingerprint Module FPM10A: Handles fingerprint-based user authentication.
4. Servo Motor for Arduino: Controls the locking and unlocking mechanism of the locker.
5. Buzzer Module: Alerts users and security personnel in case of unauthorized access or failed attempts.
6. Phone Camera: Used as a face recognition biometric reader for second step of authentication.
