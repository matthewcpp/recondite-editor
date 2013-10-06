#include "rFreetypeFontLoader.hpp"

rFreetypeFontLoader::rFreetypeFontLoader(){
	SetAsciiGlyphs();
}

void rFreetypeFontLoader::SetAsciiGlyphs(){
	for (char ch = ' '; ch <='~'; ch++){
		m_glyphSet.insert((int)ch);
	}
}

void rFreetypeFontLoader::SetGlyph(int glyph){
	m_glyphSet.insert(glyph);
}

void rFreetypeFontLoader::UnsetGlyph(int glyph){
	m_glyphSet.erase(glyph);
}

void rFreetypeFontLoader::ClearGlyphs(){
	m_glyphSet.clear();
}

size_t rFreetypeFontLoader::GlyphCount(){
	return m_glyphSet.size();
}

int rFreetypeFontLoader::LoadChar(int scancode, FT_Face fontFace, rFontData& data){
	int error = FT_Load_Char( fontFace, scancode , FT_LOAD_RENDER) ;

	if (!error){
		FT_GlyphSlot glyph = fontFace->glyph;

		data.AddGlyph(scancode, glyph->bitmap.width, glyph->bitmap.rows, glyph->bitmap_top, glyph->bitmap_left, (glyph->advance.x >> 6), glyph->bitmap.buffer);
	}

	return error;
}


int rFreetypeFontLoader::LoadFont(const rString& path, int size, rFontData& data){
	FT_Library freetypeLibrary;
	FT_Face fontFace;

	int error = FT_Init_FreeType(&freetypeLibrary);

	error = FT_New_Face (freetypeLibrary, path.c_str(), 0 , &fontFace);

	FT_Set_Pixel_Sizes(fontFace,0,size);

	data.SetSize(size);
	data.SetName(fontFace->family_name);

	for (rGlyphSet::iterator it = m_glyphSet.begin(); it != m_glyphSet.end(); ++it){
		LoadChar(*it, fontFace, data);
	}

	FT_Done_Face(fontFace);
	FT_Done_FreeType(freetypeLibrary);

	return error;
}