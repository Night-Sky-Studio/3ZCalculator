#pragma once

namespace lib {
    // TPrimary suits for class name
    template<typename TPrimary, typename TSecondary>
    class IConverter {
    public:
        virtual ~IConverter() = default;

        virtual TPrimary from(const TSecondary& data) = 0;
    };
}
