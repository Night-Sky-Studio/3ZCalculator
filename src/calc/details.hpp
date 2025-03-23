#pragma once

//std
#include <array>
#include <map>
#include <vector>

//zzz
#include "zzz/details.hpp"

namespace calc {
    struct enemy_t {
        double dmg_reduction, defense, stun_mult;
        std::array<double, 5> res;
        bool is_stunned;
    };

    template<typename T>
    class RequestCell {
    public:
        explicit RequestCell(uint64_t id) :
            m_id(id) {
        }

        uint64_t id() const { return m_id; }

        T& value() { return *m_ptr; }
        const T& value() const { return *m_ptr; }

    protected:
        uint64_t m_id;
        std::shared_ptr<T> m_ptr;
    };

    // TODO: properly organize
    class Request {
    public:
        Request(uint64_t agent_id, uint64_t wengine_id, uint64_t rotation_id,
            const std::vector<uint64_t>& dds_ids, std::array<zzz::Ddp, 6> ddps) :
            m_agent(agent_id),
            m_wengine(wengine_id),
            m_rotation(rotation_id),
            m_ddps(std::move(ddps)) {
            m_dds_list.reserve(dds_ids.size());
            for (auto id : dds_ids)
                m_dds_list.emplace_back(id);
        }

        RequestCell<zzz::Agent>& agent() { return m_agent; }
        const RequestCell<zzz::Agent>& agent() const { return m_agent; }

        RequestCell<zzz::Wengine>& wengine() { return m_wengine; }
        const RequestCell<zzz::Wengine>& wengine() const { return m_wengine; }

        RequestCell<zzz::Rotation>& rotation() { return m_rotation; }
        const RequestCell<zzz::Rotation>& rotation() const { return m_rotation; }

        std::multimap<size_t, zzz::DdsPtr>& dds_by_count() { return m_dds_by_count; }
        const std::multimap<size_t, zzz::DdsPtr>& dds_by_count() const { return m_dds_by_count; }

        std::vector<RequestCell<zzz::Dds>>& dds_list() { return m_dds_list; }
        const std::vector<RequestCell<zzz::Dds>>& dds_list() const { return m_dds_list; }

        const zzz::Ddp& ddp(size_t index) const { return m_ddps[index]; }

    protected:
        RequestCell<zzz::Agent> m_agent;
        RequestCell<zzz::Wengine> m_wengine;
        RequestCell<zzz::Rotation> m_rotation;

        std::multimap<size_t, zzz::DdsPtr> m_dds_by_count;
        std::vector<RequestCell<zzz::Dds>> m_dds_list;

        std::array<zzz::Ddp, 6> m_ddps;
    };
}
