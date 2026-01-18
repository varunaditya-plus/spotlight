#include "settings.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QFileInfo>
#include <algorithm>

SettingsSearch::SettingsSearch(QObject* parent) : Search(parent) {}

std::vector<SearchResult> SettingsSearch::performSearch(const QString& query)
{
  if (!m_settingsLoaded) {
    loadSettings();
    m_settingsLoaded = true;
  }
  
  std::vector<SearchResult> results;
  
  if (query.isEmpty()) {
    // don't return results for no query
    return results;
  }
  
  // calculate similarity scores
  for (const auto& setting : m_settings) {
    int score = calculateSimilarity(query, setting.name);
    
    // check description
    if (score < 50) {
      int descScore = calculateSimilarity(query, setting.description);
      score = qMax(score, descScore / 2); // description matches worth less
    }
    
    if (score > 0) {
      SearchResult result;
      result.name = setting.name;
      result.description = setting.description;
      result.exec = setting.exec;
      result.data = setting.desktopFile;
      result.score = score;
      results.push_back(result);
    }
  }
  
  // sort by score
  std::sort(results.begin(), results.end());
  
  return results;
}

void SettingsSearch::loadSettings()
{
  m_settings.clear();
  
  QStringList dirs = getDesktopFileDirectories();
  
  for (const QString& dirPath : dirs) {
    QDir dir(dirPath);
    if (!dir.exists()) continue;
    
    QStringList filters;
    filters << "gnome-*-panel.desktop";
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);
    
    for (const QFileInfo& fileInfo : files) {
      SettingsInfo setting = parseDesktopFile(fileInfo.absoluteFilePath());
      
      if (!setting.name.isEmpty() && !setting.exec.isEmpty()) {
        bool duplicate = false;
        for (const auto& existing : m_settings) {
          if (existing.name == setting.name) {
            duplicate = true;
            break;
          }
        }
        if (!duplicate) { 
          m_settings.append(setting); 
        }
      }
    }
  }
}

SettingsInfo SettingsSearch::parseDesktopFile(const QString& filePath)
{
  SettingsInfo setting;
  setting.desktopFile = filePath;
  
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return setting;
  }
  
  QTextStream in(&file);
  QString currentSection;
  bool isDesktopEntry = false;
  bool isHidden = false;
  bool noDisplay = false;
  
  while (!in.atEnd()) {
    QString line = in.readLine().trimmed();
    
    if (line.isEmpty() || line.startsWith("#")) {
      continue;
    }
    
    if (line.startsWith("[") && line.endsWith("]")) {
      currentSection = line.mid(1, line.length() - 2);
      isDesktopEntry = (currentSection == "Desktop Entry");
      continue;
    }
    
    if (!isDesktopEntry) { continue; }
    
    int eqPos = line.indexOf('=');
    if (eqPos == -1) continue;
    
    QString key = line.left(eqPos).trimmed();
    QString value = line.mid(eqPos + 1).trimmed();
    
    if (key == "Hidden" && value == "true") { isHidden = true; }
    if (key == "NoDisplay" && value == "true") { noDisplay = true; }
    
    if (key == "Exec") { setting.exec = value; }
    else if (key == "Name" && setting.name.isEmpty()) { 
      // use first Name field (usually the default/English one)
      setting.name = value; 
    }
    else if (key == "Comment" && setting.description.isEmpty()) { 
      setting.description = value; 
    }
  }
  
  file.close();
  
  // unly filter out Hidden=true cause NoDisplay=true only hides from menus but should still be searchable
  if (isHidden) { 
    setting.name.clear(); 
    setting.exec.clear(); 
  }
  
  return setting;
}

QStringList SettingsSearch::getDesktopFileDirectories()
{
  QStringList dirs;
  
  // system-wide apps
  QStringList commonDirs = {
    "/usr/share/applications",
    "/usr/local/share/applications"
  };
  
  for (const QString& dir : commonDirs) {
    if (QDir(dir).exists() && !dirs.contains(dir)) {
      dirs.append(dir);
    }
  }
  
  return dirs;
}
