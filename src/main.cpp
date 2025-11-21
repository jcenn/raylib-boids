#include <cmath>
#include <cstdlib>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

// Customizable Parameters
const int screen_width = 200;
const int screen_height = 200;

const float color_saturation = 0.8f;
const float color_value = 0.7f;

// higher number - smoother boid steering
const float smoothing = 0.02f;

// Boid parameters
float boid_size = 20;
const float boid_speed = 85;
const float sight_radius = 100;
const float separation_radius = 50;
const float cone_of_vision = PI * 0.8;

const int wrap_padding = 10; // needed so boids don't "teleport" at the edges of the screen
float cohesion_factor = 0.7f;
float alignment_factor = 0.8f;
float separation_factor = 1.4f;

Color random_color(){
   const float hue = static_cast<float>(rand()%360);
   return ColorFromHSV(hue, color_saturation, color_value);
}

// Frame rate independent lerp
// https://www.rorydriscoll.com/2016/03/07/frame-rate-independent-damping-using-lerp/
float Damp(float source, float target, float delta)
{
    return lerp(source, target, 1 - powf(smoothing, delta));
}

class Boid{
public:
   Vector2 position;
   Vector2 direction;
   
   //float speed = 85.0;
   //float size = 20;
   
   Color baseColor;
   Color current_color;

   //float sigth_radius = 120;

   Boid(int screen_width, int screen_height){
      // std::cout << "Initialized boid "<< std::endl;
      float randf = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
      float angle = 2 *3.14 * randf;
      direction = Vector2(
         cos(angle),
         sin(angle)
      );
      randf = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
      float x = randf * screen_width;
      randf = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
      float y = randf * screen_height;
      position = Vector2(x, y);

      baseColor = random_color();
      current_color = baseColor;
   };
   bool CheckVisibility(Vector2 point){
      float dot_mapped = (-Vector2DotProduct(Vector2Normalize(point - this->position), this->direction) + 1.0) / 2.0; // [0, 1] 0 - both vectors face the same direction, 1 - opposite directions
      return dot_mapped < cone_of_vision / PI; 
   }
};
// Basic rules of Boids
//    separation: steer to avoid crowding local flockmates
//    alignment: steer towards the average heading of local flockmates
//    cohesion: steer to move towards the average position (center of mass) of local flockmates
class BoidManager{
   size_t boid_count;
   std::vector<Boid> boids;
   int screen_width, screen_height;

private:
   Vector2 GetLocalCenterOfMass(Boid& main_boid){
      Vector2 sum = Vector2(0, 0);
      size_t i = 0;
      for(Boid& boid : boids){
         if(!main_boid.CheckVisibility(boid.position)) continue;
         if(Vector2Length(boid.position - main_boid.position) < sight_radius) {
            sum = Vector2Add(sum, boid.position);
            i++;
         }
      }
      if(i == 0) return sum;
      return Vector2Scale(sum, 1.0/i);
   }
   Vector2 GetLocalAlignment(Boid& main_boid){
      Vector2 sum = Vector2(0, 0);
      size_t i = 0;
      for(Boid& boid : boids){
         if(!main_boid.CheckVisibility(boid.position)) continue;
         if(Vector2Length(boid.position - main_boid.position) < sight_radius) {
            sum = Vector2Add(sum, boid.direction);
            i++;
         }
      }
      if(i == 0) return sum;
      return Vector2Scale(sum, 1.0/i);
   }
   Vector2 GetLocalSeparation(Boid& main_boid){
      Vector2 sum = Vector2(0, 0);
      size_t i = 0;
      for(Boid& boid : boids){
         if(!main_boid.CheckVisibility(boid.position)) continue;
         float distance = Vector2Length(boid.position - main_boid.position);
         if(distance < separation_radius && !FloatEquals(distance, 0.0)) {
            sum = Vector2Add(sum, Vector2Scale(Vector2Normalize(main_boid.position - boid.position), pow((1 - distance/separation_radius), 2)));
            i++;
         }
      }
      if(i == 0) return sum;
      return Vector2Scale(sum, 1.0/i);
   }
   Color GetLocalColor(Boid& main_boid){
      Vector3 hsv = ColorToHSV(main_boid.baseColor);
      Vector2 hue_vector = Vector2(cos(hsv.x / 180 * PI), sin(hsv.x / 180 * PI));
      size_t i = 1;
      for(Boid& boid : boids){
         //if(!main_boid.CheckVisibility(boid.position)) continue;
         if(Vector2Length(boid.position - main_boid.position) < sight_radius) {
            float boid_hue = ColorToHSV(boid.current_color).x;
            Vector2 vec = Vector2(cos(boid_hue / 180 * PI), sin(boid_hue / 180 * PI));
            hue_vector += vec;
            i++;
         }
      }
      //hue_vector = Vector2Scale(hue_vector, 1.0f/i);
      hsv.x = atan2(hue_vector.y, hue_vector.x) * 180 / PI;
      return ColorFromHSV(hsv.x, hsv.y, hsv.z);
   }
public: 
   BoidManager(int size, int screen_width, int screen_height) : boid_count(size), screen_width(screen_width), screen_height(screen_height){
      std::cout << "Initializing " << size << " boids." << std::endl;

      boids = std::vector<Boid>();
      for(size_t i = 0; i < boid_count; i++){
         boids.emplace_back(screen_width, screen_height);
      }
   }
   void Update(float delta){
      for(Boid& boid : boids){
         Vector2 cohesion_dir = Vector2Normalize(Vector2Subtract(GetLocalCenterOfMass(boid), boid.position));
         Vector2 alignment_dir = Vector2Normalize(GetLocalAlignment(boid));
         Vector2 separation_dir = Vector2Normalize(GetLocalSeparation(boid));
         
         Vector2 new_dir = boid.direction;
         new_dir = Vector2Add(new_dir, Vector2Scale(cohesion_dir, cohesion_factor));
         if(!Vector2Equals(alignment_dir, Vector2Zero()))
             new_dir = Vector2Add(new_dir, Vector2Scale(alignment_dir, alignment_factor));
         if(!Vector2Equals(separation_dir, Vector2Zero()))
            new_dir = Vector2Add(new_dir, Vector2Scale(separation_dir, separation_factor));
         //new_dir = Vector2Scale(new_dir, 1.0/3.0);

         Vector2 dir_norm = Vector2Normalize(new_dir);
         //boid.direction = Vector2Normalize(Vector2Lerp(boid.direction, , 0.04));
      
         Vector2 lerp_vec = {
            Damp(boid.direction.x, dir_norm.x, delta),
            Damp(boid.direction.y, dir_norm.y, delta),
         };

         boid.direction = Vector2Normalize(lerp_vec);
         Vector2 velocity(boid.direction.x * boid_speed * delta, boid.direction.y * boid_speed * delta);
         boid.position = Vector2Add(boid.position, velocity);
         
         boid.position.x = Wrap(boid.position.x, -wrap_padding, screen_width + wrap_padding);
         boid.position.y = Wrap(boid.position.y, -wrap_padding, screen_height + wrap_padding);
      }
   };
   
   void Draw(){
      for(Boid& boid : boids){
         // DrawCircleLinesV(boid.position, sight_radius, GREEN);
         // DrawCircleLinesV(boid.position, separation_radius, RED);
         float draw_scale = boid_size;
         Vector2 scaled_dir = Vector2Scale(boid.direction, draw_scale);
         boid.current_color = GetLocalColor(boid);
         DrawTriangle(boid.position + scaled_dir, boid.position + Vector2Rotate(scaled_dir, -PI * 0.8), boid.position + Vector2Rotate(scaled_dir, PI * 0.8), boid.current_color);
         //DrawCircleV(boid.position, boid.size, MAROON);
      }
   }
   
   ~BoidManager() = default;
};
void draw_ui(){
   // int GuiSlider(Rectangle bounds, const char *textLeft, const char *textRight, float *value, float minValue, float maxValue)
   std::string size_text = "size: " + std::to_string((int)std::round(boid_size));
   GuiSlider(Rectangle(0, 0, 200, 50), "", size_text.c_str(), &boid_size, 10.0, 50.0);
   
   std::string cohesion_text = "cohesion: " + std::to_string(cohesion_factor);
   GuiSlider(Rectangle(0, 50, 200, 50), "", cohesion_text.c_str(), &cohesion_factor, 0.0f, 1.0f);
   std::string alignment_text = "alignment: " + std::to_string(alignment_factor);
   GuiSlider(Rectangle(0, 100, 200, 50), "", alignment_text.c_str(), &alignment_factor, 0.0f, 1.0f);
   std::string separation_text = "separation: " + std::to_string(separation_factor);
   GuiSlider(Rectangle(0, 150, 200, 50), "", separation_text.c_str(), &separation_factor, 0.0f, 1.0f);
   return;
}

int main()
{
   srand(time(NULL));
   const int screenWidth = 1200;
   const int screenHeight = 900;

   InitWindow(screenWidth, screenHeight, "Raylib Boids");


   SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
   //--------------------------------------------------------------------------------------
   
   BoidManager boid_manager(60, screenWidth, screenHeight);

    // Main game loop
   while (!WindowShouldClose())    // Detect window close button or ESC key
   {
     // Update
     //----------------------------------------------------------------------------------
        
      boid_manager.Update(GetFrameTime());

     //----------------------------------------------------------------------------------

     // Draw
     //----------------------------------------------------------------------------------
      BeginDrawing();

      ClearBackground(RAYWHITE);


      DrawFPS(0, 0);

      boid_manager.Draw();

      draw_ui();
      EndDrawing();
        //----------------------------------------------------------------------------------
   }

   // De-Initialization
   //--------------------------------------------------------------------------------------
   CloseWindow();        // Close window and OpenGL context
   //--------------------------------------------------------------------------------------

   return 0;
}
