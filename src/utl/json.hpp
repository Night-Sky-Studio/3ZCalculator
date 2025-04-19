// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ DmitriBogdanov/UTL ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
// Module:        utl::json
// Documentation: https://github.com/DmitriBogdanov/UTL/blob/master/docs/module_json.md
// Source repo:   https://github.com/DmitriBogdanov/UTL
//
// This project is licensed under the MIT License
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#pragma once

// _______________________ INCLUDES _______________________

#include <cmath>            // isfinite()
#include <initializer_list> // initializer_list<>
#include <map>              // map<>
#include <string>           // string
#include <string_view>      // string_view
#include <type_traits>      // enable_if<>, void_t, is_convertible<>, is_same<>,
// conjunction<>, disjunction<>, negation<>
#include <utility>          // move(), declval<>()
#include <variant>          // variant<>
#include <vector>           // vector<>

// ____________________ DEVELOPER DOCS ____________________

// Reasonably simple (if we discound reflection) parser / serializer, doesn't use any intrinsics or compiler-specific
// stuff. Unlike some other implementations, doesn't include the tokenizing step - we parse everything in a single 1D
// scan over the data, constructing recursive JSON struct on the fly. The main reason we can do this so easily is
// due to a nice quirk of JSON: when parsing nodes, we can always determine node type based on a single first
// character, see '_parser::parse_node()'.
//
// Struct reflection is implemented through macros - alternative way would be to use templates with __PRETTY_FUNCTION__
// (or __FUNCSIG__) and do some constexpr string parsing to perform "magic" reflection without requiring macros, but
// that relies on the implementation-defined format of those strings and adds quite a lot more complexity.
// 'nlohmann_json' provides similar macros but also has a way of specializing things manually.
//
// Proper type traits and 'if constexpr' recursive introspection are a key to making APIs that can convert stuff
// between JSON and other types seamlessly, which is exactly what we do here, it even accounts for reflection.

// ____________________ IMPLEMENTATION ____________________

namespace utl::json {
    // ===================================
    // --- JSON type conversion traits ---
    // ===================================

    template<class T>
    using _object_type_impl = std::map<std::string, T, std::less<>>;
    // 'std::less<>' makes map transparent, which means we can use 'find()' for 'std::string_view' keys
    template<class T>
    using _array_type_impl = std::vector<T>;
    using _string_type_impl = std::string;
    using _integral_type_impl = int64_t;
    using _floating_type_impl = double;
    using _bool_type_impl = bool;
    struct _null_type_impl {
        [[nodiscard]] bool operator==(const _null_type_impl&) const noexcept {
            return true;
        } // so we can check 'Null == Null'
    };

    // ===================
    // --- Misc. utils ---
    // ===================

    // --- Type traits ---
    // -------------------

#define utl_json_define_trait(trait_name_, ...)                                                                        \
    template <class T, class = void>                                                                                   \
    struct trait_name_ : std::false_type {};                                                                           \
                                                                                                                       \
    template <class T>                                                                                                 \
    struct trait_name_<T, std::void_t<decltype(__VA_ARGS__)>> : std::true_type {};                                     \
                                                                                                                       \
    template <class T>                                                                                                 \
    constexpr bool trait_name_##_v = trait_name_<T>::value

    utl_json_define_trait(_has_begin, std::declval<std::decay_t<T>>().begin());
    utl_json_define_trait(_has_end, std::declval<std::decay_t<T>>().end());
    utl_json_define_trait(_has_input_it, std::next(std::declval<T>().begin()));

    utl_json_define_trait(_has_key_type, std::declval<typename std::decay_t<T>::key_type>());
    utl_json_define_trait(_has_mapped_type, std::declval<typename std::decay_t<T>::mapped_type>());

#undef utl_json_define_trait

    // --- Map-macro ---
    // -----------------

    // This is an implementation of a classic map-macro that applies some function macro
    // to all elements of __VA_ARGS__, it looks much uglier than usual because we have to prefix
    // everything with verbose 'utl_json_', but that's the price of avoiding name collisions.
    //
    // Created by William Swanson in 2012 and declared as public domain.
    //
    // Macro supports up to 365 arguments. We will need it for structure reflection.

#define utl_json_eval_0(...) __VA_ARGS__
#define utl_json_eval_1(...) utl_json_eval_0(utl_json_eval_0(utl_json_eval_0(__VA_ARGS__)))
#define utl_json_eval_2(...) utl_json_eval_1(utl_json_eval_1(utl_json_eval_1(__VA_ARGS__)))
#define utl_json_eval_3(...) utl_json_eval_2(utl_json_eval_2(utl_json_eval_2(__VA_ARGS__)))
#define utl_json_eval_4(...) utl_json_eval_3(utl_json_eval_3(utl_json_eval_3(__VA_ARGS__)))
#define utl_json_eval(...) utl_json_eval_4(utl_json_eval_4(utl_json_eval_4(__VA_ARGS__)))

#define utl_json_map_end(...)
#define utl_json_map_out
#define utl_json_map_comma ,

#define utl_json_map_get_end_2() 0, utl_json_map_end
#define utl_json_map_get_end_1(...) utl_json_map_get_end_2
#define utl_json_map_get_end(...) utl_json_map_get_end_1
#define utl_json_map_next_0(test, next, ...) next utl_json_map_out
#define utl_json_map_next_1(test, next) utl_json_map_next_0(test, next, 0)
#define utl_json_map_next(test, next) utl_json_map_next_1(utl_json_map_get_end test, next)

#define utl_json_map_0(f, x, peek, ...) f(x) utl_json_map_next(peek, utl_json_map_1)(f, peek, __VA_ARGS__)
#define utl_json_map_1(f, x, peek, ...) f(x) utl_json_map_next(peek, utl_json_map_0)(f, peek, __VA_ARGS__)

    // Resulting macro, applies the function macro 'f' to each of the remaining parameters
#define utl_json_map(f, ...)                                                                                           \
    utl_json_eval(utl_json_map_1(f, __VA_ARGS__, ()()(), ()()(), ()()(), 0)) static_assert(true)

    struct _dummy_type {};

    // 'possible_value_type<T>::value' evaluates to:
    //    - 'T::value_type' if 'T' has 'value_type'
    //    - '_dummy_type' otherwise
    template<class T, class = void>
    struct possible_value_type {
        using type = _dummy_type;
    };

    template<class T>
    struct possible_value_type<T, std::void_t<decltype(std::declval<typename std::decay_t<T>::value_type>())>> {
        using type = typename T::value_type;
    };

    // 'possible_mapped_type<T>::value' evaluates to:
    //    - 'T::mapped_type' if 'T' has 'mapped_type'
    //    - '_dummy_type' otherwise
    template<class T, class = void>
    struct possible_mapped_type {
        using type = _dummy_type;
    };

    template<class T>
    struct possible_mapped_type<T, std::void_t<decltype(std::declval<typename std::decay_t<T>::mapped_type>())>> {
        using type = typename T::mapped_type;
    };
    // these type traits are a key to checking properties of 'T::value_type' & 'T::mapped_type' for a 'T' which may or may
    // not have them (which is exactly the case with recursive traits that we're going to use later to deduce convertability
    // to recursive JSON). '_dummy_type' here is necessary to end the recursion of 'std::disjunction'

#define utl_json_type_trait_conjunction(trait_name_, ...)                                                              \
    template <class T>                                                                                                 \
    struct trait_name_ : std::conjunction<__VA_ARGS__> {};                                                             \
                                                                                                                       \
    template <class T>                                                                                                 \
    constexpr bool trait_name_##_v = trait_name_<T>::value

#define utl_json_type_trait_disjunction(trait_name_, ...)                                                              \
    template <class T>                                                                                                 \
    struct trait_name_ : std::disjunction<__VA_ARGS__> {};                                                             \
                                                                                                                       \
    template <class T>                                                                                                 \
    constexpr bool trait_name_##_v = trait_name_<T>::value

    // Note:
    // The reason we use 'struct trait_name : std::conjunction<...>' instead of 'using trait_name = std::conjunction<...>'
    // is because 1st option allows for recursive type traits, while 'using' syntax doesn't. We have some recursive type
    // traits here in form of 'is_json_type_convertible<>', which expands over the 'T' checking that 'T', 'T::value_type'
    // (if exists), 'T::mapped_type' (if exists) and their other layered value/mapped types are all satisfying the
    // necessary convertability trait. This allows us to make a trait which fully deduces whether some
    // complex datatype can be converted to a JSON recursively.

    utl_json_type_trait_conjunction(is_object_like, _has_begin<T>, _has_end<T>, _has_key_type<T>, _has_mapped_type<T>);
    utl_json_type_trait_conjunction(is_array_like, _has_begin<T>, _has_end<T>, _has_input_it<T>);
    utl_json_type_trait_conjunction(is_string_like, std::is_convertible<T, std::string_view>);
    utl_json_type_trait_conjunction(is_bool_like, std::is_same<T, _bool_type_impl>);
    utl_json_type_trait_conjunction(is_null_like, std::is_same<T, _null_type_impl>);

    utl_json_type_trait_disjunction(
        _is_directly_json_convertible,
        is_string_like<T>,
        is_bool_like<T>,
        std::is_integral<T>,
        std::is_floating_point<T>,
        is_null_like<T>);

    utl_json_type_trait_conjunction(
        is_json_convertible,
        std::disjunction<
        // either the type itself is convertible
        _is_directly_json_convertible<T>,
        // ... or it's an array of convertible elements
        std::conjunction<is_array_like<T>, is_json_convertible<typename possible_value_type<T>::type>>,
        // ... or it's an object of convertible elements
        std::conjunction<is_object_like<T>, is_json_convertible<typename possible_mapped_type<T>::type>>>,
        // end recursion by short-circuiting conjunction with 'false' once we arrive to '_dummy_type',
        // arriving here means the type isn't convertible to JSON
        std::negation<std::is_same<T, _dummy_type>>);

#undef utl_json_type_trait_conjunction
#undef utl_json_type_trait_disjunction

    // Workaround for 'static_assert(false)' making program ill-formed even
    // when placed inside an 'if constexpr' branch that never compiles.
    // 'static_assert(_always_false_v<T)' on the other hand doesn't,
    // which means we can use it to mark branches that should never compile.
    template<class>
    constexpr bool _always_false_v = false;

    // Note:
    // It is critical that '_object_type_impl' can be instantiated with incomplete type 'T'.
    // This allows us to declare recursive classes like this:
    //
    //    'struct Recursive { std::map<std::string, Recursive> data; }'
    //
    // Technically, there is nothing stopping any dynamically allocated container from supporting
    // incomplete types, since dynamic allocation inherently means pointer indirection at some point,
    // which makes 'sizeof(Container)' independent of 'T'.
    //
    // This requirement was only standardized for 'std::vector' and 'std::list' due to ABI breaking concerns.
    // 'std::map' is not required to support incomplete types by the standard, however in practice it does support them
    // on all compilers that I know of. Several other JSON libraries seem to rely on the same behaviour without any issues.
    // The same cannot be said about 'std::unordered_map', which is why we don't use it.
    //
    // We could make a more pedantic choice and add a redundant level of indirection, but that both complicates
    // implementation needlessly and reduces performance. A perfect solution would be to write our own map implementation
    // tailored for JSON use cases and providing explicit support for heterogeneous lookup and incomplete types, but that
    // alone would be grander in scale than this entire parser for a mostly non-critical benefit.

    // ==================
    // --- Node class ---
    // ==================

    enum class Format : uint8_t { PRETTY, MINIMIZED };

    enum class NodeType : uint8_t {
        NONE = 0, OBJECT = 1, ARRAY = 2, STRING = 3, INTEGRAL = 4, FLOATING = 5, BOOL = 6
    };

    class Node {
    public:
        using object_type = _object_type_impl<Node>;
        using array_type = _array_type_impl<Node>;
        using string_type = _string_type_impl;
        using integral_type = _integral_type_impl;
        using floating_type = _floating_type_impl;
        using bool_type = _bool_type_impl;
        using null_type = _null_type_impl;
    private:
        // 'null_type' should go first to ensure default-initialization creates 'null' nodes
        using variant_type = std::variant<
            null_type,
            object_type,
            array_type,
            string_type,
            integral_type,
            floating_type,
            bool_type>;

        variant_type _data {};
    public:
        // -- Getters --
        // -------------

        template<class T>
        [[nodiscard]] T& as() {
            return std::get<T>(this->_data);
        }

        template<class T>
        [[nodiscard]] const T& as() const {
            return std::get<T>(this->_data);
        }

        [[nodiscard]] object_type& as_object();
        [[nodiscard]] array_type& as_array();
        [[nodiscard]] string_type& as_string();
        [[nodiscard]] integral_type& as_integral();
        [[nodiscard]] floating_type& as_floating();
        [[nodiscard]] bool_type& as_bool();
        [[nodiscard]] null_type& as_null();

        [[nodiscard]] const object_type& as_object() const;
        [[nodiscard]] const array_type& as_array() const;
        [[nodiscard]] const string_type& as_string() const;
        [[nodiscard]] const integral_type& as_integral() const;
        [[nodiscard]] const floating_type& as_floating() const;
        [[nodiscard]] const bool_type& as_bool() const;
        [[nodiscard]] const null_type& as_null() const;

        template<class T>
        [[nodiscard]] bool is() const noexcept {
            return std::holds_alternative<T>(this->_data);
        }

        [[nodiscard]] bool is_object() const noexcept;
        [[nodiscard]] bool is_array() const noexcept;
        [[nodiscard]] bool is_string() const noexcept;
        [[nodiscard]] bool is_integral() const noexcept;
        [[nodiscard]] bool is_floating() const noexcept;
        [[nodiscard]] bool is_bool() const noexcept;
        [[nodiscard]] bool is_null() const noexcept;

        template<class T>
        [[nodiscard]] T* get_if() noexcept {
            return std::get_if<T>(&this->_data);
        }

        template<class T>
        [[nodiscard]] const T* get_if() const noexcept {
            return std::get_if<T>(&this->_data);
        }

        // -- Object methods ---
        // ---------------------

        // 'std::map<K, V>::operator[]()' and 'std::map<K, V>::at()' don't support
        // support heterogeneous lookup, we have to reimplement them manually
        Node& operator[](std::string_view key);

        // 'std::map<K, V>::operator[]()' and 'std::map<K, V>::at()' don't support
        // support heterogeneous lookup, we have to reimplement them manually
        [[nodiscard]] const Node& operator[](std::string_view key) const;

        // Non-const 'operator[]' inserts non-existent keys, '.at()' should throw instead
        [[nodiscard]] Node& at(std::string_view key);

        [[nodiscard]] const Node& at(std::string_view key) const;

        [[nodiscard]] bool contains(std::string_view key) const;

        // same thing as 'this->contains(key) ? json.at(key).get<T>() : else_value' but without a second map lookup
        template<class T>
        [[nodiscard]] const T& value_or(std::string_view key, const T& else_value) const {
            const auto& object = this->as_object();
            const auto it = object.find(std::string(key));
            if (it != object.end()) return it->second.as<T>();
            return else_value;
        }

        // -- Array methods ---
        // --------------------

        [[nodiscard]] Node& operator[](std::size_t pos);

        [[nodiscard]] const Node& operator[](std::size_t pos) const;

        [[nodiscard]] Node& at(std::size_t pos);

        [[nodiscard]] const Node& at(std::size_t pos) const;

        void push_back(const Node& node);

        void push_back(Node&& node);

        // -- Assignment --
        // ----------------

        // Converting assignment
        template<class T>
        Node& operator=(const T& value) {
            // Don't take types that decay to Node/object/array/string to prevent
            // shadowing native copy/move assignment for those types

            // Several "type-like" characteristics can be true at the same time,
            // to resolve ambiguity we assign the following conversion priority:
            // string > object > array > bool > null > floating > integral

            if constexpr (is_string_like_v<T>) {
                this->_data.emplace<string_type>(value);
            } else if constexpr (is_object_like_v<T>) {
                this->_data.emplace<object_type>();
                auto& object = this->as_object();
                for (const auto& [key, val] : value) object[key] = val;
            } else if constexpr (is_array_like_v<T>) {
                this->_data.emplace<array_type>();
                auto& array = this->as_array();
                for (const auto& elem : value) array.emplace_back(elem);
            } else if constexpr (is_bool_like_v<T>) {
                this->_data.emplace<bool_type>(value);
            } else if constexpr (is_null_like_v<T>) {
                this->_data.emplace<null_type>(value);
            } else if constexpr (std::is_floating_point_v<T>) {
                this->_data.emplace<floating_type>(value);
            } else if constexpr (std::is_integral_v<T>) {
                this->_data.emplace<integral_type>(value);
            } else {
                static_assert(_always_false_v<T>, "Method is a non-exhaustive visitor of std::variant<>.");
            }

            return *this;
        }

        // "native" copy/move semantics for types that support it
        Node& operator=(const object_type& value);
        Node& operator=(object_type&& value);

        Node& operator=(const array_type& value);
        Node& operator=(array_type&& value);

        Node& operator=(const string_type& value);
        Node& operator=(string_type&& value);

        // Support for 'std::initializer_list' type deduction,
        // (otherwise the call is ambiguous)
        template<class T>
        Node& operator=(std::initializer_list<T> ilist) {
            // We can't just do 'return *this = array_type(value);' because compiler doesn't realize it can
            // convert 'std::initializer_list<T>' to 'std::vector<Node>' for all 'T' convertable to 'Node',
            // we have to invoke 'Node()' constructor explicitly (here it happens in 'emplace_back()')
            array_type array_value;
            array_value.reserve(ilist.size());
            for (const auto& e : ilist) array_value.emplace_back(e);
            this->_data = std::move(array_value);
            return *this;
        }

        // -- Constructors --
        // ------------------

        Node& operator=(const Node&);
        Node& operator=(Node&&) noexcept;

        Node();
        Node(const Node&);
        Node(Node&&) noexcept;
        // Note:
        // We suffer a lot if 'object_type' move-constructor is not marked 'noexcept', if that's the case
        // 'Node' move-constructor doesn't get 'noexcept' either which means `std::vector<Node>` will copy
        // nodes instead of moving when it grows. 'std::map' is NOT required to be noexcept by the standard,
        // but it is marked as such in both 'libc++' and 'libstdc++', 'VS' stdlib lacks behind in that regard.
        // See noexcept status summary here: http://howardhinnant.github.io/container_summary.html

        Node(const object_type& value);
        Node(object_type&& value);
        Node(const array_type& value);
        Node(array_type&& value);
        Node(std::string_view value);
        Node(const string_type& value);
        Node(string_type&& value);
        Node(integral_type value);
        Node(floating_type value);
        Node(bool_type value);
        Node(null_type value);

        // --- JSON Serializing public API ---
        // -----------------------------------

        [[nodiscard]] std::string to_string(Format format = Format::PRETTY) const;

        void to_file(const std::string& filepath, Format format = Format::PRETTY) const;

        // --- Reflection ---
        // ------------------

        template<class T>
        [[nodiscard]] T to_struct() const {
            static_assert(
                _always_false_v<T>,
                "Provided type doesn't have a defined JSON reflection. Use 'UTL_JSON_REFLECT' macro to define one.");
            // compile-time protection against calling 'to_struct()' on types that don't have reflection,
            // we can also provide a proper error message here
            return {};
            // this is needed to silence "no return in a function" warning that appears even if this specialization
            // (which by itself should cause a compile error) doesn't get compiled

            // specializations of this template that will actually perform the conversion will be defined by
            // macros outside the class body, this is a perfectly legal thing to do, even if unintuitive compared
            // to non-template members, see https://en.cppreference.com/w/cpp/language/member_template
        }

        // --- Type Information ---
        // ------------------------

        [[nodiscard]] NodeType type() const;
    };

    // Public typedefs
    using Object = Node::object_type;
    using Array = Node::array_type;
    using String = Node::string_type;
    using Integral = Node::integral_type;
    using Floating = Node::floating_type;
    using Bool = Node::bool_type;
    using Null = Node::null_type;

    // ==========================
    // --- JSON Parsing impl. ---
    // ==========================

    constexpr unsigned int _default_recursion_limit = 1000;
    // this recursion limit applies only to parsing from text, conversions from
    // structs & containers are a separate thing and don't really need it as much

    struct _parser {
        const std::string& chars;
        unsigned int recursion_limit;
        unsigned int recursion_depth = 0;
        // we track recursion depth to handle stack allocation errors
        // (this can be caused malicious inputs with extreme level of nesting, for example, 100k array
        // opening brackets, which would cause huge recursion depth causing the stack to overflow with SIGSEGV)

        // dynamic allocation errors can be handled with regular exceptions through std::bad_alloc

        _parser() = delete;
        _parser(const std::string& chars, unsigned int& recursion_limit);

        // Parser state
        std::size_t skip_nonsignificant_whitespace(std::size_t cursor);

        // Parsing methods
        std::pair<std::size_t, Node> parse_node(std::size_t cursor);

        std::size_t parse_object_pair(std::size_t cursor, Object& parent);

        std::pair<std::size_t, Object> parse_object(std::size_t cursor);

        std::size_t parse_array_element(std::size_t cursor, Array& parent);

        std::pair<std::size_t, Array> parse_array(std::size_t cursor);

        inline std::size_t parse_escaped_unicode_codepoint(std::size_t cursor, std::string& string_value);

        std::pair<std::size_t, String> parse_string(std::size_t cursor);

        std::pair<std::size_t, std::variant<Integral, Floating>> parse_number(std::size_t cursor);

        std::pair<std::size_t, Bool> parse_true(std::size_t cursor);

        std::pair<std::size_t, Bool> parse_false(std::size_t cursor);

        std::pair<std::size_t, Null> parse_null(std::size_t cursor);
    };

    // ==============================
    // --- JSON Serializing impl. ---
    // ==============================

    void _serialize_json_recursion_minimized(
        const Node& node,
        std::string& chars,
        unsigned int indent_level = 0,
        bool skip_first_indent = false);
    void _serialize_json_recursion_pretty(
        const Node& node,
        std::string& chars,
        unsigned int indent_level = 0,
        bool skip_first_indent = false);

    template<bool prettify>
    void _serialize_json_recursion(
        const Node& node,
        std::string& chars,
        unsigned int indent_level = 0,
        bool skip_first_indent = false) {
        if constexpr (prettify)
            return _serialize_json_recursion_pretty(node, chars, indent_level, skip_first_indent);
        else
            return _serialize_json_recursion_minimized(node, chars, indent_level, skip_first_indent);
    }

    // ===============================
    // --- JSON Parsing public API ---
    // ===============================

    [[nodiscard]] Node from_string(const std::string& chars,
        unsigned int recursion_limit = _default_recursion_limit);
    [[nodiscard]] Node from_file(const std::string& filepath,
        unsigned int recursion_limit = _default_recursion_limit);

    namespace literals {
        [[nodiscard]] Node operator""_utl_json(const char* c_str, std::size_t c_str_size);
    } // namespace literals
}

namespace utl {
    using JsonType = json::NodeType;
    using Json = json::Node;
}
