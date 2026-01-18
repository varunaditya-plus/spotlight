#include "Spotlight.h"
#include "actions/actions.h"
#include "actions/search.h"
#include "searches/searches.h"
#include "searches/apps.h"
#include <QKeyEvent>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QProcess>
#include <QFileInfo>
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
  layout->setSpacing(8);
  
  // draggable container
  m_inputContainer = new QWidget(this);
  m_inputContainer->setStyleSheet(
    "QWidget {"
    "  background: rgba(40, 40, 40, 250);"
    "  border-radius: 12px;"
    "}"
  );
  m_inputContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  m_inputContainer->setFixedHeight(60);
  m_inputContainer->installEventFilter(this);
  
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
  
  layout->addWidget(m_inputContainer);
  
  // results container with scroll area
  QScrollArea* scrollArea = new QScrollArea(this);
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
    "  background: rgba(40, 40, 40, 250);"
    "  border-radius: 12px;"
    "}"
  );
  m_actionsLayout = new QVBoxLayout(m_actionsContainer);
  m_actionsLayout->setContentsMargins(4, 4, 4, 4);
  m_actionsLayout->setSpacing(0);
  
  scrollArea->setWidget(m_actionsContainer);
  scrollArea->hide();
  m_scrollArea = scrollArea;
  layout->addWidget(scrollArea);
  
  // set fixed size and center on screen
  // base height = margins (32) + search box (60) = 92
  m_baseHeight = 92;
  setFixedSize(700, m_baseHeight);
  QScreen* screen = QApplication::primaryScreen();
  QRect screenGeometry = screen->geometry();
  QPoint center = screenGeometry.center() - QPoint(350, m_baseHeight / 2);
  move(center);
  m_fixedPosition = center;
  m_positionInitialized = true;
  
  // initialize apps search
  m_appsSearch = new AppsSearch(this);
  
  connect(m_input, &QLineEdit::textChanged, this, &Spotlight::onTextChanged);
}

void Spotlight::onTextChanged(const QString& text)
{
  if (text.isEmpty()) {
    clearActions();
  } else {
    updateActions(text);
  }
}

void Spotlight::updateActions(const QString& query)
{
  clearActions();
  
  // app search
  std::vector<SearchResult> appResults = m_appsSearch->performSearch(query);
  m_currentSearchResults = appResults;
  
  // create buttons for search results
  for (int i = 0; i < appResults.size(); ++i) {
    createSearchResultButton(appResults[i], i);
  }
  
  // create actions
  auto* searchAction = new SearchAction(m_actionsContainer);
  searchAction->setText("Search " + query);
  connect(searchAction, &Action::actionExecuted, this, &Spotlight::onActionExecuted);
  connect(searchAction, &QPushButton::clicked, [this, searchAction]() {
    QString query = m_input->text();
    searchAction->execute(query);
  });
  m_actions.append(searchAction);
  m_actionsLayout->addWidget(searchAction);
  
  // calculate total items
  int totalItems = m_searchResults.size() + m_actions.size();
  
  // show actions and lower area
  m_scrollArea->show();
  
  // force layout update
  m_actionsContainer->updateGeometry();
  m_actionsLayout->update();
  m_actionsContainer->adjustSize();
  
  // calculate content height
  int contentHeight = m_actionsContainer->sizeHint().height();
  if (contentHeight == 0) {
    // or estimate based on items
    contentHeight = totalItems * 50; // approx height per item
  }
  
  // limit to max height
  int resultsHeight = qMin(contentHeight, m_maxResultsHeight);
  m_scrollArea->setMaximumHeight(resultsHeight);
  
  // window height = margins (32) + search box (60) + spacing (8) + results height
  int windowHeight = 32 + 60 + 8 + resultsHeight;
  setFixedSize(700, windowHeight);
  
  if (m_positionInitialized) { move(m_fixedPosition); }
  
  // autoselect first action
  if (totalItems > 0) { selectAction(0); }
  
  // dont move focus to action
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
  setFixedSize(700, m_baseHeight);
  
  // maintain position when hiding results
  if (m_positionInitialized) { move(m_fixedPosition); }
}

void Spotlight::navigateActions(int direction)
{
  int totalItems = m_searchResults.size() + m_actions.size();
  if (totalItems == 0) return;
  
  int newIndex = m_selectedActionIndex + direction;
  if (newIndex < 0) { newIndex = totalItems - 1; }
  else if (newIndex >= totalItems) { newIndex = 0; }
  
  selectAction(newIndex);
}

void Spotlight::selectAction(int index)
{
  int totalItems = m_searchResults.size() + m_actions.size();
  if (index < 0 || index >= totalItems) return;
  
  // get button to style
  QPushButton* prevButton = nullptr;
  QPushButton* newButton = nullptr;
  
  // deselect previous
  if (m_selectedActionIndex >= 0 && m_selectedActionIndex < totalItems) {
    if (m_selectedActionIndex < m_searchResults.size()) {
      prevButton = m_searchResults[m_selectedActionIndex];
    } else {
      prevButton = m_actions[m_selectedActionIndex - m_searchResults.size()];
    }
  }
  
  // select new
  if (index < m_searchResults.size()) {
    newButton = m_searchResults[index];
  } else {
    newButton = m_actions[index - m_searchResults.size()];
  }
  
  // apply base style to previous button
  if (prevButton) {
    QWidget* prevWidget = qobject_cast<QWidget*>(prevButton);
    if (prevWidget && prevWidget->property("isResultButton").toBool()) {
      // result widget
      prevWidget->setStyleSheet(
        "QWidget {"
        "  background: transparent;"
        "}"
      );
    } else {
      // regular action button
      QString baseStyle = 
        "QPushButton {"
        "  font-size: 16px;"
        "  padding: 12px 16px;"
        "  color: white;"
        "  background: transparent;"
        "  border: none;"
        "  text-align: left;"
        "}";
      prevButton->setStyleSheet(baseStyle);
    }
  }
  
  // apply selected style to new button
  m_selectedActionIndex = index;
  if (newButton) {
    QWidget* newWidget = qobject_cast<QWidget*>(newButton);
    if (newWidget && newWidget->property("isResultButton").toBool()) {
      // result widget
      newWidget->setStyleSheet(
        "QWidget {"
        "  background: rgba(100, 150, 255, 80);"
        "}"
      );
    } else {
      QString selectedStyle = 
        "QPushButton {"
        "  font-size: 16px;"
        "  padding: 12px 16px;"
        "  color: white;"
        "  background: rgba(100, 150, 255, 80);"
        "  border: none;"
        "  text-align: left;"
        "}";
      newButton->setStyleSheet(selectedStyle);
    }
  }
  
  // keep focus on input field
}

void Spotlight::onActionExecuted() { close(); }

bool Spotlight::eventFilter(QObject* obj, QEvent* event)
{
  // keyboard events on input
  if (obj == m_input) {
    if (event->type() == QEvent::KeyPress) {
      auto* keyEvent = static_cast<QKeyEvent*>(event);
      if (keyEvent->key() == Qt::Key_Escape) {
        close();
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
        int totalItems = m_searchResults.size() + m_actions.size();
        if (m_selectedActionIndex >= 0 && m_selectedActionIndex < totalItems) {
          if (m_selectedActionIndex < m_searchResults.size()) {
            // search result
            launchApp(m_currentSearchResults[m_selectedActionIndex]);
          } else {
            // action
            QString query = m_input->text();
            m_actions[m_selectedActionIndex - m_searchResults.size()]->execute(query);
          }
          close();
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
          launchApp(m_currentSearchResults[index]);
          close();
        }
        return true;
      }
    }
  }
  
  // drag events on input container (not QLineEdit)
  else if (obj == m_inputContainer) {
    if (event->type() == QEvent::MouseButtonPress) {
      auto* mouseEvent = static_cast<QMouseEvent*>(event);
      if (mouseEvent->button() == Qt::LeftButton) {
        // if click is on QLineEdit let it handle text
        QPoint localPos = mouseEvent->pos();
        QWidget* child = m_inputContainer->childAt(localPos);
        // if child on QLineEdit don't drag
        QWidget* widget = child;
        while (widget && widget != m_inputContainer) {
          if (widget == m_input) {
            // click on QLineEdit let it handle text
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
        return true;
      }
    }
  }
  return QDialog::eventFilter(obj, event);
}

void Spotlight::createSearchResultButton(const SearchResult& result, int index)
{
  // create clickable widget
  QWidget* buttonWidget = new QWidget(m_actionsContainer);
  buttonWidget->setCursor(Qt::PointingHandCursor);
  
  // create horizontal layout
  QHBoxLayout* contentLayout = new QHBoxLayout(buttonWidget);
  contentLayout->setContentsMargins(16, 12, 16, 12);
  contentLayout->setSpacing(8);
  
  // name label with ellipsis
  QLabel* nameLabel = new QLabel(result.name, buttonWidget);
  nameLabel->setStyleSheet(
    "QLabel {"
    "  color: white;"
    "  font-size: 16px;"
    "  background: transparent;"
    "}"
  );
  nameLabel->setTextFormat(Qt::PlainText);
  QFontMetrics nameMetrics(nameLabel->font());
  QString elidedName = nameMetrics.elidedText(result.name, Qt::ElideRight, 300);
  nameLabel->setText(elidedName);
  nameLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
  contentLayout->addWidget(nameLabel, 0);
  
  // description label with ellipsis
  if (!result.description.isEmpty()) {
    QLabel* descLabel = new QLabel(result.description, buttonWidget);
    descLabel->setStyleSheet(
      "QLabel {"
      "  color: rgba(255, 255, 255, 140);"
      "  font-size: 14px;"
      "  background: transparent;"
      "}"
    );
    descLabel->setTextFormat(Qt::PlainText);
    QFontMetrics descMetrics(descLabel->font());
    QString elidedDesc = descMetrics.elidedText(result.description, Qt::ElideRight, 300);
    descLabel->setText(elidedDesc);
    descLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    contentLayout->addWidget(descLabel, 1);
  } else {
    // add spacer if no description
    contentLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
  }
  
  // make it clickable
  buttonWidget->installEventFilter(this);
  buttonWidget->setProperty("isResultButton", true);
  buttonWidget->setProperty("resultIndex", index);
  
  // store as QPushButton* for compatibility
  m_searchResults.append(reinterpret_cast<QPushButton*>(buttonWidget));
  m_actionsLayout->addWidget(buttonWidget);
}

void Spotlight::launchApp(const SearchResult& result)
{
  if (result.exec.isEmpty()) return;
  
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
