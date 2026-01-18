#include "Spotlight.h"
#include "actions/actions.h"
#include "actions/search.h"
#include "searches/searches.h"
#include "searches/apps.h"
#include "searches/settings.h"
#include "spotlightapps/spotlightapp.h"
#include "spotlightapps/demo/demoapp.h"
#include "spotlightapps/utils.h"
#include <QHash>
#include <QList>
#include <QKeyEvent>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QProcess>
#include <QFrame>
#include <QSizePolicy>
#include <QScrollArea>
#include <QScreen>
#include <QApplication>
#include <QLabel>
#include <QHBoxLayout>
#include <QFontMetrics>
#include <QSpacerItem>
#include <vector>
#include <algorithm>

// Spotlight apps
static const QList<SpotlightAppInfo> SPOTLIGHT_APPS = {
  SpotlightAppInfo("demo", "Font Demo", "Preview system fonts")
};

Spotlight::Spotlight(QWidget* parent)
  : QDialog(parent)
{
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  setAttribute(Qt::WA_TranslucentBackground);
  
  setStyleSheet(
    "QDialog { background: transparent; }"
    "QLineEdit {"
    "  font-size: 20px;"
    "  padding: 0px;"
    "  color: white;"
    "  background: transparent;"
    "  border: none;"
    "}"
    "QPushButton {"
    "  font-size: 16px;"
    "  padding: 12px 16px;"
    "  color: white;"
    "  background: transparent;"
    "  border: none;"
    "  text-align: left;"
    "}"
    "QPushButton:hover {"
    "  background: rgba(255, 255, 255, 10);"
    "}"
    "QPushButton:focus {"
    "  outline: none;"
    "}"
  );
  
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(16, 16, 16, 16);
  layout->setSpacing(0);
  
  m_unifiedContainer = new QWidget(this);
  updateBorderRadius(false);
  m_unifiedContainer->installEventFilter(this);
  
  m_unifiedLayout = new QVBoxLayout(m_unifiedContainer);
  m_unifiedLayout->setContentsMargins(0, 0, 0, 0);
  m_unifiedLayout->setSpacing(0);
  
  m_inputContainer = new QWidget(m_unifiedContainer);
  m_inputContainer->setStyleSheet(
    "QWidget {"
    "  background: transparent;"
    "  border: none;"
    "}"
  );
  m_inputContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_inputContainer->setFixedHeight(SEARCH_BOX_HEIGHT);
  
  auto* inputLayout = new QVBoxLayout(m_inputContainer);
  inputLayout->setContentsMargins(20, 16, 20, 16);
  inputLayout->setSpacing(0);
  
  m_input = new QLineEdit(m_inputContainer);
  m_input->setPlaceholderText("Type to search...");
  m_input->setStyleSheet(
    "QLineEdit {"
    "  font-size: 18px;"
    "  padding: 0px;"
    "  color: white;"
    "  background: transparent;"
    "  border: none;"
    "  border-radius: 0px;"
    "}"
  );
  m_input->installEventFilter(this);
  inputLayout->addWidget(m_input);
  
  // back button (only shown in menu mode)
  m_backButton = new QPushButton("← Back", m_inputContainer);
  m_backButton->setStyleSheet(
    "QPushButton {"
    "  font-size: 18px;"
    "  padding: 0px;"
    "  color: white;"
    "  background: transparent;"
    "  border: none;"
    "  text-align: left;"
    "}"
  );
  m_backButton->setCursor(Qt::PointingHandCursor);
  m_backButton->setFocusPolicy(Qt::StrongFocus);
  m_backButton->hide();
  m_backButton->installEventFilter(this);
  connect(m_backButton, &QPushButton::clicked, this, &Spotlight::onMenuBackClicked);
  inputLayout->addWidget(m_backButton);
  
  m_unifiedLayout->addWidget(m_inputContainer);
  
  QScrollArea* scrollArea = new QScrollArea(m_unifiedContainer);
  scrollArea->setWidgetResizable(true);
  scrollArea->setFrameShape(QFrame::NoFrame);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scrollArea->setStyleSheet(
    "QScrollArea {"
    "  background: transparent;"
    "  border: none;"
    "}"
    "QScrollBar:vertical {"
    "  background: rgba(255, 255, 255, 15);"
    "  width: 6px;"
    "  border-radius: 3px;"
    "}"
    "QScrollBar::handle:vertical {"
    "  background: rgba(255, 255, 255, 50);"
    "  border-radius: 3px;"
    "  min-height: 20px;"
    "}"
    "QScrollBar::handle:vertical:hover {"
    "  background: rgba(255, 255, 255, 70);"
    "}"
  );
  
  m_actionsContainer = new QWidget();
  m_actionsContainer->setStyleSheet(
    "QWidget {"
    "  background: transparent;"
    "  border: none;"
    "}"
  );
  m_actionsLayout = new QVBoxLayout(m_actionsContainer);
  m_actionsLayout->setContentsMargins(4, 4, 4, 4);
  m_actionsLayout->setSpacing(0);
  
  scrollArea->setWidget(m_actionsContainer);
  scrollArea->hide();
  m_scrollArea = scrollArea;
  m_unifiedLayout->addWidget(scrollArea);
  
  layout->addWidget(m_unifiedContainer);
  
  // set base height using variables
  m_baseHeight = MARGIN_TOP + SEARCH_BOX_HEIGHT + MARGIN_BOTTOM;
  setFixedSize(WINDOW_WIDTH, m_baseHeight);
  QScreen* screen = QApplication::primaryScreen();
  QRect screenGeometry = screen->geometry();
  QPoint center = screenGeometry.center() - QPoint(WINDOW_WIDTH / 2, m_baseHeight / 2);
  move(center);
  m_fixedPosition = center;
  m_positionInitialized = true;
  
  m_appsSearch = new AppsSearch(this);
  m_settingsSearch = new SettingsSearch(this);
  registerSpotlightApp("demo", new DemoApp());
  
  connect(m_input, &QLineEdit::textChanged, this, &Spotlight::onTextChanged);
}

void Spotlight::onTextChanged(const QString& text)
{
  if (m_menuMode) return;
  
  if (text.isEmpty()) {
    clearActions();
  } else {
    updateActions(text);
  }
}

void Spotlight::updateActions(const QString& query)
{
  clearActions();
  
  std::vector<SearchResult> allResults;
  
  // app & settings search
  std::vector<SearchResult> settingsResults = m_settingsSearch->performSearch(query);
  std::vector<SearchResult> appResults = m_appsSearch->performSearch(query);
  allResults.insert(allResults.end(), settingsResults.begin(), settingsResults.end());
  allResults.insert(allResults.end(), appResults.begin(), appResults.end());
  
  // add spotlight apps using registry
  QString lowerQuery = query.toLower();
  for (const SpotlightAppInfo& appInfo : SPOTLIGHT_APPS) {
    if (!m_spotlightApps.contains(appInfo.identifier)) continue;
    
    QString appNameLower = appInfo.name.toLower();
    QString appDescLower = appInfo.description.toLower();
    int score = 0;
    
    if (appNameLower.contains(lowerQuery)) {
      score = 90;
    } else if (appDescLower.contains(lowerQuery)) {
      score = 70;
    } else if (query.isEmpty() || lowerQuery.length() < 2) {
      score = 50;
    }
    
    if (score > 0) {
      SearchResult result;
      result.name = appInfo.name;
      result.description = appInfo.description;
      result.exec = "spotlightapp:" + appInfo.identifier;
      result.score = score;
      allResults.push_back(result);
    }
  }
  
  std::sort(allResults.begin(), allResults.end());
  m_currentSearchResults = allResults;
  
  // Create result buttons
  for (size_t i = 0; i < allResults.size(); ++i) {
    createItemButton(allResults[i].name, allResults[i].description, static_cast<int>(i), false);
  }
  
  // Create search action
  auto* searchAction = new SearchAction(m_actionsContainer);
  searchAction->setText("Search " + query);
  connect(searchAction, &Action::actionExecuted, this, &Spotlight::onActionExecuted);
  connect(searchAction, &QPushButton::clicked, [this, searchAction]() {
    searchAction->execute(m_input->text());
  });
  m_actions.append(searchAction);
  m_actionsLayout->addWidget(searchAction);
  
  // Show results if any
  int totalItems = m_searchResults.size() + m_actions.size();
  if (totalItems > 0) {
    m_scrollArea->show();
    updateBorderRadius(true);
    updateWindowSize(calculateResultsHeight(totalItems));
    selectAction(0);
  }
  m_input->setFocus();
}

void Spotlight::clearActions()
{
  QLayoutItem* item;
  while ((item = m_actionsLayout->takeAt(0)) != nullptr) {
    delete item->widget();
    delete item;
  }
  
  m_actions.clear();
  m_searchResults.clear();
  m_currentSearchResults.clear();
  m_selectedActionIndex = -1;
  m_scrollArea->hide();
  updateBorderRadius(false);
  setFixedSize(WINDOW_WIDTH, m_baseHeight);
  
  if (m_positionInitialized) { move(m_fixedPosition); }
}

void Spotlight::navigateActions(int direction)
{
  if (m_menuMode) {
    // navigate menu items
    int totalItems = static_cast<int>(m_menuItems.size());
    if (totalItems == 0) return;
    
    int newIndex = m_selectedActionIndex + direction;
    if (newIndex < 0) { newIndex = totalItems - 1; }
    else if (newIndex >= totalItems) { newIndex = 0; }
    
    selectAction(newIndex);
  } else {
    // navigate search results
    int totalItems = m_searchResults.size() + m_actions.size();
    if (totalItems == 0) return;
    
    int newIndex = m_selectedActionIndex + direction;
    if (newIndex < 0) { newIndex = totalItems - 1; }
    else if (newIndex >= totalItems) { newIndex = 0; }
    
    selectAction(newIndex);
  }
}

QPushButton* Spotlight::getButtonAt(int index)
{
  if (m_menuMode) {
    if (index >= 0 && index < static_cast<int>(m_menuItems.size())) {
      return m_menuItems[index];
    }
  } else {
    if (index < m_searchResults.size()) {
      return m_searchResults[index];
    } else if (index < m_searchResults.size() + m_actions.size()) {
      return m_actions[index - m_searchResults.size()];
    }
  }
  return nullptr;
}

void Spotlight::applyButtonStyle(QPushButton* button, bool selected)
{
  if (!button) return;
  
  QWidget* widget = qobject_cast<QWidget*>(button);
  bool isWidgetButton = (widget && (widget->property("isMenuItem").toBool() || 
                                    widget->property("isResultButton").toBool()));
  
  if (isWidgetButton) {
    SpotlightMenuUtils::styleMenuItemWidget(widget, selected);
  } else {
    QString style = QString(
      "QPushButton {"
      "  font-size: 16px;"
      "  padding: 12px 16px;"
      "  color: white;"
      "  background: %1;"
      "  border: none;"
      "  text-align: left;"
      "}").arg(selected ? "rgba(100, 150, 255, 80)" : "transparent");
    button->setStyleSheet(style);
  }
}

void Spotlight::selectAction(int index)
{
  int totalItems = m_menuMode ? static_cast<int>(m_menuItems.size()) : 
                                (m_searchResults.size() + m_actions.size());
  if (index < 0 || index >= totalItems) return;
  
  // deselect previous
  if (m_selectedActionIndex >= 0 && m_selectedActionIndex < totalItems) {
    QPushButton* prevButton = getButtonAt(m_selectedActionIndex);
    applyButtonStyle(prevButton, false);
  }
  
  m_selectedActionIndex = index;
  QPushButton* newButton = getButtonAt(index);
  if (newButton) {
    QWidget* newWidget = qobject_cast<QWidget*>(newButton);
    bool isWidgetButton = (newWidget && (newWidget->property("isMenuItem").toBool() || 
                                         newWidget->property("isResultButton").toBool()));
    if (isWidgetButton) {
      SpotlightMenuUtils::styleMenuItemWidget(newWidget, true);
    } else {
      applyButtonStyle(newButton, true);
    }
  }
  
  if (m_menuMode) {
    m_backButton->setFocus();
  } else {
    m_input->setFocus();
  }
}

void Spotlight::onActionExecuted() { close(); }

bool Spotlight::eventFilter(QObject* obj, QEvent* event)
{
  // keyboard events on back button (menu mode)
  if (obj == m_backButton && m_menuMode) {
    if (event->type() == QEvent::KeyPress) {
      auto* keyEvent = static_cast<QKeyEvent*>(event);
      if (keyEvent->key() == Qt::Key_Escape || keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
        exitMenuMode();
        return true;
      }
      else if (keyEvent->key() == Qt::Key_Down) {
        navigateActions(1);
        return true;
      }
      else if (keyEvent->key() == Qt::Key_Up) {
        navigateActions(-1);
        return true;
      }
    }
  }
  
  // keyboard events on input
  if (obj == m_input) {
    if (event->type() == QEvent::KeyPress) {
      auto* keyEvent = static_cast<QKeyEvent*>(event);
      if (keyEvent->key() == Qt::Key_Escape) {
        if (m_menuMode) {
          exitMenuMode();
        } else {
          close();
        }
        return true;
      }
      else if (keyEvent->key() == Qt::Key_Down) {
        navigateActions(1);
        return true;
      }
      else if (keyEvent->key() == Qt::Key_Up) {
        navigateActions(-1);
        return true;
      }
      else if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
        if (m_menuMode) {
          // menu mode - handle menu item selection
          int totalItems = static_cast<int>(m_menuItems.size());
          if (m_selectedActionIndex >= 0 && m_selectedActionIndex < totalItems) {
            onMenuItemClicked(m_selectedActionIndex);
          }
          return true;
        } else {
          // search mode
          int totalItems = m_searchResults.size() + m_actions.size();
          if (m_selectedActionIndex >= 0 && m_selectedActionIndex < totalItems) {
            if (m_selectedActionIndex < m_searchResults.size()) {
              // search result
              SearchResult result = m_currentSearchResults[m_selectedActionIndex];
              launchApp(result);
              // Don't close if spotlight app (menu mode)
              if (!result.exec.startsWith("spotlightapp:")) {
                close();
              }
            } else {
              // action
              QString query = m_input->text();
              m_actions[m_selectedActionIndex - m_searchResults.size()]->execute(query);
              close();
            }
          }
          return true;
        }
      }
    }
  }
  
  // Click events on result buttons
  if (obj->property("isResultButton").toBool()) {
    if (event->type() == QEvent::MouseButtonPress) {
      auto* mouseEvent = static_cast<QMouseEvent*>(event);
      if (mouseEvent->button() == Qt::LeftButton) {
        int index = obj->property("resultIndex").toInt();
        if (index >= 0 && index < static_cast<int>(m_currentSearchResults.size())) {
          SearchResult result = m_currentSearchResults[index];
          launchApp(result);
          // Don't close if it's a spotlight app (menu mode)
          if (!result.exec.startsWith("spotlightapp:")) {
            close();
          }
        }
        return true;
      }
    }
  }
  
  // Click events on menu item buttons
  if (obj->property("isMenuItem").toBool()) {
    if (event->type() == QEvent::MouseButtonPress) {
      auto* mouseEvent = static_cast<QMouseEvent*>(event);
      if (mouseEvent->button() == Qt::LeftButton) {
        int index = obj->property("itemIndex").toInt();
        if (index >= 0 && index < static_cast<int>(m_currentMenuItems.size())) {
          onMenuItemClicked(index);
        }
        return true;
      }
    }
  }
  
  // drag events on unified container (not QLineEdit or result buttons)
  else if (obj == m_unifiedContainer) {
    if (event->type() == QEvent::MouseButtonPress) {
      auto* mouseEvent = static_cast<QMouseEvent*>(event);
      if (mouseEvent->button() == Qt::LeftButton) {
        // if click is on QLineEdit or result buttons, don't drag
        QPoint localPos = mouseEvent->pos();
        QWidget* child = m_unifiedContainer->childAt(localPos);
        QWidget* widget = child;
        while (widget && widget != m_unifiedContainer) {
          if (widget == m_input || widget->property("isResultButton").toBool() || 
              widget->property("isMenuItem").toBool()) {
            // click on input or result button, let it handle
            return false;
          }
          widget = widget->parentWidget();
        }
        // if click in padding area start dragging
        m_dragging = true;
        m_dragStartPos = mouseEvent->globalPosition().toPoint() - frameGeometry().topLeft();
        return true;
      }
    }
    else if (event->type() == QEvent::MouseMove && m_dragging) {
      auto* mouseEvent = static_cast<QMouseEvent*>(event);
      move(mouseEvent->globalPosition().toPoint() - m_dragStartPos);
      return true;
    }
    else if (event->type() == QEvent::MouseButtonRelease) {
      auto* mouseEvent = static_cast<QMouseEvent*>(event);
      if (mouseEvent->button() == Qt::LeftButton) {
        m_dragging = false;
        m_fixedPosition = frameGeometry().topLeft();
        return true;
      }
    }
  }
  return QDialog::eventFilter(obj, event);
}

void Spotlight::createItemButton(const QString& title, const QString& description, int index, bool isMenuItem, const QFont& font)
{
  QWidget* buttonWidget = SpotlightMenuUtils::createMenuItemWidget(m_actionsContainer, title, description, font);
  
  // make it clickable
  buttonWidget->installEventFilter(this);
  if (isMenuItem) {
    buttonWidget->setProperty("isMenuItem", true);
    buttonWidget->setProperty("itemIndex", index);
    m_menuItems.append(reinterpret_cast<QPushButton*>(buttonWidget));
  } else {
    buttonWidget->setProperty("isResultButton", true);
    buttonWidget->setProperty("resultIndex", index);
    m_searchResults.append(reinterpret_cast<QPushButton*>(buttonWidget));
  }
  m_actionsLayout->addWidget(buttonWidget);
}


void Spotlight::registerSpotlightApp(const QString& appName, SpotlightApp* app)
{
  m_spotlightApps[appName] = app;
}

void Spotlight::launchApp(const SearchResult& result)
{
  if (result.exec.isEmpty()) return;
  
  // check if this is a spotlight app
  if (result.exec.startsWith("spotlightapp:")) {
    QString appName = result.exec.mid(13); // remove "spotlightapp:" prefix
    
    if (m_spotlightApps.contains(appName)) {
      SpotlightApp* app = m_spotlightApps[appName];
      showMenuMode(app->getMenuItems());
      return;
    }
  }
  
  QString execCmd = result.exec;
  
  // remove desktop file % codes
  execCmd.replace("%f", "");
  execCmd.replace("%F", "");
  execCmd.replace("%u", "");
  execCmd.replace("%U", "");
  execCmd.replace("%i", "");
  execCmd.replace("%c", result.name);
  execCmd.replace("%k", "");
  
  // launch via bash to use correct shell environment
  QProcess::startDetached("bash", QStringList() << "-c" << execCmd);
  
  emit onActionExecuted();
}

void Spotlight::showMenuMode(const std::vector<MenuItem>& items)
{
  m_menuMode = true;
  m_currentMenuItems = items;
  
  m_input->hide();
  m_input->clear();
  m_backButton->show();
  clearActions();
  
  for (size_t i = 0; i < items.size(); ++i) {
    createItemButton(items[i].title, items[i].description, static_cast<int>(i), true, items[i].font);
  }
  
  int totalItems = static_cast<int>(items.size());
  if (totalItems > 0) {
    m_scrollArea->show();
    updateBorderRadius(true);
    
    int resultsHeight = calculateResultsHeight(totalItems);
    updateWindowSize(resultsHeight);
    selectAction(0);
  }
  
  m_backButton->setFocus();
}

void Spotlight::exitMenuMode()
{
  m_menuMode = false;
  m_backButton->hide();
  m_input->show();
  m_input->clear();
  m_input->setFocus();
  clearActions();
  m_menuItems.clear();
  m_currentMenuItems.clear();
}

void Spotlight::onMenuBackClicked()
{ exitMenuMode(); }

// Helper functions
void Spotlight::updateBorderRadius(bool hasResults)
{
  QString style = QString(
    "QWidget {"
    "  background: rgba(40, 40, 40, 250);"
    "  border-top-left-radius: %1px;"
    "  border-top-right-radius: %1px;"
    "  border-bottom-left-radius: %2px;"
    "  border-bottom-right-radius: %2px;"
    "}"
  ).arg(BORDER_RADIUS).arg(hasResults ? 0 : BORDER_RADIUS);
  
  m_unifiedContainer->setStyleSheet(style);
}

int Spotlight::calculateResultsHeight(int totalItems)
{
  m_actionsContainer->updateGeometry();
  m_actionsLayout->update();
  m_actionsContainer->adjustSize();
  
  int contentHeight = m_actionsContainer->sizeHint().height();
  if (contentHeight == 0) {
    contentHeight = totalItems * 50; // approx height per item
  }
  
  int resultsHeight = qMin(contentHeight, m_maxResultsHeight);
  return qMax(resultsHeight, 1); // Minimum 1px to prevent layout issues
}

void Spotlight::updateWindowSize(int resultsHeight)
{
  m_scrollArea->setMaximumHeight(resultsHeight);
  m_scrollArea->setMinimumHeight(resultsHeight);
  
  int windowHeight = MARGIN_TOP + SEARCH_BOX_HEIGHT + resultsHeight + MARGIN_BOTTOM;
  setFixedSize(WINDOW_WIDTH, windowHeight);
  
  if (m_positionInitialized) { move(m_fixedPosition); }
}

void Spotlight::onMenuItemClicked(int index)
{
  if (index < 0 || index >= static_cast<int>(m_currentMenuItems.size())) return;
  if (m_currentMenuItems[index].action) { m_currentMenuItems[index].action(); }
  // stay in menu mode so user can select more items
}