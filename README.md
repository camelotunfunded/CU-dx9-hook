\# DAoC D3D9 Hook – Renderer Modernization Toolkit



>  Direct3D9 injection for Dark Age of Camelot — modern rendering, effects, debug tools.



---



\##  Goal / Objectif



🧭 \*\*Note:\*\* Vegetation animation is only the first step in a broader effort to modernize DAoC’s fixed-function rendering pipeline.



🧭 \*\*Remarque :\*\* L’animation de la végétation n’est que la première étape d’un effort plus large visant à moderniser le pipeline de rendu fixe de DAoC.



This project aims to gradually enhance and override DAoC’s Direct3D9 rendering by injecting shaders, screen-space effects, drawcall classification, and debug tools — one stable hook at a time.



---



\##  Current Status / État actuel



\###  Implemented Hooks (by function)



| Function                        | Description (EN)                                                  | Description (FR)                                                  |

|---------------------------------|-------------------------------------------------------------------|-------------------------------------------------------------------|

| `DrawIndexedPrimitive`          | Intercepts drawcalls for vegetation sprites, applies wind shader | Intercepte les sprites végétation et applique un shader d’animation |

| `SetTexture` \*(read-only)\*      | Tracks last texture bound to stage 0                              | Suit la dernière texture bindée sur le slot 0                     |

| `Present`                       | Hooked for potential overlay / post-process injection             | Hook en place pour futurs effets plein-écran ou HUD               |

| `Reset`                         | Not yet managed (resources not reloaded)                          | Non géré pour l’instant                                           |



---



\###  Vegetation Animation Hook



\- Detection based on hashed texture (`g\_lastTextureHash`)  

\- A pixel shader animates the billboard geometry (wind effect)  

\- Texture stage state fix prevents black/grey output  

\- Device state is saved/restored via `IDirect3DStateBlock9`



See `texturehash.cpp` for more on hash-based detection (temporary approach).



---



\##  Project Structure



| Folder / File        | Purpose |

|----------------------|---------|

| `src/hook/`          | Hook setup (MinHook), function redirection |

| `src/d3d/`           | Proxy classes for IDirect3DDevice9, state tracking |

| `src/effects/`       | HLSL shaders, pass management |

| `src/overlay/`       | HUD / debugging overlay (planned) |

| `src/analysis/`      | Drawcall classification (terrain, mesh, UI...) |

| `texturehash.cpp`    | Maps texture hashes to known vegetation resources |

| `DeviceProxy.cpp`    | Main rendering hook (`DrawIndexedPrimitive`) |



---



\##  Roadmap (Increments)



\- \[x] Hook Present → confirm backbuffer access

\- \[x] Proxy IDirect3DDevice9 → custom dispatch

\- \[x] Hook DrawIndexedPrimitive for vegetation

\- \[x] Shader effect + animation time + matrix

\- \[x] Fix texture stage state (black sprites)

\- \[ ] Detect Reset and recreate offscreen targets

\- \[ ] Insert FXAA as first screen-space effect

\- \[ ] Classify drawcalls (terrain, buildings, UI, etc.)

\- \[ ] Add toggle/hotkey system for overlays

\- \[ ] Replace FVF fixed-pipeline via custom VS/PS



---



\##  Build Instructions



\- Native C++ (Visual Studio 2022)

\- SDK DirectX 9 (required: d3d9.h, d3dx9.h)

\- Dependencies: MinHook, D3DX9 (statically or via DLL)



```bash

git clone https://github.com/youruser/daoc-d3d9-hook

cd daoc-d3d9-hook

\# Open and build DAoCHook.sln (DLL, Release/x86)



