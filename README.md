# Advanced Graphics Programming - Delivery 3 - OpenGL Features: PBR + IBL & SSAO

![Final Render](https://github.com/user-attachments/assets/1cad0e16-50e9-413e-a182-2e39492bbfe6)

## Team Members

_**Francesc Teruel Rodríguez**_
* **GitHub:** [francesctr4](https://github.com/francesctr4)

_**Mario García Sutil**_
* **GitHub:** [mariogs5](https://github.com/mariogs5)

## Application Controls

### Camera Controls

| **Action**               | **Input**                          |
|--------------------------|------------------------------------|
| **Orbit Camera**         | `Alt + Right Mouse Button + Drag`  |
| **Free Look**            | `Right Mouse Button + Drag`        |
| **Move Forward**         | `W`                                |
| **Move Backward**        | `S`                                |
| **Move Left**            | `A`                                |
| **Move Right**           | `D`                                |
| **Move Up**              | `E`                                |
| **Move Down**            | `Q`                                |
| **Speed Boost**          | `Hold Shift` (while moving)        |
| **Zoom In/Out**          | `Right Mouse Button + Scroll`      |

### Debug Keys

| Action                            | Key | Description                              |
|-----------------------------------|-----|------------------------------------------|
| **GBuffer - Final Render**        | `1` | Shows final rendered output              |
| **GBuffer - Albedo**              | `2` | Displays albedo colors                   |
| **GBuffer - Normals**             | `3` | Visualizes surface normals               |
| **GBuffer - Position**            | `4` | Shows world-space positions              |
| **GBuffer - View Direction**      | `5` | Displays camera-relative view vectors    |
| **GBuffer - Depth**               | `6` | Visualizes depth buffer                  |
| **GBuffer - Ambient Occlusion**   | `7` | Shows Screen Space AO (SSAO) map         |
| **GBuffer - Metallic**            | `8` | Displays metallic values                 |
| **GBuffer - Roughness**           | `9` | Visualizes roughness values              |
| **Cycle Cubemaps**                | `M` | Rotates through available cubemaps       |
| **Toggle Editor UI**              | `N` | Shows/hides editor interface             |
| **Cycle Rendering Modes**         | `B` | Switches between rendering techniques    |
| **PBR - Cycle Materials**         | `V` | Switches between PBR materials           |
| **PBR - Cycle Meshes**            | `C` | Switches between PBR meshes              |

# Physically Based Rendering (PBR) + Image Based Lighting (IBL)

Physically-Based Rendering is a collection of render techniques that are based on the underlying lighting theory of the physical world. Image-Based Lighting is a collection of techniques to light objects, not by direct analytical lights, but by treating the surrounding environment as one big light source.

## Showcase

![image](https://github.com/user-attachments/assets/6caa63ec-168d-4519-92fd-b7c48bc00c52)
![image](https://github.com/user-attachments/assets/1c0b19f5-9410-4f23-b7a2-1c03d49280bf)
![image](https://github.com/user-attachments/assets/8d0ee74f-75ab-4fd2-8b96-85d7c4a96e60)

## Debug Widgets

| Setting Name          | Purpose                                                                                        |
|-----------------------|------------------------------------------------------------------------------------------------|
| Image Based Lighting  | Toggle IBL calculations on/off.                                                                |
| Metallic Influence    | Slider to alter the Metallic coeficient of the material.                                       |
| Roughness Influence   | Slider to alter the Roughness coeficient of the material.                                      |

![PBR - Debug Widgets](https://github.com/user-attachments/assets/33398e77-9561-40eb-aadd-d51f68789b73)

### GBuffer Debug View to see all the attachments from GBuffer (including new ones from PBR)

![image](https://github.com/user-attachments/assets/6fa3e975-da67-4979-b26f-bfbb00af0be7)

### GBuffer Debug View window also has IBL attachments available to display

![image](https://github.com/user-attachments/assets/9034302a-32c9-4640-9e47-6e285d0089ef)

### Remember to check the other [Debug Keys](#debug-keys) for additional interactions!

## Shader Files

### PBR

| Shader Name          | Purpose                                                                                       |
|---------------------|------------------------------------------------------------------------------------------------|
| FORWARD_PBR_DIRECT_TEXTURED.glsl              | Computes direct lighting using the PBR model in a forward rendering textured pass.     |
| FORWARD_PBR_IBL_TEXTURED.glsl              | Performs forward PBR rendering with both direct lighting influence and IBL. Uses precomputed environment maps for ambient lighting. Supports full texture-based material inputs.     |
| DEFERRED_PBR_IBL_TEXTURED_GEOMETRY.glsl              | 	G-Buffer pass in deferred rendering. Writes PBR-relevant surface data (position, normal, albedo, metallic, roughness, ...) to textures for use in a later lighting pass.     |
| DEFERRED_PBR_IBL_TEXTURED_QUAD.glsl              | Lighting/composition pass in deferred rendering. Reads the G-Buffer and applies lighting including IBL precomputed environment maps for ambient lighting.    |

### IBL

| Shader Name        | Purpose                                                                                       |
|---------------------|------------------------------------------------------------------------------------------------|
| EQUIRECTANGULAR_TO_CUBEMAP.glsl              | Converts an HDR equirectangular image (panoramic environment map) into a cubemap format for skyboxes and IBL.     |
| SKYBOX.glsl              | Renders the cubemap as a skybox used for visual background and environment reflection source.     |
| DIFFUSE_IRRADIANCE_CONVOLUTION.glsl              | Generates a low-frequency cubemap used for diffuse IBL, by convolving the environment map.     |
| SPECULAR_PREFILTER_CONVOLUTION.glsl              | Generates a mipmapped cubemap used for specular IBL. Each mip level corresponds to different surface roughness.     |
| BRDF_INTEGRATION_CONVOLUTION.glsl              | Precomputes a 2D lookup texture (BRDF LUT) used for the split-sum approximation in PBR specular IBL, based on roughness and view angle.     |

# Screen Space Ambient Occlusion (SSAO)

Screen space ambient occlusion (SSAO) is a computer graphics technique for efficiently approximating the ambient occlusion effect in real time.

## Showcase
<table>
  <tr>
    <th>Attachment</th>
    <th colspan="2">Preview</th>
  </tr>
  <tr>
    <td>Final Render</td>
    <td><img src="https://github.com/user-attachments/assets/bdbe06cb-5131-4839-a04a-57b906ca702e"></td>
    <td><img src="https://github.com/user-attachments/assets/79b9c89e-c76a-4cc1-ab89-6f1d025bf401"></td>
  </tr>
  <tr>
    <td>SSAO Pass</td>
    <td><img src="https://github.com/user-attachments/assets/dce0da62-888d-4f63-a305-ecef7d149789"></td>
    <td><img src="https://github.com/user-attachments/assets/41f520c5-e0b4-4ec7-a8a2-bc41cceec869"></td>
  </tr>
  <tr>
    <td>Final Render with SSAO</td>
    <td><img src="https://github.com/user-attachments/assets/83374746-ff33-4269-9a97-ed4826eefaf4"></td>
    <td><img src="https://github.com/user-attachments/assets/5b9d145f-1ffe-449c-85e4-a0b765fbfe87"></td>
  </tr>
</table>

## Debug Widgets

| Setting Name        | Purpose                                                                                       |
|---------------------|------------------------------------------------------------------------------------------------|
| SSAO Status            | Toggle Screen Space Ambient Occlusion on/off.    |
| AO Texture             | 	Toggle the PBR AO texture on/off.     |
| Kernel Size            |  Controls the number of sample points used to estimate occlusion; larger values can improve accuracy but reduce performance.    |
| Radius             |  Defines the sampling radius around each pixel in world space; determines how far to look for occluding geometry.    |
| Bias            |   Prevents self-occlusion artifacts (a.k.a. shadow acne) by ignoring very close geometry. Higher values reduce false occlusion but may miss subtle shadows.   |
| Power            |   Adjusts the final contrast or intensity of the ambient occlusion; higher values darken occluded areas more strongly.   |

![image](https://github.com/user-attachments/assets/9a560ab1-857c-463e-8077-b0dd9283d2a4)

## Shader Files

| Shader Name          | Purpose                                                                                       |
|---------------------|------------------------------------------------------------------------------------------------|
| SSAO.glsl              | Generate SSAO Texture     |
| SSAO_BLUR.glsl              | Blur SSAO Texture     |
| FORWARD_PRE_SSAO_RENDER.glsl              | (Blinn-Phong) Get Position and Normal Texture in Forward Rendering     |
| FORWARD_SSAO_RENDER.glsl              | (Blinn-Phong) Forward Rendering using SSAO     |
| DEFERRED_RENDER_GEOMETRY.glsl              | (Blinn-Phong) Deferred Rendering using SSAO     |
| DEFERRED_RENDER_QUAD.glsl              | (Blinn-Phong) Deferred Rendering using SSAO     |
| DEFERRED_PBR_IBL_TEXTURED_GEOMETRY.glsl              | 	(PBR) Deferred Rendering using SSAO     |
| DEFERRED_PBR_IBL_TEXTURED_QUAD.glsl              | (PBR) Deferred Rendering using SSAO     |

## Deferred Pipeline Breakdown

| Attachment          | Preview                                                                                        |
|---------------------|------------------------------------------------------------------------------------------------|
| Albedo              | ![Albedo](https://github.com/user-attachments/assets/194961c8-2c82-4968-b36e-1c9641222b7c)     |
| Normals             | ![Normals](https://github.com/user-attachments/assets/b0b7919f-7fc0-4ca7-9a70-ddfd69b431a6)    |
| Position            | ![Position](https://github.com/user-attachments/assets/c5acb356-2bff-4195-9cc4-c52535ef4db1)   |
| View Direction      | ![View](https://github.com/user-attachments/assets/b84f38f3-c8c3-4401-a588-1b43c7f7e3e1)       |
| Depth               | ![Depth](https://github.com/user-attachments/assets/b0d44f54-8662-4a73-99a9-bbd145e79a3c)      |
| Ambient Occlusion   | ![AO](https://github.com/user-attachments/assets/23251b91-9cf4-44f0-b623-f3da7e1a5706)         |
| Metallic (PBR)      | ![Metallic](https://github.com/user-attachments/assets/90f3458d-eb64-4a1d-8552-316bb2c56aee)   |
| Roughness (PBR)     | ![Roughness](https://github.com/user-attachments/assets/4818939f-6c81-4444-96e2-285076c272c0)  |
| Skybox (IBL)        | ![Skybox](https://github.com/user-attachments/assets/8fa65cdd-0fd6-4186-aaf6-99f5252dcfd1)     |
| Irradiance (IBL)    | ![Irradiance](https://github.com/user-attachments/assets/86c7249d-3c53-4966-adfa-a19d64d0c02d) |
| Prefilter (IBL)     | ![Prefilter](https://github.com/user-attachments/assets/c694396a-3564-4cbc-8c2d-67e4f461e4e7)  |
| BRDF (IBL)          | ![BRDF](https://github.com/user-attachments/assets/dba39988-b0e9-44cc-bfd6-0d743606a9c8)       |
| Final Render        | ![Final](https://github.com/user-attachments/assets/841d2f10-6b14-4624-a2ae-ce429c640057)      |

# Advanced Graphics Programming - Delivery 2 - Deferred Rendering

## Final Result
![Final Render](https://github.com/user-attachments/assets/94d4fc64-ef28-4269-8876-1c4b94087142)

## Deferred Pipeline Breakdown
| Attachment          | Preview                                                                                      |
|---------------------|----------------------------------------------------------------------------------------------|
| Albedo              | ![Albedo](https://github.com/user-attachments/assets/a8c44eea-4ade-47d9-b74d-ca8eef874b80)   |
| Normals             | ![Normals](https://github.com/user-attachments/assets/52ee5be7-e1a7-4223-80d5-f9fad7689d60)  |
| Position            | ![Position](https://github.com/user-attachments/assets/abd3e7e1-f21b-4e15-9afd-6075a0c698a3) |
| View Direction      | ![View](https://github.com/user-attachments/assets/6e58b599-043d-4e97-ad2c-c536a1c9971d)     |
| Depth               | ![Depth](https://github.com/user-attachments/assets/5dcc4a61-2143-4d46-a593-5b7b85083e94)    |
| Final Render        | ![Final](https://github.com/user-attachments/assets/80e0599b-7197-44eb-b378-f183d3bdb715)    |
| Stress Test         | ![Stress](https://github.com/user-attachments/assets/0d5dcd9d-ca0e-4716-b7c3-d109572d449b)   |
