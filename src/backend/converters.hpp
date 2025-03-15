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
        eval_data_details from(const toml::value& data) const override;

    private:
        static const enemy_details enemy;
    };
}

namespace global {
    static const backend::ToEvalDataConverter to_eval_data;
}
