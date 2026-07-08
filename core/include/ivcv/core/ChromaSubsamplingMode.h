#pragma once

namespace ivcv::core {

/// Which chroma subsampling scheme is currently applied to the Cb/Cr
/// planes relative to the full-resolution Y' plane. Defined in its own
/// header (rather than inside ChromaSubsampler.h) so PipelineContext can
/// record the active mode without depending on the stage class itself --
/// PipelineContext only ever depends on small, stage-independent value
/// types (see docs/ARCHITECTURE.md, section 6). See
/// modules/chroma_subsampling/README.md for the full explanation of each
/// mode.
enum class ChromaSubsamplingMode {
  Yuv444, ///< No subsampling: Cb/Cr sampled at full resolution.
  Yuv422, ///< Horizontal-only: Cb/Cr width halved, height unchanged.
  Yuv420, ///< Horizontal and vertical: Cb/Cr width and height both halved.
};

} // namespace ivcv::core
