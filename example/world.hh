/*################################################################
Pandrift
Copyright (c) 2013 Warren Moore

This software may be redistributed under the terms of the MIT License.
See the file LICENSE for details.
################################################################*/

#ifndef PANDRIFT_EXAMPLE_WORLD_HEADER
#define PANDRIFT_EXAMPLE_WORLD_HEADER

#include "windowFramework.h"
#include "animControlCollection.h"
#include "cIntervalManager.h"
#include "boost/shared_ptr.hpp"
#include "pandrift_rift_manager.hh"

class World
{
public:
  World(PT(WindowFramework) window_ptr,
        NodePath scene_np,
        NodePath camera_np);

  ~World();

  void set_rift_manager(boost::shared_ptr<pandrift::RiftManager> rift_manager_ptr);

  void create_scene();

  void update_camera();

private:
  PT(WindowFramework) window_ptr_;
  NodePath scene_np_;
  NodePath camera_np_;
  boost::shared_ptr<pandrift::RiftManager> rift_manager_ptr_;
  AnimControlCollection anim_control_;
};

#endif
