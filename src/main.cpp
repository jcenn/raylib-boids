#include <cmath>
#include <cstdlib>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <iostream>
#include <vector>

class Boid{
public:
   Vector2 position;
   Vector2 direction;
   float speed = 75.0;
   float size = 20;
   float sigth_radius = 100;
   Boid(int screen_width, int screen_height){
      std::cout << "Initialized boid "<< std::endl;
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
   };
};
// Basic rules of Boids
//    separation: steer to avoid crowding local flockmates
//    alignment: steer towards the average heading of local flockmates
//    cohesion: steer to move towards the average position (center of mass) of local flockmates
class BoidManager{
   size_t boid_count;
   std::vector<Boid> boids;
   int screen_width, screen_height;
   const int wrap_padding = 10; // needed so boids don't "teleport" at the edges of the screen
   const float cohesion_factor = 0.2;
   const float alignment_factor = 0.8;

private:
   Vector2 GetLocalCenterOfMass(Vector2 point, float radius){
      Vector2 sum = Vector2(0, 0);
      size_t i = 0;
      for(Boid& boid : boids){
         if(Vector2Length(boid.position - point) < radius) {
            sum = Vector2Add(sum, boid.position);
            i++;
         }
      }
      if(i == 0) return sum;
      return Vector2Scale(sum, 1.0/i);
   }
   Vector2 GetLocalAlignment(Vector2 point, float radius){
      Vector2 sum = Vector2(0, 0);
      size_t i = 0;
      for(Boid& boid : boids){
         if(Vector2Length(boid.position - point) < radius) {
            sum = Vector2Add(sum, boid.direction);
            i++;
         }
      }
      if(i == 0) return sum;
      return Vector2Scale(sum, 1.0/i);
   }
   Vector2 GetLocalSeparation(Vector2 point, float radius){
      Vector2 sum = Vector2(0, 0);
      size_t i = 0;
      for(Boid& boid : boids){
         float distance = Vector2Length(boid.position - point);
         if(distance < radius && !FloatEquals(distance, 0.0)) {
            sum = Vector2Add(sum, Vector2Scale(Vector2Normalize(point - boid.position), 1.0/distance));
            i++;
         }
      }
      if(i == 0) return sum;
      return Vector2Scale(sum, 1.0/i);
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
         Vector2 cohesion_dir = Vector2Normalize(Vector2Subtract(GetLocalCenterOfMass(boid.position, boid.sigth_radius), boid.position));
         Vector2 alignment_dir = Vector2Normalize(GetLocalAlignment(boid.position, boid.sigth_radius));
         Vector2 separation_dir = Vector2Normalize(GetLocalSeparation(boid.position, boid.sigth_radius));
         
         Vector2 new_dir = boid.direction;
         new_dir = Vector2Add(new_dir, cohesion_dir);
         if(!Vector2Equals(alignment_dir, Vector2Zero()))
            new_dir = Vector2Add(new_dir, alignment_dir);
         if(!Vector2Equals(separation_dir, Vector2Zero()))
            new_dir = Vector2Add(new_dir, separation_dir);
         //new_dir = Vector2Scale(new_dir, 1.0/3.0);

         boid.direction = Vector2Normalize(Vector2Lerp(boid.direction, Vector2Normalize(new_dir), 0.05));
         
         Vector2 velocity(boid.direction.x * boid.speed * delta, boid.direction.y * boid.speed * delta);
         boid.position = Vector2Add(boid.position, velocity);
         
         boid.position.x = Wrap(boid.position.x, -wrap_padding, screen_width + wrap_padding);
         boid.position.y = Wrap(boid.position.y, -wrap_padding, screen_height + wrap_padding);
      }
   };
   
   void Draw(){
      for(Boid& boid : boids){
         //DrawCircleLinesV(boid.position, boid.sigth_radius, GREEN);
         float draw_scale = boid.size;
         Vector2 scaled_dir = Vector2Scale(boid.direction, draw_scale);
         DrawTriangle(boid.position + scaled_dir, boid.position + Vector2Rotate(scaled_dir, -PI * 0.8), boid.position + Vector2Rotate(scaled_dir, PI * 0.8), DARKGRAY);
         //DrawCircleV(boid.position, boid.size, MAROON);
      }
   }
   
   ~BoidManager() = default;
};


int main()
{
   srand(time(NULL));
   const int screenWidth = 1200;
   const int screenHeight = 900;

   InitWindow(screenWidth, screenHeight, "Raylib Boids");


   SetTargetFPS(60);               // Set our game to run at 60 frames-per-second
   //--------------------------------------------------------------------------------------
   
   BoidManager boid_manager(50, screenWidth, screenHeight);

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
      //DrawFPS(0, 0);

      boid_manager.Draw();

      EndDrawing();
        //----------------------------------------------------------------------------------
   }

   // De-Initialization
   //--------------------------------------------------------------------------------------
   CloseWindow();        // Close window and OpenGL context
   //--------------------------------------------------------------------------------------

   return 0;
}

