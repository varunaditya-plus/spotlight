#pragma once
#include "actions.h"

class SearchAction : public Action
{
  Q_OBJECT
public:
  explicit SearchAction(QWidget* parent = nullptr);
  void execute(const QString& query) override;
};
