# Caltech Large Scale Image Search Toolbox #

### Description ###

This C++/Matlab package implements several algorithms used for large scale
image search. The algorithms are implemented in C++, with an eye on large
scale databases. It can handle millions of images and hundreds of millions
of local features. It has MEX interfaces for Matlab, but can also be used
(with possible future modifications) from Python and directly from C++.

The algorithms can be divided into two broad categories, depending on the
approach taken for image search:

#### 1. Bag of Words ####
The images are represented by histograms of visual words.

It includes algorithms for computing dictionaries:
  * K-Means.
  * Approximate K-Means (AKM).
  * Hierarchical K-Means (HKM).
It also includes algorithms for fast search:
  * Inverted File Index.
  * Inverted File Index with Extra Information (for example for implementing Hamming Embedding).
  * Min-Hash.

#### 2. Full Representation ####
The images are represented by the individual features.

It includes algorithms for fast approximate nearest neighbor search:

  * Kd-Trees (Kdt).
  * Hierarchical K-Means (Hkm).
  * Locality Senstivie Hashing (LSH), with several hash functions:
    * Hamming hash function (bit sampling, approximates hamming distance) i.e. `h = x[i]`
    * Cosine hash function (random hyperplanes through the origin, approximates dot product) i.e.`h = sign(<x,r>)`
    * L1 hash function (approximates the L1 distance) i.e. `h = floor((x[i]-b) / w)`
    * L2 hash function (random hyperplanes with bias, approximates euclidean distance, similar to E2LSH) i.e. `h = floor((<x,r> - b) / w)`
    * Spherical Simplex (approximates distances on the unit hypersphere)
    * Spherical Orthoplex (approximates distances on the unit hypersphere)
    * Spherical Hypercube (approximates distances on the unit hypersphere)
    * Binary Gausian Kernels (approximates gaussian kernel)

### Supported Operating Systems ###
It was only compiled and tested on RHEL Linux 64-bits with gcc. I haven't compiled or tried it on Windows though.

### Citation ###
If you use this software, please cite the paper below:
> Mohamed Aly, Mario Munich, and Pietro Perona.
> Indexing in Large Scale Image Collections: Scaling Properties and Benchmark.
> IEEE Workshop on Applications of Computer Vision WACV, January 2011.

### Reference ###
`[`1`]` Mohamed Aly, Mario Munich, and Pietro Perona.
Indexing in Large Scale Image Collections: Scaling Properties and Benchmark.
IEEE Workshop on Applications of Computer Vision WACV, January 2011.