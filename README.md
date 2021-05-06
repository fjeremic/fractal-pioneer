# Fractal Pioneer
[![GitHub license](https://img.shields.io/badge/license-GPL_2.0-blue.svg?style=flat)](https://github.com/fjeremic/HexView.Wpf/blob/master/LICENSE)

<br/>
<p align="center">
  <a href="https://youtu.be/uBj1ZrGdKK0">
    <img align="center" alt="HexViewer demo project" src="https://raw.githubusercontent.com/fjeremic/fractal-pioneer/assets/FractalPioneerYoutube.png" href="google.com"/>
  </a>
</p>
<br/>

## Introduction

Fractal Pioneer is a Qt based application for exploring 3D fractals and generating high-FPS high-resultion keyframes from a user defined set of waypoints using a constant speed interpolated camera system. The application was used to generate the above linked YouTube video using a preloaded set of waypoints which are included with the application.
 
This animation is a screen capture of the application when the `Preview Keyframes` button is clicked which animates the preloaded waypoints.

<br/>
<p align="center">
  <img align="center" src="https://raw.githubusercontent.com/fjeremic/fractal-pioneer/assets/FractalPioneerScreenshot.gif"/>
</p>
<br/>

## Key Bindings

Click on the fractal viewport to capture the keyboard and mouse. Use your mouse to look around.

| Key         | Action                                                |
|-------------|-------------------------------------------------------|
| `W`         | Move forward                                          |
| `A`         | Move sideways (strafe) to the left                    |
| `S`         | Move backward                                         |
| `D`         | Move sideways (strafe) to the right                   |
| `E`         | Rotate camera counterclockwise                        |
| `Q`         | Rotate camera clockwise                               |
| `Space`     | Record waypoint                                       |
| `Backspace` | Remove last waypoint                                  |
| `Delete`    | Clear waypoints                                       |
| `Escape`    | Stop animation/preview or stop mouse/keyboard capture |
