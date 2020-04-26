
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
  vis.antiAlias(platform::subPixel);

  vis.text("To summarize logic by inspecting the means by which output is "
           "produced provides the capability to offer optimization.");
  vis.font("DejaVu Sans Bold 19");
  vis.textShadow(.3, 0, 0,1, 4,1,2);
   vis.textFill({{0, 1, .4, .4, 1}, {2, .4, .4, .4, 1},{4, .8, 0, 0, 1}});
  //vis.textFill("/home/anthony/development/platform/bug.png");
  vis.textOutline(.3, 0, 0, .5);
  vis.area(0, 0, 500, 300);

  vis.pen(0, 0, .2, 1);
  vis.background(0, 0, .5, .3);
  vis.drawBox();

  vis.background(1, 1, 1);
  vis.pen(0xFF0000);
  vis.drawText();

  vis.textFillNone();
  vis.textOutlineNone();
  vis.textShadowNone();

  stringstream ss;
  for (int i = 0; i < 30; i++) {
    ss << i
       << ". Through the progression of learning, better results can make an "
          "appearance.\n";
  }
  vis.font("DejaVu Sans normal 14");
  vis.text(ss);
  vis.area(10, 200, 600, 600);
  // vis.textShadow(0, .1, .0, 2);
  vis.pen(0x000fff);
  vis.textFill("/home/anthony/development/platform/text.png");
  vis.textOutline(0,0,0,.5);
  vis.drawText();

  ss.str("");
  for (int i = 0; i < 30; i++) {
    ss << i << ". A screen full of text is easily rendered.\n";
  }

  vis.text(ss);
  vis.area(650, 30, 1000, 800);
  vis.pen("brown");
  vis.textOutlineNone();
  vis.textShadow(0, .1, .0, 2);
  vis.textFillNone();
  vis.drawText();

  vis.image("/home/anthony/source/nanosvg/example/screenshot-2.png");

  vis.area(200, 200, 300, 500);
  vis.drawImage();

  vis.image("/home/anthony/source/nanosvg/example/draw.png");
  vis.area(500, 200, 900, 500);
  vis.drawImage();

  vis.processEvents();

  test0(vis);
}

eventHandler eventDispatch(const event &evt) {}

/************************************************************************
************************************************************************/
void test0(platform &vm) {}
