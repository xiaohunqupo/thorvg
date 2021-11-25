/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd. All rights reserved.

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

struct Vertex
{
   Point pt;
   Point uv;
};

struct Polygon
{
   Vertex vertex[3];
};

static inline void _swap(float& a, float& b, float& tmp)
{
    tmp = a;
    a = b;
    b = tmp;
}

//Careful! Shared resource, No support threading
static float dudx, dvdx;
static float dxdya, dxdyb, dudya, dvdya;
static float xa, xb, ua, va;


static inline void _rasterRGBA(SwSurface* surface, const SwImage* image, const SwBBox& region, int ystart, int yend, TVG_UNUSED uint32_t opacity, TVG_UNUSED uint32_t (*blendMethod)(uint32_t))
{
    float _dudx = dudx, _dvdx = dvdx;
    float _dxdya = dxdya, _dxdyb = dxdyb, _dudya = dudya, _dvdya = dvdya;
    float _xa = xa, _xb = xb, _ua = ua, _va = va;
    auto sbuf = image->data;
    auto dbuf = surface->buffer;
    int sw = static_cast<int>(image->stride);
    int sh = image->h;
    int dw = surface->stride;
    int x1, x2, x, y, ar, ab, iru, irv, px;
    int vv = 0;
    int uu = 0;
    float dx, u, v, iptr;
    uint32_t* buf;

    //Range exception handling
    if (ystart >= region.max.y) return;
    if (ystart < region.min.y) ystart = region.min.y;
    if (yend > region.max.y) yend = region.max.y;

    //Loop through all lines in the segment
    y = ystart;

    while (y < yend) {
        x1 = _xa;
        x2 = _xb;

        //Range exception handling
        if (x1 < region.min.x) x1 = region.min.x;
        if (x2 > region.max.x) x2 = region.max.x;

        if ((x2 - x1) < 1) goto next;
        if ((x1 >= region.max.x) || (x2 <= region.min.x)) goto next;

        //Perform subtexel pre-stepping on UV
        dx = 1 - (_xa - x1);
        u = _ua + dx * _dudx;
        v = _va + dx * _dvdx;

        buf = dbuf + ((y * dw) + x1);

        x = x1;

#ifdef TEXMAP_MAKSING
        auto cmp = &surface->compositor->image.data[y * surface->compositor->image.stride + x1];
#endif
        //Draw horizontal line
        while (x++ < x2) {
            uu = (int) u;
            vv = (int) v;
            /* FIXME: sometimes u and v are < 0 - don'tc crash */
            if (uu < 0) uu = 0;
            if (vv < 0) vv = 0;

            /* Range exception handling */
            /* OPTIMIZE ME, handle in advance? */
            if (uu >= sw) uu = sw - 1;
            if (vv >= sh) vv = sh - 1;

            ar = (int)(255 * (1 - modff(u, &iptr)));
            ab = (int)(255 * (1 - modff(v, &iptr)));
            iru = uu + 1;
            irv = vv + 1;
            px = *(sbuf + (vv * sw) + uu);

            /* horizontal interpolate */
            if (iru < sw) {
                /* right pixel */
                int px2 = *(sbuf + (vv * sw) + iru);
                px = _interpolate(ar, px, px2);
            }
            /* vertical interpolate */
            if (irv < sh) {
                /* bottom pixel */
                int px2 = *(sbuf + (irv * sw) + uu);

                /* horizontal interpolate */
                if (iru < sw) {
                    /* bottom right pixel */
                    int px3 = *(sbuf + (irv * sw) + iru);
                    px2 = _interpolate(ar, px2, px3);
                }
                px = _interpolate(ab, px, px2);
            }
#if defined(TEXMAP_MAKSING) && defined(TEXTMAP_TRANSLUCENT)
            auto src = ALPHA_BLEND(px, _multiplyAlpha(opacity, blendMethod(*cmp)));
#elif defined(TEXMAP_MAKSING)
            auto src = ALPHA_BLEND(px, blendMethod(*cmp));
#elif defined(TEXTMAP_TRANSLUCENT)
            auto src = ALPHA_BLEND(px, opacity);
#else
            auto src = px;
#endif
            *buf = src + ALPHA_BLEND(*buf, surface->blender.ialpha(src));
            ++buf;
#ifdef TEXMAP_MAKSING
            ++cmp;
#endif
            //Step UV horizontally
            u += _dudx;
            v += _dvdx;
        }
next:
        //Step along both edges
        _xa += _dxdya;
        _xb += _dxdyb;
        _ua += _dudya;
        _va += _dvdya;

        y++;
    }
    xa = _xa;
    xb = _xb;
    ua = _ua;
    va = _va;
}

static inline void _rasterPolygonImageSegment(SwSurface* surface, const SwImage* image, const SwBBox& region, int ystart, int yend, uint32_t opacity, uint32_t (*blendMethod)(uint32_t))
{
#define TEXTMAP_TRANSLUCENT
#define TEXMAP_MAKSING
    _rasterRGBA(surface, image, region, ystart, yend, opacity, blendMethod);
#undef TEXMAP_MASKING
#undef TEXTMAP_TRANSLUCENT
}


static inline void _rasterPolygonImageSegment(SwSurface* surface, const SwImage* image, const SwBBox& region, int ystart, int yend, uint32_t (*blendMethod)(uint32_t))
{
#define TEXMAP_MAKSING
    _rasterRGBA(surface, image, region, ystart, yend, 255, nullptr);
#undef TEXMAP_MASKING
}


static inline void _rasterPolygonImageSegment(SwSurface* surface, const SwImage* image, const SwBBox& region, int ystart, int yend, uint32_t opacity)
{
#define TEXTMAP_TRANSLUCENT
    _rasterRGBA(surface, image, region, ystart, yend, opacity, nullptr);
#undef TEXTMAP_TRANSLUCENT
}


static inline void _rasterPolygonImageSegment(SwSurface* surface, const SwImage* image, const SwBBox& region, int ystart, int yend)
{
    _rasterRGBA(surface, image, region, ystart, yend, 255, nullptr);
}


/* This mapping algorithm is based on Mikael Kalms's. */
static void _rasterPolygonImage(SwSurface* surface, const SwImage* image, const SwBBox& region, uint32_t opacity, Polygon& polygon, uint32_t (*blendMethod)(uint32_t))
{
    float x[3] = {polygon.vertex[0].pt.x, polygon.vertex[1].pt.x, polygon.vertex[2].pt.x};
    float y[3] = {polygon.vertex[0].pt.y, polygon.vertex[1].pt.y, polygon.vertex[2].pt.y};
    float u[3] = {polygon.vertex[0].uv.x, polygon.vertex[1].uv.x, polygon.vertex[2].uv.x};
    float v[3] = {polygon.vertex[0].uv.y, polygon.vertex[1].uv.y, polygon.vertex[2].uv.y};

    float off_y;
    float dxdy[3] = {0.0f, 0.0f, 0.0f};
    float tmp;

    auto upper = false;

    //Sort the vertices in ascending Y order
    if (y[0] > y[1]) {
        _swap(x[0], x[1], tmp);
        _swap(y[0], y[1], tmp);
        _swap(u[0], u[1], tmp);
        _swap(v[0], v[1], tmp);
    }
    if (y[0] > y[2])  {
        _swap(x[0], x[2], tmp);
        _swap(y[0], y[2], tmp);
        _swap(u[0], u[2], tmp);
        _swap(v[0], v[2], tmp);
    }
    if (y[1] > y[2]) {
        _swap(x[1], x[2], tmp);
        _swap(y[1], y[2], tmp);
        _swap(u[1], u[2], tmp);
        _swap(v[1], v[2], tmp);
    }

    //Y indexes
    int yi[3] = {(int)y[0], (int)y[1], (int)y[2]};

    //Skip drawing if it's too thin to cover any pixels at all.
    if ((yi[0] == yi[1] && yi[0] == yi[2]) || ((int) x[0] == (int) x[1] && (int) x[0] == (int) x[2])) return;

    //Calculate horizontal and vertical increments for UV axes (these calcs are certainly not optimal, although they're stable (handles any dy being 0)
    auto denom = ((x[2] - x[0]) * (y[1] - y[0]) - (x[1] - x[0]) * (y[2] - y[0]));

    //Skip poly if it's an infinitely thin line
    if (mathZero(denom)) return;

    denom = 1 / denom;   //Reciprocal for speeding up
    dudx = ((u[2] - u[0]) * (y[1] - y[0]) - (u[1] - u[0]) * (y[2] - y[0])) * denom;
    dvdx = ((v[2] - v[0]) * (y[1] - y[0]) - (v[1] - v[0]) * (y[2] - y[0])) * denom;
    auto dudy = ((u[1] - u[0]) * (x[2] - x[0]) - (u[2] - u[0]) * (x[1] - x[0])) * denom;
    auto dvdy = ((v[1] - v[0]) * (x[2] - x[0]) - (v[2] - v[0]) * (x[1] - x[0])) * denom;

    //Calculate X-slopes along the edges
    if (y[1] > y[0]) dxdy[0] = (x[1] - x[0]) / (y[1] - y[0]);
    if (y[2] > y[0]) dxdy[1] = (x[2] - x[0]) / (y[2] - y[0]);
    if (y[2] > y[1]) dxdy[2] = (x[2] - x[1]) / (y[2] - y[1]);

    //Determine which side of the polygon the longer edge is on
    auto side = (dxdy[1] > dxdy[0]) ? true : false;

    if (mathEqual(y[0], y[1])) side = x[0] > x[1];
    if (mathEqual(y[1], y[2])) side = x[2] > x[1];

    //Longer edge is on the left side
    if (!side) {
        //Calculate slopes along left edge
        dxdya = dxdy[1];
        dudya = dxdya * dudx + dudy;
        dvdya = dxdya * dvdx + dvdy;

        //Perform subpixel pre-stepping along left edge
        auto dy = 1.0f - (y[0] - yi[0]);
        xa = x[0] + dy * dxdya;
        ua = u[0] + dy * dudya;
        va = v[0] + dy * dvdya;

        //Draw upper segment if possibly visible
        if (yi[0] < yi[1]) {
            off_y = y[0] < region.min.y ? (region.min.y - y[0]) : 0;
            xa += (off_y * dxdya);
            ua += (off_y * dudya);
            va += (off_y * dvdya);

            // Set right edge X-slope and perform subpixel pre-stepping
            dxdyb = dxdy[0];
            xb = x[0] + dy * dxdyb + (off_y * dxdyb);

            if (blendMethod) {
                if (opacity == 255) _rasterPolygonImageSegment(surface, image, region, yi[0], yi[1], blendMethod);
                else _rasterPolygonImageSegment(surface, image, region, yi[0], yi[1], opacity, blendMethod);
            } else {
                if (opacity == 255) _rasterPolygonImageSegment(surface, image, region, yi[0], yi[1]);
                else _rasterPolygonImageSegment(surface, image, region, yi[0], yi[1], opacity);
            }

            upper = true;
        }
        //Draw lower segment if possibly visible
        if (yi[1] < yi[2]) {
            off_y = y[1] < region.min.y ? (region.min.y - y[1]) : 0;
            if (!upper) {
                xa += (off_y * dxdya);
                ua += (off_y * dudya);
                va += (off_y * dvdya);
            }
            // Set right edge X-slope and perform subpixel pre-stepping
            dxdyb = dxdy[2];
            xb = x[1] + (1 - (y[1] - yi[1])) * dxdyb + (off_y * dxdyb);
            if (blendMethod) {
                if (opacity == 255) _rasterPolygonImageSegment(surface, image, region, yi[1], yi[2], blendMethod);
                else _rasterPolygonImageSegment(surface, image, region, yi[1], yi[2], opacity, blendMethod);
            } else {
                if (opacity == 255) _rasterPolygonImageSegment(surface, image, region, yi[1], yi[2]);
                else _rasterPolygonImageSegment(surface, image, region, yi[1], yi[2], opacity);
            }
        }
    //Longer edge is on the right side
    } else {
        //Set right edge X-slope and perform subpixel pre-stepping
        dxdyb = dxdy[1];
        auto dy = 1.0f - (y[0] - yi[0]);
        xb = x[0] + dy * dxdyb;

        //Draw upper segment if possibly visible
        if (yi[0] < yi[1]) {
            off_y = y[0] < region.min.y ? (region.min.y - y[0]) : 0;
            xb += (off_y *dxdyb);

            // Set slopes along left edge and perform subpixel pre-stepping
            dxdya = dxdy[0];
            dudya = dxdya * dudx + dudy;
            dvdya = dxdya * dvdx + dvdy;

            xa = x[0] + dy * dxdya + (off_y * dxdya);
            ua = u[0] + dy * dudya + (off_y * dudya);
            va = v[0] + dy * dvdya + (off_y * dvdya);

            if (blendMethod) {
                if (opacity == 255) _rasterPolygonImageSegment(surface, image, region, yi[0], yi[1], blendMethod);
                else _rasterPolygonImageSegment(surface, image, region, yi[0], yi[1], opacity, blendMethod);
            } else {
                if (opacity == 255) _rasterPolygonImageSegment(surface, image, region, yi[0], yi[1]);
                else _rasterPolygonImageSegment(surface, image, region, yi[0], yi[1], opacity);
            }

            upper = true;
        }
        //Draw lower segment if possibly visible
        if (yi[1] < yi[2]) {
            off_y = y[1] < region.min.y ? (region.min.y - y[1]) : 0;
            if (!upper) xb += (off_y *dxdyb);

            // Set slopes along left edge and perform subpixel pre-stepping
            dxdya = dxdy[2];
            dudya = dxdya * dudx + dudy;
            dvdya = dxdya * dvdx + dvdy;
            dy = 1 - (y[1] - yi[1]);
            xa = x[1] + dy * dxdya + (off_y * dxdya);
            ua = u[1] + dy * dudya + (off_y * dudya);
            va = v[1] + dy * dvdya + (off_y * dvdya);

            if (blendMethod) {
                if (opacity == 255) _rasterPolygonImageSegment(surface, image, region, yi[1], yi[2], blendMethod);
                else _rasterPolygonImageSegment(surface, image, region, yi[1], yi[2], opacity, blendMethod);
            } else {
                if (opacity == 255) _rasterPolygonImageSegment(surface, image, region, yi[1], yi[2]);
                else _rasterPolygonImageSegment(surface, image, region, yi[1], yi[2], opacity);
            }
        }
    }
}


/*
    2 triangles constructs 1 mesh.
    below figure illustrates vert[4] index info.
    If you need better quality, please divide a mesh by more number of triangles.

    0 -- 1
    |  / |
    | /  |
    3 -- 2 
*/
static bool _rasterTexmapPolygon(SwSurface* surface, const SwImage* image, const Matrix* transform, const SwBBox& region, uint32_t opacity, uint32_t (*blendMethod)(uint32_t))
{
   /* Prepare vertices.
      shift XY coordinates to match the sub-pixeling technique. */
    Vertex vertices[4];
    vertices[0] = {{0.0f, 0.0f}, 0.0f, 0.0f};
    vertices[1] = {{float(image->w), 0.0f}, float(image->w), 0.0f};
    vertices[2] = {{float(image->w), float(image->h)}, float(image->w), float(image->h)};
    vertices[3] = {{0.0f, float(image->h)}, 0.0f, float(image->h)};

    for (int i = 0; i < 4; i++) mathMultiply(&vertices[i].pt, transform);

    Polygon polygon;

    //Draw the first polygon
    polygon.vertex[0] = vertices[0];
    polygon.vertex[1] = vertices[1];
    polygon.vertex[2] = vertices[3];

    _rasterPolygonImage(surface, image, region, opacity, polygon, blendMethod);

    //Draw the second polygon
    polygon.vertex[0] = vertices[1];
    polygon.vertex[1] = vertices[2];
    polygon.vertex[2] = vertices[3];

    _rasterPolygonImage(surface, image, region, opacity, polygon, blendMethod);
    
    return true;
}