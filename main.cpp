/*
 * This file is part of the PLATFORM_OBJ distribution
 * {https://github.com/amatarazzo777/platform_obj). Copyright (c) 2020 Anthony
 * Matarazzo.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/**
\author Anthony Matarazzo
\file main.cpp
\date 9/7/20
\version 1.0
\brief
*/

#include "uxdevice.hpp"

using namespace std;
using namespace uxdevice;

void show_time(surface_area_t &vis, double x, double y);
std::shared_ptr<std::string> insert_text(surface_area_t &vis, bool bfast,
                                         string &stxt);
void draw_shapes(surface_area_t &vis);
void draw_images(surface_area_t &vis);
void draw_text(surface_area_t &vis, bool bfast);
void draw_lines(surface_area_t &vis);
std::string generate_text(void);

void event_dispatch(const event &evt);
void handle_error(const std::string errText) {
  fprintf(stderr, "%s", errText.data());
}

// inline PNG data using the RFC2397 base 64 encoding scheme.
// This may be useful in some cases, yet raw byte data will be smaller in memory
// description but larger in the text_color_t description perhaps unless the c++
// RAW literanl input format is used for literal string data.
const char *stripes =
    "data:image/"
    "png;base64,iVBORw0KGgoAAAANSUhEUgAAACsAAAARCAYAAABEvFULAAAABmJLR0QA/wD/"
    "AP+"
    "gvaeTAAAACXBIWXMAAAsTAAALEwEAmpwYAAAAB3RJTUUH5AQZFBsOzDGg0AAABQZJREFUSMe"
    "llttvVFUUxn9r98x0oLRAW8pNLKUBrG1pC3ItEIqAQNSoGCEQQQNG/"
    "gvfjD74QIwmPpCIGgWCJiogQaQgUAWDUC4lpUCBWq51GEovM9Nz9vLhdJiZthQMK5mH2Ze1v"
    "rXWt759ZMtW52Z3F6PFgOn9Jcx1wXOT/"
    "wPB9P1UcxxYstJQkLsIS4RvvvwbgMwQLHvZ0NqiXG9W7kfQ7i7/"
    "fCAIBaNFSiuEvLyxBGQqrl5j357L3Lnln0ngmFIiyOdfO3bWPEP4X+"
    "XiBSUWTQKYPlt4vmQmRvJAY5w5X8v5eiURrK9ZC8EgxOPJpFzXP5tfALl5wsg8wfOUC2eV+"
    "5Gkn564f/"
    "aFuUJl1WQaG5u41qxEwhCL+"
    "r7l488cm6iM40A0mg4kFoW31huyh1Yi5CCEuH5jP8ePWh60Dww6FfyUEmHaDCEzMJYMGQ0Yh"
    "KEIQ4npr/"
    "zyo0esN6brQmeHf7dihjC9qhohBDh4XEVauoNeppkDBAGLp618v+OypFY4USHwg8+"
    "cZwhKNY1NRzhxTHlSS/hQC5MmCwtrSjDkI6RnfPpsLQ31SmcHbHg/"
    "m4BU+Em2RvO8wwfauXtbJdVhKhcTrXzU3tNYIAjViwzjxz2HIT9tr7HpCHWHFWNg3bvjkQ+"
    "3+DSYWCxUzBDNHTlBRbJR7aSh4ar8dUxFTP92DwbUWn8wrU2uGQMD+UmtenaOX/"
    "HSCiFkFgEGxeWXvYdouwNyIzrWO3miTe7dU7KzhbnVUzWZoQNYLGH2/"
    "HROIuEnr1ZRsTBndo0qcZQ44GL1AUoHFxpuyfEjfsUCwf6JpiaZKAyAfPSpYzMcP/"
    "NEpRYsNhQWZXL2dJQZVTUKcPRYrVxpenJ+"
    "JgIuf9UwJn9x70WLkkDiz0dLSzMH9loZTBYTJp984TzMIzcfhgwRrjUrnutrpOtCeZVwqVHp"
    "if9/Tib43qupjBqDFhaJZA0pxJFiFFfBlXB7HQf2Wjo7BqZKIAiyc1/"
    "A5hX4OldeOYyAVGmk46hkZQ1n+7Z7TzU8rtv/"
    "oUl9bHJGwJtrpmAYB1iEIM3XD1K73w44I3I3tsxTXJQOlDiq3VhusX9Pp4Tb+"
    "svOYLra19ZsGKJnT0el/qQ+8l5PHNa/"
    "NwpHSlLT4VDtHzRfTr8nu48G7LlTPqXEPBpQaYXotMpxcrzuBq0t6S/doywWhU2b56mnt/"
    "lq62UZTAne3jgeR4pTVg2eXuPwwau03fXjmcbzSiDocyLxig1k9SdVhJBWV1fr6jVLNL/"
    "g8WAzQ+BqE44UMiK3P8DqRQZr/ZjfbWtF6UgdUTJkAotfXMCq1cX0xCHjpVfMB0/"
    "SUmPgYuM9KS2bBMSZPPlZyiqU0kqPCUWeRMJK+/3+/"
    "Gxq7JKysiJi7lW5fbOP7gqsfC1Lz5zqERG40HCTsvLCvpFpunSaG/"
    "+AGeg5HKytu3YekrZInQghMqSQgFSRP2K+rli5UN/ZNF3fWGs0lSId7eBxh/"
    "Lycu1LnStNSlDmsmqtUWt9/"
    "loi6XKFw5+9mmwSmjh2vLBuY4b2FeS+1t0FP++y7Nh+QO6GfdBge6d5KFnB+"
    "WzcPF1HjUlKjqfNZJDPMxPTfamFHq1nWGg+r68xCrDz2zPpdNHmh5+"
    "pBmD0OFi6tEaDMpspJfJYSjiOX+XdP/"
    "igw+11JJvkg16xfLFmhvyVSxe7RLHMmmfIGZHs4LAcej9SXOp+t5JQh4O/HektABgZ/"
    "jDuf4YrMMm0cOGnAAAAAElFTkSuQmCC";

// inline SVG, you may build this at run time or parameterize certain aspects
// of the style. Not all of the blocks below are required,
// such as meta data. The parsing is fast, however the information is still
// transposed from text. The biggest slowdowns are the effects that use the
// Gaussian blur functions. This uses the RSVG api for its rendering and
// parsing. Files may be drawn in the inkscape application and transposed at
// this layer.
std::string sSVG_BUTTON =
    R"data(<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg
   xmlns:dc="http://purl.org/dc/elements/1.1/"
   xmlns:cc="http://creativecommons.org/ns#"
   xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
   xmlns:svg="http://www.w3.org/2000/svg"
   xmlns="http://www.w3.org/2000/svg"
   xmlns:xlink="http://www.w3.org/1999/xlink"
   xmlns:sodipodi="http://sodipodi.text_color_tforge.net/DTD/sodipodi-0.dtd"
   xmlns:inkscape="http://www.inkscape.org/namespaces/inkscape"
   enable-background="new"
   id="svg8"
   version="1.1"
   viewBox="0 0 210 297"
   height="297mm"
   width="210mm"
   sodipodi:docname="button.svg"
   inkscape:version="0.92.3 (2405546, 2018-03-11)">
  <sodipodi:namedview
     pagecolor="#ffffff"
     bordercolor="#666666"
     borderopacity="1"
     objecttolerance="10"
     gridtolerance="10"
     guidetolerance="10"
     inkscape:pageopacity="0"
     inkscape:pageshadow="2"
     inkscape:window-width="1366"
     inkscape:window-height="743"
     id="namedview834"
     showgrid="false"
     inkscape:zoom="0.4944234"
     inkscape:cx="193.34797"
     inkscape:cy="741.58772"
     inkscape:window-x="0"
     inkscape:window-y="0"
     inkscape:window-maximized="1"
     inkscape:current-layer="g1004" />
  <defs
     id="defs2">
    <linearGradient
       id="linearGradient1023">
      <stop
         id="stop1019"
         offset="0"
         style="stop-color:#333333;stop-opacity:1;" />
      <stop
         id="stop1021"
         offset="1"
         style="stop-color:#333333;stop-opacity:0;" />
    </linearGradient>
    <linearGradient
       id="linearGradient995">
      <stop
         id="stop991"
         offset="0"
         style="stop-color:#ececec;stop-opacity:1;" />
      <stop
         id="stop993"
         offset="1"
         style="stop-color:#ececec;stop-opacity:0;" />
    </linearGradient>
    <radialGradient
       gradientUnits="userSpaceOnUse"
       gradientTransform="matrix(1,0,0,0.27292226,0,343.86233)"
       r="319.82966"
       fy="294.56268"
       fx="331.81046"
       cy="294.56268"
       cx="331.81046"
       id="radialGradient997"
       xlink:href="#linearGradient995" />
    <linearGradient
       gradientUnits="userSpaceOnUse"
       y2="146.22223"
       x2="108.20866"
       y1="172.41464"
       x1="107.85616"
       id="linearGradient1025"
       xlink:href="#linearGradient1023" />
    <filter
       height="1.708382"
       y="-0.35419101"
       width="1.1541983"
       x="-0.077099127"
       id="filter1027"
       style="color-interpolation-filters:sRGB">
      <feGaussianBlur
         id="feGaussianBlur1029"
         stdDeviation="19.09772" />
    </filter>
    <filter
       height="1.0443415"
       y="-0.022170732"
       width="1.0164525"
       x="-0.0082262443"
       id="filter1031"
       style="color-interpolation-filters:sRGB">
      <feGaussianBlur
         id="feGaussianBlur1033"
         stdDeviation="0.57263392" />
    </filter>
    <filter
       inkscape:label="Combined Lighting"
       inkscape:menu="Bevels"
       inkscape:menu-tooltip="Basic specular bevel to use for building textures"
       style="color-interpolation-filters:sRGB;"
       id="filter891">
      <feColorMatrix
         type="luminanceToAlpha"
         result="result2"
         id="feColorMatrix883" />
      <feComposite
         in2="SourceGraphic"
         operator="arithmetic"
         in="result2"
         k3="2"
         result="result4"
         id="feComposite885" />
      <feBlend
         in="result4"
         mode="multiply"
         result="result3"
         in2="result2"
         id="feBlend887" />
      <feComposite
         in2="SourceGraphic"
         operator="in"
         in="result3"
         id="feComposite889"
         result="fbSourceGraphic" />
      <feColorMatrix
         result="fbSourceGraphicAlpha"
         in="fbSourceGraphic"
         values="0 0 0 -1 0 0 0 0 -1 0 0 0 0 -1 0 0 0 0 1 0"
         id="feColorMatrix1250" />
      <feGaussianBlur
         id="feGaussianBlur1252"
         stdDeviation="6"
         in="fbSourceGraphic"
         result="result0" />
      <feDiffuseLighting
         id="feDiffuseLighting1254"
         lighting-color="rgb(255,255,255)"
         diffuseConstant="1"
         surfaceScale="4"
         result="result5">
        <feDistantLight
           id="feDistantLight1256"
           elevation="45"
           azimuth="235" />
      </feDiffuseLighting>
      <feComposite
         in2="fbSourceGraphic"
         id="feComposite1258"
         k1="1.4"
         in="result5"
         result="fbSourceGraphic"
         operator="arithmetic" />
      <feGaussianBlur
         id="feGaussianBlur1260"
         result="result0"
         in="fbSourceGraphic"
         stdDeviation="6" />
      <feSpecularLighting
         id="feSpecularLighting1262"
         specularExponent="25"
         specularConstant="1"
         surfaceScale="4"
         lighting-color="rgb(255,255,255)"
         result="result1"
         in="result0">
        <feDistantLight
           id="feDistantLight1264"
           azimuth="235"
           elevation="45" />
      </feSpecularLighting>
      <feComposite
         in2="result1"
         id="feComposite1266"
         k3="1"
         k2="1"
         operator="arithmetic"
         in="fbSourceGraphic"
         result="result4" />
      <feComposite
         in2="fbSourceGraphic"
         id="feComposite1268"
         operator="in"
         result="result2"
         in="result4" />
    </filter>
  </defs>
  <metadata
     id="metadata5">
    <rdf:RDF>
      <cc:Work
         rdf:about="">
        <dc:format>image/svg+xml</dc:format>
        <dc:type
           rdf:retext_color_t="http://purl.org/dc/dcmitype/StillImage" />
        <dc:title />
      </cc:Work>
    </rdf:RDF>
  </metadata>
  <g
     id="layer1">
    <g
       id="g1004"
       transform="translate(-23.545946,-107.02703)"
       style="stroke:#0085ec;stroke-opacity:1">
      <rect
         style="opacity:1;fill:#000080;stroke:#0085ec;stroke-width:0.86500001;stroke-linecap:round;stroke-linejoin:bevel;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1;filter:url(#filter891)"
         id="rect10"
         width="167.06548"
         height="61.988094"
         x="25.702381"
         y="108.0119"
         rx="17.41297"
         ry="14.174099"
         transform="rotate(1.2541334,109.23514,139.00585)"
         inkscape:label="TEXT" />
      <rect
         ry="14.174099"
         rx="17.41297"
         y="108.0119"
         x="25.702381"
         height="61.988094"
         width="167.06548"
         id="rect22"
         style="opacity:1;fill:url(#linearGradient1025);fill-opacity:1;stroke:#0085ec;stroke-width:0.86500001;stroke-linecap:round;stroke-linejoin:bevel;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1;filter:url(#filter1031)" />
      <path
         inkscape:connector-curvature="0"
         style="opacity:0.88235294;fill:url(#radialGradient997);fill-opacity:1;stroke:#0085ec;stroke-width:2.90628815;stroke-linecap:round;stroke-linejoin:bevel;stroke-miterlimit:4;stroke-dasharray:none;stroke-opacity:1;filter:url(#filter1027)"
         d="m 162.95508,408.23438 c -14.27697,0 -27.45121,3.67612 -38.21289,9.91601 -0.21918,1.2749 0.11022,-1.27702 0,0 5.3e-4,63.87864 132.92381,119.49047 297.50195,119.49023 153.02058,-9.7e-4 281.16876,-44.98588 296.49219,-104.08007 -11.58278,-15.22205 -32.27443,-25.32617 -55.97852,-25.32617 z"
         transform="matrix(0.26326767,-0.0263528,0.033347,0.33314049,-26.454344,-14.63163)"
         id="rect862" />
    </g>
  </g>
</svg>

)data";

/****************************************************************************************************
***************************************************************************************************/

std::random_device rd;
std::mt19937 gen(rd());
// events such as mouse and keyboard input are distributed here.
// The area named during the event is tied to the mouse and keyboard
// focus input.
double sx = 1, sy = 1;
double mx = 0, my = 0;
double ox = 0, oy = 0;

#define FAST_TEXT true
#define DRAW_SLEEP 100
#define NUM_SEGMENTS 10

#if defined(__linux__)
int main(int argc, char **argv) {
  // handle command line here...
#elif defined(_WIN64)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /* hPrevInstance */,
                   LPSTR lpCmdLine, int /* nCmdShow */) {
  // command line
#endif

  std::uniform_int_distribution<> motime(500, 5000);

  std::uniform_real_distribution<> color(0, 1.0);
  std::uniform_real_distribution<> opac(.9, 1.0);

  std::uniform_real_distribution<> coord(25.0, 100.0);
  std::uniform_real_distribution<> ang(-10, 10);
#define _C color(gen)
#define _A opac(gen)

  surface_area_t vis = surface_area_t({500, 500}, "Information Title",
                                      painter_brush_t("darkorange"));

  vis << listen_keypress_t([&vis](auto &evt) {
    string s = " ";
    s[0] = evt.key;
    // draw_text(vis, true, s);
  });

  vis << listen_mousemove_t([](auto &evt) {
    if (evt.y < my) {
      oy = oy - .1;
    } else {
      oy = oy + .1;
    }
    if (oy < 0)
      oy = 0;
    // vis.deviceOffset(ox,oy);
    mx = evt.x;
    my = evt.y;
  });

  vis.clear();
  // draw_lines(vis);
  std::shared_ptr<std::string> paragraph_text =
      std::make_shared<std::string>("starting text");
  std::shared_ptr<std::string> button_caption =
      std::make_shared<std::string>("button text");

  // easily index  properties for specific access later.
  // creating shared objects allows for some interface architectures
  // to be crafted easier.
  vis << text_font_t("28px").index("paragraphfont");
  vis << text_shadow_t("green") << coordinate_t{0, 100, 600, 300}
      << text_color_t("white") << paragraph_text << '\n';

  vis[paragraph_text] =
      "New text is applied without an indirect index, more simplified syntax. ";
  vis.get<text_font_t>("paragraphfont").description = "40px";

  for (int i = 0; i < 5; i++) {
    vis << coordinate_t{i * 130.0, 200, 150, 240} << image_block_t{sSVG_BUTTON}
        << text_shadow_t("black")
        << text_fill_t(0, 0, 5, 30, {{"orange"}, {"yellow"}})
        << text_outline_t(stripes) << text_font_t("16px") << line_width_t(5)
        << coordinate_t{20.0 + i * 130.0, 210, 150, 240} << button_caption;
  }

  vis.notify_complete();
  while (vis.processing()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(DRAW_SLEEP));

    // show_time(vis, 0, 0);
    vis[paragraph_text] = generate_text();

    // window_background.value

    vis.notify_complete();
  }

  return 0;
}

void show_time(surface_area_t &vis, double x, double y) {
  std::time_t t = std::time(nullptr);
  char mbstr[100];
  std::strftime(mbstr, sizeof(mbstr), "%A %c", std::localtime(&t));

  vis << text_render_fast_t{} << text_font_t("28px")
      << text_shadow_t("darkgrey") << coordinate_t{x, y, 600, 300}
      << text_color_t(0, 0, 10, 10, {{"white"}, {"grey"}}) << mbstr << "  "
      << '\n';
}

std::shared_ptr<std::string> insert_text(surface_area_t &vis, bool bfast,
                                         string &stxt) {

  std::uniform_real_distribution<> scrn(0, 400.0);
  std::uniform_real_distribution<> dcir(25.0, 100.0);
  std::uniform_real_distribution<> color(.5, 1.0);
  std::uniform_real_distribution<> opac(.7, 1);
  std::uniform_real_distribution<> lw(0, 10.0);
  std::uniform_real_distribution<> coord(425.0, 600.0);
  std::uniform_int_distribution<> shape(1, 4);

#define _C color(gen)
#define _A opac(gen)
  std::shared_ptr<std::string> ps = std::make_shared<std::string>(stxt);

  std::uniform_int_distribution<> fill(1, 2);

  if (bfast) {
    vis << text_render_fast_t{}
        << text_alignment_t{text_alignment_options_t::left}
        << coordinate_t{10, 10, 300, 300} << ps;

  } else {

    vis << text_fill_t{coord(gen),
                       coord(gen),
                       coord(gen),
                       coord(gen),
                       {{_C, _C, _C, _C, _A},
                        {_C, _C, _C, _C, _A},
                        {_C, _C, _C, _C, _A}}}
        << text_outline_t{coord(gen),
                          coord(gen),
                          coord(gen),
                          coord(gen),
                          {{_C, _C, _C, _C, _A},
                           {_C, _C, _C, _C, _A},
                           {_C, _C, _C, _C, _A}}}
        << text_shadow_t{"green"} << line_width_t{lw(gen)}
        << text_alignment_t{text_alignment_options_t::left}
        << coordinate_t{10, 10, 300, 300} << ps;
  }
  return ps;
}

std::string generate_text(void) {
  std::string ret;

  std::uniform_int_distribution<> info(1, 5);
  switch (info(gen)) {
  case 1:
    ret = "Silver colored crafts from another galaxy seem "
          "curiously welcomed as the memorizing audio waves "
          "produced a canny type of music. A simple ten note. ";
    break;
  case 2:
    ret = "The color of text can be a choice. Yet the appearance is also a "
          "common "
          "desire. Creating animal letters colored with a furry texture is "
          "great for CPU rendering work. Perhaps the flexibility of the API "
          "gives "
          "light to incorporating other types of computer generated graphics. ";
    break;
  case 3:
    ret = "Planets orbit the mass, but this is inconsequential of "
          "the heat provided. As children, we find a balance. ";
    break;
  case 4:
    ret = "The sun sets casting its refraction upon the mountain side. ";
    break;
  case 5:
    ret =
        "The sun sets casting its refraction upon the mountain side. "
        "The glistening oil coats upon the ravens are a remark of healthiness. "
        "One that is pronounced during the day and in the moonlight. "
        "At home, a cave dweller sees this all at once. These are indeed fine "
        "things. "
        "The warmth of the sun decays as thousands of brilliant stars dictate "
        "the continual persistence of the system.  A remarkable sight. A "
        "heavenly home.";
    break;
  }
  return ret;
}
void draw_lines(surface_area_t &vis) {

  std::uniform_real_distribution<> scrn(0, 1000);
  std::uniform_real_distribution<> dcir(5.0, 20.0);
  std::uniform_real_distribution<> dimen(25.0, 300.0);
  std::uniform_real_distribution<> color(0, 1.0);
  std::uniform_real_distribution<> opac(.5, 1);
  std::uniform_real_distribution<> lw(7, 30.0);
  std::uniform_real_distribution<> coord(55.0, 100.0);
  std::uniform_int_distribution<> shape(1, 3);

  vis << coordinate_t(scrn(gen), scrn(gen));

  for (int c = 0; c < NUM_SEGMENTS; c++) {

    switch (shape(gen)) {
    case 1:
      vis << line_t(scrn(gen), scrn(gen));
      break;
    case 2:
      vis << arc_t(scrn(gen), scrn(gen), dimen(gen), dimen(gen), dimen(gen));
      break;
    case 3:
      vis << curve_t(scrn(gen), scrn(gen), scrn(gen), scrn(gen), scrn(gen),
                     scrn(gen));
      break;
    }
  }
  vis << close_path_t();

  vis << line_width_t(lw(gen));
  auto ps = painter_brush_t(
      coord(gen), coord(gen), coord(gen), coord(gen),
      {{_C, _C, _C, _C, 1}, {_C, _C, _C, _C, 1}, {_C, _C, _C, _C, 1}});

  auto pf = painter_brush_t(
      coord(gen), coord(gen), coord(gen), coord(gen),
      {{_C, _C, _C, _C, 1}, {_C, _C, _C, _C, 1}, {_C, _C, _C, _C, 1}});

  vis << stroke_fill_path_t(ps, pf);
}
