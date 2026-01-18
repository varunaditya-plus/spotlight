#pragma once
#include "../Spotlight.h"
#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QFont>
#include <QFontMetrics>
#include <QSpacerItem>
#include <QSizePolicy>

class QPushButton;
class QVBoxLayout;

namespace SpotlightMenuUtils
{
  // menu item button widget
  QWidget* createMenuItemWidget(
    QWidget* parent,
    const QString& title,
    const QString& description,
    const QFont& font = QFont()
  );
  
  // styling for menu item widget
  void styleMenuItemWidget(QWidget* widget, bool selected);
}
