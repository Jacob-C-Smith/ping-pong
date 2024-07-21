/** !
 * g10 ping pong
 * 
 * @file main.c
 * 
 * @author Jacob Smith
 */

// Standard library
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

// sync module
#include <sync/sync.h>

// json module
#include <json/json.h>

// g10
#include <g10/g10.h>
#include <g10/entity.h>
#include <g10/camera.h>
#include <g10/input.h>
#include <g10/mesh.h>
#include <g10/user_code.h>

/** !
 *  This gets called once per frame
 * 
 * @param p_instance the active instance
 * 
 * @return 1 
*/
int user_code_main ( g_instance *const p_instance );

/** !
 * Set up the example
 * 
 * @param p_instance the active instance
 * 
 * @return 1
 */
int game_setup ( g_instance *const p_instance );

// Data
static int p1_score = 0,
           p2_score = 0;

// Entry point
int main ( int argc, const char *const argv[] )
{

    // Supress compiler warnings
    (void) argc;
    (void) argv;

    // Initialized data
    g_instance *p_instance = 0;

    // Initialize g10
    if ( g_init(&p_instance, "resources/instance.json") == 0 ) goto failed_to_initialize_g10;

    // Set up the example
    game_setup(p_instance);

    // Block
    while ( p_instance->running )
    {

        // Input
        input_poll(p_instance);

        // User code
        user_code_callback(p_instance);

        // Render
        renderer_render(p_instance);

        // Present
        renderer_present(p_instance);
    }

    // Clean up g10
    if ( g_exit(&p_instance) == 0 ) goto failed_to_teardown_g10;

    // Log the final score
    log_info("\nFinal score: %d - %d\n", p1_score, p2_score);

    // Log the clean exit
    log_info("Game over!\n");

    // Success
    return EXIT_SUCCESS;

    // Error handling
    {

        // g10 errors
        {
            failed_to_initialize_g10:
                
                // Write an error message to standard out
                log_error("Error: Failed to initialize g10!\n");

                // Error
                return EXIT_FAILURE;

            failed_to_teardown_g10:
                
                // Write an error message to standard out
                log_warning("Error: Failed to teardown g10!\n");

                // Error
                return EXIT_FAILURE;
        }
    }
}

int user_code_main ( g_instance *const p_instance )
{

    // Static data
    static bool playing = false;
    static float ball_v_x = 1.0,
                 ball_v_y = 0.1;

    // Exit the game?
    if ( input_bind_value("QUIT") ) g_stop();

    // Game logic
    {
        entity *p_player_1 = dict_get(p_instance->context.p_scene->data.entities, "player 1"),
               *p_player_2 = dict_get(p_instance->context.p_scene->data.entities, "player 2"), 
               *p_ball     = dict_get(p_instance->context.p_scene->data.entities, "ball");
        mesh_data *p_player_1_paddle = p_player_1->p_mesh->_p_meshes[0], 
                  *p_player_2_paddle = p_player_2->p_mesh->_p_meshes[0],
                  *p_ball_mesh       = p_ball->p_mesh->_p_meshes[0];

        // Player 1
        {

            // Initialized data
            vec3 *p_location = &p_player_1_paddle->p_transform->location;
            float move = input_bind_value("PLAYER 1 UP") - input_bind_value("PLAYER 1 DOWN");

            // Move the paddle
            p_location->y += 0.25 * move;

            // Constraint
            if ( p_location->y >=  7.25 ) p_location->y = 7.25;
            if ( p_location->y <= -7.25 ) p_location->y = -7.25;
        }

        // Player 2
        {

            // Initialized data
            vec3 *p_location = &p_player_2_paddle->p_transform->location;
            float move = input_bind_value("PLAYER 2 UP") - input_bind_value("PLAYER 2 DOWN");

            // Move the paddle
            p_location->y += 0.25 * move;

            // Constraint
            if ( p_location->y >=  7.25 ) p_location->y = 7.25;
            if ( p_location->y <= -7.25 ) p_location->y = -7.25;
        }
    
        // Serve the ball
        if ( input_bind_value("SERVE") && playing == false )
        {

            // Set the playing flag
            playing = true;

            // Random vertical velocity [-0.5, 0.5]
            ball_v_y = ( (float) rand() / (float) RAND_MAX ) - 0.5;
        }

        // Move the ball
        if ( playing )
        {

            // Initialized data
            vec3 *p_location = &p_ball_mesh->p_transform->location;
            
            // Move the ball
            p_location->x += 0.25 * ball_v_x;
            p_location->y += 0.25 * ball_v_y;
        }

        // Collision detection
        if ( playing )
        {

            // Initialized data
            vec3 *p_player_1_location = &p_player_1_paddle->p_transform->location,
                 *p_player_2_location = &p_player_2_paddle->p_transform->location,
                 *p_ball_location     = &p_ball_mesh->p_transform->location;
            
            // Keep the ball in the play area
            if ( p_ball_location->y >=  8.75 ) ball_v_y *= -1.0;
            if ( p_ball_location->y <= -8.75 ) ball_v_y *= -1.0;
        
            // Check player 1's paddle
            if ( p_ball_location->x >= 14.0 )
            {
                
                // Hit
                if ( p_player_1_location->y + 2.5 >= p_ball_location->y && p_player_1_location->y - 2.5 <= p_ball_location->y ) ball_v_x *= -1.0;
                
                // Miss
                else
                {

                    // Raise player 2's score
                    p2_score++;

                    // Print the score
                    log_info("Score: %d - %d\n", p1_score, p2_score);

                    // Reset the ball
                    p_ball_location->x = 0.0, p_ball_location->y = 0.0;

                    // Clear the playing flag
                    playing = false;
                }
            }

            // Check player 2's paddle
            if ( p_ball_location->x <= -14.0 )
            {

                // Hit
                if ( p_player_2_location->y + 2.5 >= p_ball_location->y && p_player_2_location->y - 2.5 <= p_ball_location->y ) ball_v_x *= -1.0;
                
                // Miss
                else
                {

                    // Raise player 1's score
                    p1_score++;

                    // Print the score
                    log_info("Score: %d - %d\n", p1_score, p2_score);
                    
                    // Reset the ball
                    p_ball_location->x = 0.0, p_ball_location->y = 0.0;

                    // Clear the playing flag
                    playing = false;
                }
            }
        }
    }

    // Success
    return 1;
}


int game_setup ( g_instance *const p_instance )
{

    // Initialized data
    shell *p_shell = (void *) 0; 

    // Set a user code callback
    user_code_callback_set(p_instance, user_code_main);
    
    // Set the running flag
    p_instance->running = true;
    
    // Log
    log_info("Game setup is complete!\n");
    log_info("Press [SPACE] to serve!\n");

    // Success
    return 1;
}
