TODO
=========

1. Model materials
 * Solids will have per-vertex materials. done!
 * A uniform will contain an array of material properties, and
    each vertex will have an associated material index. The material
    values will be pulled out in the vertex shader, and smoothly passed
    to the frag shader. done! (For Solid.glsl only)
 * Appropriate aspects of these materials will be used in PRT preprocesses. done? (test much more).

2. Shader pointers
 * Each renderable has a shader of type Shader*.
 * This does not do call the correct virtual functions for the particular
    derived renderable's shader type!
 * Fix this somehow?

3. SH - lighting and rotations
 * More recent tests have left me worried over whether both SH and 
    SH rotation are really working as expected. Now seems OK!
 * Add a collection of useful basic SH lights funcs - useful for both
    testing and future progress.
 * Add a method of visualising SH projections of functions, so it is possible
    to check out how they actually look, check for ringing etc.

4. Faster Baking
 * Octree has been written, but needs testing and integration into prebaking.

5. Better CMake
 * Cmake kind of works, but needs tweaking and a full description of how to use it.
 * This may be best left for a bit later.
