#pragma once
#include <QPushButton>
#include <QString>

class Action : public QPushButton
{
  Q_OBJECT
public:
  explicit Action(const QString& label, QWidget* parent = nullptr);
  virtual ~Action() = default;
  
  virtual void execute(const QString& query) = 0;
  
signals:
  void actionExecuted();
};
