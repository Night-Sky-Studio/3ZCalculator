#pragma once

//std
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

//utl
#include "utl/json.hpp"

//zzz
#include "zzz/enums.hpp"

namespace zzz {
    class StatsGrid;
    class IStat;

    using StatPtr = std::shared_ptr<IStat>;

    class IStat {
        friend class StatsGrid;

    public:
        explicit IStat(size_t type);
        virtual ~IStat() = default;

        virtual StatPtr copy_as_ptr() const = 0;

        virtual double value() const = 0;

        double base() const;
        StatId id() const;
        Tag tag() const;

    protected:
        double m_base = 0;
        StatId m_id = StatId::None;
        Tag m_tag = Tag::Universal;

    private:
        size_t _type;
    };

    class RegularStat : public IStat {
    public:
        static StatPtr make(double base, StatId id, Tag tag);

        static StatPtr make_from_floating(const utl::Json& json);
        static StatPtr make_from_object(const utl::Json& json);

        explicit RegularStat();

        StatPtr copy_as_ptr() const override;

        double value() const override;
    };

    class RelativeStat : public IStat {
    public:
        static StatPtr make_from_string(const utl::Json& json);
        static StatPtr make_from_object(const utl::Json& json);

        explicit RelativeStat();

        //double value() const override;

    protected:
        double m_mult = 0;
        StatId m_dependency = StatId::None;
    };

    class StatFactory {
    public:
        using StatMaker = std::function<StatPtr(const utl::Json&)>;

        static std::string default_type_name;

        static void init_default();

        static bool add_maker(std::string key, StatMaker value);
        static StatPtr make(const utl::Json& json);

    protected:
        static std::unordered_map<std::string, StatMaker> m_makers;
    };
}
