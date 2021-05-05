// Original Copyright HackerPoet/MarbleMarcher (https://github.com/HackerPoet/MarbleMarcher).
//
// The following code is a derivative work of the code from the MarbleMarcher project,
// which is licensed GPLv2. This code therefore is also licensed under the terms
// of the GNU Public License, verison 2.
//
// For information on the license of this code when distributed with and used
// in conjunction with the other modules in the MarbleMarcher project, please see
// the root-level LICENSE file.

#version 120

uniform vec2 in_resolution;

void main()
{
    gl_Position = vec4(gl_Vertex.xy / in_resolution, 0.0, 1.0);
}
