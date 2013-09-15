/*################################################################
Pandrift
Copyright (c) 2013 Warren Moore

This software may be redistributed under the terms of the MIT License.
See the file LICENSE for details.

This code is based on the Panda3D Hello World tutorial.
################################################################*/

#include "world.hh"
#include "pandaFramework.h"
#include "cLerpNodePathInterval.h"
#include "cMetaInterval.h"
#include "auto_bind.h"
#include "mouseWatcher.h"

using namespace pandrift;

namespace
{

const float cMouseScale = 45;

AsyncTask::DoneStatus step_interval_manager(GenericAsyncTask *task_ptr,
                                            void *data_ptr)
{
  CIntervalManager::get_global_ptr()->step();

  return AsyncTask::DS_cont;
}

AsyncTask::DoneStatus update_camera_task(GenericAsyncTask *task_ptr,
                                       void *data_ptr)
{
  World *world_ptr = reinterpret_cast<World*>(data_ptr);
  assert(world_ptr);

  world_ptr->update_camera();

  return AsyncTask::DS_cont;
}

}

World::World(PT(WindowFramework) window_ptr,
             NodePath scene_np,
             NodePath camera_np) :
  window_ptr_(window_ptr),
  scene_np_(scene_np),
  camera_np_(camera_np)
{
  assert(window_ptr_);
}

World::~World()
{
}

void World::set_rift_manager(boost::shared_ptr<pandrift::RiftManager> rift_manager_ptr)
{
  rift_manager_ptr_ = rift_manager_ptr;
}

void World::create_scene()
{
  // Create a task for the interval manager
  AsyncTaskManager::get_global_ptr()->add(new GenericAsyncTask("interval manager task",
                                                               &step_interval_manager,
                                                               NULL));

  // Create a task for the camera update
  AsyncTaskManager::get_global_ptr()->add(new GenericAsyncTask("camera update task",
                                                               &update_camera_task,
                                                               this));

  // Place the camera
  camera_np_.set_pos(0, -20, 3);
  camera_np_.set_hpr(0, 0, 0);

  PandaFramework *framework = window_ptr_->get_panda_framework();
  assert(framework);

  // Load the background
  NodePath environ = window_ptr_->load_model(framework->get_models(),
                                             "models/environment");

  environ.reparent_to(scene_np_);
  environ.set_scale(0.25, 0.25, 0.25);
  environ.set_pos(-8, 42, 0);

  // Load the panda
  NodePath panda_actor = window_ptr_->load_model(framework->get_models(),
                                                 "panda-model");
  panda_actor.set_scale(0.005);
  panda_actor.reparent_to(scene_np_);

  // Load the walk animation
  window_ptr_->load_model(panda_actor, "panda-walk4");
  auto_bind(scene_np_.node(), anim_control_, 0);
  anim_control_.loop_all(true);

  // Create the lerp intervals needed to walk back and forth
  PT(CLerpNodePathInterval) panda_pos_interval1,
    panda_pos_interval2,
    panda_hpr_interval1,
    panda_hpr_interval2;
  panda_pos_interval1 = new CLerpNodePathInterval("panda_pos_interval1",
                                                  13.0,
                                                  CLerpInterval::BT_no_blend,
                                                  true,
                                                  false,
                                                  panda_actor,
                                                  NodePath());
  panda_pos_interval1->set_start_pos(LPoint3f(0, 10, 0));
  panda_pos_interval1->set_end_pos(LPoint3f(0, -10, 0));

  panda_pos_interval2 = new CLerpNodePathInterval("panda_pos_interval2",
                                                  13.0,
                                                  CLerpInterval::BT_no_blend,
                                                  true,
                                                  false,
                                                  panda_actor,
                                                  NodePath());
  panda_pos_interval2->set_start_pos(LPoint3f(0, -10, 0));
  panda_pos_interval2->set_end_pos(LPoint3f(0, 10, 0));

  panda_hpr_interval1 = new CLerpNodePathInterval("panda_hpr_interval1",
                                                  3.0,
                                                  CLerpInterval::BT_no_blend,
                                                  true,
                                                  false,
                                                  panda_actor,
                                                  NodePath());
  panda_hpr_interval1->set_start_hpr(LPoint3f(0, 0, 0));
  panda_hpr_interval1->set_end_hpr(LPoint3f(180, 0, 0));

  panda_hpr_interval2 = new CLerpNodePathInterval("panda_hpr_interval2",
                                                  3.0,
                                                  CLerpInterval::BT_no_blend,
                                                  true,
                                                  false,
                                                  panda_actor,
                                                  NodePath());
  panda_hpr_interval2->set_start_hpr(LPoint3f(180, 0, 0));
  panda_hpr_interval2->set_end_hpr(LPoint3f(0, 0, 0));

  // Create and play the sequence that coordinates the intervals
  PT(CMetaInterval) panda_pace = new CMetaInterval("panda_pace");
  panda_pace->add_c_interval(panda_pos_interval1,
                             0,
                             CMetaInterval::RS_previous_end);
  panda_pace->add_c_interval(panda_hpr_interval1,
                             0,
                             CMetaInterval::RS_previous_end);
  panda_pace->add_c_interval(panda_pos_interval2,
                             0,
                             CMetaInterval::RS_previous_end);
  panda_pace->add_c_interval(panda_hpr_interval2,
                             0,
                             CMetaInterval::RS_previous_end);
  panda_pace->loop();
}

void World::update_camera()
{
  float yaw = 0, pitch = 0, roll = 0;
  if (rift_manager_ptr_ && rift_manager_ptr_->get_sensor_euler_angles(yaw, pitch, roll))
  {
    yaw *= 180.0 / M_PI;
    pitch *= 180.0 / M_PI;
    roll *= 180.0 / M_PI;
  }
  else
  {
    float mouse_x = 0, mouse_y = 0;
    PT(MouseWatcher) mouse_watcher_ptr = DCAST(MouseWatcher, window_ptr_->get_mouse().node());
    if (mouse_watcher_ptr->has_mouse())
    {
       const LPoint2f &mouse_pos = mouse_watcher_ptr->get_mouse();
       mouse_x = mouse_pos.get_x(); // -1 to 1
       mouse_y = mouse_pos.get_y(); // -1 to 1
    }

    yaw = mouse_x * cMouseScale;
    pitch = mouse_y * cMouseScale;
  }

  camera_np_.set_hpr(yaw, pitch, -roll);
}
