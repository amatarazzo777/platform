
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

vis.text("To summarize output logic by inspecting the means by which it is "
           "produced provides the capability to offer optimization.");
  vis.fontDescription("Arial normal 19");
  vis.area(0, 0, 500, 300);

  vis.pen(0, 0, .2, 1);
  vis.background(0, 0, .5, .3);
  vis.drawBox();

  vis.background(1, 1, 1);
  vis.pen(0x00);
  vis.drawText();


  vis.move_to(5,5);
  vis.line_to(300,250);
  vis.stroke();

  stringstream ss;
  for (int i = 0; i < 30; i++) {
    ss << i
       << ". Through the progression of learning, better results can make an "
          "appearance\n";
  }
  vis.text(ss);
  vis.area(10, 100, 600, 600);
  vis.pen(0x000fff);
  vis.drawText();

  ss.str("");
  for (int i = 0; i < 30; i++) {
    ss << i << ". A screen full of text is easily rendered.\n";
  }

  vis.text(ss);
  vis.area(650, 30, 1000, 800);
  vis.pen("brown");
  vis.drawText();

  vis.image("/home/anthony/source/nanosvg/example/screenshot-2.png");
  vis.charcoal();
  vis.blur();
  vis.area(200, 200, 300, 500);
  vis.drawImage();

  vis.image("/home/anthony/source/nanosvg/example/draw.png");
  vis.area(500, 200, 900, 500);
  vis.shadow(20, .5, 20, 20);
  vis.drawImage();

  vis.pen(0, 0, .2, .1);
  vis.move_to(5, 5);
  vis.line_to(300, 250);
  vis.stroke();

  vis.processEvents();

  test0(vis);
}

eventHandler eventDispatch(const event &evt) {}

/************************************************************************
************************************************************************/
void test0(platform &vm) {}
