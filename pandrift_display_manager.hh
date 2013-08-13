/*################################################################
Pandrift
Copyright (c) 2013 Warren Moore

This software may be redistributed under the terms of the MIT License.
See the file LICENSE for details.
################################################################*/

#ifndef PANDRIFT_DISPLAY_MANAGER_HEADER
#define PANDRIFT_DISPLAY_MANAGER_HEADER

#include "pandrift_rift_manager.hh"
#include "pandaFramework.h"
#include "pandaSystem.h"
#include "boost/shared_ptr.hpp"

namespace pandrift
{

enum EyeSelect
{
  cEyeLeft = 0,
  cEyeRight = 1
};

class DisplayManager
{
public:
  enum WarpMode
  {
    cStereo = 0,
    cShader,
    cShaderChromaticAberration
  };

  DisplayManager(PT(WindowFramework) window_ptr);

  ~DisplayManager();

  void set_warp_mode(WarpMode warp_mode);

  void set_scene_resolution(int width, int height);

  void set_lookup_resolution(int width, int height);

  NodePath get_camera_root();

  bool set_rift_manager(boost::shared_ptr<RiftManager> rift_manager_ptr);

  bool is_created();

  bool create_display(bool enabled = true);

  void destroy_display();

  bool is_enabled();

  void set_enabled(bool enabled);

private:
  bool create_render_region();

  void destroy_render_region();

  bool create_render_camera();

  void destroy_render_camera();

  void create_shader_cards(NodePath root_np);

  void destroy_shader_cards();

  bool create_scene_buffer();

  void destroy_scene_buffer();

  bool create_scene_cameras();

  void destroy_scene_cameras();

  bool apply_shader();

  void remove_shader();

  WarpMode warp_mode_;
  int scene_width_, scene_height_;
  int lookup_width_, lookup_height_;
  PT(WindowFramework) window_ptr_;
  boost::shared_ptr<RiftManager> rift_manager_ptr_;
  PT(DisplayRegion) render_region_ptr_;
  NodePath render_root_np_;
  NodePath render_camera_np_;
  NodePath render_card_np_[2];
  PT(Shader) render_shader_;
  PT(GraphicsOutput) scene_buffer_ptr_;
  PT(DisplayRegion) scene_region_ptr_[2];
  NodePath scene_camera_root_np_;
  NodePath scene_camera_np_[2];
};

}

#endif
