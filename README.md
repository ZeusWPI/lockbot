# Lockbot

Lockbot is a robot attached to our door that makes it possible for trusted members who don't have a key to open the door. It consists of a servo motor, two gears, a potentiometer and an Arduino Uno.

![A closeup of just the motor and gears](./pictures/lockbot_closeup.jpg)

![Global picture of lockbot](./pictures/lockbot_full.jpg)


## Build instructions

1. Install the Arduino IDE
2. Install the needed libraries in the Library Manager (see the top of `lockbot.ino`)
3. Copy `tokens.h.example` to `tokens.h` and edit them if nescessary
4. Select 'Arduino Uno' from the Board options
5. Program the Arduino

## 3D-print instructions

1. Install OpenSCAD
2. To print each part, comment out the other parts, export as STL then 3D print it. The big gear should be printed with 5 perimeter walls and solid infill, so it can handle the forces.

![The two gears and support used in lockbot](./lockbot.png "Gears")

## Components

- Arduino Uno + power supply for Arduino Uno (5V USB)
- Servo motor: DF15RSMG 360° Servo Motor - 20kg/cm + separate power supply for the servo motor
- Relay (also connected to motor power supply)
- Ethernet shield
- Buzzer
- Button
- 10k multiturn potentiometer

See `lockbot.ino` for how to connect these components together

Make sure that when turning the lock, the multiturn potentiometer does not go out of bounds
(overturning it WILL break it).

## Calibration

Before lockbot can be used, you'll need to calibrate 4 points using `client.py <subcommand>`

- `calibrate_open_pos`: the position you'd like lockbot to go to when opening the door
- `calibrate_open_bnd`: the last position where you consider the door as 'open'
- `calibrate_close_bnd`: the first position where you consider the door as 'locked'
- `calibrate_close_pos`: the position you'd like lockbot to go to when closing the door
