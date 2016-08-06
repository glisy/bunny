#include <glfw-shell/glfw-shell.h>
#include <glisy/geometry.h>
#include <glisy/program.h>
#include <glisy/uniform.h>
#include <glisy/shader.h>
#include <glisy/camera.h>
#include <glisy/math.h>
#include <fs/fs.h>
#include <stdio.h>
#include "bunny.h"

#define WINDOW_NAME "StanfordBunny"
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 640

#define MAX_FOV 120
#define MIN_FOV 1

#define dtor(d) d * (M_PI / 180)
#define min(a, b) (a < b ? a : b < a ? b : a)
#define max(a, b) (a > b ? a : b > a ? b : a)

#define LoadShader(path) fs_read(path)
#define CreateProgram(vertex, fragment) ({   \
  GlisyProgram program;                      \
  GlisyShader vertexShader;                  \
  GlisyShader fragmentShader;                \
  glisyProgramInit(&program);                \
  glisyShaderInit(&vertexShader,             \
                  GL_VERTEX_SHADER,          \
                  LoadShader(vertex));       \
  glisyShaderInit(&fragmentShader,           \
                  GL_FRAGMENT_SHADER,        \
                  LoadShader(fragment));     \
  glisyProgramAttachShader(&program,         \
                           &vertexShader);   \
  glisyProgramAttachShader(&program,         \
                           &fragmentShader); \
  glisyProgramLink(&program);                \
  glisyProgramBind(&program);                \
  (program);                                 \
})

void
UpdateCamera(GlisyCamera *camera,
             GlisyProgram *program,
             mat4 transform,
             float fov,
             float aspect,
             float near,
             float far) {
  static mat4 projection;
  static mat4 view;
  GLint pid = program->id;

  glisyCameraUpdate(camera);
  glisyProgramBind(program);

  projection = mat4_perspective(fov, aspect, near, far);
  view = glisyCameraGetViewMatrix(camera);
  view = mat4_multiply(view, transform);
  if (glGetUniformLocation(pid, "view") > -1) {
    GlisyUniform uview; {
      glisyUniformInit(&uview, "view", GLISY_UNIFORM_MATRIX, 4);
      glisyUniformSet(&uview, &view, sizeof(mat4));
      glisyUniformBind(&uview, 0);
    }
  }

  if (glGetUniformLocation(pid, "projection") > -1) {
    GlisyUniform uprojection; {
      glisyUniformInit(&uprojection, "projection", GLISY_UNIFORM_MATRIX, 4);
      glisyUniformSet(&uprojection, &projection, sizeof(mat4));
      glisyUniformBind(&uprojection, 0);
    }
  }
}

// model
typedef struct Bunny Bunny;
struct Bunny {
  GlisyGeometry geometry;
  int faceslen;
};

static mat4 transform;
static float aspect = (float) WINDOW_WIDTH / (float) WINDOW_HEIGHT;
static float near = 1.0;
static float far = 1000.0;
static float fov = M_PI / 2.0;

static GlisyProgram program;
static GlisyCamera camera;
static GLFWwindow *window;
static Bunny bunny;

// forward decl
static void InitializeBunny(Bunny *bunny);

// bunny constructor
void
InitializeBunny(Bunny *bunny) {
  GlisyVAOAttribute position = {
    .buffer = {
      .data = (void *) StanfordBunny.positions,
      .type = GL_FLOAT,
      .size = sizeof(StanfordBunny.positions),
      .usage = GL_STATIC_DRAW,
      .offset = 0,
      .stride = 0,
      .dimension = 3,
    }
  };

  bunny->faceslen = 3 * STANFORD_BUNNY_CELLS_COUNT;

  glisyGeometryInit(&bunny->geometry);
  glisyGeometryAttr(&bunny->geometry, "position", &position);
  glisyGeometryFaces(&bunny->geometry,
                       GL_UNSIGNED_SHORT,
                       bunny->faceslen,
                       (void *) StanfordBunny.cells);

  glisyGeometryUpdate(&bunny->geometry);
}

void
DrawBunny(Bunny *bunny) {
  glisyGeometryBind(&bunny->geometry, 0);
  glisyGeometryDraw(&bunny->geometry, GL_POINTS, 0, bunny->faceslen);
  glisyGeometryUnbind(&bunny->geometry);
}

static void
onMouseScroll(GLFWwindow* window, double xoffset, double yoffset) {
  fov += yoffset * 0.05;
  fov = min(MAX_FOV, max(fov, MIN_FOV));
}

int
main(void) {

  // init gl
  GLFW_SHELL_CONTEXT_INIT(3, 2);
  GLFW_SHELL_WINDOW_INIT(window, WINDOW_WIDTH, WINDOW_HEIGHT);
  glfwSetScrollCallback(window, onMouseScroll);
  glfwSetWindowUserPointer(window, &camera);

  // init shader program
  program = CreateProgram("vertex.glsl", "fragment.glsl");

  // init object
  InitializeBunny(&bunny);

  // bind current shader program
  glisyProgramBind(&program);

  // configure camera
  glisyCameraInitialize(&camera);
  transform = mat4_identity(mat4());
  fov = M_PI / 2.0;
  camera.position = vec3(0, 5, -10);
  UpdateCamera(&camera, &program, transform, fov, aspect, near, far);

  glisyProgramBind(&program);
  GLFW_SHELL_RENDER(window, {
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    const float time = glfwGetTime();
    const float angle = time * 22.5f;
    const float radians = dtor(angle);
    const vec3 rotation = vec3(0, 1, 0);
    (void) mat4_rotate(transform, radians, rotation);

    aspect = (float) width / (float) height;
    UpdateCamera(&camera, &program, transform, fov, aspect, near, far);
    DrawBunny(&bunny);
  });

  return 0;
}
