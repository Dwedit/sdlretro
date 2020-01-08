#include "ttf_font.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#ifdef USE_STB_TRUETYPE
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include <fstream>
#else
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

#include <climits>

namespace drivers {

enum :uint16_t {
    RECTPACK_WIDTH = 1024
};

struct rect_pack_data {
    stbrp_context context;
    stbrp_node nodes[RECTPACK_WIDTH];
    uint8_t pixels[RECTPACK_WIDTH * RECTPACK_WIDTH];
};

ttf_font::ttf_font() {
#ifndef USE_STB_TRUETYPE
    FT_Init_FreeType(&ft_lib);
#endif
}

ttf_font::~ttf_font() {
    for (auto *&p: rectpack_data) delete p;
    rectpack_data.clear();
    for (auto &p: fonts) {
#ifdef USE_STB_TRUETYPE
        delete static_cast<stbtt_fontinfo *>(p.font);
		delete p.ttf_buffer;
#else
        FT_Done_Face(p.face);
#endif
    }
#ifndef USE_STB_TRUETYPE
    FT_Done_FreeType(ft_lib);
#endif
}

void ttf_font::init(int size, uint8_t width) {
    font_size = size;
    mono_width = width;
}

bool ttf_font::add(const std::string &filename, int index) {
    font_info fi;
#ifdef USE_STB_TRUETYPE
    auto *info = new stbtt_fontinfo;
	std::ifstream fin(filename, std::ios::binary);
    if (fin.fail()) return false;
	fin.seekg(0, std::ios::end);
	size_t sz = fin.tellg();
	fi.ttf_buffer = new uint8_t[sz];
	fin.seekg(0, std::ios::beg);
	fin.read(reinterpret_cast<char*>(fi.ttf_buffer), sz);
	fin.close();
	stbtt_InitFont(info, fi.ttf_buffer, stbtt_GetFontOffsetForIndex(fi.ttf_buffer, index));
	fi.font_scale = stbtt_ScaleForMappingEmToPixels(info, static_cast<float>(font_size));
	fi.font = info;
#else
    if (FT_New_Face(ft_lib, filename.c_str(), index, &fi.face)) return false;
    FT_Set_Pixel_Sizes(fi.face, 0, font_size);
#endif
    fonts.push_back(fi);
    new_rect_pack();
    return true;
}

uint8_t ttf_font::get_char_width(uint16_t ch) const {
    auto ite = font_cache.find(ch);
    if (ite == font_cache.end()) return 0;
    if (mono_width)
        return std::max(ite->second.advW, mono_width);
    return ite->second.advW;
}

const ttf_font::font_data *ttf_font::make_cache(uint16_t ch) {
    font_info *fi = nullptr;
#ifdef USE_STB_TRUETYPE
    stbtt_fontinfo *info;
#endif
    uint32_t index = 0;
    for (auto &f: fonts) {
        fi = &f;
#ifdef USE_STB_TRUETYPE
        info = static_cast<stbtt_fontinfo*>(f.font);
        index = stbtt_FindGlyphIndex(info, ch);
        if (index == 0) continue;
#else
        index = FT_Get_Char_Index(f.face, ch);
        if (index == 0) continue;
        if (!FT_Load_Glyph(f.face, index, FT_LOAD_DEFAULT)) break;
#endif
    }
    font_data *fd = &font_cache[ch];
    if (fi == nullptr) {
        memset(fd, 0, sizeof(font_data));
        return nullptr;
    }

#ifdef USE_STB_TRUETYPE
    /* Read font data to cache */
    int advW, leftB;
    stbtt_GetGlyphHMetrics(info, index, &advW, &leftB);
    fd->advW = static_cast<uint8_t>(fi->font_scale * advW);
    leftB = static_cast<uint8_t>(fi->font_scale * leftB);
    int ix0, iy0, ix1, iy1;
    stbtt_GetGlyphBitmapBoxSubpixel(info, index, fi->font_scale, fi->font_scale, 3, 3, &ix0, &iy0, &ix1, &iy1);
    fd->ix0 = leftB;
    fd->iy0 = iy0;
    fd->w = ix1 - ix0;
    fd->h = iy1 - iy0;
#else
    unsigned char *src_ptr;
    int bitmap_pitch;
    if (FT_Render_Glyph(fi->face->glyph, FT_RENDER_MODE_NORMAL)) return nullptr;
    FT_GlyphSlot slot = fi->face->glyph;
    fd->ix0 = slot->bitmap_left;
    fd->iy0 = -slot->bitmap_top;
    fd->w = slot->bitmap.width;
    fd->h = slot->bitmap.rows;
    fd->advW = slot->advance.x >> 6;
    src_ptr = slot->bitmap.buffer;
    bitmap_pitch = slot->bitmap.pitch;
#endif

    /* Get last rect pack bitmap */
    auto rpidx = rectpack_data.size() - 1;
    auto *rpd = rectpack_data[rpidx];
    stbrp_rect rc = {0, fd->w, fd->h};
    if (!stbrp_pack_rects(&rpd->context, &rc, 1)) {
        /* No space to hold the bitmap,
         * create a new bitmap */
        new_rect_pack();
        rpidx = rectpack_data.size() - 1;
        rpd = rectpack_data[rpidx];
        stbrp_pack_rects(&rpd->context, &rc, 1);
    }
    /* Do rect pack */
    fd->rpx = rc.x;
    fd->rpy = rc.y;
    fd->rpidx = rpidx;

#ifdef USE_STB_TRUETYPE
    stbtt_MakeGlyphBitmapSubpixel(info, &rpd->pixels[rc.y * RECTPACK_WIDTH + rc.x], fd->w, fd->h, RECTPACK_WIDTH, fi->font_scale, fi->font_scale, 3, 3, index);
#else
    auto *dst_ptr = &rpd->pixels[rc.y * RECTPACK_WIDTH + rc.x];
    for (int k = 0; k < fd->h; ++k) {
        memcpy(dst_ptr, src_ptr, fd->w);
        src_ptr += bitmap_pitch;
        dst_ptr += RECTPACK_WIDTH;
    }
#endif
    return fd;
}

void ttf_font::new_rect_pack() {
    auto *rpd = new rect_pack_data;
    stbrp_init_target(&rpd->context, RECTPACK_WIDTH, RECTPACK_WIDTH, rpd->nodes, RECTPACK_WIDTH);
    rectpack_data.push_back(rpd);
}

const uint8_t *ttf_font::get_rect_pack_data(uint8_t idx, int16_t x, int16_t y) {
    return &rectpack_data[idx]->pixels[y * RECTPACK_WIDTH + x];
}
uint16_t ttf_font::get_rect_pack_width() {
    return RECTPACK_WIDTH;
}

}
