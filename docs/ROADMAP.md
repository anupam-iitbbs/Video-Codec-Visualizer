# Development Roadmap

## Interactive Video Codec Visualizer

Status legend: Done, In Progress, Planned.

Each stage must compile cleanly, pass its unit tests, and be independently demoable before the next stage begins.

| Stage | Title | Scope | Status |
|---|---|---|---|
| 0 | Software Requirements Specification | SRS, architecture, folder structure, roadmap (this document set) | Done |
| 1 | Project Scaffolding | CMake skeleton, CI, folder structure, empty Qt shell that launches with the docked layout, no real modules yet | Done |
| 2 | RGB to YUV Module | First real vertical slice: image ingestion + FR-2 | Done |
| 3 | Chroma Subsampling Module | FR-3 | Done |
| 4 | Block Partitioning Module | FR-4 | Done |
| 5 | DCT / IDCT Module | FR-5 | Planned |
| 6 | Quantization Module | FR-6, live PSNR feedback | Planned |
| 7 | Zig-zag Scan Module | FR-7 | Planned |
| 8 | Run-Length Encoding Module | FR-8 | Planned |
| 9 | Entropy Coding Module | FR-9, completes the still-image pipeline end-to-end with FR-15 bitstream view | Planned |
| 10 | Quality Metrics and Reporting | FR-14 generalized across all stages, exportable reports | Planned |
| 11 | Video Data Model Foundation | Multi-frame FrameSequence, prerequisite for all v2 stages | Planned |
| 12 | Motion Estimation Module | FR-10 | Planned |
| 13 | Motion Compensation / Inter Prediction Module | FR-11 | Planned |
| 14 | Intra Prediction Module | FR-12 | Planned |
| 15 | Deblocking Filter Module | FR-13 | Planned |

## Implemented Modules

- **Stage 1 - Project Scaffolding**: CMake build graph (ivcv_core, ivcv_app, ivcv_ui, test target), CI workflow, docked Qt shell with a placeholder canvas and the pipeline stage list.
- **Stage 2 - RGB to YUV** (modules/color_space/README.md): ImageBuffer<T> data model, ColorSpaceConverter (BT.601/BT.709, forward and inverse), ImageLoader (OpenCV-backed file loading), PipelineContext extended with rgbImage()/yuvImage(), and ui::RgbYuvView (load image, pick standard, view input/luma/chroma side by side). Unit tests: ImageBufferTest, ColorSpaceConverterTest.
- **Stage 3 - Chroma Subsampling** (modules/chroma_subsampling/README.md): ChromaSubsamplingMode/ChromaFilterMethod types, ChromaSubsampler (4:4:4/4:2:2/4:2:0, Nearest/Box filters, extractPlane/downsample/upsample), PipelineContext extended with yPlane()/cbPlane()/crPlane() and chromaSubsamplingMode(), and ui::ChromaSubsamplingView (load image, pick mode/filter, view full-res vs. subsampled chroma plus a difference heatmap and storage-savings stats). Unit tests: ChromaSubsamplerTest.
- **Stage 4 - Block Partitioning** (modules/block_partitioning/README.md): Block leaf-node type, BlockPartitioner (recursive variance-driven quadtree, maxBlockSize/minBlockSize/varianceThreshold, computeVariance/partition), PipelineContext extended with blocks(), and ui::BlockPartitioningView (load image, tune block-size/threshold parameters, clickable grid overlay reporting block id/coordinates/variance, plus a variance heatmap). Unit tests: BlockPartitionerTest.

## Upcoming Modules

See the Stage table above. Stage 5 (DCT / IDCT) is next.

## Versioning

v1.0.0 will be tagged when Stages 1 through 10 (the complete still-image pipeline) are done. v2.0.0 will be tagged when Stages 11 through 15 (the video pipeline) are done.
