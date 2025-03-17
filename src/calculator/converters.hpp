#pragma once

//toml11
#include "toml.hpp"

//library
#include "library/converter.hpp"

//calculator
#include "calculator/details.hpp"

namespace calculator {
    class ToEvalDataDetailsConverter : lib::IConverter<eval_data_details, toml::value> {
    public:
        eval_data_details from(const toml::value& data) const override;

    private:
        static const enemy enemy;
    };
}

namespace global {
    static const calculator::ToEvalDataDetailsConverter to_eval_data_details;
}
