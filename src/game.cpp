#include "game.h"
#include "utils.h"
#include "input.h"
#include "image.h"

#include <cmath>

Game* Game::instance = NULL;

Game::Game(int window_width, int window_height, SDL_Window* window)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;

	fps = 0;
	frame = 0;
	time = 0.0f;
	elapsed_time = 0.0f;

	bg = Color(65,135,35);
	cover.loadTGA("../data/cover.tga");
	tileset = new Image();
	tileset->loadTGA("../data/tileset.tga");
	font.loadTGA("../data/bitmap-font-white.tga"); //load bitmap-font image
	font2.loadTGA("../data/bitmap-font-black.tga"); //load bitmap-font image
	minifont.loadTGA("../data/mini-font-white-4x6.tga"); //load bitmap-font image
	minifont2.loadTGA("../data/mini-font-black-4x6.tga"); //load bitmap-font image

	enableAudio(); //enable this line if you plan to add audio to your application
	synth.loadSample("../data/point.wav");
	synth.loadSample("../data/eaten.wav");
	synth.loadSample("../data/drown.wav");
	synth.loadSample("../data/shout1.wav");
	synth.loadSample("../data/shout2.wav");
	
	//stages[INTRO_STAGE] = new IntroStage(); 
	stages[MENU_STAGE] = new MenuStage(); 
	stages[TUTORIAL_STAGE] = new TutorialStage();
	stages[PAUSE_STAGE] = new PauseStage(); 
	stages[END_STAGE] = new EndStage();

	n_playstages = load_play_stages(play_stages, "../data/playable.txt");
	if(!n_playstages){
		std::cout<<"NOT FOUND"<<std::endl;
	};
	//play_stages[0] = new PlayStage(loadGameMap("../data/play1.map"),1,1);
	//play_stages[1] = new PlayStage(loadGameMap("../data/play2.map"),1,1);
}

int Game::load_play_stages(Stage * stages[], const char* filename){
	FILE* file = fopen(filename,"r");
    if (file == NULL) //file not found
        return 0;
	
	char char_n[2];
	fread( &char_n, sizeof(char), 2, file);
	int int_n = atoi(char_n);
	
	short str_length = sizeof("../data/playX.map");
	char str_path[str_length];
	int n_sheep;
	int n_wolves;
    for(int i = 0; i<int_n; i++){
		fscanf(file, "%s", str_path);
		fscanf(file, "%d", &n_sheep);
		fscanf(file, "%d", &n_wolves);
		stages[i] = new PlayStage(loadGameMap(str_path),n_sheep,n_wolves);
	}
    fclose(file);
	return int_n;
}
//what to do when the image has to be draw
void Game::render(void)
{
	Image framebuffer(160, 120);
	if (to_render != PLAY_STAGE)
		stages[to_render]->Render(&framebuffer);
	else
		play_stages[curr_playstage]->Render(&framebuffer);
	
	if (transition >= 0.5){framebuffer.multiplyByColor(Color(transition/2*255,transition/2*255,transition/2*255));}
	else if (transition > 0){framebuffer.multiplyByColor(Color(128-transition*255,128-transition*255,128-transition*255));}
	//framebuffer.quantize(3);
	
	showFramebuffer(&framebuffer);
}

void Game::update(double seconds_elapsed)
{
	
	to_render = curr_stage_enum;
	if (transition > 0.5){
		to_render = prev_stage_enum;
	}
	if (transition>0){transition-=seconds_elapsed;}
	if (curr_stage_enum != PLAY_STAGE)
		stages[curr_stage_enum]->Update(seconds_elapsed);
	else
		play_stages[curr_playstage]->Update(seconds_elapsed);
	
}

//Keyboard event handler (sync input)
void Game::onKeyDown( SDL_KeyboardEvent event )
{
	switch(event.keysym.sym)
	{
		case SDLK_ESCAPE: must_exit = true; break; //ESC key, kill the app
	}
	if(curr_stage_enum==PLAY_STAGE) 
		switch(event.keysym.sym){
			case SDLK_a: 
				((PlayStage*)play_stages[curr_playstage])->world->player.size = 4; 
				break;
			case SDLK_b: ((PlayStage*)play_stages[curr_playstage])->world->player.size = 2; break;
		}
}
void Game::onKeyUp(SDL_KeyboardEvent event)
{
	if(curr_stage_enum==PLAY_STAGE) 
		switch(event.keysym.sym){
			case SDLK_a: 
			case SDLK_b: 
				((PlayStage*)play_stages[curr_playstage])->world->player.size = 3; break;
		}
}

void Game::onGamepadButtonDown(SDL_JoyButtonEvent event)
{
}
void Game::onGamepadButtonUp(SDL_JoyButtonEvent event)
{
}

void Game::onMouseMove(SDL_MouseMotionEvent event)
{
}
void Game::onMouseButtonDown( SDL_MouseButtonEvent event )
{
}
void Game::onMouseButtonUp(SDL_MouseButtonEvent event)
{
}
void Game::onMouseWheel(SDL_MouseWheelEvent event)
{
}

void Game::onResize(int width, int height)
{
    std::cout << "window resized: " << width << "," << height << std::endl;
	glViewport( 0,0, width, height );
	window_width = width;
	window_height = height;
}

//sends the image to the framebuffer of the GPU
void Game::showFramebuffer(Image* img)
{
	static GLuint texture_id = -1;
	static GLuint shader_id = -1;
	if (!texture_id)
		glGenTextures(1, &texture_id);

	//upload as texture
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, img->width, img->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->pixels);

	glDisable(GL_CULL_FACE); glDisable(GL_DEPTH_TEST); glEnable(GL_TEXTURE_2D);
	float startx = -1.0; float starty = -1.0;
	float width = 2.0; float height = 2.0;

	//center in window
	float real_aspect = window_width / (float)window_height;
	float desired_aspect = img->width / (float)img->height;
	float diff = desired_aspect / real_aspect;
	width *= diff;
	startx = -diff;

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(startx, starty + height);
	glTexCoord2f(1.0, 0.0); glVertex2f(startx + width, starty + height);
	glTexCoord2f(1.0, 1.0); glVertex2f(startx + width, starty);
	glTexCoord2f(0.0, 1.0); glVertex2f(startx, starty);
	glEnd();

	/* this version resizes the image which is slower
	Image resized = *img;
	//resized.quantize(1); //change this line to have a more retro look
	resized.scale(window_width, window_height);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	if (1) //flip
	{
	glRasterPos2f(-1, 1);
	glPixelZoom(1, -1);
	}
	glDrawPixels( resized.width, resized.height, GL_RGBA, GL_UNSIGNED_BYTE, resized.pixels );
	*/
}

//AUDIO STUFF ********************

SDL_AudioSpec audio_spec;

void AudioCallback(void*  userdata, Uint8* stream, int len)
{
	static double audio_time = 0;

	memset(stream, 0, len);//clear
	if (!Game::instance)
		return;

	Game::instance->onAudio((float*)stream, len / sizeof(float), audio_time, audio_spec);
	audio_time += len / (double)audio_spec.freq;
}

void Game::enableAudio()
{
	SDL_memset(&audio_spec, 0, sizeof(audio_spec)); /* or SDL_zero(want) */
	audio_spec.freq = 48000;
	audio_spec.format = AUDIO_F32;
	audio_spec.channels = 1;
	audio_spec.samples = 1024;
	audio_spec.callback = AudioCallback; /* you wrote this function elsewhere. */
	if (SDL_OpenAudio(&audio_spec, &audio_spec) < 0) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		exit(-1);
	}
	SDL_PauseAudio(0);
}

void Game::onAudio(float *buffer, unsigned int len, double time, SDL_AudioSpec& audio_spec)
{
	//fill the audio buffer using our custom retro synth
	synth.generateAudio(buffer, len, audio_spec);
}

/*void IntroStage::Render(Image* framebuffer){
	
	framebuffer->fill(Color::RED);
	framebuffer->drawText( "INTRO", 0, 0, game->font );				//draws some text using a bitmap font in an image (assuming every char is 7x9)
	framebuffer->drawText( toString(game->time), 1, 10, game->minifont,4,6);	//draws some text using a bitmap font in an image (assuming every char is 4x6)
}
void IntroStage::Update(float elapsed_time){
	if (Input::wasKeyPressed(SDL_SCANCODE_A)){
		game->prev_stage_enum = game->curr_stage_enum;
		game->curr_stage_enum = MENU_STAGE;
		game->transition = 1;
	}
}*/

void MenuStage::Render(Image* framebuffer){
	
	*framebuffer = game->cover;
	framebuffer->drawText( "WHAT THE FLOCK", 2,10, game->font );				//draws some text using a bitmap font in an image (assuming every char is 7x9)
	framebuffer->drawText( "TUTORIAL", 35, 30, game->minifont,4,6);	//draws some text using a bitmap font in an image (assuming every char is 4x6)
	framebuffer->drawText( "PLAY", 35, 40, game->minifont,4,6);	//draws some text using a bitmap font in an image (assuming every char is 4x6)
	for(int i = 0; i<world->flock.size; i++){
		framebuffer->drawRectangle(
			world->flock.arr[i].pos.x,world->flock.arr[i].pos.y,
			world->flock.arr[i].alive,world->flock.arr[i].alive,
			Color::WHITE);
	}
}
void MenuStage::Update(float elapsed_time){
	if (Input::wasKeyPressed(SDL_SCANCODE_A)){
		game->synth.playSample(game->synth.samples["../data/point.wav"],1,false);
		if(option==0){
			game->prev_stage_enum = game->curr_stage_enum;
			game->curr_stage_enum = TUTORIAL_STAGE;
			game->transition = 1;
			((TutorialStage*)game->stages[game->curr_stage_enum])->tutorial_stage=0;
			game->curr_playstage = -1;
			//the position of the example entities isn't re-init from the last tutorial
		}
		else{
			game->prev_stage_enum = game->curr_stage_enum;
			game->curr_stage_enum = PLAY_STAGE;
			game->transition = 1;
			game->curr_playstage = 0;
			((PlayStage*)game->play_stages[game->curr_playstage])->init_time=int(game->time);
			((PlayStage*)game->play_stages[game->curr_playstage])->world->init(); 
		}
	}
	else if (Input::wasKeyPressed(SDL_SCANCODE_UP)){
		option = 0;
	}
	else if (Input::wasKeyPressed(SDL_SCANCODE_DOWN)){
		option = 1;
	}
	world->player.pos.set(25, 30+option*10);
	if(int(game->time*10)%5==0){world->influence(&(world->flock),-1,100,2);}
	int m[6] = {1,1,1,0,0,0};
	world->flow_group(&world->flock, m);
	world->move_group(&world->flock,elapsed_time);
}

void TutorialStage::Render(Image* framebuffer){
	std::string sub[] = {"Separation","Alignment","Cohesion","Flocking behaviour","... and there's you!"};
	framebuffer->fill(Color::WHITE);
	framebuffer->drawText("TUTORIAL:", 5, 5, game->font2 );			
	framebuffer->drawText( sub[tutorial_stage], 70, 7, game->minifont2, 4,6);
	framebuffer->drawText( "P to leave", 5, 110, game->minifont2, 4,6);
	
	if (tutorial_stage == 4){
		framebuffer->drawRectangle(world->player.pos.x, world->player.pos.y, world->player.size, world->player.size, Color::BROWN);	
		framebuffer->drawText( "A to scatter", 108, 100, game->minifont2, 4,6);
		framebuffer->drawText( "B to attract", 108, 110, game->minifont2, 4,6);
	}
	else framebuffer->drawText( "Press A ->", 115, 110, game->minifont2, 4,6);
	for(int i = 0; i<world->flock.size; i++){
		framebuffer->drawRectangle(
			world->flock.arr[i].pos.x,world->flock.arr[i].pos.y,
			world->flock.arr[i].alive,world->flock.arr[i].alive,
			Color(100+155.0/4 * int(world->flock.arr[i].pos.x)/40,
				 100+155.0/10 * int(world->flock.arr[i].pos.y)/12,
				 1.5*std::abs(world->flock.arr[i].pos.x-world->flock.arr[i].pos.y)));
	}
}
void TutorialStage::Update(float elapsed_time){
	int m[6] = {0,0,0,1,0,0};
	if (tutorial_stage < 3) {
		m[tutorial_stage] = 1;
	}
	else if (tutorial_stage >= 3) {
		for (int i=0; i<3; i++) m[i] = 1;
	}
	world->flow_group(&world->flock, m);
	world->move_group(&world->flock,elapsed_time);
	if (tutorial_stage < 4){
		if (Input::wasKeyPressed(SDL_SCANCODE_A)){
			tutorial_stage+=1;
			if (tutorial_stage<4)
				game->synth.playSample(game->synth.samples["../data/point.wav"],1,false);
			else
				game->synth.playSample(game->synth.samples["../data/shout1.wav"],1,false);
		}
	}
	else{
		world->player.vel *= 0; 
		if (Input::isKeyPressed(SDL_SCANCODE_UP)) world->player.vel.y = -world->player.max_vel;
		if (Input::isKeyPressed(SDL_SCANCODE_DOWN)) world->player.vel.y = world->player.max_vel;
		if (Input::isKeyPressed(SDL_SCANCODE_LEFT)) world->player.vel.x = -world->player.max_vel;
		if (Input::isKeyPressed(SDL_SCANCODE_RIGHT)) world->player.vel.x = world->player.max_vel;
		world->player.pos += max_mag(world->player.vel,world->player.max_vel) * elapsed_time;
		world->magic_edges();

		if (Input::wasKeyPressed(SDL_SCANCODE_A)) {
			world->influence(&world->flock, 1, 30, 5); 
			game->synth.playSample(game->synth.samples["../data/shout1.wav"],1,false);
		}
		if (Input::wasKeyPressed(SDL_SCANCODE_B)) {
			world->influence(&world->flock, -1, 30, 5);
			game->synth.playSample(game->synth.samples["../data/shout2.wav"],1,false);
		}
	}
	if (Input::wasKeyPressed(SDL_SCANCODE_P)){
		game->prev_stage_enum = game->curr_stage_enum;
		game->curr_stage_enum = PAUSE_STAGE;
		((PauseStage*)game->stages[game->curr_stage_enum])->pun = rand()%40;
	}
}

void PlayStage::Render(Image* framebuffer){
	framebuffer->fill(game->bg);
	
	int cs = game->tileset->width / 16; 
	//PAINTS EACH CELL (BACKGROUND)
	for (int x = 0; x < world->map->width; ++x)
		for (int y = 0; y < world->map->height; ++y)
		{
			//get cell info
			sCell& cell = world->map->getCell(x, y);
			if(cell.type == 0) //skip empty
				continue;
			int type = (int)cell.type;
			if (type == FIELD2){
				if(int((10*game->time+x+y))%40<10)type+=18;
			}
			if (type >= WATERC1 && type <= WATER5){
				if(int((3*game->time+x+y))%6<2)type+=16;
			}
			if (type == SPAWN){
				if(int((5*game->time+x+y))%3<1)type+=1;
			}
			if (type == TREE1 || type == TREE3){
				if(int((game->time+x+y))%4<2)type+=1;
			}
			//compute tile pos in tileset image
			int tilex = (type % 16) * cs;     	//x pos in tileset
			int tiley = floor(type/16) * cs;    //y pos in tileset
			Area area( tilex, tiley, cs, cs );	//tile area
			int screenx = x*cs; //place offset here if you want
			int screeny = y*cs;
			//avoid rendering out of screen stuff
			if( screenx < -cs || screenx > framebuffer->width ||
				screeny < -cs || screeny > framebuffer->height )
				continue;

			//draw region of tileset inside framebuffer
			framebuffer->drawImage(  *(game->tileset),	//image
				screenx,screeny,     					//pos in screen
				area );   								//area

			if (type==FARM){
				if (world->punctuation[POINT]<100)
					framebuffer->drawText( toString(world->punctuation[POINT]), screenx, screeny, game->minifont,4,6);
				else
				framebuffer->drawText( ":)", screenx, screeny, game->minifont,4,6);	
			}      						
		}

	framebuffer->drawRectangle(world->player.pos.x, world->player.pos.y, world->player.size, world->player.size, Color::BROWN);
	
	for(int i = 0; i<world->pack.size; i++){
		int coloralive = 100-std::min(10*world->pack.arr[i].alive,100);
		int size = 1+world->pack.arr[i].alive/(world->flock.size/5);
		framebuffer->drawRectangle(
			world->pack.arr[i].pos.x,world->pack.arr[i].pos.y,
			size,size,
			Color(coloralive,coloralive,coloralive));
			//Color::BLACK);
	}
	for(int i = 0; i<world->flock.size; i++){
		Color range[] = {Color::WHITE, Color::YELLOW, Color::RED, Color::BLUE};
		Color sheepC = range[world->flock.arr[i].cause];
		framebuffer->drawRectangle(
			world->flock.arr[i].pos.x,world->flock.arr[i].pos.y,
			std::max(1,world->flock.arr[i].alive),std::max(1,world->flock.arr[i].alive),
			sheepC);
	}
		
	framebuffer->drawText( toString(game->time-init_time), 1, 1, game->minifont,4,6);	
	framebuffer->drawText( "Alive: "+toString(world->flock.n_alive), 1, 10, game->minifont,4,6);	
	framebuffer->drawText( "Points: "+toString(world->punctuation[POINT]), 1, 20, game->minifont,4,6);	 
	
}
void PlayStage::Update(float elapsed_time){
	if (Input::wasKeyPressed(SDL_SCANCODE_R)){
		game->synth.playSample(game->synth.samples["../data/drown.wav"],1,false);
		init_time = int(game->time);
		world->init(); 
	}
	else if (Input::wasKeyPressed(SDL_SCANCODE_P)){
		game->prev_stage_enum = game->curr_stage_enum;
		game->curr_stage_enum = PAUSE_STAGE;
		((PauseStage*)game->stages[game->curr_stage_enum])->pun = rand()%40;
	}
	else if (world->flock.n_alive==0){
		game->curr_stage_enum = END_STAGE;
		((EndStage*)game->stages[game->curr_stage_enum])->stats[0] = int(game->time) - init_time;
		((EndStage*)game->stages[game->curr_stage_enum])->stats[1] = world->punctuation[POINT];
		((EndStage*)game->stages[game->curr_stage_enum])->stats[2] = world->punctuation[EATEN];
		((EndStage*)game->stages[game->curr_stage_enum])->stats[3] = world->punctuation[DROWN];
	}
	else{
		world->updateFlock(elapsed_time);
		world->updatePack(elapsed_time);

		world->player.vel *= 0; 
		if (Input::isKeyPressed(SDL_SCANCODE_UP)) world->player.vel.y = -world->player.max_vel;
		if (Input::isKeyPressed(SDL_SCANCODE_DOWN)) world->player.vel.y = world->player.max_vel;
		if (Input::isKeyPressed(SDL_SCANCODE_LEFT)) world->player.vel.x = -world->player.max_vel;
		if (Input::isKeyPressed(SDL_SCANCODE_RIGHT)) world->player.vel.x = world->player.max_vel;
		world->player.pos += max_mag(world->player.vel,world->player.max_vel) * elapsed_time;
		world->magic_edges();

		if (Input::wasKeyPressed(SDL_SCANCODE_A)) {
			world->shoutA(); 
			game->synth.playSample(game->synth.samples["../data/shout1.wav"],1,false);
		}
		if (Input::wasKeyPressed(SDL_SCANCODE_B)) {
			world->shoutB(); 
			game->synth.playSample(game->synth.samples["../data/shout2.wav"],1,false);
		}
	}
}	

void PauseStage::Render(Image* framebuffer){
	framebuffer->fill(Color::GRAY);
	framebuffer->drawText( "STOP THE FLOCK...", 20, 20, game->font );
	framebuffer->drawText( "THE CLOCK!, I meant", 15, 30, game->font );			
	framebuffer->drawText( "B for menu", 10, 105, game->minifont,4,6);
	framebuffer->drawText( "A for next level", 90, 105, game->minifont,4,6);
	if (game->prev_stage_enum == PLAY_STAGE){
		framebuffer->drawText( "You can always R to reset", 30, 95, game->minifont,4,6);

		std::ifstream file("../data/puns.txt");
		
		std::string q;
		std::string a;
		for(int i = 0; i<=pun; i++){
			std::getline(file, q);
			std::getline(file, a);
		}
		framebuffer->drawPun(q, 10, 50, game->minifont,4,6);
		framebuffer->drawPun(a, 20, 70, game->minifont,4,6);
	}
	else{
		framebuffer->drawText( "More info:", 10, 50, game->minifont,4,6);
		framebuffer->drawText( "- Wolves kill sheep", 10, 60, game->minifont,4,6);
		framebuffer->drawText( "- Water kill sheep", 10, 70, game->minifont,4,6);
		framebuffer->drawText( "- Wolves avoid water", 10, 80, game->minifont,4,6);
		framebuffer->drawText( "- Who's the goodest boy? You are!", 10, 90, game->minifont,4,6);
	}
}
void PauseStage::Update(float elapsed_time){
	if (Input::wasKeyPressed(SDL_SCANCODE_A)){
		game->curr_playstage += 1;
		if (game->curr_playstage < game->n_playstages){
			game->synth.playSample(game->synth.samples["../data/point.wav"],1,false);
			game->curr_stage_enum = PLAY_STAGE;
			((PlayStage*)game->play_stages[game->curr_playstage])->init_time = int(game->time);
			((PlayStage*)game->play_stages[game->curr_playstage])->world->init();
		}else game->curr_stage_enum = END_STAGE;
	}
	else if (Input::wasKeyPressed(SDL_SCANCODE_P))game->curr_stage_enum = game->prev_stage_enum ;
	else if (Input::wasKeyPressed(SDL_SCANCODE_R) && game->prev_stage_enum==PLAY_STAGE ){
		game->synth.playSample(game->synth.samples["../data/drown.wav"],1,false);
		game->curr_stage_enum = PLAY_STAGE;
		((PlayStage*)game->play_stages[game->curr_playstage])->init_time = int(game->time);
		((PlayStage*)game->play_stages[game->curr_playstage])->world->init(); 
	}
	else if (Input::wasKeyPressed(SDL_SCANCODE_B)){
		game->synth.playSample(game->synth.samples["../data/eaten.wav"],1,false);
		game->curr_stage_enum = MENU_STAGE;
		//((MenuStage*)game->stages[game->curr_playstage])->option = 0;
	}
}

void EndStage::Render(Image* framebuffer){
	
	framebuffer->fill(Color(135,206,235));
	if (game->curr_playstage<game->n_playstages){
		framebuffer->drawText( "TIME TO", 50, 10, game->font );			
		framebuffer->drawText( "COUNT SHEepZzZzz...", 15, 20, game->font );

		framebuffer->drawText( "TIME: "+toString(stats[0])+" sec", 20, 35, game->minifont,4,6);	
		framebuffer->drawText( "EATEN", 20, 43, game->minifont,4,6);
		for (int e = 0; e<stats[2]; e++){
			framebuffer->drawRectangle(20+(e%40)*3,51+3*floor(e/40),2,2,Color::RED);
		}
		framebuffer->drawText( "DROWN", 20, 63, game->minifont, 4,6);
		for (int d = 0; d<stats[3]; d++){
			framebuffer->drawRectangle(20+(d%40)*3,71+3*floor(d/40),2,2,Color::BLUE);
		}
		framebuffer->drawText( "POINTS!:)", 20, 83, game->minifont,4,6);
		for (int p = 0; p<stats[1]; p++){
			framebuffer->drawRectangle(20+(p%40)*3,91+3*floor(p/40),2,2,Color::YELLOW);
		}
		framebuffer->drawText( "B for menu", 10, 110, game->minifont,4,6);
		framebuffer->drawText( "You can always R to reset", 30, 100, game->minifont,4,6);
		framebuffer->drawText( "A for next level", 90, 110, game->minifont,4,6);
	}
	else{
		framebuffer->drawText( "Yey! You made it. Molt Be-eee-eh!;)", 10, 40, game->minifont,4,6);
		framebuffer->drawText( "Made by Quim H.M", 25, 60, game->font);
		framebuffer->drawText( "Further credits in the README(perhaps?)", 3, 70, game->minifont,4,6);
		framebuffer->drawText( "B for menu", 10, 110, game->minifont,4,6);
	}
}
void EndStage::Update(float elapsed_time){
	
	if (Input::wasKeyPressed(SDL_SCANCODE_A)){
		game->curr_playstage += 1;
		if (game->curr_playstage < game->n_playstages){
			game->synth.playSample(game->synth.samples["../data/point.wav"],1,false);
			game->curr_stage_enum = PLAY_STAGE;
			((PlayStage*)game->play_stages[game->curr_playstage])->init_time = int(game->time);
			((PlayStage*)game->play_stages[game->curr_playstage])->world->init();
		}
		else if (game->curr_playstage > game->n_playstages){
			game->synth.playSample(game->synth.samples["../data/eaten.wav"],1,false);
			game->curr_stage_enum = MENU_STAGE;
		}
	}
	else if (Input::wasKeyPressed(SDL_SCANCODE_R)&& game->curr_playstage < game->n_playstages){
		game->synth.playSample(game->synth.samples["../data/drown.wav"],1,false);
		game->curr_stage_enum = PLAY_STAGE;
		((PlayStage*)game->play_stages[game->curr_playstage])->init_time = int(game->time);
		((PlayStage*)game->play_stages[game->curr_playstage])->world->init(); 
	}
	else if (Input::wasKeyPressed(SDL_SCANCODE_B)){
		game->synth.playSample(game->synth.samples["../data/eaten.wav"],1,false);
		game->curr_stage_enum = MENU_STAGE;
		//((MenuStage*)game->stages[game->curr_playstage])->option = 0;
	}
}