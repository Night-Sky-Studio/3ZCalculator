#include "json.hpp"

// _______________________ INCLUDES _______________________

#include <array>        // array<>
#include <charconv>     // to_chars(), from_chars()
#include <climits>      // CHAR_BIT
#include <filesystem>   // path, exists, create_directory
#include <fstream>      // ifstream, ofstream
#include <limits>       // numeric_limits<>::max_digits10, numeric_limits<>::max_exponent10
#include <stdexcept>    // runtime_error
#include <system_error> // errc

namespace fs = std::filesystem;

// ===================
// --- Misc. utils ---
// ===================

namespace utl::json {
    // Codepoint conversion function. We could use <codecvt> to do the same in a few lines,
    // but <codecvt> was marked for deprecation in C++17 and fully removed in C++26, as of now
    // there is no standard library replacement so we have to roll our own. This is likely to
    // be more performant too due to not having any redundant locale handling.
    //
    // The function was tested for all valid codepoints (from U+0000 to U+10FFFF)
    // against the <codecvt> implementation and proved to be exactly the same.
    //
    // Codepoint <-> UTF-8 convertion table (see https://en.wikipedia.org/wiki/UTF-8):
    //
    // | Codepoint range      | Byte 1   | Byte 2   | Byte 3   | Byte 4   |
    // |----------------------|----------|----------|----------|----------|
    // | U+0000   to U+007F   | 0eeeffff |          |          |          |
    // | U+0080   to U+07FF   | 110dddee | 10eeffff |          |          |
    // | U+0800   to U+FFFF   | 1110cccc | 10ddddee | 10eeffff |          |
    // | U+010000 to U+10FFFF | 11110abb | 10bbcccc | 10ddddee | 10eeffff |
    //
    // Characters 'a', 'b', 'c', 'd', 'e', 'f' correspond to the bits taken from the codepoint 'U+ABCDEF'
    // (each letter in a codepoint is a hex corresponding to 4 bits, 6 positions => 24 bits of info).
    // In terms of C++ 'U+ABCDEF' codepoints can be expressed as an integer hex-literal '0xABCDEF'.
    //
    inline bool _codepoint_to_utf8(std::string& destination, std::uint32_t cp) {
        // returns success so we can handle the error message inside the parser itself.

        std::array<char, 4> buffer;
        std::size_t count;

        // 1-byte ASCII (codepoints U+0000 to U+007F)
        if (cp <= 0x007F) {
            buffer[0] = static_cast<char>(cp);
            count = 1;
        }
        // 2-byte unicode (codepoints U+0080 to U+07FF)
        else if (cp <= 0x07FF) {
            buffer[0] = static_cast<char>(((cp >> 6) & 0x1F) | 0xC0);
            buffer[1] = static_cast<char>(((cp >> 0) & 0x3F) | 0x80);
            count = 2;
        }
        // 3-byte unicode (codepoints U+0800 to U+FFFF)
        else if (cp <= 0xFFFF) {
            buffer[0] = static_cast<char>(((cp >> 12) & 0x0F) | 0xE0);
            buffer[1] = static_cast<char>(((cp >> 6) & 0x3F) | 0x80);
            buffer[2] = static_cast<char>(((cp >> 0) & 0x3F) | 0x80);
            count = 3;
        }
        // 4-byte unicode (codepoints U+010000 to U+10FFFF)
        else if (cp <= 0x10FFFF) {
            buffer[0] = static_cast<char>(((cp >> 18) & 0x07) | 0xF0);
            buffer[1] = static_cast<char>(((cp >> 12) & 0x3F) | 0x80);
            buffer[2] = static_cast<char>(((cp >> 6) & 0x3F) | 0x80);
            buffer[3] = static_cast<char>(((cp >> 0) & 0x3F) | 0x80);
            count = 4;
        }
        // invalid codepoint
        else {
            return false;
        }

        destination.append(buffer.data(), count);
        return true;
    }

    // JSON '\u' escapes use UTF-16 surrogate pairs to encode codepoints outside of basic multilingual plane,
    // see https://unicodebook.readthedocs.io/unicode_encodings.html
    //     https://en.wikipedia.org/wiki/UTF-16
    [[nodiscard]] constexpr std::uint32_t _utf16_pair_to_codepoint(std::uint16_t high, std::uint16_t low) noexcept {
        return 0x10000 + ((high & 0x03FF) << 10) + (low & 0x03FF);
    }

    [[nodiscard]] inline std::string _utf8_replace_non_ascii(std::string str, char replacement_char) noexcept {
        for (auto& e : str)
            if (static_cast<std::uint8_t>(e) > 127) e = replacement_char;
        return str;
    }

    [[nodiscard]] inline std::string _read_file_to_string(const std::string& path) {
        using namespace std::string_literals;

        // This seems the to be the fastest way of reading a text file
        // into 'std::string' without invoking OS-specific methods
        // See this StackOverflow thread:
        // https://stackoverflow.com/questions/32169936/optimal-way-of-reading-a-complete-file-to-a-string-using-fstream
        // And attached benchmarks:
        // https://github.com/Sqeaky/CppFileToStringExperiments

        std::ifstream file(path, std::ios::ate); // open file and immediately seek to the end
        if (!file.good()) throw std::runtime_error("Could not open file {"s + path + "."s);

        const auto file_size = file.tellg(); // returns cursor pos, which is the end of file
        file.seekg(std::ios::beg);           // seek to the beginning
        std::string chars(file_size, 0);     // allocate string of appropriate size
        file.read(chars.data(), file_size);  // read into the string
        return chars;
    }

    template<class T>
    [[nodiscard]] constexpr int _log_10_ceil(T num) noexcept {
        return num < 10 ? 1 : 1 + _log_10_ceil(num / 10);
    }

    [[nodiscard]] inline std::string _pretty_error(std::size_t cursor, const std::string& chars) {
        // Special case for empty buffers
        if (chars.empty()) return "";

        // "Normalize" cursor if it's at the end of the buffer
        if (cursor >= chars.size()) cursor = chars.size() - 1;

        // Get JSON line number
        std::size_t line_number = 1; // don't want to include <algorithm> just for a single std::count()
        for (std::size_t pos = 0; pos < cursor; ++pos)
            if (chars[pos] == '\n') ++line_number;

        // Get contents of the current line
        constexpr std::size_t max_left_width = 24;
        constexpr std::size_t max_right_width = 24;

        std::size_t line_start = cursor;
        for (; line_start > 0; --line_start)
            if (chars[line_start - 1] == '\n' || cursor - line_start >= max_left_width) break;

        std::size_t line_end = cursor;
        for (; line_end < chars.size() - 1; ++line_end)
            if (chars[line_end + 1] == '\n' || line_end - cursor >= max_right_width) break;

        const std::string_view line_contents(chars.data() + line_start, line_end - line_start + 1);

        // Format output
        const std::string line_prefix =
            "Line " + std::to_string(line_number) + ": "; // fits into SSO buffer in almost all cases

        std::string res;
        res.reserve(7 + 2 * line_prefix.size() + 2 * line_contents.size());

        res += '\n';
        res += line_prefix;
        res += _utf8_replace_non_ascii(std::string(line_contents), '?');
        res += '\n';
        res.append(line_prefix.size(), ' ');
        res.append(cursor - line_start, '-');
        res += '^';
        res.append(line_end - cursor, '-');
        res += " [!]";

        // Note:
        // To properly align cursor in the error message we would need to count "visible characters" in a UTF-8
        // string, properly iterating over grapheme clusters is a very complex task, usually done by a dedicated
        // library. We could just count codepoints, but that wouldn't account for combining characters. To prevent
        // error message from being misaligned we can just replace all non-ascii symbols with '?', this way errors
        // might be less pretty, but they will reliably show the true location of the error.

        return res;
    }
}

// ==================
// --- Node class ---
// ==================

namespace utl::json {
    inline void _serialize_json_to_buffer(std::string& chars, const Node& node, Format format) {
        if (format == Format::PRETTY) _serialize_json_recursion<true>(node, chars);
        else _serialize_json_recursion<false>(node, chars);
    }

    // -- Getters --
    // -------------

    Node::object_type& Node::as_object() { return this->as<object_type>(); }
    Node::array_type& Node::as_array() { return this->as<array_type>(); }
    Node::string_type& Node::as_string() { return this->as<string_type>(); }
    Node::integral_type& Node::as_integral() { return this->as<integral_type>(); }
    Node::floating_type& Node::as_floating() { return this->as<floating_type>(); }
    Node::bool_type& Node::as_bool() { return this->as<bool_type>(); }
    Node::null_type& Node::as_null() { return this->as<null_type>(); }

    const Node::object_type& Node::as_object() const { return this->as<object_type>(); }
    const Node::array_type& Node::as_array() const { return this->as<array_type>(); }
    const Node::string_type& Node::as_string() const { return this->as<string_type>(); }
    const Node::integral_type& Node::as_integral() const { return this->as<integral_type>(); }
    const Node::floating_type& Node::as_floating() const { return this->as<floating_type>(); }
    const Node::bool_type& Node::as_bool() const { return this->as<bool_type>(); }
    const Node::null_type& Node::as_null() const { return this->as<null_type>(); }

    bool Node::is_object() const noexcept { return this->is<object_type>(); }
    bool Node::is_array() const noexcept { return this->is<array_type>(); }
    bool Node::is_string() const noexcept { return this->is<string_type>(); }
    bool Node::is_integral() const noexcept { return this->is<integral_type>(); }
    bool Node::is_floating() const noexcept { return this->is<floating_type>(); }
    bool Node::is_bool() const noexcept { return this->is<bool_type>(); }
    bool Node::is_null() const noexcept { return this->is<null_type>(); }

    // -- Object methods ---
    // ---------------------

    Node& Node::operator[](std::string_view key) {
        // 'null' converts to objects automatically
        if (this->is_null()) {
            this->_data = object_type {};
        }
        auto& object = this->as_object();
        auto it = object.find(key);
        if (it == object.end()) {
            it = object.emplace(key, Node {}).first;
        }
        return it->second;
    }
    const Node& Node::operator[](std::string_view key) const {
        const auto& object = this->as_object();
        const auto it = object.find(key);
        if (it == object.end())
            throw std::runtime_error("Accessing non-existent key {" + std::string(key) + "} in JSON object.");
        return it->second;
    }

    Node& Node::at(std::string_view key) {
        auto& object = this->as_object();
        const auto it = object.find(key);
        if (it == object.end())
            throw std::runtime_error("Accessing non-existent key {" + std::string(key) + "} in JSON object.");
        return it->second;
    }
    const Node& Node::at(std::string_view key) const { return this->operator[](key); }

    bool Node::contains(std::string_view key) const {
        const auto& object = this->as_object();
        const auto it = object.find(std::string(key));
        return it != object.end();
    }

    // -- Array methods ---
    // --------------------

    Node& Node::operator[](std::size_t pos) { return this->as_array()[pos]; }
    const Node& Node::operator[](std::size_t pos) const { return this->as_array()[pos]; }

    Node& Node::at(std::size_t pos) { return this->as_array().at(pos); }
    const Node& Node::at(std::size_t pos) const { return this->as_array().at(pos); }

    void Node::push_back(const Node& node) {
        if (this->is_null()) this->_data = array_type {}; // 'null' converts to arrays automatically
        this->as_array().push_back(node);
    }
    void Node::push_back(Node&& node) {
        if (this->is_null()) this->_data = array_type {}; // 'null' converts to arrays automatically
        this->as_array().push_back(std::move(node));
    }

    // -- Assignment --
    // ----------------

    Node& Node::operator=(const object_type& value) {
        this->_data = value;
        return *this;
    }
    Node& Node::operator=(object_type&& value) {
        this->_data = std::move(value);
        return *this;
    }
    Node& Node::operator=(const array_type& value) {
        this->_data = value;
        return *this;
    }
    Node& Node::operator=(array_type&& value) {
        this->_data = std::move(value);
        return *this;
    }
    Node& Node::operator=(const string_type& value) {
        this->_data = value;
        return *this;
    }
    Node& Node::operator=(string_type&& value) {
        this->_data = std::move(value);
        return *this;
    }

    // -- Constructors --
    // ------------------

    Node& Node::operator=(const Node&) = default;
    Node& Node::operator=(Node&&) noexcept = default;

    Node::Node() = default;
    Node::Node(const Node&) = default;
    Node::Node(Node&&) noexcept = default;

    Node::Node(const object_type& value) { this->_data = value; }
    Node::Node(object_type&& value) { this->_data = std::move(value); }
    Node::Node(const array_type& value) { this->_data = value; }
    Node::Node(array_type&& value) { this->_data = std::move(value); }
    Node::Node(std::string_view value) { this->_data = string_type(value); }
    Node::Node(const string_type& value) { this->_data = value; }
    Node::Node(string_type&& value) { this->_data = std::move(value); }
    Node::Node(integral_type value) { this->_data = value; }
    Node::Node(floating_type value) { this->_data = value; }
    Node::Node(bool_type value) { this->_data = value; }
    Node::Node(null_type value) { this->_data = value; }

    // --- JSON Serializing public API ---
    // -----------------------------------

    std::string Node::to_string(Format format) const {
        std::string buffer;
        _serialize_json_to_buffer(buffer, *this, format);
        return buffer;
    }
    void Node::to_file(const std::string& filepath, Format format) const {
        const auto chars = this->to_string(format);

        const fs::path path = filepath;
        if (path.has_parent_path() && fs::exists(path.parent_path()))
            fs::create_directories(fs::path(filepath).parent_path());
        // no need to do an OS call in a trivial case, some systems might also have limited permissions
        // on directory creation and calling 'create_directories()' straight up will cause them to error
        // even when there is no need to actually perform directory creation because it already exists

        // if user doesn't want to pay for 'create_directories()' call (which seems to be inconsequential
        // on my benchmarks) they can always use 'std::ofstream' and 'to_string()' to export manually

        std::ofstream(filepath).write(chars.data(), chars.size());
        // maybe a little faster than doing 'std::ofstream(filepath) << node.to_string(format)'
    }

    NodeType Node::type() const {
        return (NodeType) this->_data.index();
    }
}

// =====================
// --- Lookup Tables ---
// =====================

namespace utl::json {
    constexpr std::uint8_t _u8(char value) { return static_cast<std::uint8_t>(value); }

    static_assert(CHAR_BIT == 8); // we assume a sane platform, perhaps this isn't even necessary

    constexpr std::size_t _number_of_char_values = 256;

    // Lookup table used to check if number should be escaped and get a replacement char on at the same time.
    // This allows us to replace multiple checks and if's with a single array lookup that.
    //
    // Instead of:
    //    if (c == '"') { chars += '"' }
    //    ...
    //    else if (c == '\t') { chars += 't' }
    // we get:
    //    if (const char replacement = _lookup_serialized_escaped_chars[_u8(c)]) { chars += replacement; }
    //
    // which ends up being a bit faster and also nicer.
    //
    // Note:
    // It is important that we explicitly cast to 'uint8_t' when indexing, depending on the platform 'char' might
    // be either signed or unsigned, we don't want our array to be indexed at '-71'. While we can reasonably expect
    // ASCII encoding on the platform (which would put all char literals that we use into the 0-127 range) other chars
    // might still be negative. This shouldn't have any cost as trivial int casts like these involve no runtime logic.
    //
    constexpr std::array<char, _number_of_char_values> _lookup_serialized_escaped_chars = [] {
        std::array<char, _number_of_char_values> res {};
        // default-initialized chars get initialized to '\0',
        // ('\0' == 0) is mandated by the standard, which is why we can use it inside an 'if' condition
        res[_u8('"')] = '"';
        res[_u8('\\')] = '\\';
        // res['/']  = '/'; escaping forward slash in JSON is allowed, but redundant
        res[_u8('\b')] = 'b';
        res[_u8('\f')] = 'f';
        res[_u8('\n')] = 'n';
        res[_u8('\r')] = 'r';
        res[_u8('\t')] = 't';
        return res;
    }();

    // Lookup table used to determine "insignificant whitespace" characters when
    // skipping whitespace during parser. Seems to be either similar or marginally
    // faster in performance than a regular condition check.
    constexpr std::array<bool, _number_of_char_values> _lookup_whitespace_chars = [] {
        std::array<bool, _number_of_char_values> res {};
        // "Insignificant whitespace" according to the JSON spec:
        // [https://ecma-international.org/wp-content/uploads/ECMA-404.pdf]
        // constitutes following symbols:
        // - Whitespace      (aka ' ' )
        // - Tabs            (aka '\t')
        // - Carriage return (aka '\r')
        // - Newline         (aka '\n')
        res[_u8(' ')] = true;
        res[_u8('\t')] = true;
        res[_u8('\r')] = true;
        res[_u8('\n')] = true;
        return res;
    }();

    // Lookup table used to get an appropriate char for the escaped char in a 2-char JSON escape sequence.
    constexpr std::array<char, _number_of_char_values> _lookup_parsed_escaped_chars = [] {
        std::array<char, _number_of_char_values> res {};
        res[_u8('"')] = '"';
        res[_u8('\\')] = '\\';
        res[_u8('/')] = '/';
        res[_u8('b')] = '\b';
        res[_u8('f')] = '\f';
        res[_u8('n')] = '\n';
        res[_u8('r')] = '\r';
        res[_u8('t')] = '\t';
        return res;
    }();
}

// ==========================
// --- JSON Parsing impl. ---
// ==========================

namespace utl::json {
    _parser::_parser(const std::string& chars, unsigned int& recursion_limit):
        chars(chars),
        recursion_limit(recursion_limit) {
    }
    std::size_t _parser::skip_nonsignificant_whitespace(std::size_t cursor) {
        using namespace std::string_literals;

        while (cursor < this->chars.size()) {
            if (!_lookup_whitespace_chars[_u8(this->chars[cursor])]) return cursor;
            ++cursor;
        }

        throw std::runtime_error("JSON parser reached the end of buffer at pos "s + std::to_string(cursor) +
            " while skipping insignificant whitespace segment."s +
            _pretty_error(cursor, this->chars));
    }
    std::pair<std::size_t, Node> _parser::parse_node(std::size_t cursor) {
        using namespace std::string_literals;

        // Node selector assumes it is starting at a significant symbol
        // which is the first symbol of the node to be parsed

        const char c = this->chars[cursor];

        // Assuming valid JSON, we can determine node type based on a single first character
        if (c == '{') {
            return this->parse_object(cursor);
        } else if (c == '[') {
            return this->parse_array(cursor);
        } else if (c == '"') {
            return this->parse_string(cursor);
        } else if (('0' <= c && c <= '9') || (c == '-')) {
            auto [ptr, number] = this->parse_number(cursor);
            switch (number.index()) {
            case 0:
                return { ptr, std::get<Integral>(number) };
            case 1:
                return { ptr, std::get<Floating>(number) };
            }
        } else if (c == 't') {
            return this->parse_true(cursor);
        } else if (c == 'f') {
            return this->parse_false(cursor);
        } else if (c == 'n') {
            return this->parse_null(cursor);
        }
        throw std::runtime_error(
            "JSON node selector encountered unexpected marker symbol {"s + this->chars[cursor] +
            "} at pos "s + std::to_string(cursor) + " (should be one of {0123456789{[\"tfn})."s +
            _pretty_error(cursor, this->chars));

        // Note: using a lookup table instead of an 'if' chain doesn't seem to offer any performance benefits here
    }
    std::size_t _parser::parse_object_pair(std::size_t cursor, Object& parent) {
        using namespace std::string_literals;

        // Object pair parser assumes it is starting at a '\"'

        // Parse pair key
        std::string key; // allocating a string here is fine since we will std::move() it into a map key
        std::tie(cursor, key) = this->parse_string(cursor);

        // Handle stuff in between
        cursor = this->skip_nonsignificant_whitespace(cursor);
        if (this->chars[cursor] != ':')
            throw std::runtime_error("JSON object node encountered unexpected symbol {"s + this->chars[cursor] +
                "} after the pair key at pos "s + std::to_string(cursor) + " (should be {:})."s +
                _pretty_error(cursor, this->chars));
        ++cursor; // move past the colon ':'
        cursor = this->skip_nonsignificant_whitespace(cursor);

        // Parse pair value
        if (++this->recursion_depth > this->recursion_limit)
            throw std::runtime_error("JSON parser has exceeded maximum allowed recursion depth of "s +
                std::to_string(this->recursion_limit) +
                ". If stated depth wasn't caused by an invalid input, "s +
                "recursion limit can be increased with json::set_recursion_limit()."s);

        Node value;
        std::tie(cursor, value) = this->parse_node(cursor);

        --this->recursion_depth;

        // Note 1:
        // Whether JSON allows duplicate keys is non-trivial but the resulting answer is YES.
        // JSON is governed by 2 standards:
        // 1) ECMA-404 https://ecma-international.org/wp-content/uploads/ECMA-404.pdf
        //    which doesn't say anything about duplicate kys
        // 2) RFC-8259 https://www.rfc-editor.org/rfc/rfc8259
        //    which states "The names within an object SHOULD be unique.",
        //    however as defined in RFC-2119 https://www.rfc-editor.org/rfc/rfc2119:
        //       "SHOULD This word, or the adjective "RECOMMENDED", mean that there may exist valid reasons in
        //       particular circumstances to ignore a particular item, but the full implications must be understood
        //       and carefully weighed before choosing a different course."
        // which means at the end of the day duplicate keys are discouraged but still valid

        // Note 2:
        // There is no standard specification on which JSON value should be preferred in case of duplicate keys.
        // This is considered implementation detail as per RFC-8259:
        //    "An object whose names are all unique is interoperable in the sense that all software implementations
        //    receiving that object will agree on the name-value mappings. When the names within an object are not
        //    unique, the behavior of software that receives such an object is unpredictable. Many implementations
        //    report the last name/value pair only. Other implementations report an error or fail to parse the object,
        //    and some implementations report all the name/value pairs, including duplicates."

        // Note 3:
        // We could easily check for duplicate keys since 'std::map<>::emplace()' returns insertion success as a bool
        // (the same isn't true for 'std::map<>::emplace_hint()' which returns just the iterator), however we will
        // not since that goes against the standard

        // Note 4:
        // 'parent.emplace_hint(parent.end(), ...)' can drastically speed up parsing of sorted JSON objects, however
        // since most JSONs in the wild aren't sorted we will resort to a more generic option of regular '.emplace()'
        parent.try_emplace(std::move(key), std::move(value));

        return cursor;
    }
    std::pair<std::size_t, Object> _parser::parse_object(std::size_t cursor) {
        using namespace std::string_literals;

        ++cursor; // move past the opening brace '{'

        // Empty object that will accumulate child nodes as we parse them
        Object object_value;

        // Handle 1st pair
        cursor = this->skip_nonsignificant_whitespace(cursor);
        if (this->chars[cursor] != '}') {
            cursor = this->parse_object_pair(cursor, object_value);
        } else {
            ++cursor; // move past the closing brace '}'
            return { cursor, std::move(object_value) };
        }

        // Handle other pairs

        // Since we are staring past the first pair, all following pairs are going to be preceded by a comma.
        //
        // Strictly speaking, commas in objects aren't necessary for decoding a JSON, this is
        // a case of redundant information, included into the format to make it more human-readable.
        // { "key_1":1 "key_1":2 "key_3":"value" } <- enough information to parse without commas.
        //
        // However, commas ARE in fact necessary for array parsing. By using commas to detect when we have
        // to parse another pair, we can reuse the same algorithm for both objects pairs and array elements.
        //
        // Doing so also has a benefit of inherently adding comma-presence validation which we would have
        // to do manually otherwise.

        while (cursor < this->chars.size()) {
            cursor = this->skip_nonsignificant_whitespace(cursor);
            const char c = this->chars[cursor];

            if (c == ',') {
                ++cursor; // move past the comma ','
                cursor = this->skip_nonsignificant_whitespace(cursor);
                cursor = this->parse_object_pair(cursor, object_value);
            } else if (c == '}') {
                ++cursor; // move past the closing brace '}'
                return { cursor, std::move(object_value) };
            } else {
                throw std::runtime_error(
                    "JSON array node could not find comma {,} or object ending symbol {}} after the element at pos "s
                    +
                    std::to_string(cursor) + "."s + _pretty_error(cursor, this->chars));
            }
        }

        throw std::runtime_error("JSON object node reached the end of buffer while parsing object contents." +
            _pretty_error(cursor, this->chars));
    }
    std::size_t _parser::parse_array_element(std::size_t cursor, Array& parent) {
        using namespace std::string_literals;

        // Array element parser assumes it is starting at the first symbol of some JSON node

        // Parse pair key
        if (++this->recursion_depth > this->recursion_limit)
            throw std::runtime_error("JSON parser has exceeded maximum allowed recursion depth of "s +
                std::to_string(this->recursion_limit) +
                ". If stated depth wasn't caused by an invalid input, "s +
                "recursion limit can be increased with json::set_recursion_limit()."s);

        Node value;
        std::tie(cursor, value) = this->parse_node(cursor);

        --this->recursion_depth;

        parent.emplace_back(std::move(value));

        return cursor;
    }
    std::pair<std::size_t, Array> _parser::parse_array(std::size_t cursor) {
        using namespace std::string_literals;

        ++cursor; // move past the opening bracket '['

        // Empty array that will accumulate child nodes as we parse them
        Array array_value;

        // Handle 1st pair
        cursor = this->skip_nonsignificant_whitespace(cursor);
        if (this->chars[cursor] != ']') {
            cursor = this->parse_array_element(cursor, array_value);
        } else {
            ++cursor; // move past the closing bracket ']'
            return { cursor, std::move(array_value) };
        }

        // Handle other pairs
        // (the exact same way we do with objects, see the note here)
        while (cursor < this->chars.size()) {
            cursor = this->skip_nonsignificant_whitespace(cursor);
            const char c = this->chars[cursor];

            if (c == ',') {
                ++cursor; // move past the comma ','
                cursor = this->skip_nonsignificant_whitespace(cursor);
                cursor = this->parse_array_element(cursor, array_value);
            } else if (c == ']') {
                ++cursor; // move past the closing bracket ']'
                return { cursor, std::move(array_value) };
            } else {
                throw std::runtime_error(
                    "JSON array node could not find comma {,} or array ending symbol {]} after the element at pos "s
                    +
                    std::to_string(cursor) + "."s + _pretty_error(cursor, this->chars));
            }
        }

        throw std::runtime_error("JSON array node reached the end of buffer while parsing object contents." +
            _pretty_error(cursor, this->chars));
    }
    std::size_t _parser::parse_escaped_unicode_codepoint(std::size_t cursor, std::string& string_value) {
        using namespace std::string_literals;

        // Note 1:
        // 4 hex digits can encode every character in a basic multilingual plane, to properly encode all valid unicode
        // chars 6 digits are needed. If JSON was a bit better we would have a longer escape sequence like '\Uxxxxxx',
        // (the way ECMAScript, Python and C++ do it), but for historical reasons longer codepoints are instead
        // represented using a UTF-16 surrogate pair like this: '\uxxxx\uxxxx'. The way such pair can be distinguished
        // from 2 independent codepoints is by checking a range of the first codepoint: values from 'U+D800' to 'U+DFFF'
        // are reserved for surrogate pairs. This is abhorrent and makes implementation twice as cumbersome, but we
        // gotta to do it in order to be standard-compliant.

        // Note 2:
        // 1st surrogate contains high bits, 2nd surrogate contains low bits.

        const auto throw_parsing_error = [&](std::string_view hex) {
            throw std::runtime_error("JSON string node could not parse unicode codepoint {"s + std::string(hex) +
                "} while parsing an escape sequence at pos "s + std::to_string(cursor) + "."s +
                _pretty_error(cursor, this->chars));
        };

        const auto throw_surrogate_error = [&](std::string_view hex) {
            throw std::runtime_error("JSON string node encountered invalid unicode escape sequence in " +
                "secong half of UTF-16 surrogate pair starting at {"s + std::string(hex) +
                "} while parsing an escape sequence at pos "s + std::to_string(cursor) + "."s +
                _pretty_error(cursor, this->chars));
        };

        const auto throw_end_of_buffer_error = [&]() {
            throw std::runtime_error("JSON string node reached the end of buffer while "s +
                "parsing a unicode escape sequence at pos "s + std::to_string(cursor) + "."s +
                _pretty_error(cursor, this->chars));
        };

        const auto throw_end_of_buffer_error_for_pair = [&]() {
            throw std::runtime_error("JSON string node reached the end of buffer while "s +
                "parsing a unicode escape sequence surrogate pair at pos "s +
                std::to_string(cursor) + "."s + _pretty_error(cursor, this->chars));
        };

        const auto parse_utf16 = [&](std::string_view hex) -> std::uint16_t {
            std::uint16_t utf16 {};
            const auto [end_ptr, error_code] = std::from_chars(hex.data(), hex.data() + hex.size(), utf16, 16);

            const bool sequence_is_valid = (error_code == std::errc {});
            const bool sequence_parsed_fully = (end_ptr == hex.data() + hex.size());

            if (!sequence_is_valid || !sequence_parsed_fully) throw_parsing_error(hex);

            return utf16;
        };

        // | '\uxxxx\uxxxx' | '\uxxxx\uxxxx'   | '\uxxxx\uxxxx' | '\uxxxx\uxxxx'   | '\uxxxx\uxxxx'  |
        // |   ^            |    ^             |       ^        |          ^       |             ^   |
        // | start (+0)     | hex_1_start (+1) | hex_1_end (+4) | hex_2_start (+7) | hex_2_end (+10) |
        constexpr std::size_t hex_1_start = 1;
        constexpr std::size_t hex_1_end = 4;
        constexpr std::size_t hex_2_backslash = 5;
        constexpr std::size_t hex_2_prefix = 6;
        constexpr std::size_t hex_2_start = 7;
        constexpr std::size_t hex_2_end = 10;

        const auto start = this->chars.data() + cursor;

        if (cursor + hex_1_end >= this->chars.size()) throw_end_of_buffer_error();

        const std::string_view hex_1(start + hex_1_start, 4);
        const std::uint16_t utf16_1 = parse_utf16(hex_1);

        // Surrogate pair case
        if (0xD800 <= utf16_1 && utf16_1 <= 0xDFFF) {
            if (cursor + hex_2_end >= this->chars.size()) throw_end_of_buffer_error_for_pair();
            if (start[hex_2_backslash] != '\\') throw_surrogate_error(hex_1);
            if (start[hex_2_prefix] != 'u') throw_surrogate_error(hex_1);

            const std::string_view hex_2(start + hex_2_start, 4);
            const std::uint16_t utf16_2 = parse_utf16(hex_2);

            const std::uint32_t codepoint = _utf16_pair_to_codepoint(utf16_1, utf16_2);
            if (!_codepoint_to_utf8(string_value, codepoint)) throw_parsing_error(hex_1);
            return cursor + hex_2_end;
        }
        // Regular case
        else {
            const std::uint32_t codepoint = utf16_1;
            if (!_codepoint_to_utf8(string_value, codepoint)) throw_parsing_error(hex_1);
            return cursor + hex_1_end;
        }
    }
    std::pair<std::size_t, String> _parser::parse_string(std::size_t cursor) {
        using namespace std::string_literals;

        // Empty string that will accumulate characters as we parse them
        std::string string_value;

        ++cursor; // move past the opening quote '\"'

        // Serialize string while handling escape sequences.
        //
        // Doing 'string_value += c' for every char is ~50-60% slower than appending whole string at once,
        // which is why we 'buffer' appends by keeping track of 'segment_start' and 'cursor', and appending
        // whole chunks of the buffer to 'string_value' when we encounter an escape sequence or end of the string.
        //
        for (std::size_t segment_start = cursor; cursor < this->chars.size(); ++cursor) {
            const char c = this->chars[cursor];

            // Reached the end of the string
            if (c == '"') {
                string_value.append(this->chars.data() + segment_start, cursor - segment_start);
                ++cursor; // move past the closing quote '\"'
                return { cursor, std::move(string_value) };
            }
            // Handle escape sequences inside the string
            else if (c == '\\') {
                ++cursor; // move past the backslash '\'

                string_value.append(this->chars.data() + segment_start, cursor - segment_start - 1);
                // can't buffer more than that since we have to insert special characters now

                if (cursor >= this->chars.size())
                    throw std::runtime_error("JSON string node reached the end of buffer while"s +
                        "parsing an escape sequence at pos "s + std::to_string(cursor) + "."s +
                        _pretty_error(cursor, this->chars));

                const char escaped_char = this->chars[cursor];

                // 2-character escape sequences
                if (const char replacement_char = _lookup_parsed_escaped_chars[_u8(escaped_char)]) {
                    string_value += replacement_char;
                }
                // 6/12-character escape sequences (escaped unicode HEX codepoints)
                else if (escaped_char == 'u') {
                    cursor = this->parse_escaped_unicode_codepoint(cursor, string_value);
                    // moves past first 'uXXX' symbols, last symbol will be covered by the loop '++cursor',
                    // in case of paired hexes moves past the second hex too
                } else {
                    throw std::runtime_error("JSON string node encountered unexpected character {"s +
                        std::string { escaped_char } + "} while parsing an escape sequence at pos "s +
                        std::to_string(cursor) + "."s + _pretty_error(cursor, this->chars));
                }

                // This covers all non-hex escape sequences according to ECMA-404 specification
                // [https://ecma-international.org/wp-content/uploads/ECMA-404.pdf] (page 4)

                // moving past the escaped character will be done by the loop '++cursor'
                segment_start = cursor + 1;
                continue;
            }
            // Reject unescaped control characters (codepoints U+0000 to U+001F)
            else if (_u8(c) <= 31)
                throw std::runtime_error(
                    "JSON string node encountered unescaped ASCII control character character \\"s +
                    std::to_string(static_cast<int>(c)) + " at pos "s + std::to_string(cursor) + "."s +
                    _pretty_error(cursor, this->chars));
        }

        throw std::runtime_error("JSON string node reached the end of buffer while parsing string contents." +
            _pretty_error(cursor, this->chars));
    }
    std::pair<std::size_t, std::variant<Integral, Floating>> _parser::parse_number(std::size_t cursor) {
        using namespace std::string_literals;

        std::variant<Integral, Floating> number_value;

        std::size_t offset = 0;
        bool is_floating = false;
        char c;
        do {
            c = this->chars[cursor + offset];
            offset++;

            if (c == '.')
                is_floating = true;
        } while ((c >= '0' && c <= '9') || c == '-');

        std::from_chars_result result;
        if (is_floating) {
            Floating floating_value;
            result = std::from_chars(
                this->chars.data() + cursor,
                this->chars.data() + this->chars.size(),
                floating_value
            );
            number_value = floating_value;
        } else {
            Integral integral_value;
            result = std::from_chars(
                this->chars.data() + cursor,
                this->chars.data() + this->chars.size(),
                integral_value
            );
            number_value = integral_value;
        }

        // Note:
        // std::from_chars() converts the first complete number it finds in the string,
        // for example "42 meters" would be converted to 42. We rely on that behaviour here.

        if (result.ec != std::errc {}) {
            // std::errc(0) is a valid enumeration value that represents success
            // even though it does not appear in the enumerator list (which starts at 1)
            if (result.ec == std::errc::invalid_argument)
                throw std::runtime_error("JSON floating node could not be parsed as a number at pos "s +
                    std::to_string(cursor) + "."s + _pretty_error(cursor, this->chars));
            else if (result.ec == std::errc::result_out_of_range)
                throw std::runtime_error(
                    "JSON number node parsed to floating larger than its possible binary representation at pos "s +
                    std::to_string(cursor) + "."s + _pretty_error(cursor, this->chars));
        }

        return { result.ptr - this->chars.data(), number_value };
    }
    std::pair<std::size_t, Bool> _parser::parse_true(std::size_t cursor) {
        using namespace std::string_literals;
        constexpr std::size_t token_length = 4;

        if (cursor + token_length > this->chars.size())
            throw std::runtime_error("JSON bool node reached the end of buffer while parsing {true}." +
                _pretty_error(cursor, this->chars));

        const bool parsed_correctly =         //
            this->chars[cursor + 0] == 't' && //
            this->chars[cursor + 1] == 'r' && //
            this->chars[cursor + 2] == 'u' && //
            this->chars[cursor + 3] == 'e';   //

        if (!parsed_correctly)
            throw std::runtime_error(
                "JSON bool node could not parse {true} at pos "s + std::to_string(cursor) + "."s +
                _pretty_error(cursor, this->chars));

        return { cursor + token_length, Bool(true) };
    }
    std::pair<std::size_t, Bool> _parser::parse_false(std::size_t cursor) {
        using namespace std::string_literals;
        constexpr std::size_t token_length = 5;

        if (cursor + token_length > this->chars.size())
            throw std::runtime_error("JSON bool node reached the end of buffer while parsing {false}." +
                _pretty_error(cursor, this->chars));

        const bool parsed_correctly =         //
            this->chars[cursor + 0] == 'f' && //
            this->chars[cursor + 1] == 'a' && //
            this->chars[cursor + 2] == 'l' && //
            this->chars[cursor + 3] == 's' && //
            this->chars[cursor + 4] == 'e';   //

        if (!parsed_correctly)
            throw std::runtime_error(
                "JSON bool node could not parse {false} at pos "s + std::to_string(cursor) + "."s +
                _pretty_error(cursor, this->chars));

        return { cursor + token_length, Bool(false) };
    }
    std::pair<std::size_t, Null> _parser::parse_null(std::size_t cursor) {
        using namespace std::string_literals;
        constexpr std::size_t token_length = 4;

        if (cursor + token_length > this->chars.size())
            throw std::runtime_error("JSON null node reached the end of buffer while parsing {null}." +
                _pretty_error(cursor, this->chars));

        const bool parsed_correctly =         //
            this->chars[cursor + 0] == 'n' && //
            this->chars[cursor + 1] == 'u' && //
            this->chars[cursor + 2] == 'l' && //
            this->chars[cursor + 3] == 'l';   //

        if (!parsed_correctly)
            throw std::runtime_error(
                "JSON null node could not parse {null} at pos "s + std::to_string(cursor) + "."s +
                _pretty_error(cursor, this->chars));

        return { cursor + token_length, Null() };
    }
}

// ==============================
// --- JSON Serializing impl. ---
// ==============================

namespace utl::json {
    // First indent should be skipped when printing after a key
    //
    // Example:
    //
    // {
    //     "object": {              <- first indent skipped (Object)
    //         "something": null    <- first indent skipped (Null)
    //     },
    //     "array": [               <- first indent skipped (Array)
    //          1,                  <- first indent NOT skipped (Number)
    //          2                   <- first indent NOT skipped (Number)
    //     ]
    // }
    //

    // We handle 'prettify' segments through 'if constexpr'
    // to avoid  any "trace" overhead on non-prettified serializing

    // Note:
    // The fastest way to append strings to a preallocated buffer seems to be with '+=':
    //    > chars += string_1; chars += string_2; chars += string_3;
    //
    // Using operator '+' slows things down due to additional allocations:
    //    > chars +=  string_1 + string_2 + string_3; // slow
    //
    // '.append()' performs exactly the same as '+=', but has no overload for appending single chars.
    // However, it does have an overload for appending N of some character, which is why we use if for indentation.
    //
    // 'std::ostringstream' is painfully slow compared to regular appends
    // so it's out of the question.

    void _serialize_json_recursion_minimized(
        const Node& node,
        std::string& chars,
        unsigned int indent_level,
        bool skip_first_indent) {
        using namespace std::string_literals;

        // JSON Object
        if (auto* ptr = node.get_if<Object>()) {
            const auto& object_value = *ptr;

            // Skip all logic for empty objects
            if (object_value.empty()) {
                chars += "{}";
                return;
            }

            chars += '{';

            for (auto it = object_value.cbegin();;) {
                // Key
                chars += '"';
                chars += it->first;
                chars += "\":";
                // Value
                _serialize_json_recursion_minimized(it->second, chars, indent_level + 1, true);
                // Comma
                if (++it != object_value.cend()) { // prevents trailing comma
                    chars += ',';
                } else
                    break;
            }

            chars += '}';
        }
        // JSON Array
        else if (auto* ptr = node.get_if<Array>()) {
            const auto& array_value = *ptr;

            // Skip all logic for empty arrays
            if (array_value.empty()) {
                chars += "[]";
                return;
            }

            chars += '[';

            for (auto it = array_value.cbegin();;) {
                //Node
                _serialize_json_recursion_minimized(*it, chars, indent_level + 1);
                // Comma
                if (++it != array_value.cend()) { // prevents trailing comma
                    chars += ',';
                } else {
                    break;
                }
            }
            chars += ']';
        }
        // String
        else if (auto* ptr = node.get_if<String>()) {
            const auto& string_value = *ptr;

            chars += '"';

            // Serialize string while handling escape sequences.
            /// Without escape sequences we could just do 'chars += string_value'.
            //
            // Since appending individual characters is ~twice as slow as appending the whole string, we use a
            // "buffered" way of appending, appending whole segments up to the currently escaped char.
            // Strings with no escaped chars get appended in a single call.
            //
            std::size_t segment_start = 0;
            for (std::size_t i = 0; i < string_value.size(); ++i) {
                if (const char escaped_char_replacement = _lookup_serialized_escaped_chars[_u8(string_value[i])]) {
                    chars.append(string_value.data() + segment_start, i - segment_start);
                    chars += '\\';
                    chars += escaped_char_replacement;
                    segment_start = i + 1; // skip over the "actual" technical character in the string
                }
            }
            chars.append(string_value.data() + segment_start, string_value.size() - segment_start);

            chars += '"';
        }
        // Integral
        else if (auto* ptr = node.get_if<Integral>()) {
            const auto& integral_value = *ptr;

            std::array<char, 20> buffer; // "-9223372036854775808".size()

            const auto [number_end_ptr, error_code] =
                std::to_chars(buffer.data(), buffer.data() + buffer.size(), integral_value);

            if (error_code != std::errc {})
                throw std::runtime_error(
                    "JSON serializing encountered std::to_chars() formatting error while serializing value {"s +
                    std::to_string(integral_value) + "}."s);

            //const std::string_view number_string(buffer.data(), number_end_ptr - buffer.data());

            chars.append(buffer.data(), number_end_ptr - buffer.data());
        }
        // Floating
        else if (auto* ptr = node.get_if<Floating>()) {
            const auto& floating_value = *ptr;

            constexpr int max_exponent = std::numeric_limits<Floating>::max_exponent10;
            constexpr int max_digits =
                4 + std::numeric_limits<Floating>::max_digits10 + std::max(2, _log_10_ceil(max_exponent));
            // should be the smallest buffer size to account for all possible 'std::to_chars()' outputs,
            // see [https://stackoverflow.com/questions/68472720/stdto-chars-minimal-floating-point-buffer-size]

            std::array<char, max_digits> buffer;

            const auto [number_end_ptr, error_code] =
                std::to_chars(buffer.data(), buffer.data() + buffer.size(), floating_value);

            if (error_code != std::errc {})
                throw std::runtime_error(
                    "JSON serializing encountered std::to_chars() formatting error while serializing value {"s +
                    std::to_string(floating_value) + "}."s);

            //const std::string_view number_string(buffer.data(), number_end_ptr - buffer.data());

            // Save NaN/Inf cases as strings, since JSON spec doesn't include IEEE 754.
            // (!) May result in non-homogenous arrays like [ 1.0, "inf" , 3.0, 4.0, "nan" ]
            if (std::isfinite(floating_value)) {
                chars.append(buffer.data(), number_end_ptr - buffer.data());
            } else {
                chars += '"';
                chars.append(buffer.data(), number_end_ptr - buffer.data());
                chars += '"';
            }
        }
        // Bool
        else if (auto* ptr = node.get_if<Bool>()) {
            const auto& bool_value = *ptr;
            chars += (bool_value ? "true" : "false");
        }
        // Null
        else if (node.is<Null>()) {
            chars += "null";
        }
    }

    void _serialize_json_recursion_pretty(
        const Node& node,
        std::string& chars,
        unsigned int indent_level,
        bool skip_first_indent) {
        using namespace std::string_literals;
        constexpr std::size_t indent_level_size = 4;
        const std::size_t indent_size = indent_level_size * indent_level;

        if (!skip_first_indent) chars.append(indent_size, ' ');

        // JSON Object
        if (auto* ptr = node.get_if<Object>()) {
            const auto& object_value = *ptr;

            // Skip all logic for empty objects
            if (object_value.empty()) {
                chars += "{}";
                return;
            }

            chars += '{';
            chars += '\n';

            for (auto it = object_value.cbegin();;) {
                chars.append(indent_size + indent_level_size, ' ');
                // Key
                chars += '"';
                chars += it->first;
                chars += "\": ";
                // Value
                _serialize_json_recursion_pretty(it->second, chars, indent_level + 1, true);
                // Comma
                if (++it != object_value.cend()) { // prevents trailing comma
                    chars += ',';
                    chars += '\n';
                } else {
                    chars += '\n';
                    break;
                }
            }

            chars.append(indent_size, ' ');
            chars += '}';
        }
        // JSON Array
        else if (auto* ptr = node.get_if<Array>()) {
            const auto& array_value = *ptr;

            // Skip all logic for empty arrays
            if (array_value.empty()) {
                chars += "[]";
                return;
            }

            chars += '[';
            chars += '\n';

            for (auto it = array_value.cbegin();;) {
                // Node
                _serialize_json_recursion_pretty(*it, chars, indent_level + 1);
                // Comma
                if (++it != array_value.cend()) { // prevents trailing comma
                    chars += ',';
                    chars += '\n';
                } else {
                    chars += '\n';
                    break;
                }
            }
            chars.append(indent_size, ' ');
            chars += ']';
        }
        // String
        else if (auto* ptr = node.get_if<String>()) {
            const auto& string_value = *ptr;

            chars += '"';

            // Serialize string while handling escape sequences.
            /// Without escape sequences we could just do 'chars += string_value'.
            //
            // Since appending individual characters is ~twice as slow as appending the whole string, we use a
            // "buffered" way of appending, appending whole segments up to the currently escaped char.
            // Strings with no escaped chars get appended in a single call.
            //
            std::size_t segment_start = 0;
            for (std::size_t i = 0; i < string_value.size(); ++i) {
                if (const char escaped_char_replacement = _lookup_serialized_escaped_chars[_u8(string_value[i])]) {
                    chars.append(string_value.data() + segment_start, i - segment_start);
                    chars += '\\';
                    chars += escaped_char_replacement;
                    segment_start = i + 1; // skip over the "actual" technical character in the string
                }
            }
            chars.append(string_value.data() + segment_start, string_value.size() - segment_start);

            chars += '"';
        }
        // Integral
        else if (auto* ptr = node.get_if<Integral>()) {
            const auto& integral_value = *ptr;

            std::array<char, 20> buffer; // "-9223372036854775808".size()

            const auto [number_end_ptr, error_code] =
                std::to_chars(buffer.data(), buffer.data() + buffer.size(), integral_value);

            if (error_code != std::errc {})
                throw std::runtime_error(
                    "JSON serializing encountered std::to_chars() formatting error while serializing value {"s +
                    std::to_string(integral_value) + "}."s);

            //const std::string_view number_string(buffer.data(), number_end_ptr - buffer.data());

            chars.append(buffer.data(), number_end_ptr - buffer.data());
        }
        // Floating
        else if (auto* ptr = node.get_if<Floating>()) {
            const auto& floating_value = *ptr;

            constexpr int max_exponent = std::numeric_limits<Floating>::max_exponent10;
            constexpr int max_digits =
                4 + std::numeric_limits<Floating>::max_digits10 + std::max(2, _log_10_ceil(max_exponent));
            // should be the smallest buffer size to account for all possible 'std::to_chars()' outputs,
            // see [https://stackoverflow.com/questions/68472720/stdto-chars-minimal-floating-point-buffer-size]

            std::array<char, max_digits> buffer;

            const auto [number_end_ptr, error_code] =
                std::to_chars(buffer.data(), buffer.data() + buffer.size(), floating_value);

            if (error_code != std::errc {})
                throw std::runtime_error(
                    "JSON serializing encountered std::to_chars() formatting error while serializing value {"s +
                    std::to_string(floating_value) + "}."s);

            //const std::string_view number_string(buffer.data(), number_end_ptr - buffer.data());

            // Save NaN/Inf cases as strings, since JSON spec doesn't include IEEE 754.
            // (!) May result in non-homogenous arrays like [ 1.0, "inf" , 3.0, 4.0, "nan" ]
            if (std::isfinite(floating_value)) {
                chars.append(buffer.data(), number_end_ptr - buffer.data());
            } else {
                chars += '"';
                chars.append(buffer.data(), number_end_ptr - buffer.data());
                chars += '"';
            }
        }
        // Bool
        else if (auto* ptr = node.get_if<Bool>()) {
            const auto& bool_value = *ptr;
            chars += (bool_value ? "true" : "false");
        }
        // Null
        else if (node.is<Null>()) {
            chars += "null";
        }
    }
}

// ===============================
// --- JSON Parsing public API ---
// ===============================

namespace utl::json {
    Node from_string(const std::string& chars, unsigned int recursion_limit) {
        _parser parser(chars, recursion_limit);
        const std::size_t json_start = parser.skip_nonsignificant_whitespace(0); // skip leading whitespace
        auto [end_cursor, node] = parser.parse_node(json_start); // starts parsing recursively from the root node

        // Check for invalid trailing symbols
        using namespace std::string_literals;

        for (auto cursor = end_cursor; cursor < chars.size(); ++cursor)
            if (!_lookup_whitespace_chars[_u8(chars[cursor])])
                throw std::runtime_error("Invalid trailing symbols encountered after the root JSON node at pos "s +
                    std::to_string(cursor) + "."s + _pretty_error(cursor, chars));

        return std::move(node); // implicit tuple blocks copy elision, we have to move() manually

        // Note: Some code analyzers detect 'return std::move(node)' as a performance issue, it is
        //       not, NOT having 'std::move()' on the other hand is very much a performance issue
    }
    Node from_file(const std::string& filepath, unsigned int recursion_limit) {
        const std::string chars = _read_file_to_string(filepath);
        return from_string(chars, recursion_limit);
    }

    Node literals::operator ""_utl_json(const char* c_str, std::size_t c_str_size) {
        return from_string(std::string(c_str, c_str_size));
    }
}
