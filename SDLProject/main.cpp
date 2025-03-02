/**
* Author: Si Yue Jiang
* Assignment: Pong Clone
* Date due: 2025-03-01, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>

enum AppStatus { RUNNING, TERMINATED };

constexpr float WINDOW_SIZE_MULT = 1.0f;

constexpr int WINDOW_WIDTH  = 640 * WINDOW_SIZE_MULT,
              WINDOW_HEIGHT = 480 * WINDOW_SIZE_MULT;

constexpr float BG_RED     = 0.9765625f,
                BG_GREEN   = 0.97265625f,
                BG_BLUE    = 0.9609375f,
                BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr float MII_HEIGHT = 3.2f,
                MII_WIDTH = 0.5f,
                BALL_RADIUS = 1.0f;

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

constexpr char P1_SPRITE_FILEPATH[] = "mii-1.png",
               P2_SPRITE_FILEPATH[]  = "mii-2.png",
               BACKGROUND_SPRITE_FILEPATH[] = "tennis-court.jpg",
               BALL_SPRITE_FILEPATH[] = "tennis-ball.png",
               P1_WINS_FILEPATH[] = "player-one-wins.png",
               P2_WINS_FILEPATH[] = "player-two-wins.png",
               GAME_START_FILEPATH[] = "game-screen.png";

constexpr float MINIMUM_COLLISION_DISTANCE = 1.0f;
constexpr glm::vec3 INIT_SCALE_P1 = glm::vec3(1.9f, 2.0f, 0.0f),
                    INIT_SCALE_P2 = glm::vec3(1.9f, 2.0f, 0.0f),
                    INIT_SCALE_BACKGROUND = glm::vec3(10.0f, 7.5f, 0.0f),
                    INIT_SCALE_BALL = glm::vec3(1.0f, 1.0f, 0.0f),
                    INIT_SCALE_WIN = glm::vec3(10.0f, 7.5f, 0.0f),

                    INIT_POS_P1 = glm::vec3(3.7f, 0.0f, 0.0f),
                    INIT_POS_P2 = glm::vec3(-3.7f, 0.0f, 0.0f),
                    INIT_POS_BACKGROUND = glm::vec3(0.0f, 0.0f, 0.0f),
                    INIT_POS_BALL = glm::vec3(0.0f, 0.0f, 0.0f);

// PLAYER TWO AUTO-PLAY
bool playing = true;
bool direction = true; // up = true, left = false

// BALL MOVEMENT
int ball_num = 1;

bool ball_direction_h = true; // up = true, down = false
bool ball_direction_w = true; // right = true, left = false

bool ball2_direction_h = true; // up = true, down = false
bool ball2_direction_w = false; // right = true, left = false

bool ball3_direction_h = false; // up = true, down = false
bool ball3_direction_w = true; // right = true, left = false


bool game_start = false;
bool game_over = false;
bool winner = true;

SDL_Window* g_display_window;

AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();
glm::mat4 g_view_matrix, g_p1_matrix, g_projection_matrix, g_p2_matrix, g_background_matrix, g_ball_matrix, g_ball2_matrix, g_ball3_matrix,
          g_p1_wins_matrix, g_p2_wins_matrix, g_game_start_matrix;

float g_previous_ticks = 0.0f;

GLuint g_p1_texture_id;
GLuint g_p2_texture_id;
GLuint g_background_texture_id;
GLuint g_ball_texture_id;
GLuint g_p1_wins_texture_id;
GLuint g_p2_wins_texture_id;
GLuint g_game_screen_texture_id;

glm::vec3 g_p1_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_p1_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_p2_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_p2_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_ball_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_ball_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_ball2_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_ball2_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_ball3_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_ball3_movement = glm::vec3(0.0f, 0.0f, 0.0f);


float g_p1_speed = 4.0f;  // move 4 units per second
float g_p2_speed = 4.0f;
float g_ball_speed = 3.0f;
float g_ball2_speed = 3.0f;
float g_ball3_speed = 3.0f;


void initialise();
void process_input();
void update();
void render();
void shutdown();

constexpr GLint NUMBER_OF_TEXTURES = 1;  // to be generated, that is
constexpr GLint LEVEL_OF_DETAIL    = 0;  // base image level; Level n is the nth mipmap reduction image
constexpr GLint TEXTURE_BORDER     = 0;  // this value MUST be zero

GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }
    
    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);
    
    return textureID;
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Wii Sports - Tennis",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
    
    if (g_display_window == nullptr)
    {
        shutdown();
    }
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_p1_matrix = glm::mat4(1.0f);
    g_p2_matrix = glm::mat4(1.0f);
    g_background_matrix = glm::mat4(1.0f);
    g_ball_matrix = glm::mat4(1.0f);
    g_ball2_matrix = glm::mat4(1.0f);
    g_ball3_matrix = glm::mat4(1.0f);
    g_p1_wins_matrix = glm::mat4(1.0f);
    g_p2_wins_matrix = glm::mat4(1.0f);
    g_game_start_matrix = glm::mat4(1.0f);
    
    g_background_matrix = glm::scale(g_background_matrix, INIT_SCALE_BACKGROUND);
    g_p1_wins_matrix = glm::scale(g_p1_wins_matrix, INIT_SCALE_WIN);
    g_p2_wins_matrix = glm::scale(g_p2_wins_matrix, INIT_SCALE_WIN);
    g_game_start_matrix = glm::scale(g_game_start_matrix, INIT_SCALE_WIN);
        
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);
    
    glUseProgram(g_shader_program.get_program_id());
    
    glClearColor(BG_RED, BG_GREEN, BG_BLUE, BG_OPACITY);
    
    g_p1_texture_id = load_texture(P1_SPRITE_FILEPATH);
    g_p2_texture_id = load_texture(P2_SPRITE_FILEPATH);
    g_background_texture_id = load_texture(BACKGROUND_SPRITE_FILEPATH);
    g_ball_texture_id = load_texture(BALL_SPRITE_FILEPATH);
    g_p1_wins_texture_id = load_texture(P1_WINS_FILEPATH);
    g_p2_wins_texture_id = load_texture(P2_WINS_FILEPATH);
    g_game_screen_texture_id = load_texture(GAME_START_FILEPATH);
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    g_p1_movement = glm::vec3(0.0f);
    g_p2_movement = glm::vec3(0.0f);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            // End game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_1:
                        ball_num = 1;
                        break;
                        
                    case SDLK_2:
                        ball_num = 2;
                        g_ball2_position = INIT_POS_BALL;
                        break;
                    
                    case SDLK_3:
                        ball_num = 3;
                        g_ball3_position = INIT_POS_BALL;
                        break;
                        
                    case SDLK_t:
                        playing = not playing;
                        break;
                        
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_app_status = TERMINATED;
                        break;
                                
                    case SDLK_SPACE:
                        if (not game_start){
                            game_start = true;
                        }
                        
                        if (game_over){
                            g_ball_position = INIT_POS_BALL;
                            g_ball2_position = INIT_POS_BALL;
                            g_ball3_position = INIT_POS_BALL;
                            
                            g_p1_position = INIT_POS_BALL;
                            g_p2_position = INIT_POS_BALL;

//                            g_p1_matrix = glm::mat4(1.0f);
//                            g_p1_matrix = glm::translate(g_p1_matrix, INIT_POS_P1);
//                            
//                            g_p2_matrix = glm::mat4(1.0f);
//                            g_p2_matrix = glm::translate(g_p2_matrix, INIT_POS_P2);
                                                        
                            g_ball_speed = 3.0f;
                            g_ball2_speed = 3.0f;
                            g_ball3_speed = 3.0f;
                            
                            game_over = false;
                        }
                    default:
                        break;
                }
                                                                             
            default:
                break;
        }
    }
                                                                             
                                                                             
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
                                    
    // PLAYER ONE MOVEMENT
    if (key_state[SDL_SCANCODE_UP])
    {
        if (g_p1_position.y + MII_HEIGHT < 6.0f)
        {
            g_p1_movement.y = 1.0f;
        }
    }
    else if (key_state[SDL_SCANCODE_DOWN])
    {
        if (g_p1_position.y - MII_HEIGHT > -6.0f)
        {
            g_p1_movement.y = -1.0f;
        }
    }
    
    // PLAYER TWO MOVEMENT
    if (playing)
    {
        if (key_state[SDL_SCANCODE_W])
        {
            if (g_p2_position.y + MII_HEIGHT < 6.0f)
            {
                g_p2_movement.y = 1.0f;
            }
        }
        else if (key_state[SDL_SCANCODE_S])
        {
            if (g_p2_position.y - MII_HEIGHT > -6.0f)
            {
                g_p2_movement.y = -1.0f;
            }
        }
    }
                                                                             
}

void update()
{
    float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECOND; // get the current number of ticks
    float delta_time = ticks - g_previous_ticks; // the delta time is the difference from the last frame
    g_previous_ticks = ticks;
    
//    LOG("Player One Position: (" << g_p1_position.x << ", " << g_p1_position.y << ")");
//    LOG("Player Two Position: (" << g_p2_position.x << ", " << g_p2_position.y << ")");
//    LOG("Ball Position: (" << g_ball_position.x << ", " << g_ball_position.y << ")" << std::endl);

    
    // RESET PLAYERS
    g_p1_matrix = glm::mat4(1.0f);
    g_p1_matrix = glm::translate(g_p1_matrix, INIT_POS_P1);
    
    g_p2_matrix = glm::mat4(1.0f);
    g_p2_matrix = glm::translate(g_p2_matrix, INIT_POS_P2);
    
    g_ball_matrix = glm::mat4(1.0f);
    g_ball_matrix = glm::translate(g_ball_matrix, INIT_POS_BALL);
    
    if (ball_num >= 2){
        g_ball2_matrix = glm::mat4(1.0f);
        g_ball2_matrix = glm::translate(g_ball2_matrix, INIT_POS_BALL);
    }
    
    if (ball_num == 3){
        g_ball3_matrix = glm::mat4(1.0f);
        g_ball3_matrix = glm::translate(g_ball3_matrix, INIT_POS_BALL);
    }

    
    // PLAYER TWO AUTO-MOVEMENT
    if (not playing){
        if (fabs(g_p2_position.y) + MII_HEIGHT > 6.0f){
            direction = not direction;
        }
        
        if (direction && g_p2_position.y + MII_HEIGHT < 6.0f){
            g_p2_movement.y = 1.0f;
        } else if (not direction){
            if (g_p2_position.y - MII_HEIGHT < -6.0f){
                g_p2_movement.y = 1.0f;
            } else {
                g_p2_movement.y = -1.0f;
            }
        }
    }
    
    // COLLISION DETECTION BALL 1 -- the border is a little bit infront of the actual mii and not the racket
    float ball_p1_x = fabs(g_p1_position.x + INIT_POS_P1.x - g_ball_position.x) -
        ((INIT_SCALE_BALL.x + INIT_SCALE_P1.x) / 2.0f) + 1.0f;
    float ball_p1_y = fabs(g_p1_position.y + INIT_POS_P1.y - g_ball_position.y) -
        ((INIT_SCALE_BALL.y + INIT_SCALE_P1.y) / 2.0f + 0.5f);
    
    if (ball_p1_x < 0.0f && ball_p1_y < 0.0f){
        g_ball_speed += 0.1f;
        ball_direction_w = false;
    }
    
    float ball_p2_x = fabs(g_p2_position.x + INIT_POS_P2.x - g_ball_position.x) -
        ((INIT_SCALE_BALL.x + INIT_SCALE_P2.x) / 2.0f) + 1.0f;
    float ball_p2_y = fabs(g_p2_position.y + INIT_POS_P2.y - g_ball_position.y) -
        ((INIT_SCALE_BALL.y + INIT_SCALE_P2.y) / 2.0f) + 0.5f;
    
    if (ball_p2_x < 0.0f && ball_p2_y < 0.0f){
        g_ball_speed += 0.1f;
        ball_direction_w = true;
    }
    
    // BALL MOVEMENT DIRECTION
    if (fabs(g_ball_position.y) > 3.5f){
        ball_direction_h = not ball_direction_h;
    }
    
    if (ball_direction_w){
        g_ball_movement.x = 1.0f;
    } else {
        g_ball_movement.x = -1.0f;
    }
    
    if (ball_direction_h && g_ball_position.y < 3.5f){
        g_ball_movement.y = 1.0f;
    } else if (not ball_direction_h){
        if ( g_ball_position.y < -3.5f){
            g_ball_movement.y = 1.0f;
        } else {
            g_ball_movement.y = -1.0f;
        }
    }
    
    // CHECK IF THE BALL HIT THE SIDE OF THE WINDOW
    if (g_ball_position.x > 4.7f){
        game_over = true;
        winner = false;
    } else if (g_ball_position.x < -4.7f){
        game_over = true;
        winner = true;
    }
    
    if (ball_num >= 2){
        float ball2_p1_x = fabs(g_p1_position.x + INIT_POS_P1.x - g_ball2_position.x) -
            ((INIT_SCALE_BALL.x + INIT_SCALE_P1.x) / 2.0f) + 1.0f;
        float ball2_p1_y = fabs(g_p1_position.y + INIT_POS_P1.y - g_ball2_position.y) -
            ((INIT_SCALE_BALL.y + INIT_SCALE_P1.y) / 2.0f + 0.5f);
        
        if (ball2_p1_x < 0.0f && ball2_p1_y < 0.0f){
            g_ball2_speed += 0.1f;
            ball2_direction_w = false;
        }
        
        float ball2_p2_x = fabs(g_p2_position.x + INIT_POS_P2.x - g_ball2_position.x) -
            ((INIT_SCALE_BALL.x + INIT_SCALE_P2.x) / 2.0f) + 1.0f;
        float ball2_p2_y = fabs(g_p2_position.y + INIT_POS_P2.y - g_ball2_position.y) -
            ((INIT_SCALE_BALL.y + INIT_SCALE_P2.y) / 2.0f) + 0.5f;
        
        if (ball2_p2_x < 0.0f && ball2_p2_y < 0.0f){
            g_ball2_speed += 0.1f;
            ball2_direction_w = true;
        }
        
        // BALL MOVEMENT DIRECTION
        if (fabs(g_ball2_position.y) > 3.5f){
            ball2_direction_h = not ball2_direction_h;
        }
        
        if (ball2_direction_w){
            g_ball2_movement.x = 1.0f;
        } else {
            g_ball2_movement.x = -1.0f;
        }
        
        if (ball2_direction_h && g_ball2_position.y < 3.5f){
            g_ball2_movement.y = 1.0f;
        } else if (not ball2_direction_h){
            if ( g_ball2_position.y < -3.5f){
                g_ball2_movement.y = 1.0f;
            } else {
                g_ball2_movement.y = -1.0f;
            }
        }
        
        // CHECK IF THE BALL HIT THE SIDE OF THE WINDOW
        if (g_ball2_position.x > 4.7f){
            game_over = true;
            winner = false;
        } else if (g_ball2_position.x < -4.7f){
            game_over = true;
            winner = true;
        }
    }
    
    if (ball_num == 3){
        float ball3_p1_x = fabs(g_p1_position.x + INIT_POS_P1.x - g_ball3_position.x) -
            ((INIT_SCALE_BALL.x + INIT_SCALE_P1.x) / 2.0f) + 1.0f;
        float ball3_p1_y = fabs(g_p1_position.y + INIT_POS_P1.y - g_ball3_position.y) -
            ((INIT_SCALE_BALL.y + INIT_SCALE_P1.y) / 2.0f + 0.5f);
        
        if (ball3_p1_x < 0.0f && ball3_p1_y < 0.0f){
            g_ball3_speed += 0.1f;
            ball3_direction_w = false;
        }
        
        float ball3_p2_x = fabs(g_p2_position.x + INIT_POS_P2.x - g_ball3_position.x) -
            ((INIT_SCALE_BALL.x + INIT_SCALE_P2.x) / 2.0f) + 1.0f;
        float ball3_p2_y = fabs(g_p2_position.y + INIT_POS_P2.y - g_ball3_position.y) -
            ((INIT_SCALE_BALL.y + INIT_SCALE_P2.y) / 2.0f) + 0.5f;
        
        if (ball3_p2_x < 0.0f && ball3_p2_y < 0.0f){
            g_ball3_speed += 0.1f;
            ball3_direction_w = true;
        }
        
        // BALL MOVEMENT DIRECTION
        if (fabs(g_ball3_position.y) > 3.5f){
            ball3_direction_h = not ball3_direction_h;
        }
        
        if (ball3_direction_w){
            g_ball3_movement.x = 1.0f;
        } else {
            g_ball3_movement.x = -1.0f;
        }
        
        if (ball3_direction_h && g_ball3_position.y < 3.5f){
            g_ball3_movement.y = 1.0f;
        } else if (not ball3_direction_h){
            if ( g_ball3_position.y < -3.5f){
                g_ball3_movement.y = 1.0f;
            } else {
                g_ball3_movement.y = -1.0f;
            }
        }
        
        // CHECK IF THE BALL HIT THE SIDE OF THE WINDOW
        if (g_ball3_position.x > 4.7f){
            game_over = true;
            winner = false;
        } else if (g_ball3_position.x < -4.7f){
            game_over = true;
            winner = true;
        }
    }
    
    if (not game_over) {
        g_p1_position += g_p1_movement * g_p1_speed * delta_time;
        g_p2_position += g_p2_movement *g_p2_speed * delta_time;
        
        g_p1_matrix = glm::translate(g_p1_matrix, g_p1_position);
        g_p2_matrix = glm::translate(g_p2_matrix, g_p2_position);
        
        g_ball_position += g_ball_movement * g_ball_speed * delta_time;
        g_ball_matrix = glm::translate(g_ball_matrix, g_ball_position);

        if (ball_num >= 2){
            g_ball2_position += g_ball2_movement * g_ball2_speed * delta_time;
            g_ball2_matrix = glm::translate(g_ball2_matrix, g_ball2_position);
        }
        if (ball_num == 3){
            g_ball3_position += g_ball3_movement * g_ball3_speed * delta_time;
            g_ball3_matrix = glm::translate(g_ball3_matrix, g_ball3_position);
        }
        
        
            
    }
    
    // RESET SCALE
    g_p1_matrix = glm::scale(g_p1_matrix, INIT_SCALE_P1);
    g_p2_matrix  = glm::scale(g_p2_matrix, INIT_SCALE_P2);

    
}

void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id)
{
    g_shader_program.set_model_matrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so we use 6 instead of 3
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Vertices
    float vertices[] = {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] = {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };
    
    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());
    
    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    // Bind texture
    if (not game_start) {
        draw_object(g_game_start_matrix, g_game_screen_texture_id);
    } else if (not game_over){
        draw_object(g_background_matrix, g_background_texture_id);
        draw_object(g_p1_matrix, g_p1_texture_id);
        draw_object(g_p2_matrix, g_p2_texture_id);
        draw_object(g_ball_matrix, g_ball_texture_id);
        if (ball_num >= 2){
            draw_object(g_ball2_matrix, g_ball_texture_id);
        }
        if (ball_num == 3 ){
            draw_object(g_ball3_matrix, g_ball_texture_id);
        }
    } else {
        if (winner) { // player one wins
            draw_object(g_p1_wins_matrix, g_p1_wins_texture_id);
        } else { // player two wins
            draw_object(g_p2_wins_matrix, g_p2_wins_texture_id);
        }
    }
    
    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() { SDL_Quit(); }


int main(int argc, char* argv[])
{
    initialise();
    
    while (g_app_status == RUNNING)
    {
        process_input();
        update();
        render();
    }
    
    shutdown();
    return 0;
}
