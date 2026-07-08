# Software Requirements Specification (SRS)

## Interactive Video Codec Visualizer

Status: Living document, updated at the start/end of each development stage.

## 1. Purpose and Scope

The Interactive Video Codec Visualizer (IVCV) is an educational desktop application that decomposes the still-image and video compression pipeline into independently explorable, visually interactive stages.

Version 1.0 scope covers the still-image (JPEG-like) pipeline end-to-end: RGB to YUV conversion, chroma subsampling, block partitioning, DCT, quantization, zig-zag scan, run-length encoding, entropy coding, and quality metrics.

Motion estimation, motion compensation, intra/inter prediction, and the deblocking filter are video-pipeline stages that depend on a multi-frame data model. They are scoped for v2, built on the same architecture defined in this document so no rewrite is required later.

## 2. Users and Primary Use Cases

- Students and interview candidates: need step-by-step, inspectable transformations rather than equations.
- Codec/DSP engineers and researchers: need accurate, parameterized reference implementations comparable against real codecs.
- Professors: need a tool that can drive a lecture, pausing at any stage.
- General audience: curious how real-world video compression (e.g. streaming services) works.

The common requirement across all personas: load real image data, watch a specific transform happen to it, tweak a parameter, and see the numeric and visual consequence immediately.

## 3. Functional Requirements

| ID | Module | Requirement |
|---|---|---|
| FR-1 | Image/Frame Ingestion | Load PNG/JPEG/BMP stills and (v2) video files; expose raw pixel buffers to the pipeline. |
| FR-2 | Color Space Conversion | Convert RGB<->YUV (BT.601/BT.709 selectable); per-pixel inspection; channel toggling. |
| FR-3 | Chroma Subsampling | Support 4:4:4 / 4:2:2 / 4:2:0; live comparison view; configurable filter (nearest, average, box). |
| FR-4 | Block Partitioning | Recursive quad partitioning (64 down to 8); clickable blocks showing ID, coordinates, variance. |
| FR-5 | Transform (DCT) | 8x8 (later 4x4/16x16) forward/inverse DCT; coefficient matrix and frequency heatmap. |
| FR-6 | Quantization | QP-driven quantization matrices (JPEG-style luma/chroma tables); live PSNR/bitrate feedback. |
| FR-7 | Zig-zag Scan | Deterministic reordering visualization with animated path overlay. |
| FR-8 | Run-Length Encoding | RLE of the zig-zag stream with symbol/run pair breakdown. |
| FR-9 | Entropy Coding | Huffman coding (canonical, JPEG-style) with tree visualization and generated bitstream. |
| FR-10 | Motion Estimation (v2) | Block search (full/diamond/three-step) between two frames; SAD/SSD cost surface. |
| FR-11 | Motion Compensation / Inter Prediction (v2) | MV field application, residual generation, reconstructed frame. |
| FR-12 | Intra Prediction (v2) | H.264/HEVC-style directional prediction modes on block boundaries. |
| FR-13 | Deblocking Filter (v2) | Configurable in-loop filter with before/after boundary comparison. |
| FR-14 | Quality Metrics | PSNR, SSIM, compression ratio, encode time, computed for any two buffers in the pipeline. |
| FR-15 | Bitstream Visualization | Byte/bit-level view of generated stream with symbol-to-bit mapping. |
| FR-16 | Pipeline Orchestration | "Step through" mode advancing one stage at a time; full-run mode; parameter panel per stage. |

## 4. Non-Functional Requirements

- NFR-1: The core algorithm library (\`core/\`) has zero UI dependencies and is independently unit-testable.
- NFR-2: The application runs on Windows, macOS, and Linux.
- NFR-3: A full 512x512 image processes through the still-image pipeline in under 500 ms on a typical laptop. This is an educational tool, not a production encoder, so there is no hard real-time requirement.
- NFR-4: All public APIs are documented with Doxygen.
- NFR-5: Test coverage target of at least 80% on \`core/\`.
- NFR-6: The UI supports dark mode and resizable/dockable panels.
- NFR-7: Every new algorithmic stage ships with unit tests before being considered complete.

## 5. Out of Scope (v1)

- Full standards-compliant encoder/decoder conformance (this is an educational visualizer, not a certified codec).
- Real-time video playback/streaming.
- Hardware-accelerated encoding (GPU/ASIC).
- Audio handling.

## 6. Technology Stack Decisions

| Layer | Choice | Rationale |
|---|---|---|
| Language | C++20 | Concepts/ranges clean up template-heavy numeric code. |
| Build | CMake | Cross-platform, industry standard. |
| GUI | Qt 6 (Widgets + Graphics View) | Confirmed as the sole v1 frontend; a WASM export of \`core/\` is a possible v3+ stretch goal, not a parallel React track, to avoid tripling maintenance surface for a small team. |
| Linear Algebra | Eigen | Readable, safe block/transform math versus raw arrays. |
| Image/Video I/O | OpenCV | File decode/encode and a correctness oracle in tests; not a replacement for our own from-scratch conversion/transform code, since visualizing the math ourselves is the point of the project. |
| Logging | spdlog + fmt | Leveled, structured logging without iostream boilerplate. |
| Testing | GoogleTest | Unit tests per pipeline stage. |
| Benchmarking | Google Benchmark | Regression tracking for perf-sensitive code (DCT, motion search). |

## 7. Confirmed Decisions (from Stage 0 review)

- License: MIT.
- Frontend: Qt-only for v1. No parallel React/web frontend in v1.
- v1 pipeline scope: still-image pipeline only (FR-1 through FR-9, FR-14, FR-15, FR-16). Video-dependent stages (FR-10 through FR-13) are deferred to v2.

## 8. Traceability

Each FR above maps 1:1 to a module folder under \`modules/\` and a stage in [ROADMAP.md](ROADMAP.md). Architecture details, class diagrams, and data flow are in [ARCHITECTURE.md](ARCHITECTURE.md).
