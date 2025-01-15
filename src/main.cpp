#include <iostream>
#include <ostream>
#include <qbsp/qbsp.h>

#include <raylib.h>
#include <raymath.h>
#include <GL/gl.h>

using std::cout, std::endl;

using std::string;

class App
{
public:
  App(string bspFile);
  void Run();
  void PrintInfo();
  void LoadMap();

private:
  Camera3D camera;
  std::vector<Mesh> meshes;
  Model model{0};
  qformats::qbsp::QBsp *bsp;
  float inverseScale = 32;
  Material defaultMaterial = LoadMaterialDefault();
};

App::App(string bspFile)
{
  this->bsp = new qformats::qbsp::QBsp(qformats::qbsp::QBspConfig{.loadTextures = true});
  auto res = bsp->LoadFile(bspFile.c_str());
  if (res != qformats::qbsp::QBSP_OK)
  {
    cout << "BSP ERROR: " << res << endl;
    return;
  }

  SetTraceLogLevel(LOG_ERROR);
  SetConfigFlags(FLAG_MSAA_4X_HINT);
  InitWindow(800, 600, "QBSP Viewer");
  SetTargetFPS(60);
  camera = {{2.0f, 5.0f, 2.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, 45.0f, 0};
}

void App::LoadMap()
{
  auto ws = bsp->WorldSpawn();
  for (const auto f : ws->Faces())
  {
    auto mesh = Mesh{0};
    mesh.triangleCount = f->indices.size() / 3;
    mesh.vertexCount = f->verts.size();
    mesh.vertices = (float *)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.indices = (unsigned short *)MemAlloc(f->indices.size() * sizeof(unsigned short));
    int iv = 0;
    for (int v = 0; v < f->verts.size(); v++)
    {
      mesh.vertices[iv++] = f->verts[v].point.y / inverseScale;
      mesh.vertices[iv++] = f->verts[v].point.x / inverseScale;
      mesh.vertices[iv++] = f->verts[v].point.z / inverseScale;
    }

    for (int i = 0; i < f->indices.size(); i++)
    {
      mesh.indices[i] = f->indices[i];
    }

    UploadMesh(&mesh, false);
    meshes.push_back(mesh);
  }

  defaultMaterial = LoadMaterialDefault();
  model.transform = MatrixMultiply(MatrixIdentity(), MatrixRotateX(DEG2RAD * -90));
  model.meshCount = meshes.size();
  model.meshes = (Mesh *)MemAlloc(model.meshCount * sizeof(Mesh));
  model.materials = (Material *)MemAlloc(1 * sizeof(Material));
  model.materials[0] = defaultMaterial;
  model.meshMaterial = (int *)MemAlloc(model.meshCount * sizeof(int));

  for (int m = 0; m < meshes.size(); m++)
  {
    model.meshes[m] = meshes[m];
    model.meshMaterial[m] = 0;
  }
}

void App::Run()
{
  DisableCursor();
  // glFrontFace(GL_CW);
  while (!WindowShouldClose()) // Detect window close button or ESC key
  {
    if (IsKeyPressed(KEY_M))
    {
      EnableCursor();
    }

    if (IsCursorHidden())
    {
      UpdateCamera(&camera, CAMERA_FREE);
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(camera);
    DrawModel(model, {0, 0, 0}, 1, BLUE);
    DrawModelWires(model, {0, 0, 0}, 1, RED);
    DrawGrid(64, 1.0f); // Draw a grid

    EndMode3D();
    DrawFPS(10, 10);

    EndDrawing();
  }
}

void App::PrintInfo()
{
  cout << "BSP Version:\t" << bsp->Version() << endl;
  cout << "NUM Entities:\t" << bsp->SolidEntities().size() << endl;
  cout << "NUM textures:\t" << bsp->Textures().size() << endl;
  for (const auto &tex : bsp->Textures())
  {
    cout << "   " << tex.name << " " << tex.width << "x" << tex.height << endl;
  }

  cout << "faces" << endl;
  for (auto &node : bsp->Content().nodes)
  {
    cout << "   face: " << node.face_id << " " << (node.front == -1 ? node.back : node.front) << endl;
  }

  cout << "verts" << endl;
  for (auto &vert : bsp->Content().vertices)
  {
    cout << "   verts: " << vert.x << " " << vert.y << " " << vert.z << " " << endl;
  }
}

int main()
{
  auto app = App("/home/tino/engines/ironwail-0.8.0-linux-x64/id1/maps/cube.bsp");
  app.PrintInfo();
  app.LoadMap();
  app.Run();
}
