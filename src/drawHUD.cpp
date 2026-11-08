#include "drawHUD.h" // Include your updated header
#include "helperMethods.h"
#include "Constants.h"
#include <GL/glut.h> // Assuming your OpenGL/GLUT headers are here
#include <cstdio>

using namespace std;

std::function<void()> drawText(std::string text, float x, float y,
    void* font, TextAlignment alignment) {
    return [=]() {
        float left = x;
        if (alignment == TextAlignment::Center) {
            int width = 0;
            for (unsigned char character : text)
                width += glutBitmapWidth(font, character);
            left -= width / 2.0f;
        }
        glRasterPos2f(left, y);
        for (unsigned char character : text)
            glutBitmapCharacter(font, character);
    };
}

std::function<void()> drawText(std::string text) {
    return [text]() {
        drawText(text, (float)windowwidth() - 210.0f,
            (float)windowheight() - 250.0f, GLUT_BITMAP_HELVETICA_18)();
    };
}

std::function<void()> draw_ortho_compass(float rot_x) {
    return [rot_x]() {
        // 5. DEFINE RADIAL ORIENTATION & SPIN
        // Anchor position in pixels (Top Right Corner)
        float cx = (float)windowwidth() - 140.0f;
        float cy = (float)windowheight() - 140.0f;

        // Convert rot_x radians back into degrees for OpenGL's rotation tool
        // (We multiply by -57.2957f because OpenGL rotates counter-clockwise)
        float rot_degrees = rot_x * 57.2957795f;

        // Move drawing origin to the compass center, spin it, then draw locally
        glTranslatef(cx, cy, 0.0f);
        glRotatef(rot_degrees, 0.0f, 0.0f, 1.0f);

        // 6. DRAW THE COMPASS OBJECT (Pointing towards standard X-Axis)
        glBegin(GL_QUADS);
        // Red Pointer Arrow Pointing Right (+X Direction)
        glColor3f(1.0f, 0.2f, 0.2f); // Red
        glVertex2f(70.0f, 0.0f);  // Tip pointing directly along X
        glVertex2f(0.0f, 24.0f);  // Top corner
        glVertex2f(10.0f, 0.0f);  // Inner center groove
        glVertex2f(0.0f, -24.0f);  // Bottom corner

        // Grey Base Weight Tail (Opposite Side)
        glColor3f(0.5f, 0.5f, 0.5f); // Grey
        glVertex2f(-10.0f, 0.0f);  // Center point
        glVertex2f(0.0f, 24.0f);  // Top corner
        glVertex2f(-50.0f, 0.0f);  // Tail end pointing away
        glVertex2f(0.0f, -24.0f);  // Bottom corner
        glEnd();
    };
}


void draw_HUD(const std::function<void()>& drawUI) {
    // 1. Save all existing 3D attributes and server states
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

    // 2. Disable EVERYTHING that could interfere with flat colors
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);

    // Turn off vertex arrays in case your maze rendering engine leaves them active
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    // 3. SWITCH TO THE PROJECTION MATRIX (Crucial step)
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); // Save your 3D perspective setup matrix
    glLoadIdentity(); // Reset it to clean slate
    gluOrtho2D(0.0, (double)windowwidth(), 0.0, (double)windowheight());

    // 4. SWITCH TO THE MODELVIEW MATRIX
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); // Save your 3D camera translation matrix
    glLoadIdentity(); // Reset it to clean slate

    // Create a pixel-for-pixel flat 2D coordinate box matching the viewport

    // 5. Draw the HUD
    drawUI();

    // 6. CLEANUP MATRIX STACKS COMPLETELY
    // Pop the Modelview transformation modifications
    glPopMatrix();

    // Switch back to Projection and pop it back to your original 3D perspective
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    // Reset the matrix focus context mode back to normal operations
    glMatrixMode(GL_MODELVIEW);

    // Restore all pipeline properties (Lighting, textures, masks) exactly how they were
    glPopAttrib();
    glPopClientAttrib();
}

namespace {
void panel(float left, float bottom, float right, float top, float gray) {
    glColor3f(gray, gray, gray);
    glRectf(left, bottom, right, top);
}
void centeredLabel(float x, float y, const char* text) {
    glColor3f(0.85f, 0.9f, 0.95f);
    drawText(text, x, y, GLUT_BITMAP_HELVETICA_12, TextAlignment::Center)();
}
}

void draw_ssvep_targets(unsigned long long frame, bool active, double refreshHz) {
    draw_HUD([=]() {
        const float w = (float)windowwidth(), h = (float)windowheight();
        const float widthScale = w / SSVEP_REFERENCE_WIDTH_PX;
        const float heightScale = h / SSVEP_REFERENCE_HEIGHT_PX;
        const float scale = widthScale < heightScale ? widthScale : heightScale;
        const float half = SSVEP_TARGET_SIZE_PX * scale / 2.0f;
        // Bitmap fonts have fixed pixel sizes, so text spacing stays in pixels.
        const float padding = SSVEP_PANEL_PADDING_PX;
        const float border = SSVEP_BORDER_WIDTH_PX;
        const float labelOffset = SSVEP_LABEL_OFFSET_PX;
        const float frequencyOffset = labelOffset + SSVEP_TEXT_LINE_HEIGHT_PX;
        const float panelBelow = frequencyOffset + padding;
        const float minY = SSVEP_FOOTER_BASELINE_PX + SSVEP_TEXT_LINE_HEIGHT_PX + half + panelBelow;
        const float maxY = h - half - padding;
        for (const auto& target : SSVEP_TARGETS) {
            const float x = target.x * w;
            float y = target.y * h;
            if (y < minY) y = minY;
            if (y > maxY) y = maxY;
            panel(x-half-padding, y-half-panelBelow, x+half+padding, y+half+padding, 0.08f);
            panel(x-half-border, y-half-border, x+half+border, y+half+border, 0.4f);
            // Odd periods have floor(period/2) bright frames (e.g. 2/5 duty).
            const bool bright = frame % target.periodFrames < target.periodFrames / 2;
            panel(x-half, y-half, x+half, y+half, active ? (bright ? 1.0f : 0.0f) : 0.25f);
            centeredLabel(x, y-half-labelOffset, target.label);
            char frequency[80];
            if (!active) std::snprintf(frequency, sizeof(frequency), "PAUSED");
            else if (refreshHz > 0) std::snprintf(frequency, sizeof(frequency), "nominal %.2f Hz", refreshHz / target.periodFrames);
            else std::snprintf(frequency, sizeof(frequency), "%u frames/cycle", target.periodFrames);
            centeredLabel(x, y-half-frequencyOffset, frequency);
        }
        centeredLabel(w * 0.5f, SSVEP_FOOTER_BASELINE_PX, "F1: pause/resume flicker | Optical timing not yet validated");
    });
}

void draw_navigation_hud(bool showRoute, bool showAcceptedOverlay, RouteHint hint, DecoderState state,
                         const DecoderSample& sample, const char* steeringBlock,
                         bool armed, const std::string& lastSteeringEvent,
                         const char* launchStatus, bool keyboardOnly) {
    draw_HUD([&]() {
        constexpr float inset = 14, line = 19;
        const float top = static_cast<float>(windowheight()) - inset;
        float y = top - line;
        panel(inset, top - 4 * line, inset + 305, top, 0.08f);
        glColor3f(0.85f, 0.9f, 0.95f);
        auto text = [&](const std::string& value) {
            drawText(value, inset + 9, y, GLUT_BITMAP_HELVETICA_12)();
            y -= line;
        };
        const char* status = "Waiting for decoder";
        switch (state) {
        case DecoderState::Warming: status = "Collecting first EEG window"; break;
        case DecoderState::Scored:
            status = steeringBlock ? steeringBlock :
                armed ? "EEG steering active" : "EEG steering paused (F3)";
            break;
        case DecoderState::Invalid: status = "EEG window rejected"; break;
        case DecoderState::Stopped: status = "Decoder stopped"; break;
        case DecoderState::Stale: status = "Decoder signal expired"; break;
        case DecoderState::Unavailable: status = "Decoder link unavailable"; break;
        default: break;
        }
        if (launchStatus && (state == DecoderState::Waiting ||
            state == DecoderState::Stopped || state == DecoderState::Stale)) status = launchStatus;
        if (keyboardOnly) status = "Keyboard only";
        text(std::string("Decoder: ") + status);
        if (state == DecoderState::Scored) {
            char score[96];
            std::snprintf(score, sizeof(score), "Top: %s %.3f | margin %.3f",
                          actionLabel(sample.winner), sample.score, sample.margin);
            text(score);
        } else {
            text("Top: --");
        }
        text("Last pulse: " + lastSteeringEvent);
        text(keyboardOnly ? "F2: route hint" : "F2: route hint | F3: EEG control");

        const bool showAccepted = showAcceptedOverlay && (state == DecoderState::Scored ||
            state == DecoderState::Warming || state == DecoderState::Invalid);
        if (showRoute || showAccepted) {
            const char* direction = hint == RouteHint::Forward ? "Go forward" :
                hint == RouteHint::Left ? "Turn left" :
                hint == RouteHint::Right ? "Turn right" : "No route available";
            const float cx = windowwidth() * 0.5f, cy = windowheight() * 0.5f;
            panel(cx - 110, cy - (showRoute && showAccepted ? 36 : 17),
                  cx + 110, cy + 17, 0.08f);
            glColor3f(0.85f, 0.9f, 0.95f);
            if (showRoute)
                drawText(direction, cx, cy - 6, GLUT_BITMAP_HELVETICA_18, TextAlignment::Center)();
            if (showAccepted)
                drawText(std::string("Accepted: ") +
                         (state == DecoderState::Scored ? actionLabel(sample.accepted) : "--"),
                         cx, cy - (showRoute ? 25 : 6), GLUT_BITMAP_HELVETICA_12, TextAlignment::Center)();
        }
    });
}
