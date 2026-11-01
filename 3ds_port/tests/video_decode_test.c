#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "3ds_video.h"

int main(void)
{
    /* All GBA colours survive the reduced atlas format, including black's
     * opaque alpha (transparent index zero is handled separately). */
    assert(CtrVideo_RGBA5551(0x001f) == 0xf801);
    assert(CtrVideo_RGBA5551(0x03e0) == 0x07c1);
    assert(CtrVideo_RGBA5551(0x7c00) == 0x003f);
    assert(CtrVideo_RGBA8(0x001f, true) == 0xff0000ff);
    assert(CtrVideo_RGBA8(0x7c00, false) == 0x0000ff00);
    for (unsigned color = 0; color < 32768; ++color)
    {
        uint16_t packed = CtrVideo_RGBA5551(color);
        assert((packed & 1) == 1);
        assert((packed >> 11) == (color & 31));
        assert(((packed >> 6) & 31) == ((color >> 5) & 31));
        assert(((packed >> 1) & 31) == ((color >> 10) & 31));
    }

    /* Every pixel maps to exactly one in-range Morton texel, and each 8x8
     * tile is contiguous: the decoder's slot*64 optimization relies on it. */
    static unsigned char seen[1024 * 1024];
    for (unsigned y = 0; y < 1024; ++y)
        for (unsigned x = 0; x < 1024; ++x)
        {
            unsigned offset = CtrVideo_Texel(x, y, 1024);
            assert(offset < sizeof(seen) && !seen[offset]);
            seen[offset] = 1;
            assert(offset / 64 == (y / 8) * 128 + x / 8);
        }
    assert(CtrVideo_Texel(1, 0, 8) == 1);
    assert(CtrVideo_Texel(0, 1, 8) == 2);
    assert(CtrVideo_Texel(4, 4, 8) == 48);

    /* Screen-block boundaries, wrapping and OBJ 1D/2D layouts. */
    assert(CtrVideo_TextMapOffset(32, 0, 0) == 0);
    assert(CtrVideo_TextMapOffset(32, 0, 1) == 0x800);
    assert(CtrVideo_TextMapOffset(0, 32, 2) == 0x800);
    assert(CtrVideo_TextMapOffset(32, 32, 3) == 0x1800);
    assert(CtrVideo_TextMapOffset(63, 63, 3) == 0x1ffe);
    assert(CtrVideo_ObjTile(1, 1, 1, 32, true, true) == 10);
    assert(CtrVideo_ObjTile(1, 1, 1, 32, true, false) == 34);
    assert(CtrVideo_ObjTile(1023, 1, 0, 8, false, true) == 0);
    assert(CtrVideo_AffineReference(0x0fffff00) == -256);
    assert(CtrVideo_AffineReference(0x07ffffff) == 0x07ffffff);

    /* Single screen: the bottom screen's framebuffer (column after column,
     * each bottom to top) into a 512x256 texture, every pixel where
     * CtrVideo_Texel puts it and nothing outside the picture touched. */
    {
        enum { W = 320, H = 240, TW = 512, TH = 256 };
        static uint16_t screen[W * H], texels[TW * TH];

        for (unsigned x = 0; x < W; ++x)
            for (unsigned y = 0; y < H; ++y)
                screen[x * H + (H - 1 - y)] = (uint16_t)(x * 241 + y * 7 + 1);
        memset(texels, 0, sizeof(texels));
        CtrVideo_ScreenToTexels(texels, TW, screen, W, H);
        for (unsigned y = 0; y < TH; ++y)
            for (unsigned x = 0; x < TW; ++x)
            {
                uint16_t want = x < W && y < H ? (uint16_t)(x * 241 + y * 7 + 1) : 0;

                assert(texels[CtrVideo_Texel(x, y, TW)] == want);
            }
        /* The top-left pixel is the first column's last entry. */
        assert(texels[CtrVideo_Texel(0, 0, TW)] == screen[H - 1]);
    }
    /* The battle box band's pages (textbox_map.bin's top frame rows): the
     * message box is one box to its right cap; the action menu's box starts
     * at column 15 beside the prompt's; the PP and type box at column 19,
     * with the moves box's right edge. An empty row is no box. */
    {
        static const uint16_t message[30] = {
            0x0003, 0x0004, 0x0005, 0x0005, 0x0005, 0x0005, 0x0005, 0x0005, 0x0005, 0x0005,
            0x0005, 0x0005, 0x0005, 0x0005, 0x0005, 0x0005, 0x0005, 0x0005, 0x0005, 0x0005,
            0x0005, 0x0005, 0x0005, 0x0005, 0x0005, 0x0005, 0x0005, 0x0005, 0x0006, 0x0007};
        static const uint16_t action[30] = {
            0x0003, 0x0004, 0x0005, 0x0005, 0x0005, 0x0005, 0x0005, 0x0005, 0x0005, 0x0005,
            0x0005, 0x0005, 0x0005, 0x0005, 0x0005, 0x1012, 0x1013, 0x1013, 0x1013, 0x1013,
            0x1013, 0x1013, 0x1013, 0x1013, 0x1013, 0x1013, 0x1013, 0x1013, 0x1013, 0x1014};
        static const uint16_t moves[30] = {
            0x1012, 0x1013, 0x1013, 0x1013, 0x1013, 0x1013, 0x1013, 0x1013, 0x1013, 0x1013,
            0x1013, 0x1013, 0x1013, 0x1013, 0x1013, 0x1013, 0x1013, 0x1013, 0x1013, 0x1014,
            0x1012, 0x1013, 0x1013, 0x1013, 0x1013, 0x1013, 0x1013, 0x1013, 0x1013, 0x1014};
        static const uint16_t empty[30];

        assert(CtrVideo_BandSecondBox(message, 30, 2) == 0);
        assert(CtrVideo_BandSecondBox(action, 30, 2) == 15);
        assert(CtrVideo_BandSecondBox(moves, 30, 2) == 19);
        assert(CtrVideo_BandSecondBox(empty, 30, 2) == 0);
    }
    puts("PASS video decode: 32768 colours, full atlas, BG/OBJ addressing, affine sign, screen to texels, "
         "battle box pages");
    return 0;
}
