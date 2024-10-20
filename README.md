# Garbage Collector in C

This is a garbage collector in C that I wrote as part of a systems programming course. I am not listing the precise course number, my university, or the name of the professor to avoid aiding plaigiarism, though I'm sure many similar projects exist online.

This code was written in Fall of 2022. It predates ChatGPT.
The garbage collector has a known bug, possibly having to do with freeing memory that is only indirectly referenced from the root memory. This is documented in the video "Recording of Gradescope for Main Project.mov".

# Plans
I intend to reimplement this code in Rust, probably in this repository. I choose Rust because Rust made me love programming again and goes a long way towards making even systems programming somewhat "fearless" - or at least, *way* less fear*ful*.
I may even augment the Rust example with an implementation of [Stop-and-Copy](https://en.wikipedia.org/wiki/Tracing_garbage_collection#Copying_vs._mark-and-sweep_vs._mark-and-don.27t-sweep) garbage collection. But I wouldn't hold your breath :).