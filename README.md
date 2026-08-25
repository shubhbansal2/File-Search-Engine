# README: C++ Assignment

## Prerequisites

You will need a standard C++11 (or newer) compiler. This works perfectly with `g++` on Linux/macOS, or MinGW/Git Bash on Windows.

## Compilation

To compile the program, open your terminal in the directory where the source code is located and run this command. I highly recommend using the `-O2` flag, as it tells the compiler to optimize the code for speed, making a huge difference on massive datasets!

```bash
g++ -O2 230994_Shubh.cpp -o analyzer
```

## Execution Examples

The program reads arguments from the command line exactly as requested. Here is how you can test it:

**1. Word Count Query (Single Version)**
To find out how many times the word "error" shows up in version v1:

```bash
./analyzer --file dataset_v1.txt --version v1 --buffer 512 --query word --word error
```

**2. Top-K Query (Single Version)**
To see the top 10 most used words in version v1:

```bash
./analyzer --file dataset_v1.txt --version v1 --buffer 512 --query top --top 10
```

**3. Difference Query (Two Versions)**
To find the difference in frequency for the word "error" across two different files:

```bash
./analyzer --file1 dataset_v1.txt --version1 v1 --file2 dataset_v2.txt --version2 v2 --buffer 512 --query diff --word error
```

## Expected Output

When you run the commands above, the program will print:

1. The version name(s) being queried.
2. The result of your query (the frequency, the top K list, or the absolute difference).
3. The buffer size it allocated (in KB).
4. The total time it took to execute (in seconds).