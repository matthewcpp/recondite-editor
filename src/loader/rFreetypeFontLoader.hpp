#ifndef R_FREETYPEFONTLOADER_HPP
#define R_FREETYPEFONTLOADER_HPP

#include <set>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "data/rFontData.hpp"

typedef std::set<int> rGlyphSet;

class rFreetypeFontLoader {
public:
	rFreetypeFontLoader();

	int LoadFont(const rString& path, int size, rFontData& data);

	void SetAsciiGlyphs();
	void ClearGlyphs();
	size_t GlyphCount();
	void SetGlyph(int glyph);
	void UnsetGlyph(int glyph);

private:

	int LoadChar(int scancode, FT_Face fontFace, rFontData& data);

private:
	rGlyphSet m_glyphSet;
};

#endif