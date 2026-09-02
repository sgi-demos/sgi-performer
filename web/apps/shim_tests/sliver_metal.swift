// sliver_metal.swift - pure-Metal minimal repro: the Metal rasterizer
// silently drops certain triangles that cross the camera plane (mixed
// clip-space w signs). No OpenGL anywhere in this program.
//
// Draws two adjacent ground-plane triangles from a vis-sim database, camera
// 2 units above the ground inside both triangles' y-span (a vehicle driving
// over them):
//   control: (2592.5,2404.5) (2577.5,2380.5) (2592.5,apex)  - rasterizes
//   sliver:  (2577.5,2380.5) (2592.5,apex)   (2577.5,2404.5) - VANISHES
// Both cross the camera plane; only the second is dropped. Offscreen render,
// no window; probes barycentric interior points of each triangle.
//
// Measured on Apple M3, macOS 14.8.3 (23J220):
//   baseline: 15u-wide 92u sliver, eye inside .......... FAIL (dropped)
//   small far plane (1000) ............................. FAIL
//   shorter sliver (64u) ............................... FAIL
//   wide triangle (62u wide - not a sliver) ............ FAIL
//   no camera-plane crossing (all verts ahead) ......... PASS
//   higher eye (z 8, still crossing) ................... FAIL
// The same geometry renders correctly on conformant rasterizers, and renders
// on this GPU once subdivided (see sliver_check.c / pfb2osg PFOSG_SUBDIV).
// GL front ends inherit the bug: Apple GL 2.1->Metal, ANGLE->GL->Metal, and
// ANGLE->Metal-native all drop it (sliver_check.c).
//
// Build & run:  swiftc sliver_metal.swift -o sliver_metal && ./sliver_metal

import Metal
import simd

// camera: eye (2589, 2419.6, 2) looking +y, up +z (z-up world)
func lookNorth(_ ex: Float, _ ey: Float, _ ez: Float) -> float4x4 {
    // rows: s=(1,0,0), u=(0,0,1), -f=(0,-1,0)
    return float4x4(columns: (
        SIMD4<Float>(1, 0, 0, 0),
        SIMD4<Float>(0, 0, -1, 0),
        SIMD4<Float>(0, 1, 0, 0),
        SIMD4<Float>(-ex, -ez, ey, 1)))
}

// Metal-convention perspective: z maps to [0,1]
func perspectiveMetal(fovyDeg: Float, aspect: Float, zn: Float, zf: Float)
    -> float4x4
{
    let f = 1.0 / tan(fovyDeg * .pi / 360.0)
    return float4x4(columns: (
        SIMD4<Float>(f / aspect, 0, 0, 0),
        SIMD4<Float>(0, f, 0, 0),
        SIMD4<Float>(0, 0, zf / (zn - zf), -1),
        SIMD4<Float>(0, 0, zf * zn / (zn - zf), 0)))
}

let W = 735, H = 415

struct Config {
    let name: String
    let eye: SIMD3<Float>
    let lx: Float      // x of the two left-edge vertices (right edge 2592.5)
    let apexY: Float
    let far: Float
}
let configs = [
    Config(name: "baseline: 15u-wide 92u sliver, eye inside",
           eye: [2589.0, 2419.6, 2.0], lx: 2577.5, apexY: 2473.0,
           far: 32357.7),
    Config(name: "small far plane (1000)",
           eye: [2589.0, 2419.6, 2.0], lx: 2577.5, apexY: 2473.0,
           far: 1000.0),
    Config(name: "shorter sliver (apex y 2445, 64u)",
           eye: [2589.0, 2419.6, 2.0], lx: 2577.5, apexY: 2445.0,
           far: 32357.7),
    Config(name: "wide triangle (left x 2530, 62u wide)",
           eye: [2589.0, 2419.6, 2.0], lx: 2530.0, apexY: 2473.0,
           far: 32357.7),
    Config(name: "no eye-plane crossing (eye y 2350, all verts ahead)",
           eye: [2589.0, 2350.0, 2.0], lx: 2577.5, apexY: 2473.0,
           far: 32357.7),
    Config(name: "higher eye (z 8, still crossing)",
           eye: [2589.0, 2419.6, 8.0], lx: 2577.5, apexY: 2473.0,
           far: 32357.7),
]

// MTLCreateSystemDefaultDevice needs a WindowServer connection; the device
// list works headless too
let dev = MTLCopyAllDevices().first ?? MTLCreateSystemDefaultDevice()!
print("device: \(dev.name)")
let queue = dev.makeCommandQueue()!

let src = """
    #include <metal_stdlib>
    using namespace metal;
    struct VOut { float4 pos [[position]]; };
    vertex VOut vmain(const device packed_float3* verts [[buffer(0)]],
                      constant float4x4& mvp [[buffer(1)]],
                      uint vid [[vertex_id]]) {
        VOut o;
        o.pos = mvp * float4(float3(verts[vid]), 1.0);
        return o;
    }
    fragment float4 fmain() { return float4(1.0, 0.0, 0.0, 1.0); }
    """
let lib = try! dev.makeLibrary(source: src, options: nil)
let pd = MTLRenderPipelineDescriptor()
pd.vertexFunction = lib.makeFunction(name: "vmain")
pd.fragmentFunction = lib.makeFunction(name: "fmain")
pd.colorAttachments[0].pixelFormat = .rgba8Unorm
let pso = try! dev.makeRenderPipelineState(descriptor: pd)

let td = MTLTextureDescriptor.texture2DDescriptor(
    pixelFormat: .rgba8Unorm, width: W, height: H, mipmapped: false)
td.usage = [.renderTarget]
td.storageMode = .shared

typealias V2 = SIMD2<Float>

var anyFail = false
for cfg in configs {
    let rNear = V2(2592.5, 2404.5)    // right edge
    let lFar = V2(cfg.lx, 2380.5)     // left edge, southernmost
    let apex = V2(2592.5, cfg.apexY)  // right edge, north end
    let lNear = V2(cfg.lx, 2404.5)    // left edge
    let tri1 = [rNear, lFar, apex]   // control
    let tri2 = [lFar, apex, lNear]   // the dropped sliver

    var verts: [Float] = []
    for v in tri1 + tri2 { verts += [v.x, v.y, 0] }

    var mvp = perspectiveMetal(fovyDeg: 26.51, aspect: Float(W) / Float(H),
                               zn: 1.0, zf: cfg.far)
        * lookNorth(cfg.eye.x, cfg.eye.y, cfg.eye.z)

    let tex = dev.makeTexture(descriptor: td)!
    let rp = MTLRenderPassDescriptor()
    rp.colorAttachments[0].texture = tex
    rp.colorAttachments[0].loadAction = .clear
    rp.colorAttachments[0].storeAction = .store
    rp.colorAttachments[0].clearColor = MTLClearColor(red: 0, green: 0,
                                                      blue: 0, alpha: 1)
    let cb = queue.makeCommandBuffer()!
    let enc = cb.makeRenderCommandEncoder(descriptor: rp)!
    enc.setRenderPipelineState(pso)
    let vbuf = dev.makeBuffer(bytes: verts,
                              length: verts.count * 4, options: [])!
    enc.setVertexBuffer(vbuf, offset: 0, index: 0)
    enc.setVertexBytes(&mvp, length: MemoryLayout<float4x4>.size, index: 1)
    enc.drawPrimitives(type: .triangle, vertexStart: 0, vertexCount: 6)
    enc.endEncoding()
    cb.commit()
    cb.waitUntilCompleted()

    var pixels = [UInt8](repeating: 0, count: W * H * 4)
    tex.getBytes(&pixels, bytesPerRow: W * 4,
                 from: MTLRegionMake2D(0, 0, W, H), mipmapLevel: 0)

    // sample a barycentric interior point of a triangle; nil if it projects
    // behind the eye or off-screen
    func probe(_ tri: [V2], _ wApex: Float, apexIdx: Int) -> Bool? {
        var p = V2(0, 0)
        for (i, v) in tri.enumerated() {
            p += v * (i == apexIdx ? wApex : (1 - wApex) / 2)
        }
        let c = mvp * SIMD4<Float>(p.x, p.y, 0, 1)
        guard c.w > 0 else { return nil }
        let px = Int((c.x / c.w * 0.5 + 0.5) * Float(W))
        let py = Int((1.0 - (c.y / c.w * 0.5 + 0.5)) * Float(H))
        guard px >= 0, px < W, py >= 0, py < H else { return nil }
        return pixels[(py * W + px) * 4] > 200
    }

    var sliverDrawn: Bool? = nil, ctrlDrawn: Bool? = nil
    for wApex: Float in [0.55, 0.7, 0.85] {
        if let hit = probe(tri2, wApex, apexIdx: 1) {
            sliverDrawn = (sliverDrawn ?? true) && hit
        }
        if let hit = probe(tri1, wApex, apexIdx: 2) {
            ctrlDrawn = (ctrlDrawn ?? true) && hit
        }
    }
    let verdict: String
    switch (ctrlDrawn, sliverDrawn) {
    case (nil, _), (_, nil): verdict = "TEST-BROKEN (no valid probes)"
    case (false, _): verdict = "TEST-BROKEN (control missing)"
    case (true, true): verdict = "PASS"
    case (true, false): verdict = "FAIL (sliver dropped)"
    default: verdict = "?"
    }
    print("\(cfg.name): \(verdict)")
    if verdict.hasPrefix("FAIL") { anyFail = true }
}
exit(anyFail ? 1 : 0)
