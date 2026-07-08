# Chroma Subsampling (Stage 3)

Educational note for FR-3 in docs/SRS.md. Code lives in
core/include/ivcv/core/ChromaSubsampler.h and core/src/ChromaSubsampler.cpp;
this document explains the *why* and the *math* behind that code.

## 1. Intuitive Explanation

The human eye is built mostly out of rod cells (sensitive to brightness) and
a smaller number of cone cells (sensitive to color). We are far better at
noticing small changes in brightness than small changes in color. Stage 2
already separated an image into a luma plane (Y', brightness) and two
chroma planes (Cb, Cr, color). Chroma subsampling exploits the eye's weaker
color acuity by storing fewer chroma samples than luma samples: instead of
one Cb and one Cr value per pixel, we keep one Cb/Cr pair per small block of
pixels and let every pixel in that block share it. The picture keeps looking
"full resolution" to a viewer, but the file is smaller because there is
simply less chroma data to store.

Three sampling grids are common, named `J:a:b`:
- **4:4:4** -- no subsampling; every pixel keeps its own Cb/Cr.
- **4:2:2** -- Cb/Cr are halved horizontally only (2x1 blocks share one sample).
- **4:2:0** -- Cb/Cr are halved both horizontally and vertically (2x2 blocks share one sample).

## 2. Mathematical Derivation

Let a chroma plane have width `W` and height `H`. For a subsampling block of
size `bw x bh` (1x1, 2x1, or 2x2 above), the output plane has dimensions:

```
W' = ceil(W / bw)
H' = ceil(H / bh)
```

`ChromaSubsampler` supports two ways of combining a block's samples into
one output sample (`ChromaFilterMethod` in the code):

**Nearest** -- takes one representative pixel per block (this implementation
uses the top-left pixel of the block):

```
out[ox, oy] = in[ox * bw, oy * bh]
```

**Box (a.k.a. average)** -- the mean of every source pixel in the block,
the de facto default used by libjpeg and most encoders:

```
out[ox, oy] = ( 1 / (w * h) ) * sum over (sx, sy) in block of in[sx, sy]
```

where `w` and `h` are the block's actual width/height after clamping to the
plane edge (see below).

**Handling odd dimensions:** if `W` or `H` is odd, the last block along that
axis is only 1 pixel wide/tall instead of `bw`/`bh`. The implementation
clamps the block's source range to `min(srcStart + blockSize, planeSize)`
rather than assuming every block is full-size, so no source pixel is ever
read out of bounds and no pixel is silently dropped.

**Upsampling (for visualization/reconstruction):** to display what a
decoder would see, or to compare against the original, a subsampled plane
is expanded back to full resolution by nearest-neighbor replication:

```
out[x, y] = in[ x / bw, y / bh ]
```

Every pixel in a `bw x bh` block therefore shows the same, shared chroma
value -- this replication, not any new information, is exactly what
produces the visible "color blockiness" of subsampled images when zoomed
in. Real decoders typically use bilinear upsampling instead of nearest
neighbor for a smoother result; see Future Improvements below.

## 3. Pseudocode

```
function downsample(plane, mode, filter):
    bw, bh = blockSize(mode)              # (1,1) / (2,1) / (2,2)
    W' = ceil(plane.width / bw)
    H' = ceil(plane.height / bh)
    out = new Plane(W', H')
    for oy in 0..H'-1:
        for ox in 0..W'-1:
            x0, y0 = ox * bw, oy * bh
            if filter == Nearest:
                out[ox, oy] = plane[x0, y0]
            else: # Box
                x1 = min(x0 + bw, plane.width)
                y1 = min(y0 + bh, plane.height)
                out[ox, oy] = mean(plane[x0:x1, y0:y1])
    return out
```

## 4. Computational Complexity

Both `downsample` and `upsample` visit every output pixel exactly once and
do O(1) work per pixel (the box filter averages at most 4 source pixels),
so both run in **O(W * H)** time with **O(W * H)** additional memory for
the output plane -- the same asymptotic cost as Stage 2's color conversion,
and comfortably within NFR-3's 500 ms budget for a 512x512 image.

**Storage savings** are the actual point of this stage. Counting samples
per pixel of luma resolution:
- 4:4:4: `1 (Y) + 1 (Cb) + 1 (Cr) = 3` samples/pixel.
- 4:2:2: `1 + 0.5 + 0.5 = 2` samples/pixel (33% smaller than 4:4:4).
- 4:2:0: `1 + 0.25 + 0.25 = 1.5` samples/pixel (50% smaller than 4:4:4).

## 5. Standards and Literature References

- ITU-T H.264/AVC and ITU-T H.265/HEVC both default to 4:2:0 for consumer video.
- JPEG (ISO/IEC 10918-1) commonly uses 4:2:0 or 4:2:2 sampling factors in its SOF marker.
- Charles Poynton, *Digital Video and HD: Algorithms and Interfaces* -- chapter on chroma subsampling covers the visual-acuity rationale in depth.

## 6. Worked Example

Take a 2x2 luma-resolution chroma block with values:

```
10  20
30  40
```

**4:2:0, Box filter:** one output sample = mean(10, 20, 30, 40) = **25**.
**4:2:0, Nearest filter:** one output sample = top-left = **10**.

Upsampling the Box result (25) back to 2x2 for display replicates it across
the whole block:

```
25  25
25  25
```

Compared with the original block, the box-filtered-and-upsampled version
has lost the original per-pixel variation (10, 20, 30, 40) and replaced it
with a single flat value -- visually this is the "loss" chroma subsampling
introduces, traded for a 50% (4:2:0) or 33% (4:2:2) reduction in chroma
data.

## 7. Where the Code Lives

- `core/include/ivcv/core/ChromaSubsamplingMode.h` -- the `ChromaSubsamplingMode` enum, shared with `PipelineContext`.
- `core/include/ivcv/core/ChromaSubsampler.h` / `core/src/ChromaSubsampler.cpp` -- the `ChromaSubsampler` stage and its static `extractPlane`/`downsample`/`upsample` helpers.
- `ui/ChromaSubsamplingView.h` / `ui/ChromaSubsamplingView.cpp` -- the Qt module view.
- `tests/core/ChromaSubsamplerTest.cpp` -- unit tests for every helper above.

## 8. Future Improvements

- Bilinear (or higher-order) upsampling as a second reconstruction option, for a more realistic decoder comparison.
- A configurable chroma siting convention (co-sited vs. centered), which affects exactly where the shared sample is considered to "sit" relative to its luma block.
- Exposing `filterMethod()` selection and a live storage-savings readout in the UI parameter panel (the view already shows plane dimensions; a numeric percentage is a natural follow-up).
