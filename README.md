# Multithreaded Search Engine

A multithreaded scientific search engine designed to find the best matching abstracts of scientific articles based on a given query. The program tokenizes the abstract text and calculates the Jaccard similarity between the abstract and the query. It supports multithreaded processing using the POSIX thread (pthread) library for efficient search across a large number of articles.

## Features
- **Multithreading:** Utilizes POSIX threads (pthread) for parallel processing.
- **Jaccard Similarity:** Computes the similarity between the query and abstracts.
- **Efficient Search:** For T threads, searches over A articles and returns the top N best matches with their summaries.
- **Thread Synchronization:** Mutex locks are used to handle synchronization issues across multiple threads.

## Usage


To compile:

```
g++ abstractor.cpp -o abstractor.out -lpthread
```

To run:
```
./abstractor.out <input_path> <output_path>
```

- `input_path` is a file containing the abstracts to be searched
- `output_path` is a file that lists the top N matching articles along with their summaries

## Libraries Used
- **POSIX Thread (pthread):** For multithreading.
- **C++ Standard Libraries:** Used for handling text processing and file I/O.
