
#include "uxdevice.hpp"

using namespace std;
using namespace uxdevice;

void test0(platform &vm);

void testStart(string_view sFunc) {
#if defined(CONSOLE)
  cout << sFunc << endl;
#elif defined(_WIN64)

#endif
}

void eventDispatch(const event &evt);
void handleError(const std::string errText) {
  fprintf(stderr, "%s", errText.data());
}

/****************************************************************************************************
***************************************************************************************************/
#if defined(__linux__)
int main(int argc, char **argv) {
  // handle command line here...
#elif defined(_WIN64)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE /* hPrevInstance */,
                   LPSTR lpCmdLine, int /* nCmdShow */) {
  // command line
#endif

  // create the main window area. This may this is called a Viewer object.
  // The main browsing window. It is an element as well.
  auto vis = platform(eventDispatch, handleError);
  vis.openWindow("Information Title", 800, 600);

  vis.clear();

  vis.scale(1.1, 1.1);

  vis.antiAlias(antialias::subPixel);

  vis.textAlignment(alignment::left);
  vis.text(
      "The sun sets casting its refraction upon the mountain side. "
      "The glistening oil coats upon the ravens are a remark of healthiness. "
      "One that is pronounced during the day and in the moonlight. "
      "At home, a cave dweller sees this all at once. These are indeed fine "
      "things. "
      "The warmth of the sun decays as thousands of brilliant stars dictate "
      "the continual persistence of the system.  A remarkable sight. A "
      "heavenly "
      "home.");
  vis.font("DejaVu Sans Bold 14");
  vis.area(0, 0, 800, 600, 120, 120);

  Paint tiger = Paint("/home/anthony/development/platform/23.svg", 50, 50);
  tiger.rotate(PI / 180 * 10);
  tiger.extend(extendType::reflect);
  tiger.filter(filterType::bilinear);

  vis.pen("lime");
  vis.lineWidth(5);
  vis.background(tiger);
  vis.drawArea();

  vis.pen("white");
  vis.lineWidth(1);
  vis.background(Paint(.3, .3, .3, .7));
  vis.drawArea();

  vis.pen("aqua");
  vis.textShadow("grey");
  vis.drawText();

  vis.areaEllipse(00, 300, 500, 100);
  Paint bugs3 = Paint("/home/anthony/development/platform/text.png");
  vis.background(bugs3);
  vis.drawArea();

  // button
  vis.save();
  vis.translate(200, 200);
  vis.area(25.702381, 108.0119, 167.06548, 61.988094, 17.41297, 14.174099);
  vis.background(0, 0, .7, .58);
  vis.lineWidth(.86);
  vis.pen(0, .6, 8, 1);
  vis.drawArea();

  vis.translate(0, 16);
  vis.pen(1, 1, 1);
  vis.textAlignment(alignment::center);
  vis.text("OK");
  vis.textShadow(0, 0, 0);
  vis.drawText();
  vis.translate(0, -16);
  vis.area(25.702381, 108.0119, 167.06548, 61.988094, 17.41297, 14.174099);
  vis.background(Paint(0, 0, 0, 25, {{0, 0, 0, .5, 1}, {1, 0, 0, .5, 0}}));
  vis.drawArea();
  vis.restore();

  // draw svg
  vis.area(15, 300, 200, 220);
  vis.image("/home/anthony/development/platform/button.svg");
  vis.drawImage();

  vis.area(400, 300, 200, 200);
  vis.image("/home/anthony/development/platform/drawing.svg");
  vis.drawImage();

  vis.area(600, 300, 200, 200);
  vis.image("/home/anthony/development/platform/23.svg");
  vis.drawImage();

  // draw objects in a row
  // color stops can name the colors in various forms,
  // these are passed as a vector of color stops.
  // when the offset is not given, it is automatically
  // added. This shortens the gradient creation.
  // The gradients below are linear. The first part is the
  // line.

  // circle
  vis.area(400, 200, 100);
  vis.background(0, 0, 0, 100,
                 {{"white"},
                  {.2, "red"},
                  {.8, "darkred", .5},
                  {.9, "maroon", .4},
                  {"black"}});
  vis.pen("black");
  vis.pen(0, 0, 6, 2, {{"white"}, {"red"}});
  vis.lineWidth(2);
  vis.drawArea();

  // round box
  vis.area(0, 200, 200, 100, 29, 20);
  vis.background(0, 0, 5, 10, {{"lime"}, {"red"}});
  vis.pen("black");
  vis.pen(0, 0, 0, 100, {{0, 0, 1}, {0, 0, .30}});
  vis.lineWidth(3);
  vis.drawArea();

  // circle
  vis.area(200, 200, 100);
  vis.background(0, 0, 0, 100, {{"yellow"}, {.8, "lightbrown"}, {"brown"}});
  vis.pen(0, 0, 0, 100, {{"yellow"}, {"orange"}});
  vis.lineWidth(5);
  vis.drawArea();

  // circle
  vis.area(300, 200, 100);
  Paint bugs2 = Paint("/home/anthony/development/platform/bug.png");
  vis.lineWidth(4);
  bugs2.extend(extendType::reflect);
  bugs2.filter(filterType::bilinear);
  vis.background(bugs2);
  vis.pen("orange");
  vis.drawArea();

  // draw text
  vis.font("DejaVu Sans Bold 44");
  vis.textShadow("black", 5, 2, 2);

  vis.save();
  vis.translate(0, 400);
  vis.rotate(PI / 180 * -35);
  // gradient TEXT
  vis.area(0, 0, 300, 300);
  vis.text("Text Gradient");
  vis.textFill(
      0, 0, 300, 300,
      {{"purple"}, {"red"}, {"orange"}, {"green"}, {"blue"}, {"lightbrown"}});

  // the gradient repeats, so a smaller line will create a
  // stripe.
  vis.textOutline(0, 0, 5, 10, {{"pink", .4}, {"red", .6}}, 3);
  vis.drawText();
  vis.restore();

  vis.area(300, 400, 300, 300);
  vis.text("Text Textured");
  // svg should have width and height
  vis.textFill("/home/anthony/development/platform/26.svg", 30, 30);
  vis.textOutline("/home/anthony/development/platform/bug.png", 3);
  vis.drawText();

  vis.processEvents();

  test0(vis);
}

void eventDispatch(const event &evt) {}

/************************************************************************
************************************************************************/
void test0(platform &vm) {}
