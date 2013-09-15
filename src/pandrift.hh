/*################################################################
Pandrift
Copyright (c) 2013 Warren Moore

This software may be redistributed under the terms of the MIT License.
See the file LICENSE for details.
################################################################*/

#ifndef PANDRIFT_HEADER
#define PANDRIFT_HEADER

#include "notifyCategoryProxy.h"

// We're not building as a Panda SO yet but follow the form
#define EXPCL_PANDRIFT
#define EXPTP_PANDRIFT

NotifyCategoryDecl(pandrift, EXPCL_PANDRIFT, EXPCL_PANDRIFT);

namespace pandrift
{

enum EyeSelect
{
  cEyeLeft = 0,
  cEyeRight = 1
};

}

#endif
