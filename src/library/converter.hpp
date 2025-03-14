#pragma once

namespace lib {
    // TPrimary suits for class name
    template<typename TResult, typename... TArgs>
    class IConverter {
    public:
        virtual ~IConverter() = default;

        virtual TResult from(const TArgs&... data) const = 0;
    };
}
