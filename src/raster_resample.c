#include <stdlib.h>
#include "raster_resample.h"
#include "raster_resample_port.h"

// Our resampling routine for rasters is just a bilinear downsample at heart.
// We pass in a nearest neighbour sampled image that is twice the size,
// and then we bilinearly downsample the image using OpenGL.
// At low zoom levels this will produce blocky images, but it is fast,
// simple and produces fine enough results.
// I'm fully open to other ideas on how to do this.
// We also have to use OpenGL 2.1 b/c ubuntu packages are so silly old.
int
simplet_resample(uint32_t **resampled, uint32_t *data, uint16_t width, uint16_t height){
  void *ctx = simplet_grab_gl_context(width, height);
  if(ctx == NULL) return -1;

  GLuint framebuffer;
  glGenFramebuffers(1, &framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

  GLuint color;
  glGenRenderbuffers(1, &color);
  glBindRenderbuffer(GL_RENDERBUFFER, color);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color);

  GLuint depth;
  glGenRenderbuffers(1, &depth);
  glBindRenderbuffer(GL_RENDERBUFFER, depth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth);

  GLfloat vertices[] = {
    -1.0,  1.0,
     1.0,  1.0,
     1.0, -1.0,
    -1.0, -1.0
  };

  GLuint vbuf;
  glGenBuffers(1, &vbuf);
  glBindBuffer(GL_ARRAY_BUFFER, vbuf);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  GLuint triangles[] = {
    0, 1, 2,
    2, 3, 0
  };

  GLuint ebuf;
  glGenBuffers(1, &ebuf);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebuf);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangles), triangles, GL_STATIC_DRAW);

  const GLchar *vertex =
    "#version 120\n"
    "attribute vec2 position;"
    "varying vec2 coord;"
    "void main(){"
    "  coord = position;"
    "  gl_Position = vec4(position, 0.0, 1.0);"
    "}"
  ;

  GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex, NULL);
  glCompileShader(vertex_shader);

  const GLchar *fragment =
    "#version 120\n"
    "uniform sampler2D tex;"
    "varying vec2 coord;"
    "void main(){"
    "  gl_FragColor = texture2D(tex, vec2((coord.x + 1.0) / 2.0, (coord.y + 1.0) / 2.0));"
    "}"
  ;

  GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment, NULL);
  glCompileShader(fragment_shader);

  GLint program = glCreateProgram();
  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glBindFragDataLocation(program, 0, "color");
  glLinkProgram(program);
  glUseProgram(program);

  if(glGetError()) puts("poo");

  int infologLength = 0;
  int charsWritten  = 0;

  glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &infologLength);

  if (infologLength > 0)
  {
      GLchar* infoLog = (GLchar *)malloc(infologLength);
      if (infoLog == NULL)
      {
          printf( "ERROR: Could not allocate InfoLog buffer");
          exit(1);
      }
      glGetShaderInfoLog(fragment_shader, infologLength, &charsWritten, infoLog);
      printf( "Shader InfoLog:\n%s", infoLog );
      free(infoLog);
  }


  GLint pos = glGetAttribLocation(program, "position");
  glEnableVertexAttribArray(pos);
  glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 0, 0);

  GLuint tex;
  glGenTextures(1, &tex);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glUniform1i(glGetUniformLocation(program, "tex"), 0);

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glViewport(0, 0, width/2, height/2);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  width /= 2; height /= 2;
  uint32_t *out = malloc(sizeof(uint32_t) * width * height);
  glReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, out);

  *resampled = out;

  glDeleteTextures(1, &tex);
  glDeleteProgram(program);
  glDeleteShader(fragment_shader);
  glDeleteShader(vertex_shader);
  glDeleteBuffers(1, &ebuf);
  glDeleteBuffers(1, &vbuf);
  glDeleteBuffers(1, &color);
  glDeleteBuffers(1, &depth);
  glDeleteBuffers(1, &framebuffer);
  glDeleteTextures(1, &tex);

  simplet_destroy_gl_context(ctx);
  return 0;
}


