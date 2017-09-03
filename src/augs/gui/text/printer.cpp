#include <algorithm>

#include "augs/drawing/drawing.h"

#include "augs/gui/text/ui.h"
#include "augs/gui/text/drafter.h"
#include "augs/gui/text/printer.h"

namespace augs {
	namespace gui {
		namespace text {
			void caret_blinker::update() {
				if (static_cast<unsigned>(timer.get<std::chrono::milliseconds>()) > interval_ms) {
					caret_visible = !caret_visible;
					timer.reset();
				}
			}

			void caret_blinker::reset() {
				timer.reset();
				caret_visible = true;
			}

			void printer::draw_text(
				const drawer_with_default out,
				const vec2i pos,
				const drafter& d,
				const formatted_string& colors,
				const caret_info caret,
				const ltrbi clipper
			) const {
				auto& lines = d.lines;
				auto& sectors = d.sectors;
				const bool clip = clipper.good();

				auto getf = [&](auto& f) {
					return *f;
				};

				/* here we highlight the line caret is currently on */
				if (active && highlight_current_line) {
					drafter::line highlighted = lines.size() ? lines[d.get_line(caret.pos)] : drafter::line();
					out.aabb_clipped(
						xywhi(
							0, 
							highlighted.top, 
							clip ? d.get_bbox().x + clipper.w() : d.get_bbox().x,
							/* snap to default style's height */
							highlighted.empty() ? 
								getf(caret.default_style.font).meta_from_file.get_height()
								: highlighted.height()
						) + pos, 
						clipper,
						highlight_col
					);
				}
				
				auto caret_rect = xywhi(0, 0, 0, 0);

				if (!lines.empty() && !sectors.empty()) {
					/* only these lines we want to process */
					std::pair<int, int> visible;

					if (clip)
						visible = d.get_line_visibility(clipper - pos);
					else visible = std::make_pair(0, lines.size() - 1);

					/* if this happens:
					- check if there is always an empty line
					- check if we return when clipper is not valid
					- check if scroll is always aligned */
					if (visible.first == -1) {
						return;
					}

					/* we'll need these variables later so we declare them here */
					unsigned select_left = 0;
					unsigned select_right = 0;
					unsigned caret_line = 0;

					caret_line = d.get_line(caret.pos);

					/* let's calculate some values only once */
					select_left = caret.get_left_selection();
					select_right = caret.get_right_selection();

					if (caret.selection_offset) {
						const unsigned select_left_line = d.get_line(select_left);
						const unsigned select_right_line = d.get_line(select_right);
						const unsigned first_visible_selection = std::max(select_left_line, static_cast<unsigned>(visible.first));
						const unsigned last_visible_selection = std::min(select_right_line, static_cast<unsigned>(visible.second));

						/* manage selections */
						for (unsigned i = first_visible_selection; i <= last_visible_selection; ++i) {
							/* init selection rect on line rectangle;
							its values won't change if selecting between first and the last line
							*/
							ltrbi sel_rect = d.lines[i].get_rect();

							/* if it's the first line to process and we can see it, we have to trim its x coordinate */
							if (i == first_visible_selection && select_left_line >= first_visible_selection) {
								sel_rect.l = d.sectors[select_left];
							}

							/* similiarly with the last one
							note that with only one line selecting, it is still correct to apply these two conditions
							*/
							if (i == last_visible_selection && select_right_line <= last_visible_selection) {
								sel_rect.r = d.sectors[select_right];
							}

							out.aabb_clipped(sel_rect + pos, clipper, active ? selection_bg_col : selection_inactive_bg_col);
						}
					}

					/* for every visible line */
					for (unsigned l = visible.first; l <= static_cast<unsigned>(visible.second); ++l) {
						/* for every character in line */
						for (unsigned i = lines[l].begin; i < lines[l].end; ++i) {
							/* shortcut */
							auto& g = *d.cached[i];

							/* if it's not a whitespace */
							if (d.cached_atlas_entries[i]->exists()) {
								rgba charcolor = style(colors[i]).color;

								/* if a character is between selection bounds, we change its color to the one specified in selected_text_color
								if there's no caret, this is never true
								*/
								if (i > select_left && i < select_right) {
									charcolor = selected_text_color;
								}

								/* add the resulting character taking bearings into account */
								out.aabb_clipped(
									*d.cached_atlas_entries[i],
									ltrb(xywhi(sectors[i] + g.bear_x, lines[l].top + lines[l].asc - g.bear_y, g.size.x, g.size.y) + pos),
									ltrb(clipper),
									charcolor
								);
							}
						}
					}

					if (active) {
						/* if we can retrieve some sane values */
						if (!lines[caret_line].empty()) {
							if (align_caret_height)
								caret_rect = xywhi(sectors[caret.pos], lines[caret_line].top, caret_width, lines[caret_line].height());
							else {
								const auto pos = std::max(1u, caret.pos);
								auto& glyph_font = getf(colors[pos - 1].format.font);
								
								caret_rect = xywhi(
									sectors[caret.pos], 
									lines[caret_line].top + lines[caret_line].asc - glyph_font.meta_from_file.ascender,
									caret_width, 
									glyph_font.meta_from_file.get_height()
								);
							}
						}
						/* otherwise set caret's height to default style's height to avoid strange situations */
						else {
							caret_rect = xywhi(0, d.lines[caret_line].top, caret_width, getf(caret.default_style.font).meta_from_file.get_height());
						}
					}
				}
				/* there is nothing to draw, but we are still active so we want to draw caret anyway */
				else if (active) {
					caret_rect = xywhi(0, 0, caret_width, getf(caret.default_style.font).meta_from_file.get_height());
				}

				if (blink.caret_visible) {
					out.aabb_clipped(caret_rect + pos, clipper, caret_col);
				}
			}
			
			void printer::draw_text(
				const drawer out,
				const vec2i pos,
				const drafter& d,
				const formatted_string& colors,
				const ltrbi clipper
			) const {
				auto& lines = d.lines;
				auto& sectors = d.sectors;
				const bool clip = clipper.good();

				auto getf = [&](auto& f) {
					return *f;
				};

				auto caret_rect = xywhi(0, 0, 0, 0);

				if (!lines.empty() && !sectors.empty()) {
					std::pair<int, int> visible;

					if (clip) {
						visible = d.get_line_visibility(clipper - pos);
					}
					else {
						visible = std::make_pair(0, lines.size() - 1);
					}

					if (visible.first == -1) {
						return;
					}

					unsigned select_left = 0;
					unsigned select_right = 0;
					unsigned caret_line = 0;

					/* for every visible line */
					for (unsigned l = visible.first; l <= static_cast<unsigned>(visible.second); ++l) {
						/* for every character in line */
						for (unsigned i = lines[l].begin; i < lines[l].end; ++i) {
							/* shortcut */
							auto& g = *d.cached[i];

							/* if it's not a whitespace */
							if (d.cached_atlas_entries[i]->exists()) {
								rgba charcolor = style(colors[i]).color;

								out.aabb_clipped(
									*d.cached_atlas_entries[i],
									xywhi(sectors[i] + g.bear_x, lines[l].top + lines[l].asc - g.bear_y, g.size.x, g.size.y) + pos,
									clipper,
									charcolor
								);
							}
						}
					}
				}
			}

			vec2i get_text_bbox(const formatted_string& str, const unsigned wrapping_width) {
				drafter dr;
				dr.wrap_width = wrapping_width;
				dr.draw(str);
				return dr.get_bbox();
			}

			vec2i print(
				const drawer out,
				const vec2i pos,
				const formatted_string& str,
				const unsigned wrapping_width,
				const ltrbi clipper
			) {
				drafter draft;
				printer print;
				
				draft.wrap_width = wrapping_width;

				draft.draw(str);
				print.draw_text(out, pos, draft, str, clipper);
				
				return draft.get_bbox();
			}

			vec2i print_stroked(
				const drawer out,
				const vec2i pos,
				const formatted_string& str,
				const rgba stroke_color,
				const unsigned wrapping_width,
				const ltrbi clipper
			) {
				drafter draft;
				printer print;

				draft.wrap_width = wrapping_width;

				draft.draw(str);

				auto coloured_str = str;

				for (auto& c : coloured_str) {
					c.set_color(stroke_color);
				}

				print.draw_text(out, pos + vec2i(-1, 0), draft, coloured_str, clipper);
				print.draw_text(out, pos + vec2i(1, 0), draft, coloured_str, clipper);
				print.draw_text(out, pos + vec2i(0, -1), draft, coloured_str, clipper);
				print.draw_text(out, pos + vec2i(0, 1), draft, coloured_str, clipper);

				print.draw_text(out, pos, draft, str, clipper);

				return draft.get_bbox() + vec2i(2, 2);
			}

			vec2i print(
				const drawer_with_default out,
				const vec2i pos,
				const formatted_string& str,
				const caret_info caret,
				const unsigned wrapping_width,
				const ltrbi clipper
			) {
				drafter draft;
				printer print;

				draft.wrap_width = wrapping_width;

				draft.draw(str);
				print.draw_text(out, pos, draft, str, caret, clipper);

				return draft.get_bbox();
			}

			vec2i print_stroked(
				const drawer_with_default out,
				const vec2i pos,
				const formatted_string& str,
				const caret_info caret,
				const rgba stroke_color,
				const unsigned wrapping_width,
				const ltrbi clipper
			) {
				drafter draft;
				printer print;

				draft.wrap_width = wrapping_width;

				draft.draw(str);

				auto coloured_str = str;

				for (auto& c : coloured_str) {
					c.set_color(stroke_color);
				}

				print.draw_text(out, pos + vec2i(-1, 0), draft, coloured_str, caret, clipper);
				print.draw_text(out, pos + vec2i(1, 0), draft, coloured_str, caret, clipper);
				print.draw_text(out, pos + vec2i(0, -1), draft, coloured_str, caret, clipper);
				print.draw_text(out, pos + vec2i(0, 1), draft, coloured_str, caret, clipper);

				print.draw_text(out, pos, draft, str, caret, clipper);

				return draft.get_bbox() + vec2i(2, 2);
			}
		}
	}
}