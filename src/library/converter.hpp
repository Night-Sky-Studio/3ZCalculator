#pragma once

namespace lib {
    template<typename TResult, typename... TArgs>
    class IConverter {
    public:
        virtual ~IConverter() = default;

        virtual TResult from(const TArgs&... data) const = 0;
    };
}
