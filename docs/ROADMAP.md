# Development Roadmap

## Interactive Video Codec Visualizer

Status legend: Done, In Progress, Planned.

Each stage must compile cleanly, pass its unit tests, and be independently demoable before the next stage begins.

| Stage | Title | Scope | Status |
|---|---|---|---|
| 0 | Software Requirements Specification | SRS, architecture, folder structure, roadmap (this document set) | Done |
| 1 | Project Scaffolding | CMake skeleton, CI, folder structure, empty Qt shell that launches with the docked layout, no real modules yet | In Progress |
| 2 | RGB to YUV Module | First real vertical slice: image ingestion + FR-2 | Planned |
| 3 | Chroma Subsampling Module | FR-3 | Planned |
| 4 | Block Partitioning Module | FR-4 | Planned |
| 5 | DCT / IDCT Module | FR-5 | Planned |
| 6 | Quantization Module | FR-6, live PSNR feedback | Planned |
| 7 | Zig-zag Scan Module | FR-7 | Planned |
| 8 | Run-Length Encoding Module | FR-8 | Planned |
| 9 | Entropy Coding Module | FR-9, completes the still-image pipeline end-to-end with FR-15 bitstream view | Planned |
| 10 | Quality Metrics and Reporting | FR-14 generalized across all stages, exportable reports | Planned |
| 11 | Video Data Model Foundation | Multi-frame `FrameSequence`, prerequisite for all v2 stages | Planned |
| 12 | Motion Estimation Module | FR-10 | Planned |
| 13 | Motion Compensation / Inter Prediction Module | FR-11 | Planned |
| 14 | Intra Prediction Module | FR-12 | Planned |
| 15 | Deblocking Filter Module | FR-13 | Planned |

## Implemented Modules

None yet outside of Stage 1 scaffolding. This section is updated at the end of every stage with a short description, screenshot placeholder, and link to the module's design doc under `docs/modules/`.

## Upcoming Modules

See the Stage table above. Stage 2 (RGB to YUV) is next after Stage 1 scaffolding lands.

## Versioning

v1.0.0 will be tagged when Stages 1 through 10 (the complete still-image pipeline) are done. v2.0.0 will be tagged when Stages 11 through 15 (the video pipeline) are done.
