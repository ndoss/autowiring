// Copyright (C) 2012-2014 Leap Motion, Inc. All rights reserved.
#pragma once
#include <stdexcept>

class autowiring_error:
  public std::runtime_error
{
public:
  autowiring_error(const char* what):
    std::runtime_error(what)
  {}
};

