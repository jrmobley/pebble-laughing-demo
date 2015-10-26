/*
 *
 */

#include "pebble.h"
#include "fctx/fctx.h"
#include "fctx/ffont.h"
#include "fctx/fpath.h"

struct Globals {
	Window* window;
	Layer* layer;
	FFont* font;
    FPath* path;
    FPoint center;
} g;

/* TAU = 2*PI * FIXED_POINT_SCALE * TRIG_MAX_ANGLE
   325 is the largest number you can multiply by TAU without overflow.
*/
static const int32_t TAU = 6588397;

void draw_string_radial(
    FContext* fctx,
    const char* text,
    FFont* font,
    int16_t font_size,
    FPoint center,
    uint16_t radius,
    uint32_t angle)
{
    fctx_set_text_size(fctx, font, font_size);
    fctx_set_offset(fctx, center);
    fctx_set_rotation(fctx, 0);

    fixed_t s = radius * TAU / TRIG_MAX_ANGLE;
    fixed_t t = s * fctx->transform_scale_from.x / fctx->transform_scale_to.x;
    fixed_t r = INT_TO_FIXED(radius) * fctx->transform_scale_from.x / fctx->transform_scale_to.x;
    fixed_t arc = 0;
    const char* p;

    for (p = text; *p; ++p) {
        char ch = *p;
        FGlyph* glyph = ffont_glyph_info(font, ch);
        if (glyph) {

            if (p != text) {
                arc += glyph->horiz_adv_x / 2;
            }
            fixed_t rotation = angle + arc * TRIG_MAX_ANGLE / t;
            fctx_set_rotation(fctx, rotation);
            arc += glyph->horiz_adv_x / 2;

            FPoint advance;
            advance.x = glyph->horiz_adv_x / -2;
            advance.y = r;

            void* path_data = ffont_glyph_outline(font, glyph);
            fctx_draw_commands(fctx, advance, path_data, glyph->path_data_length);
        }
    }
}

static void demo_draw(Layer* layer, GContext* ctx) {

    GRect bounds = layer_get_bounds(layer);
    GRect fill = bounds;
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, fill, 0, GCornerNone);

    FContext fctx;
    fctx_init_context(&fctx, ctx);
    fctx_set_fill_color(&fctx, GColorDukeBlue);
    fctx_set_color_bias(&fctx, 0);

    fctx_begin_fill(&fctx);
    fctx_set_offset(&fctx, FPointI(6, 6));
    fctx_set_scale(&fctx, FPointI(180,180), FPointI(168,168));
    fctx_draw_commands(&fctx, FPointZero, g.path->data, g.path->size);
    fctx_end_fill(&fctx);

    fctx_begin_fill(&fctx);
    draw_string_radial(&fctx,
        "I thought what I'd do was, I'd pretend I was one of those deaf-mutes",
        g.font, 19,
        g.center, 71, DEG_TO_TRIGANGLE(110));
    fctx_end_fill(&fctx);

    fctx_deinit_context(&fctx);

}

static void init() {

    setlocale(LC_ALL, "");

    g.window = window_create();
    window_set_background_color(g.window, GColorBlack);
    window_stack_push(g.window, true);
    Layer* window_layer = window_get_root_layer(g.window);
    GRect window_frame = layer_get_frame(window_layer);

    g.center.x = INT_TO_FIXED(window_frame.size.w) / 2;
    g.center.y = INT_TO_FIXED(window_frame.size.h) / 2;
    g.font = ffont_create_from_resource(RESOURCE_ID_DIN_LATIN_FFONT);
    g.path = fpath_create_from_resource(RESOURCE_ID_LAUGHING_MAN_FPATH);

    g.layer = layer_create(window_frame);
    layer_set_update_proc(g.layer, &demo_draw);
    layer_add_child(window_layer, g.layer);
}

static void deinit() {
    window_destroy(g.window);
    layer_destroy(g.layer);
    ffont_destroy(g.font);
    fpath_destroy(g.path);
}

int main() {
    init();
    app_event_loop();
    deinit();
}
