#pragma once

#include <string>     // Required for std::string
#include <functional> // Required for std::function

// Explicitly use std::function<void()> instead of auto
std::function<void()> drawText(std::string text);
enum class TextAlignment { Left, Center };
// Uses the current drawing color; coordinates are in HUD pixels.
std::function<void()> drawText(std::string text, float x, float y,
    void* font, TextAlignment alignment = TextAlignment::Left);
std::function<void()> draw_ortho_compass(float rot_x);

void draw_HUD(const std::function<void()>& drawUI);

// Frame periods are provisional calibration settings, not validated EEG frequencies.
void draw_ssvep_targets(unsigned long long frame, bool active, double refreshHz);
