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

Fractal Pioneer is a Qt based application for exploring 3D fractals and generating high-FPS high-resolution keyframes
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

## Keyframes To Video

The tool outputs a sequence of keyframes as PNG images in the desired resolution named in a sequential order. To create
a video we can use the the ffmpeg tool to stitch it all together. For example:

```
ffmpeg -framerate 60 -i %03d.png fractal.mp4
```

The YouTube video seen above was made using the preloaded waypoints included with the application. The output video was
then edited in Premiere Pro to produce the final version seen on YouTube.

## Technical Details

### Drawing The Fractal

The 3D fractal in the Fractal Pioneer application is drawn using a fragment shader ([`frag.glsl`][1]). The fragment which
the fragment shader processes is a simple square covering the canvas where the 3D fractal is drawn. This square is
drawn on an OpenGL Vertex Buffer Object in the [`resizeGL`][2] function. For each pixel on the fragment (our square) the
fragment shader casts a ray from the camera through the pixel to [calculate the colour][3] for this pixel.

We use the [ray marching method][4] and a [distance estimate formula][5] for our fractal to calculate the maximum 
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

### Camera Position And Rotation

The camera position is represented by a [3D vector][11] representing the X, Y, and Z coordinates. The camera rotation
is represented by [Euler angles][12] which are converted to quaternions for each axis from which we [calculate the
rotation matrix][14]. Both the position and the rotation matrix [are passed to the fragment shader][15] which then uses
these values to cast rays from the camera through each pixel and onto the 3D fractal.

When the user clicks on the canvas displaying the fractal we capture the mouse and keyboard and using the defined hotkeys
we apply linear translations to the position via the keyboard and angular translations to the rotation via the mouse. For
example holding the `W` key will translate the camera forward in the look direction by using the rotation matrix to
extract the look direction vector and apply a scaled vector addition to the current position using the look direction.
For rotations, moving the mouse will result in angular rotation of the Euler angles, which is a simple vector addition.
Camera position and rotation control is implemented in the [`updatePhysics`][16] function.


[11]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/FractalWidget.cpp#L338-L352
[12]: https://en.wikipedia.org/wiki/Euler_angles
[13]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/FractalWidget.cpp#L354-L395
[14]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/FractalWidget.cpp#L354-L395
[15]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/FractalWidget.cpp#L880-L881
[16]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/FractalWidget.cpp#L774-L832

### Interpolating Position

The application allows the user to record a sequence of waypoints via the [`addWaypoint`][17] function. To animate a
path through the waypoints we have to interpolate the 3D position of the camera. For this we chose to use a 
[Catmull-Rom spline][18] which has a very simple representation and has the nice property that it interpolates through 
the waypoints precisely. The linked whitepaper describes a simple matrix representation to interpolate the spline using
at least four waypoints. The user can of course record as many points as they wish.

The [`interpolatePosition`][19] function implements interpolation of the user defined waypoints as described in the
whitepaper by Christopher Twigg. Catmull-Rom splines require two extra waypoints, called the control points, which
dictate the tangents for the first and the last waypoint. We use the [camera look direction][20] to ensure a smooth
animation at the start and end of the spline.

[17]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/FractalWidget.cpp#L296-L307
[18]: http://graphics.cs.cmu.edu/nsp/course/15-462/Fall04/assts/catmullRom.pdf
[19]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/FractalWidget.cpp#L136-L191
[20]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/FractalWidget.cpp#L165-L181

### Interpolating Rotation

For each recorded waypoint we also record the rotation at that waypoint which we will use to interpolate the rotation
of the camera as the position is being interpolated. Unlike the position however, we cannot simply use a spline to
interpolate the rotation. Attempting to do so will result in "jerky" camera movement. The reason for this is because
3D rotations form a [non-ablian group][21] which can be projected onto a four-dimensional unit sphere of quaternions.
Attempting to apply a vector-space interpolation function such as linear interpolation on rotational waypoints will
result in camera movement which has non-constant angular momentum. If we project a linear interpolation onto a unit
sphere of quaternions, it will be a line between two points on a sphere. The line will cross through the the sphere
which is not what we want. This results in the camera rotating slower at the start and end, and fast during the middle
of the animation. The mathematics behind this is explained in detail in an excellent whitepaper [_Quaternions,
Interpolation and Animation_][22] by Erik B. Dam et. al.

In section 6.2.1. of the whitepaper an interpolation method called _Spherical Spline Quaternion interpolation_ (Squad)
of unit quaternions is explored which does a fairly good job at preserving angular momentum while interpolating
rotations. As explained in the whitepaper, the Squad algorithm interpolates unit quaternions through a series of
waypoints, similar to that of the spline interpolation described in the previous section. Equation 6.14 on pg. 51
defines the interpolation function which is remarkably simple:

<p align="center">
  <br>
  <img src="https://render.githubusercontent.com/render/math?math=%5CLarge%20Squad(q_i,q_{i%2B1},s_i,s_{i%2B1},h) = Slerp(Slerp(q_i, q_{i%2B1}, h), Slerp(s_i, s_{i%2B1}, h), 2h(1 - h))">
  <br>
  <br>
  <img src="https://render.githubusercontent.com/render/math?math=%5CLarge%20s_i = q_i exp(-\frac{log(q_i^{-1}q_{i%2B1})%2Blog(q_i^{-1}q_{i-1})}{4})">
  <br>
</p>

Qt does not implement the logarithm and exponential functions for quaternions, so we roll out our own [`log`][23] and
[`exp`][24] functions as defined by [Wikipedia][25]. Thankfully the `slerp` function is implemented by the Qt
`QQuaternion` class which makes our implementation simple. With that we have all the tools to implement rotational
interpolation using the Squad definitions above. This is implemented in [`interpolateRotation`][26] function by 
converting our recorded Euler angle rotational waypoints into unit quaternions and applying the Squad function.

[21]: https://en.wikipedia.org/wiki/3D_rotation_group
[22]: https://web.mit.edu/2.998/www/QuaternionReport1.pdf
[23]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/FractalWidget.cpp#L38-L65
[24]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/FractalWidget.cpp#L12-L36
[25]: https://en.wikipedia.org/wiki/Quaternion#Exponential,_logarithm,_and_power_functions
[26]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/FractalWidget.cpp#L193-L253

### Keyframe Interpolation At Constant Speed

For simplicity let's consider a user recording waypoints in 2D space, and we wish for our camera to interpolate through
the user recorded waypoints at a constant velocity. Using a target FPS and a target duration we want the interpolation to
last, we can calculate the velocity at which we want the camera to travel.

Consider the following spline with three user recorded waypoints: `p0`, `p1`, and `p2`:

<br/>
<p align="center">
  <img align="center" src="https://raw.githubusercontent.com/fjeremic/fractal-pioneer/assets/Spline.png"/>
</p>
<br/>

This spline, much like the user recorded waypoints, is defined by a discrete set of points. An obvious first thought to
animate the camera may be to interpolate the camera position between pairs of waypoints, `[p0, p1]`, `[p1, p2]`, etc.
Attempting to do this will work functionally, but will not yield a visually pleasing result because the camera velocity
will not be constant throughout the animation. The distance between points `p0` and `p1` is smaller than the distance
between points `p1` and `p2`. Approximately twice as small. This means that the camera will travel twice as fast
between the first pair of points, with an abrupt slowdown for the second pair of points.

The problem at hand is that given a sequence of waypoints which define a continuously differentiable spline, how do we
interpolate through the spine at a constant velocity?

We know our target FPS and we know the duration we want to take to interpolate the camera through the entirety of the
spline. If we knew the arc length of the spline, we could calculate how far along the spline we need to travel for each
timestep so that our camera velocity would be constant. According to Wikipedia, given a function `f(t)` the [arc length][27]
of `f` denoted as `L(f)` is defined as:

<p align="center">
  <br>
  <img src="https://render.githubusercontent.com/render/math?math=%5CLarge%20L(f) = \int_{a}^{b} \lvert f'(t) \rvert dt">
  <br>
</p>

If `f` represents our interpolation function defined in the previous sections, then to calculate the arc length we need
to be able to numerically differentiate our interpolation function, and then numerically calculate the integral of length
of the derivative between some range.

#### Calculating the derivative

Thankfully, due to the simplicity of the definition of Catmull-Rom splines as defined in Christopher Twigg's [whitepaper][18],
taking the derivative of `p(u)` with respect to `u` is as easy as differentiating each of the terms in the initial vector of
`[1 u u^2 u^3]` which yields `[0 1 2u 3u^2]`. This is implemented via a [`takeDerivative`][28] parameter in the 
`interpolatePosition` function.

#### Calculating the integral

Unfortunately, finding an analytical solution for the integral will not be possible in our case. We must resort to numerical
approximations of the integral, which for all intents and purposes will serve us just as well. The canonical numerical
integration method I learned through school was using [Gaussian quadrature][29]. The Wikipedia article defines the integral
equation for us:

<p align="center">
  <br>
  <img src="https://render.githubusercontent.com/render/math?math=%5CLarge%20\int _{a}^{b}f(x)\,dx\approx {\frac {b-a}{2}}\sum _{i=1}^{n}w_{i}f\left({\frac {b-a}{2}}\xi _{i}+{\frac {a+b}{2}}\right)">
  <br>
</p>

using `n`-point Gaussian quadrature. In our case we'll be using the 5-point version since Wikipedia also defines the points
and the weights for us as well. The integration function is implemented as a lambda within [`blend`][30].

#### Putting it all together

We are now able to differentiate our position function `f` and we know how to take an integral of it as well. We can put
all of this together to calculate the arc length of our spline using the equation above. However we'll go one step further.
Recall that our goal is to know how far along the spline we want to move the camera for a given time delta such that the
camera appears to move at a constant velocity.

Our position interpolation function `interpolatePosition` takes as input an interpolation parameter `u` which ranges
between `[0, n - 1]` where `n` is the number of waypoints. We know how to calculate the arc length, and so we know the
distance we want to travel along the spline, but what we don't know is what interpolation parameter will get us to that
distance.

An easy way to solve this problem is to precalculate a table, mapping distances along the spline to interpolation
parameters. We subdivide our spline into small segments by evaluating `f(u)` and `f(u + 0.01)`. The `0.01` constant was
chosen as a reasonable value to yield good results. We then apply Gaussian quadrature between this interval to find the
length of this spline segment, we'll call that `s`. We then store the current accumulated arc length corresponding to
the current value of `u` in a table. We continue until we reach the end of the spline.

The last value in the spline will correspond to a reasonably accurate arc length of the entire spline. The table just
described which we call `s2uTable` is calculated once per animation in the [`blend`][31] function. Moreover we now posses
a way to determine what value of `u` we need to pass to `f(u)` such that we travel a certain distance. This is implemented
in the [`s2u`][32] function as a simple binary search.

Finally when the user triggers an animation we use the target FPS to calculate a time delta, we use the target output
duration to calculate the arc length per millisecond that we want to travel, and we use our `s2u` function to find the
interpolation parameter which will make the camera go the desired distance. All of this is implemented in a few lines
within the [`updatePhysics`][33] function.

[27]: https://en.wikipedia.org/wiki/Arc_length
[28]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/FractalWidget.cpp#L153-L158
[29]: https://en.wikipedia.org/wiki/Gaussian_quadrature
[30]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/FractalWidget.cpp#L78-L99
[31]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/FractalWidget.cpp#L101-L111
[32]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/FractalWidget.cpp#L114-L134
[33]: https://github.com/fjeremic/fractal-pioneer/blob/acd2c19199ae9cd768d766295f6193c5cff2ea9b/FractalWidget.cpp#L834-L846