# Module: Color Space Conversion (RGB ↔ Y'CbCr)

This note is the educational companion to `core/include/ivcv/core/ColorSpaceConverter.h`.
It exists so a learner (or a new contributor) can understand *why* this
stage exists and *how* the math works without reading the implementation
first. Every module added to `modules/` from here on will include a note
in this same shape: intuitive explanation, derivation, pseudocode,
complexity, standards references, and a worked example.

## 1. Intuitive Explanation

Cameras and displays think in RGB (red, green, blue) because that is how
color sensors and phosphors/sub-pixels work physically. Compression
standards, however, do not compress RGB directly. The human eye is far
more sensitive to changes in brightness (luma) than to changes in color
(chroma). Y'CbCr separates an image into exactly those two things: Y' is
brightness, and Cb/Cr are "how blue" and "how red" the color is relative
to gray. Once brightness and color are separated, the next stage in this
pipeline (chroma subsampling) can throw away most of the color detail
while keeping all of the brightness detail, because the eye will not
notice. That is the single biggest reason video compresses so well
compared to treating R, G, and B as three independent images.

## 2. Mathematical Derivation

Luma is a weighted sum of linear RGB, where the weights approximate the
eye's sensitivity to each primary:

Y' = Kr·R + Kg·G + Kb·B, where Kr + Kg + Kb = 1

Chroma is then defined as a *scaled difference* between each primary and
luma, so that a gray pixel (R=G=B) always produces Cb=Cr=0 before the
128 offset is added:

Cb = (B − Y') / (2·(1 − Kb))
Cr = (R − Y') / (2·(1 − Kr))

The division by 2·(1 − Kb) and 2·(1 − Kr) normalizes each chroma
component to the range [−0.5, +0.5] before the implementation adds 128 to
store it as an unsigned byte. `ColorSpaceConverter` implements this in
general form using Kr/Kg/Kb rather than hard-coding a 3×3 matrix, so
adding a third standard later is a one-line change to `coefficientsFor()`.

Two coefficient sets are supported:

| Standard | Kr | Kg | Kb | Typical use |
|---|---|---|---|---|
| BT.601 | 0.299 | 0.587 | 0.114 | Standard-definition video |
| BT.709 | 0.2126 | 0.7152 | 0.0722 | High-definition video |

The inverse transform solves the same three equations for R, G, B given
Y', Cb, Cr:

R = Y' + 2·(1 − Kr)·Cr
B = Y' + 2·(1 − Kb)·Cb
G = (Y' − Kr·R − Kb·B) / Kg

## 3. Pseudocode

```
function convertRgbToYuv(rgbImage, standard):
    (Kr, Kg, Kb) = coefficientsFor(standard)
    yuvImage = new ImageBuffer(rgbImage.width, rgbImage.height, 3)
    for each pixel (x, y) in rgbImage:
        R, G, B = rgbImage[x, y]
        Y  = Kr*R + Kg*G + Kb*B
        Cb = (B - Y) / (2 * (1 - Kb)) + 128
        Cr = (R - Y) / (2 * (1 - Kr)) + 128
        yuvImage[x, y] = clampToByte(Y), clampToByte(Cb), clampToByte(Cr)
    return yuvImage
```

## 4. Computational Complexity

Both directions are a single pass over every pixel with a constant amount
of arithmetic per pixel: O(width × height) time, O(1) additional space
per pixel (the output buffer itself is O(width × height)). There is no
dependency between pixels, so this stage is trivially parallelizable
(row-level or tile-level) and a future performance pass could add
SIMD/multi-threading without changing the public API.

## 5. Standards References

- ITU-R Recommendation BT.601: "Studio encoding parameters of digital
  television for standard 4:3 and wide-screen 16:9 aspect ratios."
- ITU-R Recommendation BT.709: "Parameter values for the HDTV standards
  for production and international programme exchange."

(These are cited by name/number for further reading; this project does
not redistribute the standards documents themselves.)

## 6. Worked Numeric Example

Input pixel (R, G, B) = (200, 100, 50), BT.601:

Y'  = 0.299×200 + 0.587×100 + 0.114×50 = 59.8 + 58.7 + 5.7 = 124.2 → 124
Cb  = (50 − 124.2) / (2×0.886) + 128 = −41.9 + 128 = 86.1 → 86
Cr  = (200 − 124.2) / (2×0.701) + 128 = 54.1 + 128 = 182.1 → 182

So (200, 100, 50) in RGB becomes approximately (124, 86, 182) in Y'CbCr.
`tests/core/ColorSpaceConverterTest.cpp` includes an equivalent
hand-computed case (pure red) so this derivation is continuously checked
by CI, not just documented.

## 7. Where the Code Lives

- Interface/algorithm: `core/include/ivcv/core/ColorSpaceConverter.h`,
  `core/src/ColorSpaceConverter.cpp` (pure C++20, no Qt, no OpenCV).
- Tests: `tests/core/ColorSpaceConverterTest.cpp`.
- UI: `ui/RgbYuvView.h/.cpp` (Qt view showing input RGB and output Y'CbCr
  side by side, with a standard selector).

This module's code lives in `core/` rather than physically under this
`modules/` folder because `ImageBuffer`, `PipelineContext`, and every
stage's algorithm form one cohesive, independently-testable library (see
docs/ARCHITECTURE.md, section 1). `modules/<name>/` is where the
per-module *educational* documentation lives, alongside any future
module-specific assets that do not belong in `core/`, `ui/`, or `docs/`.

## 8. Future Improvements / Known Limitations

- Only 8-bit full-range Y'CbCr is implemented; studio range (16–235 /
  16–240) is a documented possible extension if a stage needs it.
- The per-pixel loop is a straightforward scalar implementation; SIMD or
  multi-threaded variants are a candidate future optimization once
  profiling on real images justifies the added complexity.
- No chroma subsampling happens here by design; that is Stage 3
  (`ChromaSubsampler`), kept as a separate stage so each is independently
  testable and toggleable in the UI.
