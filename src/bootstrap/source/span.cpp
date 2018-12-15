#include "span.hpp"
#include "translation_unit.hpp"

TextPos Span::lo_textpos() const { return tu->pos_from_index(lo_bit); }

TextPos Span::hi_textpos() const { return tu->pos_from_index(hi_bit); }