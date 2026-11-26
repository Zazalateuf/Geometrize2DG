#pragma once

#include <type_traits>
#include <utility>

template<typename F>
concept Callable = std::is_invocable_v<F>;

template<Callable F>
class ScopeExit {
    F m_cb;
    bool m_canceled = false;
public:
    explicit ScopeExit(F cb): m_cb(std::move(cb)) {}
    void cancel() {this->m_canceled = true;}

    ~ScopeExit() {
        if (!this->m_canceled) {
            this->m_cb();
        }
    }
};
