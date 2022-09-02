/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	This class encapsulates the game, is in charge of creating the game, getting the user input, process the update and render.
*/

#ifndef GAME_H
#define GAME_H

#include "includes.h"
#include "image.h"
#include "utils.h"
#include "synth.h"
#include "mygame.h"

#define N_STAGES 6
#define MAX_N_PLAYSTAGES 5
enum stage_types: uint8{
	//INTRO_STAGE, 
	MENU_STAGE, TUTORIAL_STAGE, PLAY_STAGE, PAUSE_STAGE, END_STAGE
};

class Game
{
public:
	static Game* instance;

	//window
	SDL_Window* window;
	int window_width;
	int window_height;

	//some globals
	long frame;
    float time;
	float elapsed_time;
	int fps;
	bool must_exit;

	//audio
	Synth synth;

	//ctor
	Game( int window_width, int window_height, SDL_Window* window );

	//main functions
	void render( void );
	void update( double dt );

	void showFramebuffer(Image* img);

	//events
	void onKeyDown( SDL_KeyboardEvent event );
	void onKeyUp(SDL_KeyboardEvent event);
	void onMouseButtonDown( SDL_MouseButtonEvent event );
	void onMouseButtonUp(SDL_MouseButtonEvent event);
	void onMouseMove(SDL_MouseMotionEvent event);
	void onMouseWheel(SDL_MouseWheelEvent event);
	void onGamepadButtonDown(SDL_JoyButtonEvent event);
	void onGamepadButtonUp(SDL_JoyButtonEvent event);
	void onResize(int width, int height);

	//audio stuff
	void enableAudio(); //opens audio channel to play sound
	void onAudio(float* buffer, unsigned int len, double time, SDL_AudioSpec &audio_spec); //called constantly to fill the audio buffer
	
	Color bg;
	float transition = 0; 
	Image cover;
	Image font;
	Image font2;
	Image minifont;
	Image minifont2;
	Image* tileset;

	class Stage {
		public:
			Game* game;
			virtual void Render(Image* framebuffer) = 0;
			virtual void Update(float elapsed_time) = 0;
	};

	Stage * stages[N_STAGES];
	stage_types curr_stage_enum = MENU_STAGE;
	stage_types prev_stage_enum = MENU_STAGE;
	stage_types to_render = MENU_STAGE ;

	Stage * play_stages[MAX_N_PLAYSTAGES];
	int n_playstages = 0;
	int curr_playstage = 0;

	int load_play_stages(Stage * stages[], const char* filename);
};


/*class IntroStage : public Game::Stage{
	public:
		//Game* game;
		IntroStage(void){game = Game::instance;}
		virtual void Render(Image* framebuffer);
		virtual void Update(float elapsed_time);
};*/
class MenuStage : public Game::Stage{
	public: 
		MenuStage(void){
			game = Game::instance;
			this->world = new World(160,120,&(game->synth),Vector2(25, 30),5,5);
		};
		World* world; 
		int option = 0;
		virtual void Render(Image* framebuffer); virtual void Update(float elapsed_time);};
class TutorialStage : public Game::Stage{
	public: 
		TutorialStage(void){
			game = Game::instance;
			this->world = new World(160,120,&(game->synth),Vector2(80,60),100,50);
		}
		World* world; 
		int tutorial_stage = 0;
		virtual void Render(Image* framebuffer); virtual void Update(float elapsed_time);};
class PlayStage : public Game::Stage{
	public: 
		PlayStage(TileMap* map, int n_s, int n_w){
			game = Game::instance; 
			this->world = new World(160,120, &(game->synth), map, n_s, n_w);
		}
		World* world; 
		int init_time;
		virtual void Render(Image* framebuffer); 
		virtual void Update(float elapsed_time);
};
class PauseStage : public Game::Stage{
	public: PauseStage(void){
			game = Game::instance;
		}
		int pun=0;

		virtual void Render(Image* framebuffer); virtual void Update(float elapsed_time);};
class EndStage : public Game::Stage{
	public: EndStage(void){game = Game::instance;}
		int stats[4];
		virtual void Render(Image* framebuffer); virtual void Update(float elapsed_time);};


#endif 