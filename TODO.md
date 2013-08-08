TODO
=========

0. Texture based PRT
 * Needs to be implemented. Should use one tex for each coefft - rgb intensities at each texel.

1. Model materials
 * Add a way to load multiple models, give them different materials & bake them together.

2. Shader pointers
 * Each renderable has a shader of type Shader*.
 * This does not call the correct virtual functions for the particular
    derived renderable's shader type!
 * Fix this somehow?

3. SH - lighting and rotations
 * More recent tests have left me worried over whether both SH and 
    SH rotation are really working as expected. Now seems OK!
 * Add a collection of useful basic SH lights funcs - useful for both
    testing and future progress. in progress...

4. Faster Baking
 * Octree has been written, but needs testing and integration into prebaking.
