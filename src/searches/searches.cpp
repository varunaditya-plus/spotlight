#include "searches.h"
#include <QString>
#include <algorithm>

Search::Search(QObject* parent)
  : QObject(parent)
{
}

int Search::calculateSimilarity(const QString& query, const QString& text)
{
  QString queryLower = query.toLower();
  QString textLower = text.toLower();
  
  // exact match gets highest score
  if (textLower == queryLower) {
    return 100;
  }
  
  // starts with query gets high score
  if (textLower.startsWith(queryLower)) {
    return 90;
  }
  
  // contains query as whole word gets medium-high score
  if (textLower.contains(" " + queryLower + " ") || 
      textLower.contains(" " + queryLower) ||
      textLower.contains(queryLower + " ")) {
    return 70;
  }
  
  // contains query anywhere gets medium score
  if (textLower.contains(queryLower)) {
    return 50;
  }
  
  // check if all characters of query appear in order (fuzzy match)
  int queryIdx = 0;
  for (int i = 0; i < textLower.length() && queryIdx < queryLower.length(); ++i) {
    if (textLower[i] == queryLower[queryIdx]) {
      queryIdx++;
    }
  }
  
  if (queryIdx == queryLower.length()) {
    // all characters found in order
    return 30;
  }
  
  // no match
  return 0;
}
