#include "demoapp.h"
#include <QFontDatabase>
#include <QClipboard>
#include <QApplication>
#include <QFont>
#include <QFontMetrics>

std::vector<MenuItem> DemoApp::getMenuItems()
{
  std::vector<MenuItem> items;
  QFontDatabase fontDb;
  QStringList fontFamilies = fontDb.families();
  QString sampleText = "The quick brown fox jumps over the lazy dog";
  
  for (const QString& fontFamily : fontFamilies) {
    QFont font(fontFamily, 16);
    QFontMetrics metrics(font);
    if (metrics.inFont(QChar('A')) && metrics.inFont(QChar(' '))) { // make sure font is valid
      QString fontName = fontFamily;
      items.emplace_back( // add font name to menu items
        sampleText,
        fontFamily,
        [fontName]() { 
          QClipboard* clipboard = QApplication::clipboard(); // copy font name to clipboard on click
          clipboard->setText(fontName);
        },
        font
      );
    }
  }
  
  return items;
}
