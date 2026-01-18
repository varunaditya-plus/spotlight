#pragma once
#include <QObject>
#include <QString>
#include <QMetaType>
#include <vector>

struct SearchResult
{
  QString name;
  QString description;
  QString exec; // command to execute (for apps)
  QString data; // additional data (e.g., desktop file path)
  int score = 0; // higher score = better match
  
  ~SearchResult() noexcept = default;
  SearchResult() = default;
  SearchResult(const SearchResult&) = default;
  SearchResult& operator=(const SearchResult&) = default;
  SearchResult(SearchResult&&) noexcept = default;
  SearchResult& operator=(SearchResult&&) noexcept = default;
  
  bool operator<(const SearchResult& other) const {
    return score > other.score; // sort descending by score
  }
};

Q_DECLARE_METATYPE(SearchResult)

class Search : public QObject
{
  Q_OBJECT
public:
  explicit Search(QObject* parent = nullptr);
  virtual ~Search() = default;
  
  // perform search and return results
  virtual std::vector<SearchResult> performSearch(const QString& query) = 0;
  
  // calculate similarity score between query and text (0-100)
  static int calculateSimilarity(const QString& query, const QString& text);
  
signals:
  void resultSelected(const SearchResult& result);
};
