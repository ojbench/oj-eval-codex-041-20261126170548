# Map implementation and validation

`map.hpp` implements `sjtu::map` using an AVL tree. Each node also has
predecessor and successor links. Searches, insertions, and erasures take
O(log n) worst-case time; iterator steps and access to either end take O(1).
Copying and clearing take O(n) time, and storage is O(n).

Rotations and erasure transplant nodes without moving stored values. Thus
iterators and references to surviving nodes remain valid. The end sentinel
has no key or mapped value, allowing non-default-constructible types.
The problem guarantees that exceptional operations are not tested.

## Local validation

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
g++ -std=c++17 -O1 -g -fsanitize=address,undefined -I. tests/map_test.cpp -o /tmp/map_sanitize
/tmp/map_sanitize
```

Both the differential suite and sanitizer run passed. Tests cover randomized
operations against `std::map`, forward and backward iteration, node address
stability, persistent end iterators, copying, assignment, clearing, custom
ordering, and types without default constructors.

The advertised `/workspace/data/041/data_test/` directory was unavailable.
The CMake executable runs local validation; the actual judge compiles the
submitted header with its own test harness.

## Online judge result

- Problem: ACMOJ 2672, map extra.
- Solution commit: `ad76d16ad0a3dd2903b82f64b5eca0c735ba98c2`.
- Submission: `879465`.
- Result: Accepted, 100/100, all ten test points passed.
- Valid submissions used: 1 of 5.

The judge API accepts C++ headers for this problem. A Git submission request
returned HTTP 400 without creating a submission. The successful command was:

```sh
source .env
python3 submit_acmoj/acmoj_client.py --token "$ACMOJ_TOKEN" submit \
    --problem-id 2672 --language cpp --code-file map.hpp
```

The submission CLI was repaired to support its existing Git API method and
the missing C++ source submission method. Credentials and generated build
files are ignored by Git.
