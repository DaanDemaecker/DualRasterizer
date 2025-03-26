# Dual rasterizer
This project is a combination hardware and software rasterizer, i made this for my Graphcis programming course at DAE where i study game development.  
In thie project we made a rasterizer using DirectX and a rasterizer made from scratch that runs on the CPU. The user can freely switch between these two rasterizers.

## Capabilities
### Shared
- Load and render meshes with diffuse texture.
- Movable camera.  
- Toggle rasterizer mode from hardware to software


### Hardware
- Load and display diffuse textures with support for opacity.
- Enable different sampler types:
    - Linear
    - Point
    - Anisotropic


### Software
- Toggle shading mode:
    - Combined (Specular, normal map and gloss included)
    - Observed area only
    - Diffuse only
    - Specular only
- Toggle normal map
- Toggle depth buffer visualization
- Toggle bounding boxes visualization


## Topics we learned
- ***Rasterization process***
  The entire process of getting a 3D model displayed on screen is way more complicated than any of us thought, but thanks to this course, we learned every step needed to get there.

- ***Clipping***
  As off-screen objects aren't useful to be rendered, they are just discarded. As a child playing games, I would have never thought of this.

- ***Culling mode***
  Just like clipping objects that are on screen, which side of each triangle is rendered never seemed important to me, it turns out I was very wrong.  
