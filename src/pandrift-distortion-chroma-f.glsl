//GLSL

uniform vec2 LensCenter;
uniform vec2 ScreenCenter;
uniform vec2 Scale;
uniform vec2 ScaleIn;
uniform vec4 HmdWarpParam;
uniform vec4 ChromAbParam;
uniform sampler2D p3d_Texture0;
varying vec2 texcoord0; 

void main()
{
  vec2 theta = (texcoord0 - LensCenter) * ScaleIn;
  float rSq= theta.x * theta.x + theta.y * theta.y;
  vec2 theta1 = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq +
                         HmdWarpParam.z * rSq * rSq + HmdWarpParam.w * rSq * rSq * rSq);
 
  vec2 thetaBlue = theta1 * (ChromAbParam.z + ChromAbParam.w * rSq);
  vec2 tcBlue = LensCenter + Scale * thetaBlue;
  if (!all(equal(clamp(tcBlue, ScreenCenter-vec2(0.25, 0.5), ScreenCenter+vec2(0.25, 0.5)), tcBlue)))
  {
    gl_FragColor = vec4(0);
    return;
  }

  float blue = texture2D(p3d_Texture0, tcBlue).b;

  vec2 tcGreen = LensCenter + Scale * theta1;
  vec4 center = texture2D(p3d_Texture0, tcGreen);

  vec2 thetaRed = theta1 * (ChromAbParam.x + ChromAbParam.y * rSq);
  vec2 tcRed = LensCenter + Scale * thetaRed;
  float red = texture2D(p3d_Texture0, tcRed).r;

  gl_FragColor = vec4(red, center.g, blue, 1);
}
