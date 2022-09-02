#ifndef MYGAME_H
#define MYGAME_H

#include "utils.h"
#include "image.h"
#include "synth.h"
#include <fstream>

#define MAX_NENTITY_PER_TYPE 100

enum DIRECTION{
	DOWN,RIGHT,LEFT,UP
};

enum eCellType : uint8 { 
EMPTY, FIELD1, FIELD2, START,
FARM,
WATERC1, WATERC2, WATERC3, WATERC4,
WATER1, WATER2, WATER3, WATER4, WATER5,
SPAWN, MOUNTAIN,
TREE1,TREE2,TREE3,TREE4
};

enum eCause : uint8 { 
ALIVE, POINT, EATEN, DROWN
};

struct sCell {
    eCellType type;   
};

class TileMap {
public:
    int width;
    int height;
    sCell* data;
	int n_spawn;
	int n_field;

    TileMap()
    {
        width = height = 0;
		data = NULL;
    }

    TileMap(int w, int h)
    {
        width = w;
        height = h;
        data = new sCell[w*h];
		n_spawn = 0;
		n_field = 0;
    }

    sCell& getCell(int x, int y)
    {
        return data[x + y * width];
    }

	sCell& getCell(Vector2 pos)
    {
        return getCell(pos.x, pos.y);
    }

	sCell& getCellFromWorldPos(Vector2 pos)
    {
        return getCell((int)pos.x/8, (int)pos.y/8);
    }
};

struct sMapHeader {
    int w; //width of map
    int h; //height of map
    unsigned char bytes; //num bytes per cell
    unsigned char extra[7]; //filling bytes, not used
};

TileMap* loadGameMap(const char* filename);

class World
{
public:
	World(int w, int h, Synth * synth, TileMap* map, int n_s, int n_w);
	World(int w, int h, Synth * synth, Vector2 pos_option, int n_s, int r);

	struct Entity{
		int size = 3;
		Vector2 pos;
		Vector2 vel;
		float max_vel = 70.0f;
		float intent = max_vel/2;
		int alive = 1;
		Vector2 acc;
		//DIRECTION dir; //TODO check if necessary
		//bool moving; //TODO check if necessary
		eCause cause = ALIVE;
	};

	struct Group{
		World::Entity arr[MAX_NENTITY_PER_TYPE];
		int size;
		int n_alive;

		float ALIGN_r = 20;
		float ALIGN_f = 1;
		float COHESION_r = 20;
		float COHESION_f = 2;
		float SEPARATION_r = 5;
		float SEPARATION_f = 3;
		float PREDATE_r = 50;
		float PREDATE_f = 2;
 		float SEPARATIONEDGES_r = 3;
		float MAX_SPEED_GROUP = 30;
		float INTENT_GROUP = 25;
	};

	//void shout(int mode, int area, int effect);//TODO check if necessary
	//void shuffle();//TODO check if necessary
	//void go_to(Entity* p, Vector2 pos, double seconds_elapsed); //TODO check if necessary
	//DIRECTION setDirection(Vector2 v);//TODO check if necessary

	void init();
	void initGroup(Group* group);
	//void initFlock();
	//void initPack();

	void shoutA();
	void shoutB();
	void influence(Group* group, int mode, int area, int effect);
	Vector2 align(int index,  Group group, int perceptionRadius, float max_force);
	Vector2 cohesion(int index,  Group group, int perceptionRadius, float max_force);
	Vector2 separation(int index, Group group, int perceptionRadius, float max_force);

	Vector2 predate(int index, int perceptionRadius, float max_force);

	Vector2 separation_cells(int index, Group group, eCellType first_coll);
	Vector2 separation_edges(int index, Group group);
	
	void flow_group(Group* group, int bitmask[6]);
	//void s_flow_ALL();
	//void w_flow_ALL();
	void move_group(Group* group, float seconds_elapsed);
	void check_condition();
	void updateFlock(float seconds_elapsed);
	void updatePack(float seconds_elapsed);
	void sheep_death(int index, eCause cause);
	void magic_edges();//TODO check if necessary
	//void hard_edges(int index);//TODO check if necessary

	int width;
	int height;
	TileMap* map;
	Entity player;
	Group flock; 
	Group pack;

	Synth * synth;

	int punctuation[4] = {0,0,0,0};
};

#endif