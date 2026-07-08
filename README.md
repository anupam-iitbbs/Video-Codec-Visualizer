# Interactive Video Codec Visualizer

**See how every pixel becomes a compressed bitstream.**

An educational desktop application that visually decomposes every stage of the video/image compression pipeline, from raw RGB pixels to a decoded, reconstructed frame. Instead of reading equations in a textbook, users upload an image, step through each transform, tweak parameters, and immediately see the visual and numeric consequences.

## Motivation

Most learning resources on video compression are either too theoretical (papers, textbooks), too math-heavy without intuition, or buried inside large production codec codebases (x264, libaom) that are hard to read as a learner. This project bridges that gap: every pipeline stage is both a visual, interactive lesson and a clean, independently testable algorithm implementation. It is built for students, codec/DSP engineers, researchers, professors, interview candidates, and anyone curious how Netflix or YouTube compress video.

## Key Features (Planned Scope)

- RGB to YUV color space conversion with per-pixel inspection
- 4:4:4 / 4:2:2 / 4:2:0 chroma subsampling comparison
- Recursive block partitioning (64x64 down to 8x8) with per-block stats
- Forward/inverse DCT with coefficient heatmaps and basis function inspection
- Quantization with a live QP slider and PSNR/bitrate feedback
- Zig-zag scan animation
- Run-length encoding breakdown
- Huffman entropy coding with tree visualization and generated bitstream
- Motion estimation and motion compensation between frame pairs (v2)
- Intra and inter prediction (v2)
- Deblocking filter before/after comparison (v2)
- PSNR / SSIM quality metrics and difference heatmaps
- Bit-level bitstream visualization
- "Step through pipeline" mode and full-run mode

See [docs/ROADMAP.md](docs/ROADMAP.md) for what is implemented today versus planned.

## Screenshots

> Placeholder — screenshots and GIFs will be added here as each module is implemented.

## Architecture Overview

The project follows a strict layered architecture so that every algorithm can be tested and reused independently of the Qt UI:

```
Presentation (Qt) -> Application/Orchestration -> Core Algorithms (pure C++20) -> Data Model -> Platform/IO
```

Each pipeline stage implements a common `IPipelineStage` interface (Strategy pattern), registered with a `PipelineController` (Command + Observer patterns) that can run the full pipeline or step through it one stage at a time. Full details, class diagrams, and data-flow diagrams are in [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md), and the complete requirements are in [docs/SRS.md](docs/SRS.md).

## Folder Structure

```
Video-Codec-Visualizer/
├── app/ Qt application entry point and MainWindow
├── core/ Pure C++20 algorithm library (no Qt dependency)
├── modules/ Per-stage educational design notes (intuitive explanation, math derivation, pseudocode, complexity, references, worked examples)
├── ui/ Qt widgets, module views, and custom painters
├── resources/ Icons, sample images, stylesheets
├── examples/ Sample images/videos and example core/ usage
├── docs/ SRS, architecture, roadmap
├── tests/ GoogleTest suites mirroring core/
├── benchmarks/ Google Benchmark targets for performance-sensitive code
├── scripts/ Build and setup helper scripts
└── third_party/ Vendored/FetchContent-pinned dependencies
```

## Technology Stack

| Layer | Choice | Rationale |
|---|---|---|
| Language | C++20 | Concepts/ranges for clean numeric/template code |
| Build | CMake | Cross-platform, industry standard |
| GUI | Qt 6 | Graphics View Framework fits block/matrix/MV visualization well |
| Linear algebra | Eigen | Readable, safe block/transform math |
| Image/Video I/O | OpenCV | File decode/encode and a correctness oracle in tests |
| Logging | spdlog + fmt | Leveled, structured logging |
| Testing | GoogleTest | Unit tests per pipeline stage |
| Benchmarking | Google Benchmark | Tracks performance regressions |

## Build Instructions

The build system is being scaffolded incrementally. This section will be kept accurate as each stage lands.

Prerequisites: CMake >= 3.24, a C++20 compiler, Qt 6, OpenCV, and Eigen3 installed and discoverable by CMake.

```bash
git clone https://github.com/anupam-iitbbs/Video-Codec-Visualizer.git
cd Video-Codec-Visualizer
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Running

```bash
./build/app/ivcv_app
```

## Running Tests

```bash
ctest --test-dir build --output-on-failure
```

## Dependencies

Qt 6, OpenCV 4.x, Eigen 3, fmt, spdlog, GoogleTest, Google Benchmark. See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) for how each is used.

## Project Roadmap

See [docs/ROADMAP.md](docs/ROADMAP.md) for the full staged plan. Current status: Stages 1, 2, 3, and 4 done; Stage 5 (DCT / IDCT) is next.

## Implemented Modules

- **RGB to YUV** — `ColorSpaceConverter` (BT.601/BT.709, forward and inverse), `ImageLoader`, and the `ui::RgbYuvView` module view. Educational note: [modules/color_space/README.md](modules/color_space/README.md).
- **Chroma Subsampling** — `ChromaSubsampler` (4:4:4/4:2:2/4:2:0, Nearest/Box filters), extended `PipelineContext` plane accessors, and the `ui::ChromaSubsamplingView` module view. Educational note: [modules/chroma_subsampling/README.md](modules/chroma_subsampling/README.md).
- **Block Partitioning** — `BlockPartitioner` (recursive variance-driven quadtree, 64 down to 8), the `Block` leaf-node type, and the `ui::BlockPartitioningView` module view with a clickable grid overlay and variance heatmap. Educational note: [modules/block_partitioning/README.md](modules/block_partitioning/README.md).

See [docs/ROADMAP.md](docs/ROADMAP.md) for the complete implemented/upcoming module list.

## Documentation

- [docs/SRS.md](docs/SRS.md) — Software Requirements Specification
- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — System architecture, class diagrams, data flow
- [docs/ROADMAP.md](docs/ROADMAP.md) — Staged development roadmap and status
- `modules/<name>/README.md` — Per-module educational notes: intuitive explanation, mathematical derivation, pseudocode, computational complexity, standards references, and a worked example, added as each module is implemented.

## Contributing

Contributions are welcome. Please open an issue to discuss significant changes before submitting a pull request. Follow the coding standards below and ensure new algorithmic code includes unit tests.

## Coding Standards

- C++20, const-correctness, RAII, smart pointers over raw pointers
- PascalCase for classes, camelCase for functions/variables, UPPER_SNAKE_CASE for constants
- No global state, no macros where avoidable, composition over inheritance
- Every public class/function documented (purpose, inputs, outputs, complexity)
- Every new algorithm ships with GoogleTest coverage

## License

This project is licensed under the MIT License.

## Contact

Repository maintained by anupam-iitbbs. Please use GitHub Issues for questions, bug reports, and feature requests.

## Acknowledgements

Inspired by decades of published video coding research and the open-source codec community (x264, libaom, libvpx, and others) whose documented algorithms make a project like this possible to build transparently and correctly.

## References

- ITU-T and ISO/IEC video coding standards (H.264/AVC, H.265/HEVC)
- "Digital Video and HD: Algorithms and Interfaces" — Charles Poynton
- JPEG standard (ISO/IEC 10918-1)
- ITU-R BT.601 and ITU-R BT.709 (color space conversion coefficients; see [modules/color_space/README.md](modules/color_space/README.md))
