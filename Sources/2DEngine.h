#ifndef CSEG_H_INCLUDED
#define CSEG_H_INCLUDED
#include "vec2f.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <cmath>
#include <algorithm>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;
using namespace cv;

class Flag
{
	public :
		bool flag;
		bool ACK;

		Flag();
		Flag(bool _flag);
		void reset(bool _flag);
	private :
};

class ObjetTemplate
{
	public :
		int objectType;
		char character;
		bool isVisible;
        int r;
		int g;
		int b;
		Mat objTexture;
		string tags;

		ObjetTemplate();
		void setCharacter(char newCharacter);
		void setVisible(bool visibility);
		void setColor(int red, int green, int blue);
	private :
};

class Circle : public ObjetTemplate
{
	public :
		vec2f pos;
		vec2f vel;
		float radius;
		float mass;
        vector<Circle*> zoneInteraction;
		void (*zoneEntry)(Circle*, Circle*);
		void (*zoneExit)(Circle*, Circle*);
		void (*contact)(Circle*, Circle*);

		Circle();
		Circle(vec2f _pos, vec2f _vel, float _radius);
		void setEntryCallback(void (*foo)(Circle*, Circle*));
		void setExitCallback(void (*foo)(Circle*, Circle*));
		void setContactCallback(void (*foo)(Circle*, Circle*));
	private :
};

class Droite : public ObjetTemplate
{
	public :
		vec2f vec1;
		vec2f vec2;

		Droite();
		Droite(vec2f _vec1, vec2f _vec2);
	private :
};

class Segment : public ObjetTemplate
{
	public :
		vec2f vec1;
		vec2f vec2;

		Segment();
		Segment(vec2f _vec1, vec2f _vec2);
	private :
};

class Event
{
	public :
		int eventType;
		ObjetTemplate* obj1;
		ObjetTemplate* obj2;
		float eventDate;
		float entryDate;
		float exitDate;
		vec2f entryPos;
		vec2f exitPos;

		Event();
		Event(ObjetTemplate* _obj1, ObjetTemplate* _obj2);
	private :
};

class MenuOption
{
    public :
        string optionName;
        void (*optionFunc)();

        MenuOption(string optionName, void (*foo)());
};
/*
class Scene
{
	public :
        //virtual void handleInput (SDL_Event* event)  {}
		virtual void handleInput ()  {}
        //virtual void displaySDL(SDL_Renderer* renderer) {}
		virtual void displaySDL() {}
};

class Menu : public Scene
{
	public :
	    //SDL_Texture* background;
	    int positionIndice;
	    string title;
	    vector<MenuOption> menuOptions;
	    Menu()
	    {
	        background = NULL;
	        positionIndice = 0;
	        title = "";
	    }
	    void add_option(string optionName, void (*foo)()) {menuOptions.push_back(MenuOption(optionName, foo));}
	    void setBackground(SDL_Texture* tempBackground) {background = tempBackground;}
        int handleInput(SDL_Event* event)
        {
            switch( event->type )
            {
            case SDL_KEYDOWN:
                switch( event->key.keysym.sym )
                {
                case SDLK_RETURN:
                    menuOptions[positionIndice].optionFunc();
                    break;
                case SDLK_DOWN:
                    positionIndice ++;
                    break;
                case SDLK_UP:
                    positionIndice --;
                    break;
                default:
                    break;
                }
            default:
                break;
            }
            //menuOptions.back().optionFunc();
        }
        void displaySDL(SDL_Renderer* renderer)
        {
            if(background != NULL) SDL_RenderCopy( renderer, background, NULL, NULL);
            for(int i=0 ; i<menuOptions.size() ; i++)
            {
                if(i==positionIndice)
                {
                    printf(">");
                }
                cout << "\t" << menuOptions[i].optionName << endl;
            }
            cout << endl << endl << endl;
        }
};
*/
class DEngine
{
	public :
        vector<Circle*> CircleList;
        vector<Circle*> ZoneList;
		vector<Droite*> DroiteList;
		vector<Segment*> SegmentList;
		vector<Mat*> BackGroundList;

		DEngine();
		DEngine(float map_X1, float map_X2, float map_Y1, float map_Y2) : map_X1(map_X1), map_X2(map_X2), map_Y1(map_Y1), map_Y2(map_Y2) {};
		Circle* add_Circle(float x, float y, float vx, float vy, float radius);
		Droite* add_Droite(float x1, float y1, float x2, float y2);
		Segment* add_Segment(float x1, float y1, float x2, float y2);
		Circle* add_Zone(float x, float y, float radius, void (*entry)(Circle*, Circle*));
        void rmv_Circle(Circle* circle);
        void rmv_Droite(Droite* droite);
        void rmv_Segment(Segment* segment);
        void rmv_Zone(Circle* zone);
		Event getEarliestEvent(float timeStep);
		Event continuousCircleCircleCollision(Circle* circle1, Circle* circle2);
		Event continuousCircleDroiteCollision(Circle* circle1, Droite* droite1);
		Event continuousCircleSegmentCollision(Circle* circle1, Segment* segment1);
		Event continuousCircleZoneTraversal(Circle* circle1, Circle* zone);
		vec2f* checklinescollide(Droite* droite1, Droite* droite2);
		vec2f closestPointOnLine(vec2f l1, vec2f l2, vec2f p);
		vec2f closestPointOnSegment(vec2f l1, vec2f l2, vec2f p);
		void drawCircle(Mat* renderer, Circle* circle1, bool filled);
		void setCircleTexture(Mat* renderer, int i, char* path);
		void displayOpenCV(Mat* renderer);
		void step(Mat* renderer);
		void run_until_nospeed(Mat* renderer);
	private :
		float map_X1;
		float map_X2;
		float map_Y1;
		float map_Y2;
};


#endif // CSEG_H_INCLUDED
