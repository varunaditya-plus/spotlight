#include "utils.h"
#include <QPushButton>

namespace SpotlightMenuUtils
{
  QWidget* createMenuItemWidget(
    QWidget* parent,
    const QString& title,
    const QString& description,
    const QFont& font)
  {
    QWidget* buttonWidget = new QWidget(parent);
    buttonWidget->setCursor(Qt::PointingHandCursor);
    
    QHBoxLayout* contentLayout = new QHBoxLayout(buttonWidget);
    contentLayout->setContentsMargins(16, 12, 16, 12);
    contentLayout->setSpacing(8);
    
    QLabel* titleLabel = new QLabel(title, buttonWidget);
    titleLabel->setStyleSheet(
      "QLabel {"
      "  color: white;"
      "  font-size: 16px;"
      "  background: transparent;"
      "}"
    );
    titleLabel->setTextFormat(Qt::PlainText);
    
    if (!font.family().isEmpty()) {
      titleLabel->setFont(font);
    }
    
    QFontMetrics titleMetrics(titleLabel->font());
    QString elidedTitle = titleMetrics.elidedText(title, Qt::ElideRight, 300);
    titleLabel->setText(elidedTitle);
    titleLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    contentLayout->addWidget(titleLabel, 0);
    
    if (!description.isEmpty()) {
      QLabel* descLabel = new QLabel(description, buttonWidget);
      descLabel->setStyleSheet(
        "QLabel {"
        "  color: rgba(255, 255, 255, 140);"
        "  font-size: 14px;"
        "  background: transparent;"
        "}"
      );
      descLabel->setTextFormat(Qt::PlainText);
      QFontMetrics descMetrics(descLabel->font());
      QString elidedDesc = descMetrics.elidedText(description, Qt::ElideRight, 300);
      descLabel->setText(elidedDesc);
      descLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
      contentLayout->addWidget(descLabel, 1);
    } else {
      contentLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    }
    
    return buttonWidget;
  }
  
  void styleMenuItemWidget(QWidget* widget, bool selected)
  {
    if (!widget) return;
    
    widget->setStyleSheet(selected ?
      "QWidget { background: rgba(100, 150, 255, 80); }" :
      "QWidget { background: transparent; }");
  }
}
