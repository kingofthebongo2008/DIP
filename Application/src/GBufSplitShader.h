#include "Shader.h"

class GBufSplitShader : public Shader
{
public:
  virtual void initUniforms();

  GLint origColTex;
  GLint origNormTex;
  GLint origWSCTex;
  GLint blocksX;
  GLint blocksY;
  GLint window;
};

void GBufSplitShader::initUniforms()
{
  origColTex = glGetUniformLocation(id, "origColTex");
  origNormTex = glGetUniformLocation(id, "origNormTex");
  origWSCTex = glGetUniformLocation(id, "origWSCTex");
  blocksX = glGetUniformLocation(id, "blocksX");
  blocksY = glGetUniformLocation(id, "blocksY");
  window = glGetUniformLocation(id, "window");
}
