#include "search.h"
#include <QDesktopServices>
#include <QUrl>

SearchAction::SearchAction(QWidget* parent)
  : Action("Search", parent)
{
}

void SearchAction::execute(const QString& query)
{ // add support for other search engines later
  QString url = "https://www.google.com/search?q=" + QUrl::toPercentEncoding(query);
  QDesktopServices::openUrl(QUrl(url));
  emit actionExecuted();
}
