#include <string>
#include <vector>
#include <format>
#include <optional>
namespace hayk10002::json_parser::lexer
{
    class Cursor
    {
    public:
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

    private:
        std::string_view m_view{};
        /// stores already encountered line starts, except first one, for every line start the previous character is '\n'
        /// first value always 0
        std::vector<size_t> m_line_starts{0};
        Position m_pos{};

    public:
        Cursor(std::string_view view): m_view(view) {}

        /// @brief moves the cursor forward or backward, and returns selected part
        ///
        /// "hel_lo world" -> move(3) -> "hello _world" and returns "lo "
        ///
        /// "hello wo_rld" -> move(-5) -> "hel_lo world" and returns "lo wo"
        ///
        /// if moving will place cursor out of bounds, it is placed exactly at start or at end
        /// @param d how much to move the cursor
        /// @return selected part
        std::string_view move(int d)
        {
            // if moving d will go out of bounds, recalculate d
            if (int(m_pos) + d < 0) d = -m_pos;
            else if (m_pos + d >= m_view.size()) d = m_view.size() - m_pos;

            std::string_view res_span{};
            
            // if moving forward
            if (d >= 0) 
            {
                res_span = m_view.substr(m_pos, d);

                // while not on last line, and if moving will change the line
                while(m_pos.line < m_line_starts.size() - 1 && m_line_starts[m_pos.line + 1] <= m_pos + d) 
                {
                    // move to the start of next line
                    d -= m_line_starts[m_pos.line + 1] - m_pos;
                    m_pos.pos = m_line_starts[m_pos.line + 1];
                    m_pos.line++;
                    m_pos.col = 0;
                }
                // not on the last line, but will stay on this line 
                if (m_pos.line != m_line_starts.size() - 1)
                {
                    m_pos.pos += d;
                    m_pos.col = d;
                }
                // on last line
                else
                {
                    while(d)
                    {
                        // encounter new line and register it
                        if (m_view[m_pos] == '\n')
                        {
                            m_pos.line++;
                            m_pos.pos++;
                            m_pos.col = 0;
                            m_line_starts.push_back(m_pos);
                        }
                        else 
                        {
                            m_pos.pos++;
                            m_pos.col++;
                        }
                        d--;
                    }
                }
            }
            // if moving backward
            if (d < 0) 
            {
                res_span = m_view.substr(m_pos + d, -d);
                m_pos.pos += d;
                // find the line that the cursor will be after moving
                while(m_line_starts[m_pos.line] > m_pos) m_pos.line--;
                m_pos.col = m_pos - m_line_starts[m_pos.line];
            }
            return res_span;
        }


        Position get_pos() { return m_pos; }
        void set_pos(size_t pos) { move(pos - m_pos); }
        void set_pos(Position pos) { set_pos(pos.pos); }

        /// @brief get next character
        /// @return return std::nullopt if at the end, or the next character
        std::optional<char> next()
        {
            if (auto v = move(1); !v.empty()) return v[0];
            else return std::nullopt;
        }

    };
}