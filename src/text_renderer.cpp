#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

#include "../include/text_renderer.h"

#include <fstream>

namespace {
int nextUtf8Codepoint(const std::string &text, std::size_t &offset) {
    const unsigned char lead = static_cast<unsigned char>(text[offset++]);
    if (lead < 0x80) return lead;

    int codepoint = 0;
    int continuationCount = 0;
    if ((lead & 0xE0) == 0xC0) { codepoint = lead & 0x1F; continuationCount = 1; }
    else if ((lead & 0xF0) == 0xE0) { codepoint = lead & 0x0F; continuationCount = 2; }
    else if ((lead & 0xF8) == 0xF0) { codepoint = lead & 0x07; continuationCount = 3; }
    else return '?';

    if (offset + static_cast<std::size_t>(continuationCount) > text.size()) {
        offset = text.size();
        return '?';
    }
    for (int i = 0; i < continuationCount; ++i) {
        const unsigned char continuation = static_cast<unsigned char>(text[offset]);
        if ((continuation & 0xC0) != 0x80) return '?';
        ++offset;
        codepoint = (codepoint << 6) | (continuation & 0x3F);
    }
    return codepoint;
}
}

struct TextRenderer::FontData {
    std::vector<unsigned char> bytes;
    stbtt_fontinfo info = {};
};

TextRenderer::TextRenderer() = default;
TextRenderer::~TextRenderer() = default;

bool TextRenderer::loadFont(const std::string &path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) return false;
    const std::streamsize length = file.tellg();
    if (length <= 0) return false;
    auto font = std::make_unique<FontData>();
    font->bytes.resize(static_cast<size_t>(length));
    file.seekg(0);
    if (!file.read(reinterpret_cast<char *>(font->bytes.data()), length)
        || !stbtt_InitFont(&font->info, font->bytes.data(), 0)) return false;
    m_font = std::move(font);
    return true;
}

float TextRenderer::CalculateWidth(const std::string &text, float height) const {
    float widest = 0.0f, width = 0.0f;
    const float scale = m_font ? stbtt_ScaleForPixelHeight(&m_font->info, height) : 0.0f;
    std::size_t offset = 0;
    while (offset < text.size()) {
        const int codepoint = nextUtf8Codepoint(text, offset);
        if (codepoint == '\n') { widest = std::max(widest, width); width = 0.0f; continue; }
        if (m_font) { int advance = 0, bearing = 0; stbtt_GetCodepointHMetrics(&m_font->info, codepoint, &advance, &bearing); width += advance * scale; }
        else width += height * characterAdvance(codepoint < 0x80 ? static_cast<char>(codepoint) : '?');
    }
    return std::max(widest, width);
}

void TextRenderer::RenderText(const std::string &text, float x, float y, float height) {
    if (!m_renderCallback || height <= 0.0f) return;
    if (!m_font) return;
    const float scale = stbtt_ScaleForPixelHeight(&m_font->info, height);
    float cursorX = x, cursorY = y;
    std::vector<Run> runs;
    std::size_t offset = 0;
    while (offset < text.size()) {
        const int codepoint = nextUtf8Codepoint(text, offset);
        if (codepoint == '\n') { cursorX = x; cursorY += height * 1.25f; continue; }
        int advance = 0, bearing = 0, width = 0, glyphHeight = 0, xoff = 0, yoff = 0;
        stbtt_GetCodepointHMetrics(&m_font->info, codepoint, &advance, &bearing);
        unsigned char *bitmap = stbtt_GetCodepointBitmap(&m_font->info, scale, scale, codepoint, &width, &glyphHeight, &xoff, &yoff);
        for (int row = 0; bitmap != nullptr && row < glyphHeight; ++row) {
            int column = 0;
            while (column < width) {
                while (column < width && bitmap[row * width + column] < 128) ++column;
                const int first = column;
                while (column < width && bitmap[row * width + column] >= 128) ++column;
                if (first != column) runs.push_back({ cursorX + xoff + first,
                    cursorY - yoff - row - 0.5f, static_cast<float>(column - first), 1.0f });
            }
        }
        stbtt_FreeBitmap(bitmap, nullptr);
        cursorX += advance * scale;
    }
    if (!runs.empty()) m_renderCallback(runs, m_color);
}
