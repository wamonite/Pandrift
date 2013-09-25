/*################################################################
Pandrift
Copyright (c) 2013 Warren Moore

This software may be redistributed under the terms of the MIT License.
See the file LICENSE for details.
################################################################*/

#include "pandrift_display_manager.hh"
#include "orthographicLens.h"
#include <iostream>
#include "cardMaker.h"
#include "lmatrix.h"
#include <math.h>
#include "matrixLens.h"

using namespace std;

namespace
{

const int cDefaultSceneWidth = 2048;
const int cDefaultSceneHeight = 1024;
const int cDefaultLookupWidth = 1280;
const int cDefaultLookupHeight = 800;
const float cOrthographicLensFilmWidth = 2.0;
const float cOrthographicLensFilmHeight = 2.0;
const float cOrthographicLensNear = -1000;
const float cOrthographicLensFar = 1000;
const char *cRenderRootName = "render 2d root";
const char *cRenderCameraName = "render 2d camera";
const char *cRenderCardRootName = "render 2d cards";
const char *cRenderShaderCardName = "render 2d shader card";
const char *cSceneBufferName = "scene buffer";
const char *cSceneCameraRootName = "scene 3d camera root";
const char *cSceneCameraName = "scene 3d camera";
const float cSceneCameraNear = 0.01;
const float cSceneCameraFar = 2000.0;
const float cDefaultFOV2D = 85.0 * (M_PI / 180.0);
const float cHUDDistance = 0.8;
const char *cHUDCameraName = "scene 2d camera";

}

namespace pandrift
{

DisplayManager::DisplayManager(PT(WindowFramework) window_ptr) :
  warp_mode_(cShader),
  scene_width_(cDefaultSceneWidth),
  scene_height_(cDefaultSceneHeight),
  lookup_width_(cDefaultLookupWidth),
  lookup_height_(cDefaultLookupHeight),
  window_ptr_(window_ptr),
  created_(false),
  render_root_np_(cRenderRootName),
  scene_camera_root_np_(cSceneCameraRootName)
{
//  pandrift_cat->set_severity(NS_debug);
}

DisplayManager::~DisplayManager()
{
  if (window_ptr_)
  {
    // Only destroy the display elements if the application/window hasn't been closed
    PT(GraphicsWindow) graphics_window_ptr = window_ptr_->get_graphics_window();
    if (graphics_window_ptr && !graphics_window_ptr->is_closed())
    {
      destroy_display();
    }
  }
}

void DisplayManager::set_warp_mode(WarpMode warp_mode)
{
  warp_mode_ = warp_mode;
}

void DisplayManager::set_scene_resolution(int width, int height)
{
  if (width > 0 && height > 0)
  {
    scene_width_ = width;
    scene_height_ = height;
  }
}

void DisplayManager::set_lookup_resolution(int width, int height)
{
  if (width > 0 && height > 0)
  {
    lookup_width_ = width;
    lookup_height_ = height;
  }
}

NodePath DisplayManager::get_camera_root()
{
  return scene_camera_root_np_;
}

bool DisplayManager::set_rift_manager(boost::shared_ptr<RiftManager> rift_manager_ptr)
{
  rift_manager_ptr_ = rift_manager_ptr;

  return true;
}

bool DisplayManager::is_created()
{
  return created_;
}

bool DisplayManager::create_display(bool enabled)
{
  assert(!created_);

  if (!window_ptr_)
  {
    pandrift_cat.error() << "create_display: Window not set";
    return false;
  }

  if (!rift_manager_ptr_)
  {
    pandrift_cat.error() << "create_display: Rift manager not set";
    return false;
  }

  // Create the common components of the display
  bool created = create_scene_buffer() &&
                 create_scene_cameras() &&
                 create_hud_cameras() &&
                 create_render_region() &&
                 create_render_camera();

  // Create and configure the mode-dependent components
  if (created)
  {
    switch (warp_mode_)
    {
      case cStereo:
        // Create the shader cards under the render root
        create_shader_cards(render_root_np_);

        // Bind the scene texture to the shader cards
        render_card_np_[cEyeLeft].set_texture(scene_buffer_ptr_->get_texture());
        render_card_np_[cEyeRight].set_texture(scene_buffer_ptr_->get_texture());

        break;

      case cShader:
      case cShaderChromaticAberration:
        // Create the shader cards under the render root
        create_shader_cards(render_root_np_);

        // Bind the scene texture to the shader cards
        render_card_np_[cEyeLeft].set_texture(scene_buffer_ptr_->get_texture());
        render_card_np_[cEyeRight].set_texture(scene_buffer_ptr_->get_texture());

        // Apply the appropriate shader to the shader cards
        created = apply_shader();

        break;

      default:
        created = false;
    }
  }

  if (created)
  {
    created_ = true;

    set_enabled(enabled);
  }
  else
  {
    destroy_display();
  }

  return created;
}

void DisplayManager::destroy_display()
{
  set_enabled(false);

  remove_shader();
  destroy_shader_cards();
  destroy_render_camera();
  destroy_render_region();
  destroy_hud_cameras();
  destroy_scene_cameras();
  destroy_scene_buffer();

  created_ = false;
}

bool DisplayManager::is_enabled()
{
  return render_region_ptr_ && render_region_ptr_->is_active();
}

void DisplayManager::set_enabled(bool enabled)
{
  if (is_enabled() == enabled)
    return;

  // As we want to ensure the default 2D region is deactivated,
  // access the root to ensure it has been created.
  NodePath scene_2d_root_np = window_ptr_->get_render_2d();

  // Enable/disable the default 3D display region
  PT(DisplayRegion) region_3d_ptr = window_ptr_->get_display_region_3d();
  assert(region_3d_ptr);
  region_3d_ptr->set_active(!enabled);

  // Enabled/disable the default 2D display region
  PT(DisplayRegion) region_2d_ptr = window_ptr_->get_display_region_2d();
  assert(region_2d_ptr);
  region_2d_ptr->set_active(!enabled);

  // Enable/disable our render region
  render_region_ptr_->set_active(enabled);
}

bool DisplayManager::create_render_region()
{
  assert(window_ptr_);
  assert(!render_region_ptr_);

  // Create the render region: our display region filling the window
  PT(GraphicsOutput) graphics_output_ptr = window_ptr_->get_graphics_output();
  render_region_ptr_ = graphics_output_ptr->make_display_region();

  // Only activate through set_enabled(true)
  render_region_ptr_->set_active(false);

  return render_region_ptr_;
}

void DisplayManager::destroy_render_region()
{
  if (!render_region_ptr_)
    return;

  // Remove our render region from window
  PT(GraphicsOutput) graphics_output_ptr = window_ptr_->get_graphics_output();
  graphics_output_ptr->remove_display_region(render_region_ptr_);
  render_region_ptr_ = NULL;
}

bool DisplayManager::create_render_camera()
{
  assert(render_camera_np_.is_empty());

  // Create an orthographic lens [-1, 1] for the render region
  PT(OrthographicLens) lens_ptr = new OrthographicLens();
  lens_ptr->set_film_size(cOrthographicLensFilmWidth,
                          cOrthographicLensFilmHeight);
  lens_ptr->set_near_far(cOrthographicLensNear,
                         cOrthographicLensFar);

  // Create a camera for the orthographic lens
  PT(Camera) camera_ptr = new Camera(cRenderCameraName);
  camera_ptr->set_lens(lens_ptr);

  // Attach the render camera to our own render root node
  render_camera_np_ = NodePath(camera_ptr);
  render_camera_np_.reparent_to(render_root_np_);

  // Attach the camera to the render region
  render_region_ptr_->set_camera(render_camera_np_);

  return true;
}

void DisplayManager::destroy_render_camera()
{
  if (render_camera_np_.is_empty())
    return;

  // Remove the render region camera
  render_camera_np_.remove_node();
}

void DisplayManager::create_shader_cards(NodePath root_np)
{
  assert(!root_np.is_empty());
  assert(render_card_np_[cEyeLeft].is_empty());
  assert(render_card_np_[cEyeRight].is_empty());

  // Card maker should white, texture-mapped cards
  CardMaker card_maker(cRenderShaderCardName);
  card_maker.set_has_uvs(true);
  card_maker.set_color(1.0, 1.0, 1.0, 1.0);

  // Generate two cards; one left and one right
  for (int eye = 0; eye <= 1; eye += 1)
  {
    const float cEye = eye;

    // Set U [0.0, 0.5] for left, [0.5, 1.0] for right. Set V [0.0, 1.0] for both.
    card_maker.set_uv_range(LTexCoord(cEye / 2.0, 0.0), LTexCoord((cEye + 1.0) / 2.0, 1.0));
    // Set X [-1.0, 0.0] for left, [0.0, 1.0] for right. Set Y [-1.0, 1.0] for both.
    card_maker.set_frame(cEye - 1.0, cEye, -1.0, 1.0);

    render_card_np_[eye] = NodePath(card_maker.generate());

    // Disable the depth buffer
    render_card_np_[eye].set_depth_test(false);
    render_card_np_[eye].set_depth_write(false);

    // Attach to the specific node
    render_card_np_[eye].reparent_to(root_np);
  }
}

void DisplayManager::destroy_shader_cards()
{
  for (int eye = 0; eye <= 1; eye += 1)
    if (!render_card_np_[eye].is_empty())
      // Remove the render cards
      render_card_np_[eye].remove_node();
}

bool DisplayManager::create_scene_buffer()
{
  assert(window_ptr_);
  assert(!scene_buffer_ptr_);
  assert(!scene_region_ptr_[cEyeLeft]);
  assert(!scene_region_ptr_[cEyeRight]);

  // Create the buffer in which to render our scene
  scene_buffer_ptr_ = window_ptr_->get_graphics_output()->make_texture_buffer(cSceneBufferName,
                                                                              scene_width_,
                                                                              scene_height_);
  if (!scene_buffer_ptr_)
  {
    pandrift_cat.error() << "create_scene_buffer: Unable to create scene buffer";
    return false;
  }

  // Make sure the scene is rendered first
  scene_buffer_ptr_->set_sort(-100);

  // Attach the buffer to a usable texture
  PT(Texture) scene_texture_ptr = scene_buffer_ptr_->get_texture();

  // Set some texture parameters
  //TODO(WM) These look good but need further investigation
  scene_texture_ptr->set_magfilter(Texture::FT_linear);
  scene_texture_ptr->set_minfilter(Texture::FT_linear);
  scene_texture_ptr->set_anisotropic_degree(2);

  for (int eye = 0; eye <= 1; ++eye)
  {
    // Create the left and right display regions in which to render the 3D scene
    // Set X [0.0, 0.5] for left, [0.5, 1.0] for right. Set Y [0.0, 1.0] for both.
    scene_region_ptr_[eye] = scene_buffer_ptr_->make_mono_display_region(float(eye) * 0.5,
                                                                         float(eye + 1) * 0.5,
                                                                         0.0,
                                                                         1.0);

    // Create the left and right display regions in which to render the 2D scene
    // Set X [0.0, 0.5] for left, [0.5, 1.0] for right. Set Y [0.0, 1.0] for both.
    hud_region_ptr_[eye] = scene_buffer_ptr_->make_mono_display_region(float(eye) * 0.5,
                                                                       float(eye + 1) * 0.5,
                                                                       0.0,
                                                                       1.0);
  }

  return true;
}

void DisplayManager::destroy_scene_buffer()
{
  for (int eye = 0; eye <= 1; ++eye)
  {
    if (scene_region_ptr_[eye])
    {
      // Remove and reset the 3D scene display regions
      scene_buffer_ptr_->remove_display_region(scene_region_ptr_[eye]);
      scene_region_ptr_[eye] = NULL;
    }

    if (hud_region_ptr_[eye])
    {
      // Remove and reset the 2D scene display regions
      scene_buffer_ptr_->remove_display_region(hud_region_ptr_[eye]);
      hud_region_ptr_[eye] = NULL;
    }
  }

  if (!scene_buffer_ptr_)
    return;

  // Remove and reset the scene render buffer
  PT(GraphicsEngine) graphics_engine_ptr = scene_buffer_ptr_->get_engine();
  graphics_engine_ptr->remove_window(scene_buffer_ptr_);
  scene_buffer_ptr_ = NULL;
}

bool DisplayManager::create_scene_cameras()
{
  assert(rift_manager_ptr_);
  assert(scene_region_ptr_[cEyeLeft]);
  assert(scene_region_ptr_[cEyeRight]);

  // Calculate the Rift projection matrix
  const float cYFOV = rift_manager_ptr_->get_y_fov_radians();
  const float cDisplayAspectRatio = rift_manager_ptr_->get_display_aspect_ratio();
  const float cTanHalfFOV = tan(cYFOV * 0.5);

  LMatrix4f projection = LMatrix4f::zeros_mat();
  projection[0][0] = 1.0 / (cTanHalfFOV * cDisplayAspectRatio); // or tan(get_y_fov_radians() * cDisplayAspectRatio * 0.5)?
  projection[2][1] = 1.0 / cTanHalfFOV;
  projection[1][2] = cSceneCameraFar / (cSceneCameraFar - cSceneCameraNear);
  projection[1][3] = 1;
  projection[3][2] = -cSceneCameraFar * cSceneCameraNear / (cSceneCameraFar - cSceneCameraNear);

  const float cIPDOffset = rift_manager_ptr_->get_interpupillary_distance() / 2.0;
  const float cProjectionCentreOffset = rift_manager_ptr_->get_projection_centre_offset();
  for (int eye = 0; eye <= 1; ++eye)
  {
    const float cSign = (eye * 2) - 1;

    // Create the scene render camera
    PT(Camera) scene_camera_ptr = new Camera(cSceneCameraName);
    PT(MatrixLens) camera_lens_ptr = new MatrixLens();

    // Translate the projection matrix by the projection offset
    LMatrix4f projection_offset = projection * LMatrix4f::translate_mat(-cSign * cProjectionCentreOffset, 0, 0);

    // Set the lens matrix and attach it to the scene camera
    camera_lens_ptr->set_user_mat(projection_offset);
    scene_camera_ptr->set_lens(camera_lens_ptr);

    scene_camera_np_[eye] = NodePath(scene_camera_ptr);

    // Move the camera by the eye separation distance
    scene_camera_np_[eye].set_pos(cSign * cIPDOffset, 0, 0);
    scene_camera_np_[eye].reparent_to(scene_camera_root_np_);

    // Attach the camera to the scene display region
    scene_region_ptr_[eye]->set_camera(scene_camera_np_[eye]);
  }

  return true;
}

void DisplayManager::destroy_scene_cameras()
{
  for (int eye = 0; eye <= 1; ++eye)
  {
    if (!scene_camera_np_[eye].is_empty())
      // Remove the scene camera
      scene_camera_np_[eye].remove_node();
  }
}

bool DisplayManager::create_hud_cameras()
{
  assert(rift_manager_ptr_);
  assert(hud_region_ptr_[cEyeLeft]);
  assert(hud_region_ptr_[cEyeRight]);

  // Get the device values
  const float cWidthPixels = rift_manager_ptr_->get_display_width_pixels();
  const float cHeightPixels = rift_manager_ptr_->get_display_height_pixels();
  const float cHalfWidthPixels = cWidthPixels * 0.5;
  const float cWidthMetres = rift_manager_ptr_->get_display_width_metres();
  const float cLensSeparationInMetres = rift_manager_ptr_->get_lens_separation();
  const float cEyeDistanceToScreenInMetres = rift_manager_ptr_->get_eye_screen_distance();

  // Calculate distances
  const float cMetresToPixels = cWidthPixels / cWidthMetres;
  const float cLensSeparationInPixels = cLensSeparationInMetres * cMetresToPixels;
  const float cEyeSeparationInPixels = rift_manager_ptr_->get_interpupillary_distance() * cMetresToPixels;

  // Calculate the target HUD resolution
  const float cHalfScreenDistance = tan(cDefaultFOV2D * 0.5) * cEyeDistanceToScreenInMetres;
  const float cFOVMetres = (2.0 * cHalfScreenDistance) / rift_manager_ptr_->get_distortion_scale();
  const float cFOVPixels = cFOVMetres * cMetresToPixels;

  // Get the HUD aspect ratio from the window aspect_2d node
  LVecBase3f aspect_scale_vec = window_ptr_->get_aspect_2d().get_scale();

  // Calculate the HUD size as orthographic camera parameters
  const float cCameraOffsetInPixels = (cEyeDistanceToScreenInMetres / cHUDDistance) * cEyeSeparationInPixels;
  const float cCameraFilmWidth = cWidthPixels / (aspect_scale_vec.get_z() * cFOVPixels);
  const float cCameraFilmHeight = (cHeightPixels * 2.0) / (aspect_scale_vec.get_x() * cFOVPixels);

  // Calculate the orthographic camera offset from the centre
  const float cLensOffsetInPixels = cHalfWidthPixels - cLensSeparationInPixels;
  const float cOffsetInPixels = cLensOffsetInPixels + cCameraOffsetInPixels / rift_manager_ptr_->get_distortion_scale();
  const float cCameraOffset = cOffsetInPixels / cFOVPixels;

  for (int eye = 0; eye <= 1; ++eye)
  {
    const float cSign = (eye * 2) - 1;

    // Create the orthographic lens
    PT(OrthographicLens) lens_ptr = new OrthographicLens();
    lens_ptr->set_film_size(cCameraFilmWidth, cCameraFilmHeight);
    lens_ptr->set_film_offset(cCameraOffset * cSign, 0);
    lens_ptr->set_near_far(cOrthographicLensNear,
                           cOrthographicLensFar);

    // Create the camera and set the lens
    PT(Camera) camera_ptr = new Camera(cHUDCameraName);
    camera_ptr->set_lens(lens_ptr);

    // Attach the camera to the render_2d node so the aspect_2d scale is applied correctly
    hud_camera_np_[eye] = NodePath(camera_ptr);
    hud_camera_np_[eye].reparent_to(window_ptr_->get_render_2d());

    // Attach the camera to the 2D display region
    hud_region_ptr_[eye]->set_camera(hud_camera_np_[eye]);
  }

  return true;
}

void DisplayManager::destroy_hud_cameras()
{
  for (int eye = 0; eye <= 1; ++eye)
  {
    if (!hud_camera_np_[eye].is_empty())

      hud_camera_np_[eye].remove_node();
  }
}

bool DisplayManager::apply_shader()
{
  assert(!render_shader_);
  assert(!render_card_np_[cEyeLeft].is_empty());
  assert(!render_card_np_[cEyeRight].is_empty());

  string vertex_shader_file_name, fragment_shader_file_name;

  // Select the appropriate shader filenames
  //TODO(WM) Hard-code the shader source into the compiled source?
  // It avoids path issues and seems to be common practice
  switch (warp_mode_)
  {
    case cShader:
      vertex_shader_file_name = "pandrift-distortion-v.glsl";
      fragment_shader_file_name = "pandrift-distortion-f.glsl";
      break;

    case cShaderChromaticAberration:
      vertex_shader_file_name = "pandrift-distortion-v.glsl";
      fragment_shader_file_name = "pandrift-distortion-chroma-f.glsl";
      break;

    default:
      return false;
  }

  // Load and compile the shader
  render_shader_ = Shader::load(Shader::SL_GLSL,
                                vertex_shader_file_name.c_str(),
                                fragment_shader_file_name.c_str());

  if (!render_shader_)
  {
    pandrift_cat.error() << "apply_shader: Unable to load shader";
    return false;
  }

  // Calculate the shader parameters
  const float cY = 0.0, cW = 0.5, cH = 1.0;
  const float cScaleFactor = 1.0 / rift_manager_ptr_->get_distortion_scale();
  const float cAspectRatio = rift_manager_ptr_->get_display_aspect_ratio();
  LVector2f rift_scale_in_v = LVector2f((2.0 / cW), (2.0 / cH) / cAspectRatio);
  LVector2f rift_scale_v = LVector2f((cW / 2.0) * cScaleFactor, (cH / 2.0) * cScaleFactor * cAspectRatio);
  LVector4f distortion_params_v = rift_manager_ptr_->get_distortion_coefficients();
  LVector4f chroma_params_v = rift_manager_ptr_->get_chromatic_aberration_coefficients();

  float cDistortionCentreOffset = rift_manager_ptr_->get_distortion_centre_offset() * 0.5;
  for (int eye = 0; eye <= 1; ++eye)
  {
    const float cSign = (eye * 2) - 1;
    const float cX = float(eye) * 0.5;

    // Apply the same shader to both shader cards
    render_card_np_[eye].set_shader(render_shader_);

    // Apply the eye offset
    LVector2f rift_screen_centre_v = LVector2f(cX + 0.25, 0.5);
    LVector2f rift_lens_centre_v = LVector2f(cX + (cW + cDistortionCentreOffset * -cSign) * 0.5, 0.5);

    // Attach the shader paramters to the card
    render_card_np_[eye].set_shader_input("ScaleIn", rift_scale_in_v);
    render_card_np_[eye].set_shader_input("Scale", rift_scale_v);
    render_card_np_[eye].set_shader_input("ScreenCenter", rift_screen_centre_v);
    render_card_np_[eye].set_shader_input("LensCenter", rift_lens_centre_v);
    render_card_np_[eye].set_shader_input("HmdWarpParam", distortion_params_v);

    // Attach the chromatic aberration parameter, if needed
    if (cShaderChromaticAberration == warp_mode_)
      render_card_np_[eye].set_shader_input("ChromAbParam", chroma_params_v);
  }

  return true;
}

void DisplayManager::remove_shader()
{
  for (int eye = 0; eye <= 1; ++eye)
  {
    if (!render_card_np_[eye].is_empty())
      // Remove the render card
      render_card_np_[eye].clear_shader();
  }

  if (render_shader_)
  {
    // Reset the shader
    render_shader_ = NULL;
  }
}

}
