#pragma once
#include "../spotlightapp.h"

class DemoApp : public SpotlightApp
{
public:
  std::vector<MenuItem> getMenuItems() override;
};
