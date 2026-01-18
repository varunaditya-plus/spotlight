#pragma once
#include <QDialog>
#include <QPoint>

class QLineEdit;
class QWidget;
class QVBoxLayout;
class Action;

class Spotlight : public QDialog
{
  Q_OBJECT
public:
  explicit Spotlight(QWidget* parent = nullptr);

protected:
  bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
  void onTextChanged(const QString& text);
  void onActionExecuted();

private:
  void updateActions(const QString& query);
  void clearActions();
  void navigateActions(int direction);
  void selectAction(int index);
  
  QLineEdit* m_input = nullptr;
  QWidget* m_inputContainer = nullptr;
  QWidget* m_actionsContainer = nullptr;
  QVBoxLayout* m_actionsLayout = nullptr;
  QList<Action*> m_actions;
  int m_selectedActionIndex = -1;
  QPoint m_dragStartPos;
  bool m_dragging = false;
  int m_baseHeight = 80;
};
