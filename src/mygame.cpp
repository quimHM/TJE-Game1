#include "mygame.h"
#include "game.h"
//#define NSHEEP 70
//#define NWOLF 10
/*#define s_ALIGN_r 20
#define s_ALIGN_f 1
#define s_COHESION_r 20
#define s_COHESION_f 1.5
#define s_SEPARATION_r 5
#define s_SEPARATION_f 2

#define w_ALIGN_r 20
#define w_ALIGN_f 0.5
#define w_COHESION_r 50
#define w_COHESION_f 1
#define w_SEPARATION_r 5
#define w_SEPARATION_f 2
#define w_PREDATE_r 50
#define w_PREDATE_f 2

#define SEPARATIONEDGES_r 5

#define MAX_SPEED_FLOCK 30
#define INTENT_SPEED_FLOCK 25
#define MAX_SPEED_PACK 30
#define INTENT_SPEED_PACK 25*/


/*Vector2 max_mag(Vector2 v, float l) {	//Moved to framework   
    float woSq = v.x*v.x + v.y*v.y;            
    if(woSq > l*l) {                   
        v = v / v.length();
        v = v * l;
    }
    return v;
}*/

World::World(int w, int h, Synth * synth, TileMap* map, int n_s, int n_w){
	width = w;
	height = h;
	this->synth = synth;
	this->map = map;
	flock.size = n_s;
	pack.size = n_w;
	init();
}

World::World(int w, int h, Synth * synth, Vector2 pos_option, int n_s, int r){ //FAKE, FOR MENU
	width = w;
	height = h;
	this->synth = synth;
	flock.size = n_s;
	flock.MAX_SPEED_GROUP = 20;
	initGroup(&flock);
	player.pos = pos_option;
	for (int s = 0; s<flock.size; s++){
		flock.arr[s].pos = Vector2(player.pos.x+r*cos(360/flock.size * s),player.pos.y+r*sin(360/flock.size * s));
		flock.arr[s].vel = flock.arr[s].vel.random(flock.arr[s].max_vel);
	}
}

void World::shoutA(){
	influence(&flock, 1, 30, 5);
	influence(&pack, 1, 30, 10);
}
void World::shoutB(){
	influence(&flock, -1, 30, 10);
	influence(&pack, -1, 30, 5);
}

void World::influence(Group* group, int mode, int area, int effect){
	for(int i = 0; i<group->size; i++){
		float d = group->arr[i].pos.distance(player.pos);
		if (d < area){
			Vector2 delta = group->arr[i].pos-player.pos;
			group->arr[i].acc += delta*mode*effect;
		}
	}
};

/*void World::shuffle(){
	for(int i = 0; i<NSHEEP; i++){
		int r = rand()%200 - 100;
		if (r > -2 && r < 2){flock[i].pos += Vector2(r,r);}
	}
};*/

Vector2 World::predate(int index, int perceptionRadius, float max_force){
	int closest_d = pack.arr[index].pos.distance(flock.arr[0].pos);
	int closest_i = 0;
	bool found = false;
	for(int i = 0; i<flock.size; i++){
		if (flock.arr[i].alive && flock.arr[i].alive<=pack.arr[index].alive){
			int d = pack.arr[index].pos.distance(flock.arr[i].pos);
			if (d < perceptionRadius && d <= closest_d){closest_d = d; closest_i = i; found = true;}
		}
	}
	Vector2 steering;
	if (found){
		if (closest_d > 0){
			steering = (flock.arr[closest_i].pos-pack.arr[index].pos);
			steering = steering.normalize() * pack.arr[index].intent;
			steering = steering - pack.arr[index].vel;
			steering = max_mag(steering,max_force);
		}
		else{
			pack.arr[index].alive += flock.arr[closest_i].alive; 
			sheep_death(closest_i,EATEN);
		}
	}
	return steering;
};

/*DIRECTION World::setDirection(Vector2 v){
	if (std::abs(v.x)>std::abs(v.y)){
		if (v.x>0){return RIGHT;}
		else{return LEFT;}
	}
	else{
		if (v.y>0){return DOWN;}
		else{return UP;}
	}
}

void World::go_to(Entity* p, Vector2 pos, double seconds_elapsed){
	if (p->pos.x != pos.x || p->pos.y != pos.y){p->moving = true;}
	Vector2 direction = (pos-p->pos).normalize();
	p->pos += direction * p->max_vel * seconds_elapsed;
	p->dir = setDirection(direction);
}*/

void World::init(){
	punctuation[0]=0;punctuation[1]=0;punctuation[2]=0;punctuation[3]=0;
	initGroup(&flock);
	initGroup(&pack);
	int w_i = 0; int w_t = pack.size; int w_spawn_left = map->n_spawn;
	int s_i = 0; int s_t = flock.size; int s_spawn_left = map->n_field;
	for (int x = 0; x<20; x++){
		for(int y = 0; y<15; y++){
			if (map->getCell(x,y).type == START){player.pos.set(x*8, y*8);}
			if (map->getCell(x,y).type == SPAWN){
				double rand_chance = 100*(double)w_t/w_spawn_left;
				int amount = ceil((double)w_t/w_spawn_left);
				for (int n_wolf = 0; n_wolf < amount && w_t>0; n_wolf++){
					pack.arr[w_i].pos.set(x*8+rand()%8, y*8+rand()%8);
					w_i++; w_t--;
				}
				w_spawn_left-=1;
			}
			if (map->getCell(x,y).type == FIELD1 || map->getCell(x,y).type == FIELD2){
				double rand_chance = 100*(double)s_t/s_spawn_left;
				int amount = ceil((double)s_t/s_spawn_left);
				if(rand()%100<=rand_chance){
					for (int n_sheep = 0; n_sheep < amount && s_t>0; n_sheep++){
						flock.arr[s_i].pos.set(x*8+rand()%8, y*8+rand()%8);
						s_i++; s_t--;
					}
				}
				s_spawn_left-=1;
			}
		}
	}
	punctuation[POINT] = 0;
}


void World::initGroup(Group * group){
	for(int e = 0; e<group->size; e++){
		group->arr[e].max_vel = group->MAX_SPEED_GROUP;
		group->arr[e].intent = group->INTENT_GROUP;//flock.arr[i].max_vel*4/5;
		group->arr[e].vel.set(0,0);// = flock.arr[i].vel.random(flock.arr[i].max_vel);
		group->arr[e].acc.set(0,0);
		group->arr[e].alive = 1+e%2; group->arr[e].cause = ALIVE;
	}
	group->n_alive = group->size;
}
/*
void World::initFlock(){
	for(int i = 0; i<flock.size; i++){
		//flock.arr[i].pos = Vector2(rand()%width, rand()%height);
		flock.arr[i].max_vel = flock.MAX_SPEED_GROUP;
		flock.arr[i].intent = flock.INTENT_GROUP;//flock.arr[i].max_vel*4/5;
		flock.arr[i].vel.set(0,0);// = flock.arr[i].vel.random(flock.arr[i].max_vel);
		flock.arr[i].acc.set(0,0);
		flock.arr[i].alive = 1+i%2; flock.arr[i].cause = ALIVE;
	}
	flock.n_alive = flock.size;
}

void World::initPack(){
	for(int i = 0; i<pack.size; i++){
		//pack.arr[i].pos = Vector2(rand()%width, rand()%height);
    	pack.arr[i].max_vel = pack.MAX_SPEED_GROUP;
		pack.arr[i].intent = pack.MAX_SPEED_GROUP;//pack.arr[i].max_vel*4/5;
		pack.arr[i].vel.set(0,0);// = pack.arr[i].vel.random(pack.arr[i].max_vel);
		pack.arr[i].acc.set(0,0);
		pack.arr[i].alive = 1;
	}
	pack.n_alive = pack.size;
}*/

void World::sheep_death(int index, eCause cause){
	std::string samples_name[] = {"empty position","../data/point.wav","../data/eaten.wav","../data/drown.wav"};
	synth->playSample(synth->samples[samples_name[cause]]);
	punctuation[cause]+=1;
	flock.arr[index].alive = 0;
	flock.arr[index].cause = cause;
	flock.arr[index].vel.set(0,0);
	flock.arr[index].acc.set(0,0);
	flock.n_alive-=1;
}
Vector2 World::align(int index, Group group, int perceptionRadius, float max_force){
    Vector2 steering;
    int n_close = 0;
    for (int i = 0; i<group.size; i++) {
		if (group.arr[i].alive){
			float d = group.arr[index].pos.distance(group.arr[i].pos);
			if (i != index && d < perceptionRadius) {
				steering += group.arr[i].vel;
				n_close += 1;
			}
		}
    }
    if (n_close > 0 && steering.length()!=0) {
      steering = steering / n_close;
      steering = steering.normalize() * group.arr[index].intent;
      steering = steering - group.arr[index].vel;
	  steering = max_mag(steering,max_force); //TO CHECK IF INTERESTING
    }
    return steering;
}


Vector2 World::cohesion(int index, Group group, int perceptionRadius, float max_force){
    Vector2 steering;
    int n_close = 0;
    for (int i = 0; i<group.size; i++) {
		if (group.arr[i].cause==ALIVE || group.arr[i].cause==POINT){
			float d = group.arr[index].pos.distance(group.arr[i].pos);
			if (i != index && d < perceptionRadius) {
				steering += group.arr[i].pos;
				n_close += 1;
			}
		}
    }
    if (n_close > 0) {
    	steering = steering / n_close;
    	steering = steering - group.arr[index].pos;
	  	if(steering.length()!=0){
			steering = steering.normalize() * group.arr[index].intent;
			steering = steering - group.arr[index].vel;
			steering = max_mag(steering, max_force);
	 	}
    }
    return steering;
}


Vector2 World::separation(int index, Group group, int perceptionRadius, float max_force){
    Vector2 steering;
    int n_close = 0;
    for (int i = 0; i<group.size; i++) {
		if(group.arr[i].alive){
			float d = group.arr[index].pos.distance(group.arr[i].pos);
			if (i != index && d < perceptionRadius ) {
				d = fmax(d,0.001);
				steering += (group.arr[index].pos - group.arr[i].pos) / (d*d);
				n_close += 1;
			}
		}
    }
    if (n_close > 0 && steering.length()!=0) {
      	steering = steering / n_close;
	  	steering = steering.normalize() * group.arr[index].intent;
		steering = steering - group.arr[index].vel;
	  	steering = max_mag(steering,max_force); //TO CHECK IF INTERESTING
    }
    return steering;
}

Vector2 World::separation_cells(int index, Group group, eCellType first_coll) {
    
	Vector2 coord = Vector2((int)group.arr[index].pos.x/8,(int)group.arr[index].pos.y/8);
	Vector2 cell_borders[4] = {
		Vector2(coord.x,coord.y-1),
		Vector2(coord.x,coord.y+1),
		Vector2(coord.x-1,coord.y),
		Vector2(coord.x+1,coord.y)
	};
    Vector2 coord_borders[4] = {
		Vector2(group.arr[index].pos.x,(coord.y)*8), 
		Vector2(group.arr[index].pos.x,(coord.y+1)*8),
		Vector2(coord.x*8,group.arr[index].pos.y),
		Vector2((coord.x+1)*8,group.arr[index].pos.y)
	};

	Vector2 steering;
    int n_close = 0;
	for (int b = 0; b<4; b++) {
		if (map->getCell(cell_borders[b]).type>=first_coll){
			float d = group.arr[index].pos.distance(coord_borders[b]);
			if (d < 1) {
				d = fmax(d,0.001);
				steering += (group.arr[index].pos - coord_borders[b]) / (d);
				n_close += 1;
			}
		}
    }
    if (n_close > 0 && steering.length()!=0) {
    	steering = steering / n_close;
		steering = steering.normalize() * group.arr[index].max_vel;
      	steering = steering - group.arr[index].vel;
	  	//no max_mag() for edges/collision cells
    }
    return steering;
}

Vector2 World::separation_edges(int index, Group group) {
    int perceptionRadius = group.SEPARATIONEDGES_r;
	Vector2 borders[4] = {
		Vector2(group.arr[index].pos.x,0), 
		Vector2(group.arr[index].pos.x,height),
		Vector2(0,group.arr[index].pos.y),
		Vector2(width,group.arr[index].pos.y)
	};

	Vector2 steering;
    int n_close = 0;
    for (int b = 0; b<4; b++) {
    	float d = group.arr[index].pos.distance(borders[b]);
      	if (d < perceptionRadius) {
			d = fmax(d,0.001);
        	steering += (group.arr[index].pos - borders[b]) / d;
        	n_close += 1;
      	}
    }
    if (n_close > 0 && steering.length()!=0) {
    	steering = steering / n_close;
    	steering = steering.normalize() * group.arr[index].max_vel;
    	steering = steering - group.arr[index].vel;
		//no max_mag() for edges/collision cells
    }
    return steering;
}

void World::flow_group(Group* group, int bitmask[6]) {
	for (int e = 0; e<group->size; e++) {
		if (group->arr[e].alive){
			if (bitmask[1]) group->arr[e].acc += align(e, *group, group->ALIGN_r, group->ALIGN_f);
			if (bitmask[2]) group->arr[e].acc += cohesion(e, *group, group->COHESION_r, group->COHESION_f);
			if (bitmask[0]) group->arr[e].acc += separation(e, *group, group->SEPARATION_r, group->SEPARATION_f);
			if (bitmask[3]) group->arr[e].acc += separation_edges(e, *group);
			if (bitmask[4]) group->arr[e].acc += separation_cells(e, *group, (eCellType)bitmask[4]);
			if (bitmask[5]) group->arr[e].acc += predate(e,group->PREDATE_r, group->PREDATE_f);
		}
	}
}
/*
void World::s_flow_ALL() {
	for (int s = 0; s<flock.size; s++) {
		if (flock.arr[s].alive){
			flock.arr[s].acc += align(s, flock, s_ALIGN_r, s_ALIGN_f);
			flock.arr[s].acc += cohesion(s, flock, s_COHESION_r, s_COHESION_f);
			flock.arr[s].acc += separation(s, flock, s_SEPARATION_r, s_SEPARATION_f);
			flock.arr[s].acc += separation_edges(s, flock);
			flock.arr[s].acc += separation_cells(s, flock, SPAWN);
		}
	}
}

void World::w_flow_ALL() {
	for (int w = 0; w<pack.size; w++) {
		if (pack.arr[w].alive){
			pack.arr[w].acc += align(w, pack, w_ALIGN_r, w_ALIGN_f);
			pack.arr[w].acc += cohesion(w, pack, w_COHESION_r, w_COHESION_f);
			pack.arr[w].acc += separation(w, pack, w_SEPARATION_r, w_SEPARATION_f); 
			pack.arr[w].acc += predate(w,w_PREDATE_r, w_PREDATE_f);
			pack.arr[w].acc += separation_edges(w, pack);
			pack.arr[w].acc += separation_cells(w, pack, WATERC1);
		}
	}
}*/

void World::move_group(Group* group, float seconds_elapsed){
	for (int e = 0; e<group->size; e++) {
		if (group->arr[e].alive){
			group->arr[e].pos += group->arr[e].vel * seconds_elapsed;
			
			group->arr[e].vel += group->arr[e].acc;
			group->arr[e].vel = max_mag(group->arr[e].vel,group->arr[e].max_vel);

			group->arr[e].acc *= 0;
		}
	}
}

void World::check_condition(){
	for (int s = 0; s<flock.size; s++) {
		if (flock.arr[s].alive){
			if (map->getCellFromWorldPos(flock.arr[s].pos).type==FARM){
				//punctuation[POINT]+=flock.arr[s].alive;
				sheep_death(s,POINT);
			}
			if (map->getCellFromWorldPos(flock.arr[s].pos).type>=WATER1 && map->getCellFromWorldPos(flock.arr[s].pos).type<=WATER5){
				sheep_death(s,DROWN);
			}
		}
	}
}

void World::updateFlock(float seconds_elapsed){
	int m[6] = {1,1,1,1,SPAWN,0};
	flow_group(&flock, m);
	move_group(&flock, seconds_elapsed);
	check_condition();
}


void World::updatePack(float seconds_elapsed){
	int m[6] = {1,1,1,1,WATER1,1};
	flow_group(&pack, m);
	move_group(&pack, seconds_elapsed);
}

void World::magic_edges(){
	if (player.pos.x >= width) player.pos.x = 0;
	if (player.pos.x < 0) player.pos.x = width;
	if (player.pos.y >= height) player.pos.y = 0;
	if (player.pos.y < 0) player.pos.y = height;
}
/*
void World::hard_edges(int index){
	if (flock[index].pos.x > width) flock[index].pos.x = width;
	if (flock[index].pos.x < 0) flock[index].pos.x = 0;
	if (flock[index].pos.y > height) flock[index].pos.y = height;
	if (flock[index].pos.y < 0) flock[index].pos.y = 0;
}*/

TileMap* loadGameMap(const char* filename)
{
    FILE* file = fopen(filename,"rb");
    if (file == NULL) //file not found
        return NULL;

    sMapHeader header; //read header and store it in the struct
    fread( &header, sizeof(sMapHeader), 1, file);
    assert(header.bytes == 1); //always control bad cases!!


    //allocate memory for the cells data and read it
    unsigned char* cells = new unsigned char[ header.w*header.h ];
    fread( cells, header.bytes, header.w*header.h, file);
    fclose(file); //always close open files
    //create the map where we will store it
    TileMap* map = new TileMap(header.w,header.h);

    for(int x = 0; x < map->width; x++)
        for(int y = 0; y < map->height; y++){
            map->getCell(x,y).type = (eCellType)cells[x+y*map->width];
			if (map->getCell(x,y).type == SPAWN) map->n_spawn+=1;
			else if (map->getCell(x,y).type == FIELD1 || map->getCell(x,y).type == FIELD2) map->n_field+=1;
		}
	//std::cout << map->n_spawn << ", " << map->n_field << std::endl;
				
    delete[] cells; //always free any memory allocated!
    
    return map;
}