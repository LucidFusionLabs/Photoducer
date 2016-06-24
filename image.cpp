/*
 * $Id: image.cpp 1336 2014-12-08 09:29:59Z justin $
 * Copyright (C) 2009 Lucid Fusion Labs

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "core/app/app.h"
#include "core/app/gui.h"

namespace LFL {
DEFINE_string(output, "", "Output");
DEFINE_string(input, "", "Input");
DEFINE_bool(input_3D, false, "3d input");
DEFINE_float(input_scale, 0, "Input scale");
DEFINE_string(input_prims, "", "Comma separated list of .obj primitives to highlight");
DEFINE_string(input_filter, "", "Filter type [dark2alpha]");
DEFINE_bool(visualize, false, "Display");
DEFINE_string(shader, "", "Apply this shader");
DEFINE_string(make_png_atlas, "", "Build PNG atlas with files in directory");
DEFINE_int(make_png_atlas_width, 256, "Build PNG atlas with this width");
DEFINE_int(make_png_atlas_height, 256, "Build PNG atlas with this height");
DEFINE_string(split_png_atlas, "", "Split PNG atlas back into individual files");
DEFINE_string(filter_png_atlas, "", "Filter PNG atlas");

struct MyAppState {
  Shader shader;
} *my_app;

struct MyGUI : public GUI {
  Scene scene;
  MyGUI(Window *W) : GUI(W) {}

  void DrawInput3D(GraphicsDevice *gd, Asset *a, Entity *e) {
    if (!FLAGS_input_prims.empty() && a && a->geometry) {
      vector<int> highlight_input_prims;
      Split(FLAGS_input_prims, isint2<',', ' '>, &highlight_input_prims);

      gd->DisableLighting();
      gd->Color4f(1.0, 0.0, 0.0, 1.0);
      int vpp = GraphicsDevice::VertsPerPrimitive(a->geometry->primtype);
      for (auto i = highlight_input_prims.begin(); i != highlight_input_prims.end(); ++i) {
        CHECK_LT(*i, a->geometry->count / vpp);
        scene.Draw(gd, a->geometry, e, *i * vpp, vpp);
      }
      gd->Color4f(1.0, 1.0, 1.0, 1.0);
    }
  }

  void Frame3D(LFL::Window *W, unsigned clicks, int flag) {
    Asset *a = app->asset("input");
    if (my_app->shader.ID) {
      W->gd->ActiveTexture(0);
      W->gd->BindTexture(GraphicsDevice::Texture2D, a->tex.ID);
      W->gd->UseShader(&my_app->shader);

      // mandelbox params
      float par[20][3] = { 0.25, -1.77 };
      my_app->shader.SetUniform1f("xres", W->width);
      my_app->shader.SetUniform3fv("par", sizeofarray(par), &par[0][0]);
      my_app->shader.SetUniform1f("fov_x", FLAGS_field_of_view);
      my_app->shader.SetUniform1f("fov_y", FLAGS_field_of_view);
      my_app->shader.SetUniform1f("min_dist", .000001);
      my_app->shader.SetUniform1i("max_steps", 128);
      my_app->shader.SetUniform1i("iters", 14);
      my_app->shader.SetUniform1i("color_iters", 10);
      my_app->shader.SetUniform1f("ao_eps", .0005);
      my_app->shader.SetUniform1f("ao_strength", .1);
      my_app->shader.SetUniform1f("glow_strength", .5);
      my_app->shader.SetUniform1f("dist_to_color", .2);
      my_app->shader.SetUniform1f("x_scale", 1);
      my_app->shader.SetUniform1f("x_offset", 0);
      my_app->shader.SetUniform1f("y_scale", 1);
      my_app->shader.SetUniform1f("y_offset", 0);

      v3 up = scene.cam.up, ort = scene.cam.ort, pos = scene.cam.pos;
      v3 right = v3::Cross(ort, up);
      float m[16] = { right.x, right.y, right.z, 0,
                      up.x,    up.y,    up.z,    0,
                      ort.x,   ort.y,   ort.z,   0,
                      pos.x,   pos.y,   pos.z,   0 };
      W->gd->LoadIdentity();
      W->gd->Mult(m);

      glShadertoyShaderWindows(W->gd, &my_app->shader, Color::black,
                               Box(-W->width/2, -W->height/2, W->width, W->height));
    } else {
      scene.cam.Look(W->gd);
      scene.Draw(W->gd, &app->asset.vec);
    }
  }

  void Frame2D(LFL::Window *W, unsigned clicks, int flag) {
    GraphicsContext gc(W->gd);
    Asset *a = app->asset("input");
    if (my_app->shader.ID) {
      W->gd->ActiveTexture(0);
      W->gd->BindTexture(GraphicsDevice::Texture2D, a->tex.ID);
      W->gd->UseShader(&my_app->shader);
      glShadertoyShaderWindows(W->gd, &my_app->shader, Color::black, Box(W->width, W->height));
    } else {
      screen->gd->EnableLayering();
      a->tex.Draw(&gc, W->Box());
    }
  }

  int Frame(LFL::Window *W, unsigned clicks, int flag) {
    if (FLAGS_input_3D) Frame3D(W, clicks, flag);
    screen->gd->DrawMode(DrawMode::_2D);
    if (!FLAGS_input_3D) Frame2D(W, clicks, flag);
    screen->DrawDialogs();
    return 0;
  }
};

void MyWindowInit(Window *W) {
  W->width = 420;
  W->height = 380;
  W->caption = "Image";
}

void MyWindowStart(Window *W) {
  MyGUI *gui = W->AddGUI(make_unique<MyGUI>(W));
  W->frame_cb = bind(&MyGUI::Frame, gui, _1, _2, _3);
  W->shell = make_unique<Shell>();

  BindMap *binds = W->AddInputController(make_unique<BindMap>());
  binds->Add(Key::Backquote, Bind::CB(bind([&](){ W->shell->console(vector<string>()); })));
  binds->Add(Key::Quote,     Bind::CB(bind([&](){ W->shell->console(vector<string>()); })));
  binds->Add(Key::Escape,    Bind::CB(bind(&Shell::quit,            W->shell.get(), vector<string>())));
  binds->Add(Key::Return,    Bind::CB(bind(&Shell::grabmode,        W->shell.get(), vector<string>())));
  binds->Add(Key::LeftShift, Bind::TimeCB(bind(&Entity::RollLeft,   &gui->scene.cam, _1)));
  binds->Add(Key::Space,     Bind::TimeCB(bind(&Entity::RollRight,  &gui->scene.cam, _1)));
  binds->Add('w',            Bind::TimeCB(bind(&Entity::MoveFwd,    &gui->scene.cam, _1)));
  binds->Add('s',            Bind::TimeCB(bind(&Entity::MoveRev,    &gui->scene.cam, _1)));
  binds->Add('a',            Bind::TimeCB(bind(&Entity::MoveLeft,   &gui->scene.cam, _1)));
  binds->Add('d',            Bind::TimeCB(bind(&Entity::MoveRight,  &gui->scene.cam, _1)));
  binds->Add('q',            Bind::TimeCB(bind(&Entity::MoveDown,   &gui->scene.cam, _1)));
  binds->Add('e',            Bind::TimeCB(bind(&Entity::MoveUp,     &gui->scene.cam, _1)));
}

}; // namespace LFL
using namespace LFL;

extern "C" void MyAppCreate(int argc, const char* const* argv) {
  FLAGS_near_plane = 0.1;
  FLAGS_enable_video = FLAGS_enable_input = true;
  app = new Application(argc, argv);
  screen = new Window();
  my_app = new MyAppState();
  app->window_start_cb = MyWindowStart;
  app->window_init_cb = MyWindowInit;
  app->window_init_cb(screen);
  app->exit_cb = [](){ delete my_app; };
}

extern "C" int MyAppMain() {
  if (app->Create(__FILE__)) return -1;
  if (app->Init()) return -1;

  // app->asset.Add(name,  texture,     scale, translate, rotate, geometry
  app->asset.Add("axis",   "",          0,     0,         0,      nullptr,                  nullptr, 0, 0, Asset::DrawCB(bind(&glAxis, _1, _2, _3)));
  app->asset.Add("grid",   "",          0,     0,         0,      Grid::Grid3D().release(), nullptr, 0, 0);
  app->asset.Add("input",  "",          0,     0,         0,      nullptr,                  nullptr, 0, 0);
  app->asset.Load();

  // app->soundasset.Add(name, filename,   ringbuf, channels, sample_rate, seconds);
  app->soundasset.Add("draw",  "Draw.wav", nullptr, 0,        0,           0      );
  app->soundasset.Load();

  app->scheduler.AddWaitForeverKeyboard(screen);
  app->scheduler.AddWaitForeverMouse(screen);
  app->StartNewWindow(screen);
  MyGUI *gui = screen->GetGUI<MyGUI>(0);

  if (!FLAGS_make_png_atlas.empty()) {
    FLAGS_atlas_dump=1;
    vector<string> png;
    DirectoryIter d(FLAGS_make_png_atlas, 0, 0, ".png");
    for (const char *fn = d.Next(); fn; fn = d.Next()) png.push_back(FLAGS_make_png_atlas + fn);
    AtlasFontEngine::MakeFromPNGFiles("png_atlas", png, point(FLAGS_make_png_atlas_width, FLAGS_make_png_atlas_height), NULL);
  }

  if (!FLAGS_split_png_atlas.empty()) {
    app->fonts->atlas_engine.get()->Init(FontDesc(FLAGS_split_png_atlas, "", 0, Color::black, Color::clear, 0));
    Font *font = app->fonts->Get(FLAGS_split_png_atlas, "", 0, Color::black, Color::clear, 0);
    CHECK(font);

    map<v4, int> glyph_index;
    for (auto b = font->glyph->table.begin(), e = font->glyph->table.end(), i = b; i != e; ++i) {
      CHECK(i->tex.width > 0 && i->tex.height > 0 && i->advance > 0);
      glyph_index[v4(i->tex.coord)] = i->id;
    }
    for (auto b = font->glyph->index.begin(), e = font->glyph->index.end(), i = b; i != e; ++i) {
      CHECK(i->second.tex.width > 0 && i->second.tex.height > 0 && i->second.advance > 0);
      glyph_index[v4(i->second.tex.coord)] = i->second.id;
    }

    map<int, v4> glyphs;
    for (auto i = glyph_index.begin(); i != glyph_index.end(); ++i) glyphs[i->second] = i->first;

    string outdir = StrCat(app->assetdir, FLAGS_split_png_atlas);
    LocalFile::mkdir(outdir, 0755);
    string atlas_png_fn = StrCat(app->assetdir, FLAGS_split_png_atlas, ",0,0,0,0,0.0000.png");
    AtlasFontEngine::SplitIntoPNGFiles(atlas_png_fn, glyphs, outdir + LocalFile::Slash);
  }

  if (!FLAGS_filter_png_atlas.empty()) {
    if (Font *f = AtlasFontEngine::OpenAtlas(FontDesc(FLAGS_filter_png_atlas, "", 0, Color::white))) {
      AtlasFontEngine::WriteGlyphFile(FLAGS_filter_png_atlas, f);
      INFO("filtered ", FLAGS_filter_png_atlas);
    }
  }

  if (FLAGS_input.empty()) FATAL("no input supplied");
  Asset *asset_input = app->asset("input");
  LocalFile input_file(FLAGS_input, "r");

  if (!FLAGS_shader.empty()) {
    string vertex_shader = LocalFile::FileContents(StrCat(app->assetdir, FLAGS_input_3D ? "" : "lfapp_", "vertex.glsl"));
    string fragment_shader = LocalFile::FileContents(FLAGS_shader);
    Shader::Create("my_shader", vertex_shader, fragment_shader, ShaderDefines(0,0,1,0), &my_app->shader);
  }

  if (SuffixMatch(FLAGS_input, ".obj", false)) {
    FLAGS_input_3D = true;
    asset_input->cb = bind(&MyGUI::DrawInput3D, gui, _1, _2, _3);
    asset_input->scale = FLAGS_input_scale;
    asset_input->geometry = Geometry::LoadOBJ(&input_file).release();
    gui->scene.Add(new Entity("axis",  app->asset("axis")));
    gui->scene.Add(new Entity("grid",  app->asset("grid")));
    gui->scene.Add(new Entity("input", asset_input));

    if (!FLAGS_output.empty()) {
      set<int> filter_prims;
      bool filter_invert = FLAGS_input_filter == "notprims", filter = false;
      if (filter_invert || FLAGS_input_filter == "prims")    filter = true;
      if (filter) Split(FLAGS_input_prims, isint2<',', ' '>, &filter_prims);
      string out = Geometry::ExportOBJ(asset_input->geometry, filter ? &filter_prims : 0, filter_invert);
      int ret = LocalFile::WriteFile(FLAGS_output, out);
      INFO("write ", FLAGS_output, " = ", ret);
    }
  } else {
    FLAGS_draw_grid = true;
    Texture pb;
    app->asset_loader->default_video_loader->LoadVideo
      (app->asset_loader->default_video_loader->LoadVideoFile(&input_file), &asset_input->tex, false);
    pb.AssignBuffer(&asset_input->tex, true);

    if (pb.width && pb.height) screen->Reshape(FLAGS_input_scale ? pb.width *FLAGS_input_scale : pb.width,
                                               FLAGS_input_scale ? pb.height*FLAGS_input_scale : pb.height);
    INFO("input dim = (", pb.width, ", ", pb.height, ") pf=", pb.pf);

    if (FLAGS_input_filter == "dark2alpha") {
      for (int i=0; i<pb.height; i++)
        for (int j=0; j<pb.width; j++) {
          int ind = (i*pb.width + j) * Pixel::Size(pb.pf);
          unsigned char *b = pb.buf + ind;
          float dark = b[0]; // + b[1] + b[2] / 3.0;
          // if (dark < 256*1/3.0) b[3] = 0;
          b[3] = dark;
        }
    }

    if (!FLAGS_output.empty()) {
      int ret = PngWriter::Write(FLAGS_output, pb);
      INFO("write ", FLAGS_output, " = ", ret);
    }
  }

  if (!FLAGS_visualize) return 0;

  // start our engine
  return app->Main();
}
