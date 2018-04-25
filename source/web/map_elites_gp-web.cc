//  This file is part of Project Name
//  Copyright (C) Michigan State University, 2017.
//  Released under the MIT Software license; see doc/LICENSE

#include "web/web.h"
#include "../map_elites_gp.h"

namespace UI = emp::web;
MEGPConfig config;

UI::Document doc("emp_base");
UI::Canvas canvas;
UI::Div program_info;
const double world_width = 800;
const double world_height = 800;

MapElitesGPWorld world;

void DrawWorldCanvas() {
  // UI::Canvas canvas = doc.Canvas("world_canvas");
  canvas.Clear("gray");

  const size_t world_x = world.GetWidth();
  const size_t world_y = world.GetHeight();
  const double canvas_x = (double) canvas.GetWidth();
  const double canvas_y = (double) canvas.GetHeight();
  std::cout << "x: " << world_x << " y: " << world_y << std::endl;
  const double org_x = canvas_x / (double) world_x;
  const double org_y = canvas_y / (double) world_y;
  const double org_r = emp::Min(org_x, org_y) / 2.0;

  for (size_t y = 0; y < world_y; y++) {
    for (size_t x = 0; x < world_x; x++) {
      const size_t org_id = y * world_x + x;
      const size_t cur_x = org_x * (0.5 + (double) x);
      const size_t cur_y = org_y * (0.5 + (double) y);
      const double fitness = world.CalcFitnessID(org_id);
      if (fitness == 0.0) {
        canvas.Rect(x*org_x, y*org_y, org_x, org_y, "#444444", "black");
      } else if (fitness < 10) {
        canvas.Rect(x*org_x, y*org_y, org_x, org_y, "pink", "black");
      } else if (fitness < 100) {
        canvas.Rect(x*org_x, y*org_y, org_x, org_y, "#EEEE33", "black");  // Pale Yellow
      } else if (fitness < 1000) {
        canvas.Rect(x*org_x, y*org_y, org_x, org_y, "#88FF88", "black");  // Pale green
      } else if (fitness < 9000) {
        canvas.Rect(x*org_x, y*org_y, org_x, org_y, "#00CC00", "black");  // Mid green
      } else {
        canvas.Rect(x*org_x, y*org_y, org_x, org_y, "green", "black");    // Full green
      }
    }
  }

  // Add a plus sign in the middle.
  const double mid_x = org_x * world_x / 2.0;
  const double mid_y = org_y * world_y / 2.0;
  const double plus_bar = org_r * world_x;
  canvas.Line(mid_x, mid_y-plus_bar, mid_x, mid_y+plus_bar, "#8888FF");
  canvas.Line(mid_x-plus_bar, mid_y, mid_x+plus_bar, mid_y, "#8888FF");

  // doc.Text("ud_text").Redraw();
}


void CanvasClick(int x, int y) {
  program_info.Clear();

  // UI::Canvas canvas = doc.Canvas("world_canvas");
  const double canvas_x = (double) canvas.GetWidth();
  const double canvas_y = (double) canvas.GetHeight();
  double px = ((double) x) / canvas_x;
  double py = ((double) y) / canvas_y;

  const size_t world_x = world.GetWidth();
  const size_t world_y = world.GetHeight();
  size_t pos_x = (size_t) (world_x * px);
  size_t pos_y = (size_t) (world_y * py);
  // std::cout << "x: " << x << " y: " << y << "world_x: " << world_x << " world_y: " << world_y << " canvas_x: " << canvas_x <<" canvas_y: " << canvas_y  << " px: " << px <<  " py: " << py <<" pos_x: " << pos_x << " pos_y: " << pos_y <<std::endl;
  size_t org_id = pos_y * world_x + pos_x;
  std::stringstream ss;
  if (world.CalcFitnessID(org_id) > 0.0) {
    ss << "Fitness: " << world.CalcFitnessID(org_id) << "<br>";
    world[org_id].PrintGenomeHTML(ss);
    program_info << UI::Text() << ss.str();
  }

  // emp::Alert("Click at (", pos_x, ",", pos_y, ") = ", org_id);
}


int main()
{
  doc << "<h1>Evolving AvidaGP Programs with MAP-Elites</h1>";
  world.Setup(config);

  // Add some Buttons
  doc << UI::Button( [](){ emp::RandomSelect(world, 1); DrawWorldCanvas(); }, "Do Birth", "birth_button");
  doc << UI::Button( [](){ emp::RandomSelect(world, 100); DrawWorldCanvas(); }, "Do Birth 100", "birth_100_button");
  doc << UI::Button( [](){ emp::RandomSelect(world, 1000); DrawWorldCanvas(); }, "Do Birth 1000", "birth_1000_button");
  doc << UI::Button( [](){ emp::RandomSelect(world, 10000); DrawWorldCanvas(); }, "Do Birth 10000", "birth_10000_button");
  doc << "<br>";

  canvas = doc.AddCanvas(world_width, world_height, "world_canvas");
  program_info = doc.AddDiv("program_info");
  canvas.On("click", CanvasClick);
  DrawWorldCanvas();
}
