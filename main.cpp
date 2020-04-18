
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
  vis.pen(0x00);
  vis.fontDescription("Arial Bold 12");
  vis.area(10, 10, 300, 300);
  vis.drawText();

  stringstream ss;
  for (int i = 0; i < 50; i++) {
    ss << i
       << ". Through the progression of learning, better results can make an "
          "appearance\n";
  }
  vis.text(ss);
  vis.area(30, 30, 600, 600);
  vis.pen(0x000fff);
  vis.drawText();

  ss.clear();
  for (int i = 0; i < 50; i++) {
    ss << i << ". A screen full of text is easily rendered.\n";
  }
  vis.text(ss);
  vis.area(700, 30, 1000, 800);
  vis.pen(0x00ffff);
  vis.drawText();

  vis.image("/home/anthony/source/nanosvg/example/screenshot-2.png");
  vis.area(400, 200, 900, 500);
  vis.drawImage();

  vis.image("/home/anthony/source/nanosvg/example/draw.png");
  vis.area(700, 200, 900, 500);
  vis.drawImage();

  vis.processEvents();

  test0(vis);
}

eventHandler eventDispatch(const event &evt) {}

/************************************************************************
************************************************************************/
void test0(platform &vm) {}
