#pragma once
#include <QDialog>
#include <QPoint>
#include <QString>
#include <QFont>
#include <QHash>
#include <functional>
#include <vector>
#include "searches/searches.h"

struct MenuItem
{
  QString title;
  QString description;
  std::function<void()> action; // callback when item is clicked
  QFont font; // optional custom font for title
  
  MenuItem(const QString& t, const QString& d, std::function<void()> a, const QFont& f = QFont())
    : title(t), description(d), action(a), font(f) {}
};

struct SpotlightAppInfo
{
  QString identifier;
  QString name;
  QString description;
  
  SpotlightAppInfo(const QString& id, const QString& n, const QString& desc)
    : identifier(id), name(n), description(desc) {}
};

class QLineEdit;
class QWidget;
class QVBoxLayout;
class QScrollArea;
class Action;
class QPushButton;
class AppsSearch;
class SpotlightApp;

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
  void applyButtonStyle(QPushButton* button, bool selected);
  QPushButton* getButtonAt(int index);
  
  // Menu mode functions
  void showMenuMode(const std::vector<MenuItem>& items);
  void exitMenuMode();
  void onMenuBackClicked();
  void onMenuItemClicked(int index);
  
  QLineEdit* m_input = nullptr;
  QPushButton* m_backButton = nullptr;
  QWidget* m_inputContainer = nullptr;
  QWidget* m_actionsContainer = nullptr;
  QScrollArea* m_scrollArea = nullptr;
  QVBoxLayout* m_actionsLayout = nullptr;
  QList<Action*> m_actions;
  QList<QPushButton*> m_searchResults; // buttons for search results
  QList<QPushButton*> m_menuItems; // buttons for menu items
  std::vector<SearchResult> m_currentSearchResults; // store search results data
  std::vector<MenuItem> m_currentMenuItems; // store menu items data
  AppsSearch* m_appsSearch = nullptr;
  QHash<QString, SpotlightApp*> m_spotlightApps; // registered spotlight apps
  int m_selectedActionIndex = -1;
  QPoint m_dragStartPos;
  QPoint m_fixedPosition;
  bool m_dragging = false;
  bool m_positionInitialized = false;
  bool m_menuMode = false; // true when showing menu, false when showing search
  int m_baseHeight = 92; // base height: margins (32) + search box (60)
  int m_maxResultsHeight = 500; // max height for results before scrolling
  
  void launchApp(const SearchResult& result);
  void createItemButton(const QString& title, const QString& description, int index, bool isMenuItem, const QFont& font = QFont());
  void registerSpotlightApp(const QString& appName, SpotlightApp* app);
};
