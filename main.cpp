
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

eventHandler eventDispatch(const event &evt);
errorHandler handleError(const std::string errText) {
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
  vis.antiAlias(antialias::subPixel);

  vis.textFillNone();
  vis.textOutlineNone();
  vis.textShadowNone();

  vis.textAlignment(alignment::left);
  vis.text(
      "The sun sets casting its refraction upon the mountain side. "
      "The glistening oil coats upon the ravens are a remark of healthiness."
      "One that is pronounced during the day and in the moonlight."
      "At home, a cave dweller sees this all at once. These are indeed fine "
      "things."
      "The warmth of the sun decays as thousands of brilliant stars dictate "
      "the continual persistence of the system. A remarkable sight. A heavenly "
      "home.");
  vis.font("DejaVu Sans Bold 14");
  vis.area(0, 0, 800, 600, 120, 120);

  vis.pen(Paint("white"));

  Paint bugs = Paint("/home/anthony/development/platform/bugu.png");
  bugs.rotate(3.14 / 180 * 10);
  bugs.scale(1.15, 1.15);
  bugs.translate(-1, -1);

  bugs.extend(extendType::reflect);
  bugs.filter(filterType::bilinear);

  vis.background(bugs);
  vis.drawArea();
  vis.pen(.3,.4,.3);
  vis.lineWidth(40);
  vis.background(Paint(.2, .2, .2, .7));
  vis.drawArea();

  vis.pen(Paint("white"));
  vis.drawText();

  vis.area(100, 100, 20);
  Paint bugs2 = Paint("/home/anthony/development/platform/bug.png");
  vis.lineWidth(4);
  bugs2.extend(extendType::reflect);
  bugs2.filter(filterType::bilinear);
  vis.background(bugs2);
  vis.drawArea();


  vis.areaEllipse(400, 300, 200, 100);
  Paint bugs3 = Paint("/home/anthony/development/platform/text.png");

  bugs3.extend(extendType::reflect);
  bugs3.filter(filterType::bilinear);
  vis.background(bugs3);
  vis.drawArea();

  //button
  vis.translate(200,200);
  vis.area(25.702381,108.0119,167.06548,61.988094,17.41297, 14.174099);
  vis.background(0,0,.7,.58);
  vis.lineWidth(.86);
  vis.pen(0,.6,8,1);
  vis.drawArea();

  vis.translate(0,16);
  vis.pen(1,1,1);
  vis.textAlignment(alignment::center);
  vis.text("OK");
  vis.textShadow(0,0,0);
  vis.drawText();
  vis.translate(0,-16);
  vis.area(25.702381,108.0119,167.06548,61.988094,17.41297, 14.174099);
  vis.background(Paint({{0,0,0,.5,1},{1,0,0,.5,0}}));
  vis.drawArea();



  vis.processEvents();

  test0(vis);
}

eventHandler eventDispatch(const event &evt) {}

/************************************************************************
************************************************************************/
void test0(platform &vm) {}
