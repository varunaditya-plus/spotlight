#pragma once
#include "../Spotlight.h"
#include <vector>
#include <QString>

// base class for spotlight apps
class SpotlightApp
{
public:
  virtual ~SpotlightApp() = default;
  
  // return menu items for this app
  virtual std::vector<MenuItem> getMenuItems() = 0;
};
