#pragma once
#include <QDialog>
#include <QPoint>
#include <vector>
#include "searches/searches.h"

class QLineEdit;
class QWidget;
class QVBoxLayout;
class QScrollArea;
class Action;
class QPushButton;
class AppsSearch;

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
  QScrollArea* m_scrollArea = nullptr;
  QVBoxLayout* m_actionsLayout = nullptr;
  QList<Action*> m_actions;
  QList<QPushButton*> m_searchResults; // buttons for search results
  std::vector<SearchResult> m_currentSearchResults; // store search results data
  AppsSearch* m_appsSearch = nullptr;
  int m_selectedActionIndex = -1;
  QPoint m_dragStartPos;
  QPoint m_fixedPosition;
  bool m_dragging = false;
  bool m_positionInitialized = false;
  int m_baseHeight = 92; // base height: margins (32) + search box (60)
  int m_maxResultsHeight = 500; // max height for results before scrolling
  
  void launchApp(const SearchResult& result);
  void createSearchResultButton(const SearchResult& result, int index);
};
