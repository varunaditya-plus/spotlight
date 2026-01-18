#include "Spotlight.h"
#include "actions/actions.h"
#include "actions/search.h"
#include <QKeyEvent>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMoveEvent>

Spotlight::Spotlight(QWidget* parent)
  : QDialog(parent)
{
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  setAttribute(Qt::WA_TranslucentBackground);
  
  setStyleSheet(
    "QDialog { background: rgba(30, 30, 30, 240); border-radius: 12px; }"
    "QLineEdit {"
    "  font-size: 18px;"
    "  padding: 0px;"
    "  color: white;"
    "  background: transparent;"
    "  border: none;"
    "}"
    "QPushButton {"
    "  font-size: 16px;"
    "  padding: 10px;"
    "  color: white;"
    "  background: rgba(255, 255, 255, 30);"
    "  border-left: 1px solid rgba(255, 255, 255, 50);"
    "  border-right: 1px solid rgba(255, 255, 255, 50);"
    "  border-bottom: 1px solid rgba(255, 255, 255, 50);"
    "  border-top: none;"
    "  border-radius: 0px;"
    "  text-align: left;"
    "}"
    "QPushButton:hover {"
    "  background: rgba(255, 255, 255, 40);"
    "}"
    "QPushButton:focus {"
    "  outline: none;"
    "}"
  );
  
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(20, 20, 20, 20);
  layout->setSpacing(0);
  
  // draggable container
  m_inputContainer = new QWidget(this);
  m_inputContainer->setStyleSheet(
    "QWidget {"
    "  background: rgba(255, 255, 255, 30);"
    "  border: 1px solid rgba(255, 255, 255, 50);"
    "  border-top-left-radius: 8px;"
    "  border-top-right-radius: 8px;"
    "  border-bottom-left-radius: 0px;"
    "  border-bottom-right-radius: 0px;"
    "  border-bottom: none;"
    "}"
  );
  m_inputContainer->installEventFilter(this);
  
  auto* inputLayout = new QVBoxLayout(m_inputContainer);
  inputLayout->setContentsMargins(12, 12, 12, 12);
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
  
  m_actionsContainer = new QWidget(this);
  m_actionsLayout = new QVBoxLayout(m_actionsContainer);
  m_actionsLayout->setContentsMargins(0, 0, 0, 0);
  m_actionsLayout->setSpacing(0);
  m_actionsContainer->hide();
  layout->addWidget(m_actionsContainer);
  
  resize(500, m_baseHeight);
  
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
  
  // show actions and lower area
  m_actionsContainer->show();
  int actionHeight = 50 * m_actions.size();
  resize(500, m_baseHeight + actionHeight);
  
  // autoselect first action
  if (!m_actions.isEmpty()) {
    selectAction(0);
  }
  
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
  m_selectedActionIndex = -1;
  m_actionsContainer->hide();
  resize(500, m_baseHeight);
}

void Spotlight::navigateActions(int direction)
{
  if (m_actions.isEmpty()) return;
  
  int newIndex = m_selectedActionIndex + direction;
  if (newIndex < 0) {
    newIndex = m_actions.size() - 1;
  } else if (newIndex >= m_actions.size()) {
    newIndex = 0;
  }
  
  selectAction(newIndex);
}

void Spotlight::selectAction(int index)
{
  if (index < 0 || index >= m_actions.size()) return;
  
  // deselect previous
  if (m_selectedActionIndex >= 0 && m_selectedActionIndex < m_actions.size()) {
    int prevIndex = m_selectedActionIndex;
    QString baseStyle = 
      "QPushButton {"
      "  font-size: 16px;"
      "  padding: 10px;"
      "  color: white;"
      "  background: rgba(255, 255, 255, 30);"
      "  border-left: 1px solid rgba(255, 255, 255, 50);"
      "  border-right: 1px solid rgba(255, 255, 255, 50);"
      "  border-bottom: 1px solid rgba(255, 255, 255, 50);"
      "  border-top: " + QString(prevIndex == 0 ? "1px" : "none") + " solid rgba(255, 255, 255, 50);"
      "  border-top-left-radius: 0px;"
      "  border-top-right-radius: 0px;"
      "  border-bottom-left-radius: " + QString(prevIndex == m_actions.size() - 1 ? "8px" : "0px") + ";"
      "  border-bottom-right-radius: " + QString(prevIndex == m_actions.size() - 1 ? "8px" : "0px") + ";"
      "  text-align: left;"
      "}";
    
    m_actions[prevIndex]->setStyleSheet(baseStyle);
  }
  
  // select new
  m_selectedActionIndex = index;
  QString selectedStyle = 
    "QPushButton {"
    "  font-size: 16px;"
    "  padding: 10px;"
    "  color: white;"
    "  background: rgba(100, 150, 255, 150);"
    "  border-left: 1px solid rgba(100, 150, 255, 200);"
    "  border-right: 1px solid rgba(100, 150, 255, 200);"
    "  border-bottom: 1px solid rgba(100, 150, 255, 200);"
    "  border-top: " + QString(index == 0 ? "1px" : "none") + " solid rgba(100, 150, 255, 200);"
    "  border-top-left-radius: 0px;"
    "  border-top-right-radius: 0px;"
    "  border-bottom-left-radius: " + QString(index == m_actions.size() - 1 ? "8px" : "0px") + ";"
    "  border-bottom-right-radius: " + QString(index == m_actions.size() - 1 ? "8px" : "0px") + ";"
    "  text-align: left;"
    "}";
  
  m_actions[m_selectedActionIndex]->setStyleSheet(selectedStyle);
  // keep focus on input field
}

void Spotlight::onActionExecuted()
{
  close();
}

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
        if (m_selectedActionIndex >= 0 && m_selectedActionIndex < m_actions.size()) {
          QString query = m_input->text();
          m_actions[m_selectedActionIndex]->execute(query);
          close();
          return true;
        }
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
