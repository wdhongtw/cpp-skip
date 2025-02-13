# Skip List and C++ Module Experiment

A simple project to try new C++ feature and practice skip list.

The SkipList class is provided through C++20 module, and iteration is
implemented through `std::generator` type.

## Usage

```cpp
import std;
import skip;

int main() {

    auto values = skip::SkipList<int> { std::views::iota(0, 6) };

    for (int idx : std::views::iota(0, 4))
        values.add(idx);
    for (int idx : std::views::iota(2, 4))
        values.remove(idx);

    std::println("values: {}", values.iter());
    // values: [0, 0, 1, 1, 2, 3, 4, 5]

    return 0;
}
```
