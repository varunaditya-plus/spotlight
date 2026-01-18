#pragma once
#include "searches.h"
#include <QString>
#include <QList>

struct AppInfo
{
  QString name;
  QString exec;
  QString icon;
  QString description;
  QString desktopFile;
};

class AppsSearch : public Search
{
  Q_OBJECT
public:
  explicit AppsSearch(QObject* parent = nullptr);
  
  std::vector<SearchResult> performSearch(const QString& query) override;
  
private:
  // load all applications from desktop files
  void loadApplications();
  
  // parse .desktop file and extract app info
  AppInfo parseDesktopFile(const QString& filePath);
  
  // get desktop file directories
  QStringList getDesktopFileDirectories();
  
  QList<AppInfo> m_applications;
  bool m_appsLoaded = false;
};
