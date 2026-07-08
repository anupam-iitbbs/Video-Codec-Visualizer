# Block Partitioning

Stage 4 of the still-image pipeline. Implements FR-4 from docs/SRS.md.

## Intuitive Explanation

A photograph is rarely uniform. A patch of clear sky or a plain wall is
nearly flat, while a patch containing hair, foliage, or text is packed
with fine detail. JPEG's classic approach chops every image into fixed
8x8 blocks regardless of content, which is simple but wasteful: a flat
64x64 sky region gets encoded as sixty-four separate 8x8 blocks even
though one number would describe it almost perfectly, while a genuinely
detailed 8x8 region gets exactly the same budget as that sky.

Block partitioning fixes this by letting block size adapt to local
content. We start by tiling the image into large maxBlockSize x
maxBlockSize squares (64x64 by default). For each square we ask "how
much detail is actually in here?" using variance -- a statistical
measure of how spread out the pixel values are. A flat region has near-
zero variance and is left as one big block. A detailed region has high
variance, so we quarter it into four smaller blocks and ask the same
question of each quarter, recursively, until either the variance drops
low enough or we hit a minimum block size (8x8 by default) below which
we refuse to split further. This is the same quadtree idea real modern
codecs use, described next.

## Mathematical Derivation

For a block region of W by H pixels with intensities x_i, the
(population) variance is:

    variance = mean(x_i^2) - mean(x_i)^2

computed in one pass by accumulating both the sum of x_i and the sum of
x_i^2 over the region, then combining them at the end -- this is the
same identity used throughout statistics to avoid a second pass over the
data.

The split rule for a candidate block of nominal size S rooted at (x, y)
is:

    split(x, y, S) =
      if S > minBlockSize and variance(x, y, S, S) > threshold:
        recurse into four quadrants of size S/2
      else:
        emit one leaf Block covering this region

Because maxBlockSize and minBlockSize are both required to be powers of
two, S always halves cleanly (64, 32, 16, 8, ...) and the recursion is
guaranteed to terminate at exactly minBlockSize in the worst case --
there is no possibility of an infinite or off-grid split. When the image
dimensions are not an exact multiple of maxBlockSize, the block actually
stored is clamped to whatever real pixels remain at that edge (so a
20-pixel-wide image with a 16-pixel maxBlockSize produces a 16-wide and
a 4-wide column of top-level blocks, not a 16-wide block that reads past
the edge).

## Pseudocode

    function partition(plane, maxBlockSize, minBlockSize, threshold):
        blocks = []
        for each maxBlockSize x maxBlockSize tile (x, y) covering plane:
            partitionRecursive(plane, x, y, maxBlockSize, blocks)
        return blocks

    function partitionRecursive(plane, x, y, size, blocks):
        width  = min(size, plane.width  - x)
        height = min(size, plane.height - y)
        v = variance(plane, x, y, width, height)
        if size > minBlockSize and v > threshold:
            half = size / 2
            partitionRecursive(plane, x,        y,        half, blocks)
            partitionRecursive(plane, x + half,  y,        half, blocks)  // if in bounds
            partitionRecursive(plane, x,         y + half, half, blocks)  // if in bounds
            partitionRecursive(plane, x + half,  y + half, half, blocks)  // if in bounds
        else:
            blocks.append(Block(x, y, width, height, v))

## Computational Complexity

Computing the variance of a region visits every pixel in it once, so the
total work across one full quadtree level (the set of blocks at a given
depth) is O(width * height): every pixel belongs to exactly one block at
each level. The number of levels is bounded by
log2(maxBlockSize / minBlockSize), a small constant (3 for the default
64 down to 8). So overall time is O(width * height * log2(maxBlockSize /
minBlockSize)), which in practice behaves like O(width * height) since
the log factor never exceeds a handful of levels. Memory is O(number of
leaf blocks), which is at most width * height / minBlockSize^2.

## Standards and Literature References

- ITU-T H.265 (HEVC): defines the Coding Tree Unit (up to 64x64) and its
  recursive quadtree split into Coding Units down to 8x8, the direct
  inspiration for this module's structure (real encoders choose splits
  by rate-distortion optimization rather than raw variance).
- ITU-T H.264/AVC: uses a fixed 16x16 macroblock with only limited,
  non-recursive sub-partitioning, a useful historical contrast to HEVC's
  fully recursive quadtree.
- ISO/IEC 10918-1 (JPEG): partitions images into fixed, non-adaptive 8x8
  blocks, the baseline this module improves on for detail-adaptive
  coding.

## Worked Example

Take a 16x16 luma block filled with an alternating 0/255 checkerboard
pattern (maximum possible detail), with maxBlockSize = 16, minBlockSize
= 8, and threshold = 0:

1. The whole 16x16 region has mean 127.5 and mean-of-squares 32512.5, so
   its variance is 32512.5 - 127.5^2 = 16256.25, which is greater than
   the threshold of 0, so it splits into four 8x8 quadrants.
2. Each 8x8 quadrant is itself a checkerboard with the same 16256.25
   variance, but 8 is not greater than minBlockSize (8), so none of them
   split further -- each becomes a leaf Block.
3. The result is four Block entries: (0,0,8,8), (8,0,8,8), (0,8,8,8),
   and (8,8,8,8), each carrying its own id, coordinates, and variance,
   ready for the UI to display or for Stage 5 (DCT) to transform.

## Where the Code Lives

- `core/include/ivcv/core/Block.h` -- the Block leaf-node value type.
- `core/include/ivcv/core/BlockPartitioner.h` /
  `core/src/BlockPartitioner.cpp` -- the stage implementation:
  `computeVariance()` and `partition()` are static and independently
  unit-testable without a PipelineContext.
- `core/include/ivcv/core/PipelineContext.h` -- `blocks()` accessor
  holding the quadtree produced by this stage.
- `tests/core/BlockPartitionerTest.cpp` -- unit tests, including a
  hand-verified variance calculation and multiple quadtree-depth cases.
- `ui/BlockPartitioningView.h` / `ui/BlockPartitioningView.cpp` -- the Qt
  view: a clickable grid overlay showing every block's boundary, a
  per-block variance heatmap, and a details panel reporting the id,
  coordinates, and variance of whichever block was last clicked.

## Future Improvements

- Replace the raw variance threshold with a rate-distortion-style
  decision that compares the actual downstream coding cost of splitting
  versus not splitting, closer to what real HEVC encoders do.
- Support non-square partitions (HEVC-style asymmetric prediction
  units), rather than only symmetric quadtree splits.
- Extend partitioning to the Cb/Cr planes, either independently or tied
  to the luma quadtree the way 4:2:0 chroma coding units are derived in
  HEVC.
- Animate the recursive split decisions in the UI so a learner can watch
  the quadtree build up level by level instead of only seeing the final
  result.
