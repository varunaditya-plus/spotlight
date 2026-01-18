#include "Spotlight.h"
#include <QApplication>

int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  
  Spotlight overlay;
  overlay.show();
  
  return app.exec();
}
