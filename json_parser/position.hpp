#include <string>
#include <format>

struct Position
{
    size_t pos{0}, line{0}, col{0};
    std::string to_string() const
    {
        return std::format("pos: {}, line: {}, column: {}", pos, line, col);
    }

    auto operator<=>(const Position& other) const = default;

    operator size_t() { return pos; }
};