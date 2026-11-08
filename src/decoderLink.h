#pragma once

#include <array>
#include <cstdint>
#include <string>

enum class MazeAction { None, Forward, Backward, Left, Right };
enum class DecoderState { Waiting, Warming, Scored, Invalid, Stopped, Stale, Unavailable };

struct DecoderSample {
    std::int64_t sentMs = 0;
    DecoderState state = DecoderState::Waiting;
    MazeAction winner = MazeAction::None;
    MazeAction accepted = MazeAction::None;
    double score = 0, margin = 0;
    bool control = false, emit = false;
    std::array<double, 4> frequencies{};
};

class DecoderLink {
public:
    DecoderLink() = default;
    ~DecoderLink();
    DecoderLink(const DecoderLink&) = delete;
    DecoderLink& operator=(const DecoderLink&) = delete;
    bool open();
    bool poll(std::int64_t nowMs);
    DecoderState state(std::int64_t nowMs) const;
    const DecoderSample& sample() const { return sample_; }

private:
    std::uintptr_t socket_ = ~std::uintptr_t{0};
    bool started_ = false;
    DecoderSample sample_;
};

std::int64_t decoderClockMs();
const char* actionLabel(MazeAction action);
