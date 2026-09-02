# Draft: Apple Feedback Assistant report

**Title:** Metal rasterizer silently drops triangles that cross the camera
plane (w-sign-mixed triangles); adjacent triangle in the same draw call
renders correctly

**Area:** Metal / GPU drivers
**Hardware:** Apple M3 (MacBook Pro), macOS 14.8.3 (23J220)
**Reproducibility:** Always (deterministic, offscreen, no window needed)

## Summary

Certain valid, front-facing, non-degenerate triangles whose vertices have
mixed clip-space w signs (the triangle spans the camera plane) are silently
discarded by the Metal rasterizer — no fragments are produced at all, as
confirmed by both color and depth readback. An adjacent triangle in the same
draw call, also spanning the camera plane, rasterizes correctly, so this is
not a wholesale limitation on w-crossing geometry but an inconsistency in
handling it.

The attached single-file repro (`sliver_metal.swift`, ~180 lines, offscreen
render + pixel readback, prints PASS/FAIL) draws two adjacent ground-plane
triangles as seen from a camera 2 units above the ground located inside the
triangles' footprint — the standard driving-simulation case of a vehicle
crossing a large ground polygon:

```
control: (2592.5, 2404.5) (2577.5, 2380.5) (2592.5, 2473)   -> rasterizes
sliver:  (2577.5, 2380.5) (2592.5, 2473)  (2577.5, 2404.5)  -> VANISHES
```

Camera: eye (2589, 2419.6, 2) looking +y, up +z; fovy 26.51°, near 1,
far 32357.7. Pipeline is a trivial pass-through vertex shader (one float4x4
multiply) and constant-color fragment shader; no culling, no depth test,
default rasterizer state.

## Variations measured (all in the repro)

- Baseline (15-unit-wide, 92-unit-long sliver): **dropped**
- Far plane reduced to 1000: **dropped** (not a far-plane precision issue)
- Sliver shortened to 64 units: **dropped**
- Triangle widened to 62 units (not a sliver): **dropped**
- Camera moved so all vertices are in front (no w-sign mix): **renders**
  (crossing the camera plane is necessary to trigger)
- Camera raised to z=8, still crossing: **dropped**
- Subdividing the triangle into pieces with edges <= 25 units: **renders**

## Steps to reproduce

1. `swiftc sliver_metal.swift -o sliver_metal`
2. `./sliver_metal`

## Expected results

All six configurations print PASS (both triangles produce fragments at the
probed interior pixels).

## Actual results

Five configurations print `FAIL (sliver dropped)`: the second triangle
produces no fragments anywhere on screen while its neighbor in the same
draw call renders.

## Impact

Real-world content hits this constantly in driving/flight-simulation
scenes: any large ground polygon the camera passes over can flash invisible,
showing the clear color through the ground. We hit it running the classic
SGI OpenGL Performer "Town" database. It reproduces identically through
every GL front end on macOS — Apple OpenGL 2.1, ANGLE's OpenGL backend, and
ANGLE's native Metal backend (i.e. WebGL content in Chrome on Apple silicon
inherits it) — which is how it was isolated to Metal/the GPU rather than any
GL implementation. Our workaround is to subdivide long triangles at load
time; correct rasterization would make that unnecessary.
