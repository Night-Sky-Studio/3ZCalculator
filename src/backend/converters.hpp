#pragma once

//toml11
#include "toml.hpp"

//library
#include "library/converter.hpp"

//backend
#include "backend/details.hpp"

namespace backend {
    class ToEvalDataConverter : lib::IConverter<eval_data_details, toml::value> {
    public:
        static void init();

        eval_data_details from(const toml::value& data) const override;

    private:
        static const uint64_t characters_ids[1];
        static const uint64_t wengines_ids[1];
        static const uint64_t dds_ids[3];

        static std::unordered_map<size_t, zzz::AgentDetails> _agents;
        static std::unordered_map<size_t, zzz::WengineDetails> _wengines;
        static std::unordered_map<size_t, zzz::DdsDetails> _dds;
        static std::unordered_map<size_t, zzz::rotation_details> _rotations;
        static const enemy_details enemy;
    };
}

namespace backend::global {
    static const ToEvalDataConverter to_eval_data;
}
