#include "apps.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QProcess>
#include <algorithm>

AppsSearch::AppsSearch(QObject* parent) : Search(parent) {}

std::vector<SearchResult> AppsSearch::performSearch(const QString& query)
{
  if (!m_appsLoaded) {
    loadApplications();
    m_appsLoaded = true;
  }
  
  std::vector<SearchResult> results;
  
  if (query.isEmpty()) {
    // don't return results for no query
    return results;
  }
  
  // calculate similarity scores
  for (const auto& app : m_applications) {
    int score = calculateSimilarity(query, app.name);
    
    // check description
    if (score < 50) {
      int descScore = calculateSimilarity(query, app.description);
      score = qMax(score, descScore / 2); // description matches worth less
    }
    
    if (score > 0) {
      SearchResult result;
      result.name = app.name;
      result.description = app.description;
      result.exec = app.exec;
      result.data = app.desktopFile;
      result.score = score;
      results.push_back(result);
    }
  }
  
  // sort by score
  std::sort(results.begin(), results.end());
  
  // limit to top 20 results
  if (results.size() > 20) {
    results.erase(results.begin() + 20, results.end());
  }
  
  return results;
}

void AppsSearch::loadApplications()
{
  m_applications.clear();
  
  QStringList dirs = getDesktopFileDirectories();
  
  for (const QString& dirPath : dirs) {
    QDir dir(dirPath);
    if (!dir.exists()) continue;
    
    QStringList filters;
    filters << "*.desktop";
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    
    for (const QFileInfo& fileInfo : files) {
      AppInfo app = parseDesktopFile(fileInfo.absoluteFilePath());
      if (!app.name.isEmpty() && !app.exec.isEmpty()) {
        bool duplicate = false;
        for (const auto& existing : m_applications) {
          if (existing.name == app.name) {
            duplicate = true;
            break;
          }
        }
        if (!duplicate) { m_applications.append(app); }
      }
    }
  }
}

AppInfo AppsSearch::parseDesktopFile(const QString& filePath)
{
  AppInfo app;
  app.desktopFile = filePath;
  
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return app;
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
    
    if (key == "Exec") { app.exec = value; }
    else if (key == "Name") { app.name = value; }
    else if (key == "Comment" && app.description.isEmpty()) { app.description = value; }
    else if (key == "Icon") { app.icon = value; }
  }
  
  file.close();
  
  if (isHidden || noDisplay) { app.name.clear(); app.exec.clear(); }
  
  return app;
}

QStringList AppsSearch::getDesktopFileDirectories()
{
  QStringList dirs;
  
  // user-specific applications
  QString userAppsDir = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation);
  if (!userAppsDir.isEmpty()) {
    dirs.append(userAppsDir);
  }
  
  // system-wide applications from XDG_DATA_DIRS
  QStringList dataDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
  for (const QString& dir : dataDirs) {
    if (!dirs.contains(dir)) {
      dirs.append(dir);
    }
  }
  
  QStringList commonDirs = {
    "/usr/share/applications",
    "/usr/local/share/applications",
    QDir::homePath() + "/.local/share/applications"
  };
  
  for (const QString& dir : commonDirs) {
    if (QDir(dir).exists() && !dirs.contains(dir)) {
      dirs.append(dir);
    }
  }
  
  return dirs;
}
