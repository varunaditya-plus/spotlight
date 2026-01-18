#pragma once
#include "searches.h"
#include <QString>
#include <QList>

struct SettingsInfo
{
  QString name;
  QString exec;
  QString description;
  QString desktopFile;
};

class SettingsSearch : public Search
{
  Q_OBJECT
public:
  explicit SettingsSearch(QObject* parent = nullptr);
  
  std::vector<SearchResult> performSearch(const QString& query) override;
  
private:
  void loadSettings();
  
  // parse .desktop file and extract settings info
  SettingsInfo parseDesktopFile(const QString& filePath);
  
  // get desktop file directories
  QStringList getDesktopFileDirectories();
  
  QList<SettingsInfo> m_settings;
  bool m_settingsLoaded = false;
};
