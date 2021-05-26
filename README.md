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

Fractal Pioneer is a Qt based application for exploring 3D fractals and generating high-FPS high-resultion keyframes
from a user defined set of waypoints using a constant speed interpolated camera system. The application was used to
generate the above linked YouTube video using a preloaded set of waypoints which are included with the application.
 
This animation is a screen capture of the application when the `Preview Keyframes` button is clicked which animates
the preloaded waypoints.

<br/>
<p align="center">
  <img align="center" src="https://raw.githubusercontent.com/fjeremic/fractal-pioneer/assets/FractalPioneerScreenshot.gif"/>
</p>
<br/>

## Key Bindings

Click on the fractal viewport to capture the keyboard and mouse. Use your mouse to look around. To record your own
custom waypoints uncheck the `Use Preloaded Waypoints` checkbox on the right and use the hotkeys below.

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

## Technical Details

### Drawing The Fractal

The 3D fractal in the Fractal Pioneer application is drawn using a fragment shader ([`frag.glsl`][1]). The fragment which
the fragment shader processes is a simple square covering the canvas where the 3D fractal is drawn. This square is
drawn on an OpenGL Vertex Buffer Object in the [`resizeGL`][2] function. For each pixel on the fragment (our square) the
fragment shader casts a ray from the camera through the pixel to [calculate the colour][3] for this pixel.

We use the [ray marching method][4] and a [distance estimate forumla][5] for our fractal to calculate the maximum 
distance we can ray march without intersecting the 3D fractal boundary. This process is [iterated][6] until we either
reach some defined minimum distance to the fractal or our ray has completely missed the fractal. Depending on the
distance returned by the distance estimator we decide whether to colour the pixel using the [background colour][7] or
using the [fractal colour][8] at that location. The fractal colour is calculated using [orbit traps][9] by the 
[`fractalColour`][10] function.

This is all that is needed to draw the 3D fractal. The rest of the code in `frag.glsl` makes the fractal more visually
pleasing by smoothing it out via anti-aliasing, adding ambient occlusion, fog, filtering, shadows, etc.

[1]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/frag.glsl
[2]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/FractalWidget.cpp#L731-L768
[3]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/frag.glsl#L230-L231
[4]: https://www.iquilezles.org/www/articles/raymarchingdf/raymarchingdf.htm
[5]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/frag.glsl#L68-L83
[6]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/frag.glsl#L108-L123
[7]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/frag.glsl#L207-L210
[8]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/frag.glsl#L150-L164
[9]: https://www.iquilezles.org/www/articles/orbittraps3d/orbittraps3d.htm
[10]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/frag.glsl#L85-L100

### Interpolating Position
### Interpolating Rotation
### Keyframe Interpolation At Constant Speed