#include "sdl_wrapper.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

const char WINDOW_TITLE[] = "CS 3";
const int WINDOW_WIDTH = 1000;
const int WINDOW_HEIGHT = 500;
const double MS_PER_S = 1e3;
const int SCORE1_X = 200;
const int SCORE1_Y = 10;
const int SCORE2_X = 750;
const int SCORE2_Y = 10;
const int SCORE_W = 50; 
const int SCORE_H = 100; 

/**
 * The coordinate at the center of the screen.
 */
vector_t center;
/**
 * The coordinate difference from the center to the top right corner.
 */
vector_t max_diff;
/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;
/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;
/**
 * The keypress handler, or NULL if none has been configured.
 */
key_handler_t key_handler = NULL;
/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

/** Computes the center of the window in pixel coordinates */
vector_t get_window_center(void) {
  int *width = malloc(sizeof(*width)), *height = malloc(sizeof(*height));
  assert(width != NULL);
  assert(height != NULL);
  SDL_GetWindowSize(window, width, height);
  vector_t dimensions = {.x = *width, .y = *height};
  free(width);
  free(height);
  return vec_multiply(0.5, dimensions);
}

/**
 * Computes the scaling factor between scene coordinates and pixel coordinates.
 * The scene is scaled by the same factor in the x and y dimensions,
 * chosen to maximize the size of the scene while keeping it in the window.
 */
double get_scene_scale(vector_t window_center) {
  // Scale scene so it fits entirely in the window
  double x_scale = window_center.x / max_diff.x,
         y_scale = window_center.y / max_diff.y;
  return x_scale < y_scale ? x_scale : y_scale;
}

/** Maps a scene coordinate to a window coordinate */
vector_t get_window_position(vector_t scene_pos, vector_t window_center) {
  // Scale scene coordinates by the scaling factor
  // and map the center of the scene to the center of the window
  vector_t scene_center_offset = vec_subtract(scene_pos, center);
  double scale = get_scene_scale(window_center);
  vector_t pixel_center_offset = vec_multiply(scale, scene_center_offset);
  vector_t pixel = {.x = round(window_center.x + pixel_center_offset.x),
                    // Flip y axis since positive y is down on the screen
                    .y = round(window_center.y - pixel_center_offset.y)};
  return pixel;
}

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
  switch (key) {
  case SDLK_LEFT:
    return LEFT_ARROW;
  case SDLK_UP:
    return UP_ARROW;
  case SDLK_RIGHT:
    return RIGHT_ARROW;
  case SDLK_DOWN:
    return DOWN_ARROW;
  case SDLK_SPACE:
    return SPACE_BAR;
  case SDLK_w:
    return W_KEY;
  case SDLK_a:
    return A_KEY;
  case SDLK_s:
    return S_KEY;
  case SDLK_d:
    return D_KEY;
  case SDLK_p:
    return P_KEY;
  case SDLK_m:
    return M_KEY;
  case SDLK_g:
    return G_KEY;
  case SDLK_i:
    return I_KEY;
  default:
    // Only process 7-bit ASCII characters
    return key == (SDL_Keycode)(char)key ? key : '\0';
  }
}

void sdl_init(vector_t min, vector_t max) {
  // Check parameters
  assert(min.x < max.x);
  assert(min.y < max.y);

  center = vec_multiply(0.5, vec_add(min, max));
  max_diff = vec_subtract(max, center);
  SDL_Init(SDL_INIT_EVERYTHING);
  int imgFlags = IMG_INIT_JPG|IMG_INIT_PNG;
  IMG_Init(imgFlags);
  int ttfinit = TTF_Init();
  if(ttfinit != 0) {
    printf("TTF_INIT ERROR \n"); 
  }
  int mixFlags = MIX_INIT_MP3; 
  Mix_Init(mixFlags);
  if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
                {
                    printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
                }
  window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
                            SDL_WINDOW_RESIZABLE);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
}

bool sdl_is_done(void *state) {
  SDL_Event *event = malloc(sizeof(*event));
  assert(event != NULL);
  while (SDL_PollEvent(event)) {
    switch (event->type) {
    case SDL_QUIT:
      IMG_Quit(); 
      TTF_Quit();
      Mix_Quit();
      free(event);
      return true;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      // Skip the keypress if no handler is configured
      // or an unrecognized key was pressed
      if (key_handler == NULL)
        break;
      char key = get_keycode(event->key.keysym.sym);
      if (key == '\0')
        break;

      uint32_t timestamp = event->key.timestamp;
      if (!event->key.repeat) {
        key_start_timestamp = timestamp;
      }
      key_event_type_t type =
          event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
      double held_time = (timestamp - key_start_timestamp) / MS_PER_S;
      key_handler((state_t *)state, key, type, held_time);
      break;
    }
  }
  free(event);
  return false;
}

void sdl_clear(void) {
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderClear(renderer);
}

void sdl_draw_polygon(list_t *points, rgb_color_t color) {
  // Check parameters
  size_t n = list_size(points);
  assert(n >= 3);
  assert(0 <= color.r && color.r <= 1);
  assert(0 <= color.g && color.g <= 1);
  assert(0 <= color.b && color.b <= 1);

  vector_t window_center = get_window_center();

  // Convert each vertex to a point on screen
  int16_t *x_points = malloc(sizeof(*x_points) * n),
          *y_points = malloc(sizeof(*y_points) * n);
  assert(x_points != NULL);
  assert(y_points != NULL);
  for (size_t i = 0; i < n; i++) {
    vector_t *vertex = list_get(points, i);
    vector_t pixel = get_window_position(*vertex, window_center);
    x_points[i] = pixel.x;
    y_points[i] = pixel.y;
  }

  // Draw polygon with the given color
  filledPolygonRGBA(renderer, x_points, y_points, n, color.r * 255,
                    color.g * 255, color.b * 255, 255);
  free(x_points);
  free(y_points);
}

void sdl_show(void) {
  // Draw boundary lines
  vector_t window_center = get_window_center();
  vector_t max = vec_add(center, max_diff),
           min = vec_subtract(center, max_diff);
  vector_t max_pixel = get_window_position(max, window_center),
           min_pixel = get_window_position(min, window_center);
  SDL_Rect *boundary = malloc(sizeof(*boundary));
  boundary->x = min_pixel.x;
  boundary->y = max_pixel.y;
  boundary->w = max_pixel.x - min_pixel.x;
  boundary->h = min_pixel.y - max_pixel.y;
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  SDL_RenderDrawRect(renderer, boundary);
  free(boundary);

  SDL_RenderPresent(renderer);
}

void sdl_render_scene_pacman(scene_t *scene, body_t *pacbody) {
  sdl_clear();
  size_t body_count = scene_bodies(scene);
  sdl_draw_polygon(body_get_shape(pacbody), body_get_color(pacbody));
  for (size_t i = 0; i < body_count; i++) {
    body_t *body = scene_get_body(scene, i);
    list_t *shape = body_get_shape(body);
    sdl_draw_polygon(shape, body_get_color(body));
    list_free(shape);
  }
  sdl_show();
}

void sdl_render_scene(scene_t *scene) {
  sdl_clear();
  size_t body_count = scene_bodies(scene);
  for (size_t i = 0; i < body_count; i++) {
    body_t *body = scene_get_body(scene, i);
    list_t *shape = body_get_shape(body);
    sdl_draw_polygon(shape, body_get_color(body));
    list_free(shape);
  }
  sdl_show();
}

void sdl_on_key(key_handler_t handler) { key_handler = handler; }

double time_since_last_tick(void) {
  clock_t now = clock();
  double difference = last_clock
                          ? (double)(now - last_clock) / CLOCKS_PER_SEC
                          : 0.0; // return 0 the first time this is called
  last_clock = now;
  return difference;
}

void render_texture(SDL_Texture *texture, int x, int y, int w, int h) {
  SDL_Rect rect;
  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;
  int sdlrc = SDL_RenderCopy(renderer, texture, NULL, &rect);
  if (sdlrc != 0) {
    printf("RENDER COPY FAILED \n"); 
  }
}

void sdl_render_text(char *string, TTF_Font *font, rgb_color_t color, vector_t position) {
  SDL_Color textColor = {color.r * 255.0, color.g * 255.0, color.b * 255.0};
  SDL_Surface *textSurface = TTF_RenderText_Solid(font, string, textColor); 
  SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
  int text_width = textSurface->w;
  int text_height = textSurface->h;
  SDL_Rect text;
  text.x = position.x * get_scene_scale(get_window_center());
  text.y = position.y * get_scene_scale(get_window_center());
  text.w = text_width;
  text.h = text_height;
  SDL_RenderCopy(renderer, textTexture, NULL, &text);
  SDL_RenderPresent(renderer); 
  SDL_DestroyTexture(textTexture);
  SDL_FreeSurface(textSurface);
}

void sdl_make_table(SDL_Surface *image, vector_t position, int w, int h) {
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, image);
  SDL_Rect rect;
  rect.x = position.x * get_scene_scale(get_window_center());
  rect.y = position.y * get_scene_scale(get_window_center());
  rect.w = w * get_scene_scale(get_window_center());
  rect.h = h * get_scene_scale(get_window_center()); 
  SDL_RenderCopy(renderer, texture, NULL, &rect);
  SDL_RenderPresent(renderer);
  SDL_DestroyTexture(texture); 
}

void sdl_make_sprite(SDL_Surface *image, body_t *body, double radius) {
  SDL_Texture *image_texture = SDL_CreateTextureFromSurface(renderer, image);
  vector_t center = body_get_centroid(body); 
  vector_t position = get_window_position(center, get_window_center()); 
  SDL_Rect rect;
  rect.x = position.x - radius * get_scene_scale(get_window_center());
  rect.y = position.y - radius * get_scene_scale(get_window_center());
  rect.w = (radius * 2) * get_scene_scale(get_window_center());
  rect.h = (radius * 2) * get_scene_scale(get_window_center());
  SDL_RenderCopy(renderer, image_texture, NULL, &rect);
  SDL_RenderPresent(renderer); 
  SDL_DestroyTexture(image_texture);
}

void render_scoreboard(SDL_Surface *score1, SDL_Surface *score2) {
  SDL_Texture *texture1 = SDL_CreateTextureFromSurface(renderer, score1); 
  SDL_Texture *texture2 = SDL_CreateTextureFromSurface(renderer, score2); 
  SDL_Rect rect1;
  rect1.x = SCORE1_X;
  rect1.y = SCORE1_Y;
  rect1.w = SCORE_W;
  rect1.h = SCORE_H;
  SDL_Rect rect2;
  rect2.x = SCORE2_X;
  rect2.y = SCORE2_Y;
  rect2.w = SCORE_W;
  rect2.h = SCORE_H;
  SDL_RenderCopy(renderer, texture1, NULL, &rect1);
  SDL_RenderCopy(renderer, texture2, NULL, &rect2);
  SDL_RenderPresent(renderer); 
  SDL_DestroyTexture(texture1);
  SDL_DestroyTexture(texture2);
}

void render_background(SDL_Surface *background) {
  SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, background); 
  SDL_Rect rect;
  rect.x = 0;
  rect.y = 0;
  rect.w = WINDOW_WIDTH;
  rect.h = WINDOW_HEIGHT;
  SDL_RenderCopy(renderer, texture, NULL, &rect);
  SDL_RenderPresent(renderer);
  SDL_DestroyTexture(texture); 
}
