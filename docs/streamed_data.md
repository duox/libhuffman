# Streamed data

Usually, due to the irregular size of Huffman codes, it's quite complicated to parallelize Huffman encoding/decoding process.
The general problem is that code boundaries are detected only after finishing walking code tree or table, thus making impossible
parallel lookup for sequential codes.

To overcome code boundaries irregularity, a Huffman bit stream can be splitted in relatively large sub-streams, placed
sequentially one after another. Each sub-stream can have separate code table if the source data are different one from another.
However, separate tables can increase pressure on the processor cache so they only used when the benefit is obvious. A usual
text data is quite uniform and all streams use the same code table. Stream sizes are chosen so that it's possible to use 
multi-threaded codec.

To increase parallelism even futher, a stream can be split in blocks of roughly equal number of codes. That would allow to use
SIMD instructions to process say four blocks per decoder iteration. Even without SIMD, a parallel loop of two blocks processed
simultaneously would be quite of benefit.

Parameters of splitting in streams and blocks are chosen accordingly to distribution of probability and target architecture,
including target instruction set and cache subsystem configuration.
