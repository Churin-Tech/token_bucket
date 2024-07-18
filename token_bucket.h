#pragma once

#include <atomic>
#include <chrono>

namespace TB {
/*
 * Thread-safe (atomic) token bucket primitive(https://en.wikipedia.org/wiki/Token_bucket).
 * Fse TokenBucket is a minimal implementation of the folly token bucket.
 */
class TokenBucket {
public:
    /**
     * Constructor.
     * @param rate rate is the number of tokens to be generated per second. max value = 1000000000;
     */
    TokenBucket(const uint64_t rate) : rate_(rate), time_(0.0) {}

    /**
     * Copy constructor.
     */
    TokenBucket(const TokenBucket& other) = delete;

    /**
     * Copy-assignment operator.
     */
    TokenBucket& operator=(const TokenBucket& other) = delete;

    /**
     * Move constructor.
     */
    TokenBucket(TokenBucket&& other) = delete;

    /**
     * Move-assignment operator.
     */
    TokenBucket& operator=(TokenBucket&& other) = delete;

    /**
     * Destructor.
     */
    ~TokenBucket() = default;

    /**
     * UpdateRate modify rate limiter rate dynamically.
     * thread safe only supports 64-bit architecture.
     * @param rate rate is the number of tokens to be generated per second.
     */
    void UpdateRate(const uint64_t rate) { rate_ = rate; }

    /**
     * Consume tokens at the given rate/bucket_size.
     * Block until the token is successfully obtained until the next second.
     * @param tokens tokens is the number of tokens you need.
     * @return Return `false` when token is not enough
     */
    bool Consume(const uint64_t tokens) {
        if (tokens == 0) {
            return true;
        }

        const auto now_point =
            std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now().time_since_epoch()).count();

        auto rate                         = rate_;
        auto gen_per_token_needed_time    = std::chrono::nanoseconds(std::chrono::seconds(1)).count() / rate;
        auto gen_all_token_needed_time    = rate * gen_per_token_needed_time;
        const auto time_needed            = tokens * gen_per_token_needed_time;
        const auto gen_all_min_time_point = now_point - gen_all_token_needed_time;
        auto last_time_point              = time_.load(std::memory_order_relaxed);

        double new_time_point;
        do {
            // find capacity boundaries
            new_time_point = last_time_point;
            if (gen_all_min_time_point > last_time_point) {
                new_time_point = gen_all_min_time_point;
            }

            // capacity exceeded
            new_time_point += time_needed;
            if (new_time_point > now_point) {
                return false;
            }
        } while (!time_.compare_exchange_weak(last_time_point, new_time_point, std::memory_order_relaxed,
                                              std::memory_order_relaxed));
        return true;
    }

private:
    uint64_t rate_;
    std::atomic<double> time_;
};

}  // namespace TB
