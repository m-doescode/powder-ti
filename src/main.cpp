#include <graphx.h>
#include <tice.h>
#include <debug.h>
#include <time.h>

#include "gfx/global_palette.h"

#define TI84
#include "linkedlist.h"

static constexpr uint16_t SCREEN_WIDTH = 160, SCREEN_HEIGHT = 120;
static constexpr uint16_t MAX_PARTS = SCREEN_WIDTH * SCREEN_HEIGHT;
static constexpr uint16_t NO_PART = 0xFFFF;

typedef uint16_t upos;
typedef uint16_t partidx_t;
typedef uint8_t parttype_t;

struct partpos_t {
    upos x;
    upos y;
};

struct Particle {
    uint8_t type : 7;
    bool movedParity : 1;
    partpos_t pos;
};

partidx_t* grid = new partidx_t[MAX_PARTS] { };
LinkedList<Particle> parts;

bool globalParity = 0;

//

Particle part_at(upos x, upos y) {
    partidx_t idx = grid[y * SCREEN_WIDTH + x];
    if (idx == NO_PART)
        return { 0, 0, { x, y } };
    return parts.get(idx);
}

partidx_t add_part(upos x, upos y, parttype_t type) {
    if (grid[y * SCREEN_WIDTH + x] != NO_PART)
        throw("Particle already exists at (%d, %d), value: %d\n", x, y, grid[y * SCREEN_WIDTH + x]);
    Particle part { type, globalParity, { x, y } };
    partidx_t idx = parts.push_back(part);
    grid[y * SCREEN_WIDTH + x] = idx;
    return idx;
}

void del_part(upos x, upos y) {
    partidx_t idx = grid[y * SCREEN_WIDTH + x];
    grid[y * SCREEN_WIDTH + x] = NO_PART;
    parts.remove(idx);
}

void del_part(partidx_t idx) {
    Particle part = parts.get(idx);
    grid[part.pos.y * SCREEN_WIDTH + part.pos.x] = NO_PART;
    parts.remove(idx);
}

void move_part(size_t idx, upos x, upos y) {
    // ListNode<Particle>* partNode = parts.at(idx);
    // grid[partNode->current.pos.y * SCREEN_WIDTH + partNode->current.pos.x] = NO_PART;
    // partNode->current.pos = { x, y };
    Particle& part = parts.get(idx);
    grid[part.pos.y * SCREEN_WIDTH + part.pos.x] = NO_PART;
    part.pos = { x, y };
}

//

void init_sim() {
    for (int i = 0; i < MAX_PARTS; i++) {
        grid[i] = NO_PART;
    }
}

void simulate_once() {

}

void draw_pixel(uint8_t x, uint8_t y, uint8_t mul) {
    gfx_FillRectangle(x * mul, y * mul, mul, mul);
}

void render_sim() {
    // Clear screen
    gfx_FillScreen(0);

    for (ListIterator it = parts.iterator(); it.has_current(); it.next()) {
        gfx_SetColor(4);
        draw_pixel(it.current().pos.x, it.current().pos.y, 2);
    }

    gfx_BlitBuffer();
}

int main() {

    // Graphics initialization
    gfx_Begin();

    gfx_SetDrawBuffer();
    gfx_SetPalette(global_palette, sizeof_global_palette, 0);
    gfx_SetTransparentColor(2);

    // Main
    dbg_printf("INIT\n");
    gfx_FillScreen(0);
    gfx_BlitBuffer();

    // simulate_once();
    init_sim();

    for (int i = 0; i < 1000; i++) {
        add_part(i % 100, i / 100, 1);
    }

    for (int i = 0; i < 1000; i++) {
        simulate_once();
        render_sim();
    }

    // sleep(1);
    os_GetKey();

    // Graphics shutdown
    gfx_End();
}