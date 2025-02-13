export module skip;

import std;

using std::nullopt;
using std::optional;

using std::make_shared;
using std::shared_ptr;

/// @brief Assert a condition to be true.
/// @param condition to be checked
void check(bool condition) {
    if (!condition)
        throw std::exception("assertion fail");
}

/// @brief Generate a random value in close-open range [lo, hi).
/// @param lo lower bound
/// @param hi upper bound
/// @return the random value
int rand(int lo, int hi) {
    static std::random_device rd {};
    static std::mt19937 gen { rd() };

    std::uniform_int_distribution<> dist { lo, hi - 1 };
    return dist(gen);
}

/// @brief skip namespace in skip module
namespace skip {

/// @brief Node is the node type for SkipList.
/// @tparam T the value type
template <std::regular T>
struct Node {

    /// @brief Value of the node, null for sentinel node.
    optional<T> value = nullopt;

    /// @brief Down link to next level.
    shared_ptr<Node<T>> down = nullptr;

    /// @brief Right link for the forward list.
    shared_ptr<Node<T>> right = nullptr;
};

/// @brief SkipList is a sorted container, can have duplicated values.
/// @tparam T the value type
export template <std::regular T>
class SkipList {
private:
    using NT = Node<T>;

public:
    SkipList() { }

    template <std::ranges::input_range R>
        requires std::same_as<T, std::ranges::range_value_t<R>>
    SkipList(R&& items) {
        for (auto&& item : items)
            add(item);
    }

public:
    /// @brief Add a value into container.
    /// @param value the value to be added
    void add(T value) {

        auto chose_level = []() -> int {
            constexpr int factor = 4;

            int level = 0;
            while (rand(0, factor) == 0)
                level += 1;
            return level;
        };

        auto ensure_height = [&](int expected) {
            check(expected > 0);

            int current = height();
            if (current >= expected)
                return;

            for (int idx : std::ranges::views::iota(current, expected))
                head_ = make_shared<NT>(nullopt, head_, nullptr);
        };

        const int half = chose_level() + 1;
        ensure_height(half);

        auto tracked = traverse(value);
        shared_ptr<NT> down = nullptr;
        for (int idx : std::ranges::views::iota(0, half)) {
            auto [pre, nex] = tracked.at(idx);
            auto node = make_shared<NT>(optional<T> { value }, down, nex);
            pre->right = node;

            down = node;
        }
    }

    /// @brief Find a value in container, if there is at lease one match.
    /// @param value the value to match
    /// @return whether is the value found or not
    bool find(T value) const {
        auto tracked = traverse(value);

        auto [_, cur] = tracked.at(0);
        return cur != nullptr && *cur->value == value;
    }

    /// @brief Remove a value in container, throws if there is no match.
    /// @param value the value to match
    void remove(T value) {
        auto clean_head = [&]() {
            while (head_->right == nullptr && head_->down != nullptr)
                head_ = head_->down;
        };

        auto tracked = traverse(value);

        auto [_, cur] = tracked.at(0);
        if (cur == nullptr || *cur->value != value)
            throw std::exception("no such value");

        for (auto [idx, record] : tracked) {
            auto [pre, cur] = record;
            if (cur == nullptr || *cur->value != value)
                continue;

            pre->right = cur->right;
        }

        clean_head();
    }

private:
    /// @brief Return number of levels in container.
    /// @return number of levels.
    int height() const {

        int height = 1;
        auto cur = head_;
        while (cur->down) {
            height += 1;
            cur = cur->down;
        }

        return height;
    }

private:
    /// @brief Main traversing logic for SkipList
    /// @param value wanted value in this traverse context
    /// @return focused pairs at each level
    std::map<int, std::tuple<shared_ptr<NT>, shared_ptr<NT>>> traverse(T value) {
        int level = height() - 1;
        auto tracked = std::map<int, std::tuple<shared_ptr<NT>, shared_ptr<NT>>> {};

        auto pre = head_;
        while (pre != nullptr) {
            auto cur = pre->right;
            while (cur != nullptr && *cur->value < value) {
                pre = cur;
                cur = cur->right;
            }

            tracked[level] = { pre, cur };
            pre = pre->down;
            level -= 1;
        }

        check(pre == nullptr);
        check(level == -1);

        return tracked;
    }

public:
    /// @brief Yield all values from the container.
    /// @return the iterator for yielding values
    std::generator<T> iter() const {
        auto row = head_;
        while (row->down != nullptr)
            row = row->down;

        auto cur = row->right;
        while (cur != nullptr) {
            co_yield *cur->value;
            cur = cur->right;
        }
    }

private:
    /// @brief the head node at top-left corner
    shared_ptr<NT> head_ = make_shared<NT>(nullopt, nullptr, nullptr);
};

}
