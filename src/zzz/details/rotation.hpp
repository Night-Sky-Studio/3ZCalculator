#pragma once

//std
#include <cstdint>
#include <list>
#include <span>
#include <string>
#include <vector>

//library
#include "library/builder.hpp"
#include "library/cached_memory.hpp"

namespace zzz::details {
    struct rotation_cell {
        std::string command;
        uint64_t index;
    };

    class Rotation {
        friend class RotationBuilder;

    public:
        std::span<uint64_t> teammates();
        std::span<const rotation_cell> cells() const;

        const rotation_cell& operator[](size_t index) const;
        size_t size() const;

    protected:
        std::vector<uint64_t> m_teammates;
        std::vector<rotation_cell> m_content;
    };

    class RotationBuilder : public lib::IBuilder<Rotation> {
    public:
        RotationBuilder& add_teammate(uint64_t id);

        RotationBuilder& add_cell(rotation_cell cell);
        RotationBuilder& set_content(std::span<const rotation_cell> content);

        bool is_built() const override;
        Rotation&& get_product() override;

    private:
        std::list<rotation_cell> _temporary_content;
    };
}

namespace zzz {
    using RotationDetails = details::Rotation;

    class Rotation : public lib::MObject {
    public:
        explicit Rotation(const std::string& name);

        RotationDetails& details();
        const RotationDetails& details() const;

        bool load_from_string(const std::string& input, size_t mode) override;
    };
    using RotationPtr = std::shared_ptr<Rotation>;
}
