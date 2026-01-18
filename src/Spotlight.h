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
class SettingsSearch;
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
  
  // Helper functions
  void updateBorderRadius(bool hasResults);
  void updateWindowSize(int resultsHeight);
  int calculateResultsHeight(int totalItems);
  
  QLineEdit* m_input = nullptr;
  QPushButton* m_backButton = nullptr;
  QWidget* m_unifiedContainer = nullptr; // Unified container for search and results
  QWidget* m_inputContainer = nullptr; // Input area inside unified container
  QWidget* m_actionsContainer = nullptr;
  QScrollArea* m_scrollArea = nullptr;
  QVBoxLayout* m_actionsLayout = nullptr;
  QVBoxLayout* m_unifiedLayout = nullptr; // Layout for unified container
  QList<Action*> m_actions;
  QList<QPushButton*> m_searchResults; // buttons for search results
  QList<QPushButton*> m_menuItems; // buttons for menu items
  std::vector<SearchResult> m_currentSearchResults; // store search results data
  std::vector<MenuItem> m_currentMenuItems; // store menu items data
  AppsSearch* m_appsSearch = nullptr;
  SettingsSearch* m_settingsSearch = nullptr;
  QHash<QString, SpotlightApp*> m_spotlightApps; // registered spotlight apps
  int m_selectedActionIndex = -1;
  QPoint m_dragStartPos;
  QPoint m_fixedPosition;
  bool m_dragging = false;
  bool m_positionInitialized = false;
  bool m_menuMode = false;
  int m_baseHeight = 92; // margins (32) + search box (60)
  int m_maxResultsHeight = 500;
  
  static constexpr int WINDOW_WIDTH = 700;
  static constexpr int SEARCH_BOX_HEIGHT = 60;
  static constexpr int MARGIN_TOP = 16;
  static constexpr int MARGIN_BOTTOM = 16;
  static constexpr int BORDER_RADIUS = 28;
  
  void launchApp(const SearchResult& result);
  void createItemButton(const QString& title, const QString& description, int index, bool isMenuItem, const QFont& font = QFont());
  void registerSpotlightApp(const QString& appName, SpotlightApp* app);
};
