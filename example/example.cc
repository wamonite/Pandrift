/*################################################################
Pandrift
Copyright (c) 2013 Warren Moore

This software may be redistributed under the terms of the MIT License.
See the file LICENSE for details.
################################################################*/

#include "pandaFramework.h"
#include "pandaSystem.h"
#include "load_prc_file.h"
#include "world.hh"
#include "pandrift_rift_manager.hh"
#include "pandrift_display_manager.hh"
#include "boost/shared_ptr.hpp"

using namespace pandrift;

void key_escape_handler(const Event *event, void *data)
{
  PandaFramework *framework = reinterpret_cast<PandaFramework*>(data);
  assert(framework);

  framework->set_exit_flag();
}

void key_fullscreen_handler(const Event *event, void *data)
{
  // Can we use DCAST here?
  WindowFramework *window_ptr = reinterpret_cast<WindowFramework*>(data);
  assert(window_ptr);

  PT(GraphicsWindow) graphics_window_ptr = window_ptr->get_graphics_window();
  
  WindowProperties properties = graphics_window_ptr->get_properties();
  properties.set_fullscreen(!properties.get_fullscreen());
  
  graphics_window_ptr->request_properties(properties);
}

void key_rift_handler(const Event *event, void *data)
{
  DisplayManager *display_manager = reinterpret_cast<DisplayManager*>(data);
  assert(display_manager);

  if (!display_manager->is_created())
  {
    display_manager->create_display();
  }
  else
  {
#if 1
    display_manager->set_enabled(!display_manager->is_enabled());
#else
    display_manager->destroy_display();
#endif
  }

  cerr << "Display created? " << (display_manager->is_created() ? "Y" : "N") << endl;
  cerr << "Display enabled? " << (display_manager->is_enabled() ? "Y" : "N") << endl;
}

int main(int argc, char *argv[])
{
  // Create the rift manager
  boost::shared_ptr<RiftManager> rift_manager_ptr(new RiftManager());

  // Start the Panda framework
  PandaFramework framework;
  framework.open_framework(argc, argv);
  framework.set_window_title("Pandrift Example");

  // Override config
//  load_prc_file_data("", "notify-level-glgsg debug");
  load_prc_file_data("", "red-blue-stereo 0");
  load_prc_file_data("", "side-by-side-stereo 0");

  // Open window
  WindowProperties window_properties;
  framework.get_default_window_props(window_properties);
  window_properties.set_size(rift_manager_ptr->get_display_width(),
                             rift_manager_ptr->get_display_height());
  window_properties.set_fixed_size(true);
//  window_properties.set_fullscreen(true);

  const int cWindowFlags = 0;
  PT(WindowFramework) window_ptr = framework.open_window(window_properties,
                                                         cWindowFlags);
//  window_ptr->set_background_type(WindowFramework::BT_black);

  // Create and configure the display manager
  DisplayManager display_manager(window_ptr);
  NodePath display_camera_group = display_manager.get_camera_root();
  display_camera_group.reparent_to(window_ptr->get_camera_group());
  display_manager.set_rift_manager(rift_manager_ptr);

  // Enable keyboard
  window_ptr->enable_keyboard();
  framework.define_key("escape", "Exit", &key_escape_handler, &framework);
  framework.define_key("f", "Toggle fullscreen", &key_fullscreen_handler, window_ptr);
  framework.define_key("r", "Toggle Rift view", &key_rift_handler, &display_manager);

  // Create the scene
  World world(window_ptr,
              window_ptr->get_render(),
              window_ptr->get_camera_group());
  world.set_rift_manager(rift_manager_ptr);
  world.create_scene();

  // Run the main loop until exit flag set
  framework.main_loop();

  framework.close_framework();

  return 0;
}
