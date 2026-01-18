#include "actions.h"

Action::Action(const QString& label, QWidget* parent)
  : QPushButton(label, parent)
{
  setCursor(Qt::PointingHandCursor);
  setFocusPolicy(Qt::NoFocus);
}
