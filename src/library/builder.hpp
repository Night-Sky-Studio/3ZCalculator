#pragma once

//std
#include <memory>

namespace lib {
    template<typename T>
    class IBuilder {
    public:
        IBuilder() : m_product(new T) {}
        virtual ~IBuilder() = default;

        IBuilder(IBuilder&& another) noexcept {
            m_product.reset(another.m_product.release());
        }

        virtual bool is_built() const = 0;

        virtual T&& get_product() {
            return std::move(*m_product);
        }

    protected:
        std::unique_ptr<T> m_product;
    };
}
