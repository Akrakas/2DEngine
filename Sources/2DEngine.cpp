//Program originally made to run with SDL on windows, and adapted to use OPENCV on unix. 
//Some functionnality were removed during the transfer, like the main menu and the enforcing of pool rules. 

#include "2DEngine.h"
#include "vec2f.h"

#define FPS 10
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 400

Flag noWhiteBall = false;
Flag noBlackBall = false;
Flag madeContact = false;
Flag touchedBlackFirst = false;
Flag oppositeColorTouchedFirst = false;

int mouse_x;
int mouse_y;
bool mouse_click;

string window_name = "Pool_window";
DEngine* game_ptr;


//For recording purposes
//VideoWriter outputvid("../DEMO/Pool_table_DEMO_9.avi", VideoWriter::fourcc('D','I','V','X'), 30, Size(WINDOW_WIDTH, WINDOW_HEIGHT), true);



//answer.opencv.org
void overlayImage(Mat* src, Mat* overlay, const Point& location)
{
    for (int y = max(location.y, 0); y < src->rows; ++y)
    {
        int fY = y - location.y;
        if (fY >= overlay->rows) break;
        for (int x = max(location.x, 0); x < src->cols; ++x)
        {
            int fX = x - location.x;
            if (fX >= overlay->cols) break;
            double opacity = ((double)overlay->data[fY * overlay->step + fX * overlay->channels() + 3]) / 255;
            for (int c = 0; opacity > 0 && c < src->channels(); ++c)
            {
                unsigned char overlayPx = overlay->data[fY * overlay->step + fX * overlay->channels() + c];
                unsigned char srcPx = src->data[y * src->step + x * src->channels() + c];
                src->data[y * src->step + src->channels() * x + c] = srcPx * (1. - opacity) + overlayPx * opacity;
            }
        }
    }
}

void contact(Circle* circle1, Circle* circle2) {
    if(!madeContact.flag && (circle1->tags.find("BLACK") != -1 || circle2->tags.find("BLACK") != -1))
    {
        touchedBlackFirst.flag = true;
    }
    madeContact.flag = true;
}

void entryBlue(Circle* circle1, Circle* zone) {
    circle1->setColor(0, 0, 255);
}

void entryGreen(Circle* circle1, Circle* zone) {
    circle1->setColor(0, 255, 0);
}

void entryHole(Circle* circle1, Circle* zone) {
    if(circle1->tags.find("WHITE") != -1) noWhiteBall.flag = true;
    else if(circle1->tags.find("BLACK") != -1) noBlackBall.flag = true;
    game_ptr->rmv_Circle(circle1);
}

void exit(Circle* circle1, Circle* zone) {
    circle1->setColor(255, 255, 255);
}

Flag::Flag(){
}

Flag::Flag(bool _flag){
	flag = _flag;
	ACK = false;
}

void Flag::reset(bool _flag){
	flag = _flag; 
	ACK = false;
}

ObjetTemplate::ObjetTemplate() {
	objectType = 0; isVisible = true; character = '#'; r = 255; g = 255; b = 255;
}

void ObjetTemplate::setCharacter(char newCharacter) {
	character = newCharacter;
}

void ObjetTemplate::setVisible(bool visibility) {
	isVisible = visibility;
}

void ObjetTemplate::setColor(int red, int green, int blue) {
	r = red; g = green; b = blue;
}

Circle::Circle() {
}

Circle::Circle(vec2f _pos, vec2f _vel, float _radius) {
	pos = _pos; vel = _vel; radius = _radius;
	objectType = 1;mass = 3.14 * radius * radius; isVisible = true;
}

void Circle::setEntryCallback(void (*foo)(Circle*, Circle*)) {
	zoneEntry = foo;
}

void Circle::setExitCallback(void (*foo)(Circle*, Circle*)) {
	zoneExit = foo;
}

void Circle::setContactCallback(void (*foo)(Circle*, Circle*)) {
	contact = foo;
}

Droite::Droite() {
}

Droite::Droite(vec2f _vec1, vec2f _vec2) {
	vec1 = _vec1; vec2 = _vec2;
	objectType = 2;
}

Segment::Segment(){
}

Segment::Segment(vec2f _vec1, vec2f _vec2){
	vec1 = _vec1; vec2 = _vec2;
	objectType = 3;
}

Event::Event() {
	obj1 = NULL;
	obj2 = NULL;
	eventDate = 0.0f;
	eventType = 0;
	entryDate = 0.0f;
	exitDate = 0.0f;
}

Event::Event(ObjetTemplate* _obj1, ObjetTemplate* _obj2) {
	obj1 = _obj1; obj2 = _obj2;
	eventDate = 1.0f;
	eventType = 0;
    entryDate = 0.0f;
	exitDate = 0.0f;
}

MenuOption::MenuOption(string _optionName, void (*foo)()) {
	optionName = _optionName,
	optionFunc = foo;
}

DEngine::DEngine() {
	map_X1 = 0;
	map_X2 = 600;
	map_Y1 = 0;
	map_Y2 = 300;
}

Circle* DEngine::add_Circle(float x, float y, float vx, float vy, float radius) {
	Circle* circle = new Circle( vec2f(x,y), vec2f(vx,vy), radius);
	circle->setContactCallback(&contact);
	CircleList.push_back(circle);
	return circle;
}

Droite* DEngine::add_Droite(float x1, float y1, float x2, float y2) {
	Droite* droite = new Droite( vec2f(x1,y1), vec2f(x2,y2));
	DroiteList.push_back(droite);
	return droite;
}

Segment* DEngine::add_Segment(float x1, float y1, float x2, float y2) {
	Segment* segment = new Segment( vec2f(x1,y1), vec2f(x2,y2));
	SegmentList.push_back(segment);
	return segment;
}

Circle* DEngine::add_Zone(float x, float y, float radius, void (*entry)(Circle*, Circle*)) {
    Circle* zone = new Circle( vec2f(x,y), vec2f(0,0), radius);
    zone->setEntryCallback(entry);
    zone->setExitCallback(&exit);
    zone->setColor(255,0,255);
    ZoneList.push_back(zone);
	return zone;
}

void DEngine::rmv_Circle(Circle* circle) {
    for(int i=0 ; i<CircleList.size() ; i++) {
        if(CircleList[i] == circle) {
            CircleList.erase(CircleList.begin()+i);
            i--;
        }
    }
    for(int i=0 ; i<ZoneList.size() ; i++) {
        for(int j=0 ; j<ZoneList[i]->zoneInteraction.size() ; j++) {
            if(ZoneList[i]->zoneInteraction[j] == circle) {
                ZoneList[i]->zoneInteraction.erase(ZoneList[i]->zoneInteraction.begin()+j);
                j--;
            }
        }
    }
}

void DEngine::rmv_Droite(Droite* droite) {
    for(int i=0 ; i<DroiteList.size() ; i++) {
        if(DroiteList[i] == droite) {
            DroiteList.erase(DroiteList.begin()+i);
            i--;
        }
    }
}

void DEngine::rmv_Segment(Segment* segment) {
    for(int i=0 ; i<SegmentList.size() ; i++) {
        if(SegmentList[i] == segment) {
            SegmentList.erase(SegmentList.begin()+i);
            i--;
        }
    }
}

void DEngine::rmv_Zone(Circle* zone) {
    for(int i=0 ; i<ZoneList.size() ; i++) {
        if(ZoneList[i] == zone) {
            ZoneList.erase(ZoneList.begin()+i);
            i--;
        }
    }
}

Event DEngine::getEarliestEvent(float timeStep) {
	Event earliestEvent;
	earliestEvent.eventDate = timeStep;
	for(int i=0 ; i<CircleList.size() ; i++) {
		for(int j=i+1 ; j<CircleList.size() ; j++) {
			Event event = continuousCircleCircleCollision(CircleList[i], CircleList[j]);
			if(event.eventDate < earliestEvent.eventDate) {
				earliestEvent = event;
			}
		}
		for(int j=0 ; j<DroiteList.size() ; j++) {
			Event event = continuousCircleDroiteCollision(CircleList[i], DroiteList[j]);
			if(event.eventDate < earliestEvent.eventDate) {
				earliestEvent = event;
			}
		}
		for(int j=0 ; j<SegmentList.size() ; j++) {
			Event event = continuousCircleSegmentCollision(CircleList[i], SegmentList[j]);
			if(event.eventDate < earliestEvent.eventDate) {
				earliestEvent = event;
			}
		}
        for(int j=0 ; j<ZoneList.size() ; j++) {
			Event event = continuousCircleZoneTraversal(CircleList[i], ZoneList[j]);
			if(event.eventDate < earliestEvent.eventDate) {
				earliestEvent = event;
			}
		}
	}
	return earliestEvent;
}

Event DEngine::continuousCircleCircleCollision(Circle* circle1, Circle* circle2) {
	Event newEvent((ObjetTemplate*)circle1, (ObjetTemplate*)circle2);
	vec2f relativeSpeed = circle1->vel - circle2->vel;
	if(relativeSpeed.length() > 0) {
		vec2f P = closestPointOnLine(circle1->pos, circle1->pos + relativeSpeed, circle2->pos);
		vec2f distanceVector = (P - circle1->pos);
		if((P - circle1->pos).dot(relativeSpeed) >= 0) {
			if ((P - circle2->pos).length() <= circle1->radius + circle2->radius) {
				float truc = sqrt((circle1->radius + circle2->radius)*(circle1->radius + circle2->radius) - (P - circle2->pos).length()*(P - circle2->pos).length());
				float distanceBeforeCollision = distanceVector.length() - truc;
				float tempsBeforeCollision = distanceBeforeCollision / relativeSpeed.length();
				if(tempsBeforeCollision < 0) {
                     tempsBeforeCollision = 0;
                }
				newEvent.eventDate = tempsBeforeCollision;
				newEvent.eventType = 1;
			}
		}
	}
	return newEvent;
}

Event DEngine::continuousCircleDroiteCollision(Circle* circle1, Droite* droite1) {
	Event newEvent((ObjetTemplate*)circle1, (ObjetTemplate*)droite1);
	vec2f* crossingPoint = checklinescollide(new Droite(circle1->pos, circle1->pos + circle1->vel), droite1);
	if(crossingPoint != NULL) {
		vec2f a = *crossingPoint;
		vec2f p1 = closestPointOnLine(droite1->vec1, droite1->vec2, circle1->pos);
		if((p1 - circle1->pos).dot(circle1->vel) > 0) {
            if((p1 - circle1->pos).length() < circle1->radius) {
                newEvent.eventDate = 0;
                newEvent.eventType = 2;
            } else {
                vec2f normalVel = circle1->vel;
                normalVel.normalize();
                vec2f temp1 = normalVel * ((a-circle1->pos).length()/(p1-circle1->pos).length());
                vec2f temp2 = temp1 * circle1->radius;
                vec2f p2 = a - temp2;
                newEvent.eventDate = (circle1->pos - p2).length() / (circle1->vel).length();
                newEvent.eventType = 2;
            }
		}
	}
	return newEvent;
}

Event DEngine::continuousCircleSegmentCollision(Circle* circle1, Segment* segment1) {
	Event newEvent((ObjetTemplate*)circle1, (ObjetTemplate*)segment1);
    vec2f closestPoint = closestPointOnSegment(segment1->vec1, segment1->vec2, circle1->pos);
    if((closestPoint - circle1->pos).dot(circle1->vel) > 0) {
        if((closestPoint - circle1->pos).length() < circle1->radius) {
            newEvent.eventDate = 0;
            newEvent.eventType = 3;
        } else {
            vec2f projection = closestPointOnLine(segment1->vec1, segment1->vec2, circle1->pos);
            //if((projection-circle1->pos).length() > circle1->radius && (projection.x != circle1->pos.x || projection.y != circle1->pos.y))
            //{
            vec2f temp1 = projection - circle1->pos;
            temp1.normalize();
            temp1 *= circle1->radius;
            vec2f contactPoint = circle1->pos + temp1;
            vec2f* crossPoint = checklinescollide(new Droite(contactPoint, contactPoint + circle1->vel), new Droite(segment1->vec1, segment1->vec2));
            if(crossPoint != NULL && ((crossPoint->x > min(segment1->vec1.x, segment1->vec2.x) && crossPoint->x < max(segment1->vec1.x, segment1->vec2.x))
            || (crossPoint->y > min(segment1->vec1.y, segment1->vec2.y) && crossPoint->y < max(segment1->vec1.y, segment1->vec2.y))))
            {
                float temp2 = (contactPoint - *crossPoint).length();
                newEvent.eventDate = temp2/(circle1->vel).length();
                newEvent.eventType = 3;
            }
            //}
            else {
                vec2f proj1 = closestPointOnLine(circle1->pos, circle1->pos + circle1->vel, segment1->vec1);
                vec2f proj2 = closestPointOnLine(circle1->pos, circle1->pos + circle1->vel, segment1->vec2);
                if((proj1 - segment1->vec1).length() < (proj2 - segment1->vec2).length()) {
                    if((proj1 - segment1->vec1).length() < circle1->radius) {
                        Event tempEvent = continuousCircleCircleCollision(circle1, new Circle(segment1->vec1, vec2f(0,0), 0));
                        newEvent.eventDate = tempEvent.eventDate;
                        newEvent.eventType = 3;
                    }
                } else {
                    if((proj2 - segment1->vec2).length() < circle1->radius) {
                        Event tempEvent = continuousCircleCircleCollision(circle1, new Circle(segment1->vec2, vec2f(0,0), 0));
                        newEvent.eventDate = tempEvent.eventDate;
                        newEvent.eventType = 3;
                    }
                }
            }
        }
    }
	return newEvent;
}

Event DEngine::continuousCircleZoneTraversal(Circle* circle1, Circle* zone) {
 	Event newEvent((ObjetTemplate*)circle1, (ObjetTemplate*)zone);
 	circle1 = new Circle(circle1->pos, circle1->vel, 0);
 	bool isPresentInList = false;
 	for(int i=0 ; i<zone->zoneInteraction.size() ; i++) {
        if(zone->zoneInteraction[i] == circle1) isPresentInList = true;
    }
    if(isPresentInList) {
        if((circle1->pos - zone->pos).length() > circle1->radius + zone->radius + 0.1) {
            newEvent.eventType = 5;
            newEvent.eventDate = 0.0f;
        }
        else if(circle1->vel.length() > 0) {
            vec2f P = closestPointOnLine(circle1->pos, circle1->pos + circle1->vel, zone->pos);
            vec2f distanceVector = (P - circle1->pos);
            float truc1 = sqrt((circle1->radius + zone->radius)*(circle1->radius + zone->radius) - (P - zone->pos).length()*(P - zone->pos).length());
            float truc2 = (P - circle1->pos).length();
            float distanceBeforeExit;
            if((P - circle1->pos).dot(circle1->vel) > 0) distanceBeforeExit = truc1 + truc2;
            else distanceBeforeExit = truc1 - truc2;
            float tempsBeforeExit = distanceBeforeExit / circle1->vel.length();
            newEvent.eventDate = tempsBeforeExit;
            newEvent.eventType = 5;
        }
    } else {
        if((circle1->pos - zone->pos).length() < circle1->radius + zone->radius - 0.1) {
            newEvent.eventType = 4;
            newEvent.eventDate = 0.0f;
        }
        else if(circle1->vel.length() > 0) {
            vec2f P = closestPointOnLine(circle1->pos, circle1->pos + circle1->vel, zone->pos);
            vec2f distanceVector = (P - circle1->pos);
            if((P - circle1->pos).dot(circle1->vel) > 0) {
                if ((P - zone->pos).length() <= circle1->radius + zone->radius) {
                    float truc = sqrt((circle1->radius + zone->radius)*(circle1->radius + zone->radius) - (P - zone->pos).length()*(P - zone->pos).length());
                    float distanceBeforeEntry = distanceVector.length() - truc;
                    float tempsBeforeEntry = distanceBeforeEntry / circle1->vel.length();
                    newEvent.eventDate = tempsBeforeEntry;
                    newEvent.eventType = 4;
                }
            }
        }
    }

	return newEvent;
}

vec2f* DEngine::checklinescollide(Droite* droite1, Droite* droite2) {
	float A1 = droite1->vec2.y - droite1->vec1.y;
	float B1 = droite1->vec1.x - droite1->vec2.x;
	float C1 = A1*droite1->vec1.x + B1*droite1->vec1.y;

	float A2 = droite2->vec2.y - droite2->vec1.y;
	float B2 = droite2->vec1.x - droite2->vec2.x;
	float C2 = A2*droite2->vec1.x + B2*droite2->vec1.y;
	float det = A1*B2-A2*B1;
	if(det != 0){
				float x = (B2*C1 - B1*C2)/det;
				float y = (A1*C2 - A2*C1)/det;
				return new vec2f(x, y);
	}
	return NULL;
}

vec2f DEngine::closestPointOnLine(vec2f l1, vec2f l2, vec2f p) {
    float A1 = l2.y - l1.y;
    float B1 = l1.x - l2.x;
    double C1 = (l2.y - l1.y)*l1.x + (l1.x - l2.x)*l1.y;
    double C2 = -B1*p.x + A1*p.y;
    double det = A1*A1 - -B1*B1;
    double cx = 0;
    double cy = 0;
    if(det != 0){
        cx = (float)((A1*C1 - B1*C2)/det);
        cy = (float)((A1*C2 - -B1*C1)/det);
    }else{
        cx = p.x;
        cy = p.y;
    }
    return vec2f(cx, cy);
}

vec2f DEngine::closestPointOnSegment(vec2f l1, vec2f l2, vec2f p) {
    float A1 = l2.y - l1.y;
    float B1 = l1.x - l2.x;
    double C1 = (l2.y - l1.y)*l1.x + (l1.x - l2.x)*l1.y;
    double C2 = -B1*p.x + A1*p.y;
    double det = A1*A1 - -B1*B1;
    double cx = 0;
    double cy = 0;
    if(det != 0){
        cx = (float)((A1*C1 - B1*C2)/det);
        cy = (float)((A1*C2 - -B1*C1)/det);
    }else{
        cx = p.x;
        cy = p.y;
    }
    if(cx<l1.x && cx<l2.x) cx = min(l1.x,l2.x);
    if(cx>l1.x && cx>l2.x) cx = max(l1.x,l2.x);
    if(cy<l1.y && cy<l2.y) cy = min(l1.y,l2.y);
    if(cy>l1.y && cy>l2.y) cy = max(l1.y,l2.y);
    return vec2f(cx, cy);
}

void DEngine::step(Mat* renderer) {
	float timeLeft = 1.0f;
	while(timeLeft > 0.0001f)
	{
		Event earliestEvent = getEarliestEvent(timeLeft);
		for(int i=0 ; i<CircleList.size() ; i++)
		{
			vec2f temp = ((CircleList[i]->vel) * earliestEvent.eventDate);
			CircleList[i]->pos += temp;
		}
		if(earliestEvent.eventType == 1)
		{
			Circle* circle1 = ((Circle*)(earliestEvent.obj1));
			Circle* circle2 = ((Circle*)(earliestEvent.obj2));
			vec2f n = circle1->pos - circle2->pos;
			n.normalize();
			float a1 = circle1->vel.dot(n);
			float a2 = circle2->vel.dot(n);
			float optimizedP = (2.0 * (a1 - a2)) / (circle1->mass + circle2->mass);
			vec2f v1tt = n* (optimizedP * circle2->mass);
			vec2f v1t = circle1->vel - v1tt;
			vec2f v2tt = n* (optimizedP * circle1->mass);
			vec2f v2t = circle2->vel + v2tt;

			//For recording purposes
			/*Mat window_copy = renderer->clone();
			displayOpenCV(&window_copy);
			line(window_copy, Point(circle1->pos.x, circle1->pos.y), Point(circle1->pos.x+(v1t.x*20), circle1->pos.y+(v1t.y*20)), Scalar(255, 255, 255), 2, 8, 0);
			line(window_copy, Point(circle2->pos.x, circle2->pos.y), Point(circle2->pos.x+(v2t.x*20), circle2->pos.y+(v2t.y*20)), Scalar(255, 255, 255), 2, 8, 0);
			imshow( "lol", window_copy );
			waitKey(200);
			for(int i=0 ; i<5 ; i++) {
				outputvid << window_copy;
			}
			displayOpenCV(&window_copy);
			imshow( "lol", window_copy );
			for(int i=0 ; i<5 ; i++) {
				outputvid << window_copy;
			}
			waitKey(100);*/

			circle1->vel = v1t + (rand()%10 + 1)*0.001;
			circle2->vel = v2t + (rand()%10 + 1)*0.001;
			circle2->contact(circle1, circle2);

			
		}
		if(earliestEvent.eventType == 2)
		{
			Circle* circle1 = ((Circle*)(earliestEvent.obj1));
			Droite* droite1 = ((Droite*)(earliestEvent.obj2));
			vec2f closestPoint = closestPointOnLine(droite1->vec1, droite1->vec2, circle1->pos);
			vec2f temp1 = circle1->pos - closestPoint;
			vec2f temp2 = temp1.normal() * (-2 * circle1->vel.dot(temp1.normal()))*0.9;
			circle1->vel += temp2;
		}
        if(earliestEvent.eventType == 3)
		{
			Circle* circle1 = ((Circle*)(earliestEvent.obj1));
			Segment* segment1 = ((Segment*)(earliestEvent.obj2));
			vec2f closestPoint = closestPointOnSegment(segment1->vec1, segment1->vec2, circle1->pos);
			vec2f temp1 = circle1->pos - closestPoint;
			vec2f temp2 = temp1.normal() * (-2 * circle1->vel.dot(temp1.normal()))*0.9;
			circle1->vel += temp2;
		}
        if(earliestEvent.eventType == 4)
		{
			Circle* circle1 = ((Circle*)(earliestEvent.obj1));
			Circle* zone = ((Circle*)(earliestEvent.obj2));
			zone->zoneEntry(circle1, zone);
            zone->zoneInteraction.push_back(circle1);
		}
        if(earliestEvent.eventType == 5)
		{
			Circle* circle1 = ((Circle*)(earliestEvent.obj1));
			Circle* zone = ((Circle*)(earliestEvent.obj2));
			zone->zoneExit(circle1, zone);
            for(int i=0 ; i<zone->zoneInteraction.size() ; i++)
            {
                if(zone->zoneInteraction[i] == circle1) zone->zoneInteraction.erase(zone->zoneInteraction.begin()+i);
            }
		}
		timeLeft -= earliestEvent.eventDate;
	}
}

void DEngine::drawCircle(Mat* renderer, Circle* circle1, bool filled) {	
	int intfilled = 2;
	if(filled == true) intfilled = -1;
    circle(*renderer, Point(circle1->pos.x, circle1->pos.y), circle1->radius, Scalar(circle1->b, circle1->g, circle1->r), intfilled, 8, 0);
}

void DEngine::setCircleTexture(Mat* renderer, int i, char* path)
{
}

void DEngine::displayOpenCV(Mat* renderer) {
    for(int i=0 ; i<BackGroundList.size() ; i++)
	{
        //SDL_RenderCopy( renderer, BackGroundList[i], NULL, NULL );
	}

    for(int i=0 ; i<CircleList.size() ; i++)
	{
		if(CircleList[i]->objectType == 1 && CircleList[i]->isVisible)
		{
			if(CircleList[i]->objTexture.rows != 0) {
				overlayImage(renderer, &(CircleList[i]->objTexture), Point(CircleList[i]->pos.x-CircleList[i]->radius, CircleList[i]->pos.y-CircleList[i]->radius));
			} else {
            	drawCircle(renderer, CircleList[i], false);
			}
		}
	}
	for(int i=0 ; i<DroiteList.size() ; i++)
	{
		if(DroiteList[i]->isVisible)
		{
			line(*renderer, Point(DroiteList[i]->vec1.x, DroiteList[i]->vec1.y), Point(DroiteList[i]->vec2.x, DroiteList[i]->vec2.y), Scalar( 255, 255, 255 ), 2, 8, 0 );
		}
	}
    for(int i=0 ; i<SegmentList.size() ; i++)
	{
		if(SegmentList[i]->isVisible)
		{
			line(*renderer, Point(SegmentList[i]->vec1.x, SegmentList[i]->vec1.y), Point(SegmentList[i]->vec2.x, SegmentList[i]->vec2.y), Scalar( 255, 255, 255 ), 2, 8, 0 );
		}
	}
    for(int i=0 ; i<ZoneList.size() ; i++)
	{
		if(ZoneList[i]->isVisible)
		{
		    drawCircle(renderer, ZoneList[i], false);
		}
	}
}

void create_pool(DEngine* game)
{
    float mapx;
    float mapy;
    if(WINDOW_HEIGHT < WINDOW_WIDTH/2)
    {
        mapy = WINDOW_HEIGHT;
        mapx = mapy*2;
    } else {
        mapx = WINDOW_WIDTH;
        mapy = WINDOW_WIDTH/2;
    }
    float cornerHoleInclinaison = 0.2;
    float middleHoleInclinaison = 0.15;

    game->add_Droite(0, 0, mapx-1, 0);
    game->add_Droite(mapx-1, 0, mapx-1, mapy-1);
	game->add_Droite(mapx-1, mapy-1, 0, mapy-1);
	game->add_Droite(0, mapy-1, 0, 0);

    game->add_Segment(mapx*(0.68/8), mapy*(0.35/4), mapx/2 - mapx*(0.23/8), mapy*(0.35/4));
    game->add_Segment(mapx/2 - mapx*(0.23/8), mapy*(0.35/4), mapx/2-mapx*(middleHoleInclinaison/8), 0);
    game->add_Segment(mapx/2 + mapx*(0.23/8), mapy*(0.35/4), mapx/2+mapx*(middleHoleInclinaison/8), 0);
    game->add_Segment(mapx/2 + mapx*(0.23/8), mapy*(0.35/4), mapx - mapx*(0.68/8), mapy*(0.35/4));

    game->add_Segment(mapx - mapx*(0.68/8), mapy*(0.35/4), mapx - mapx*(cornerHoleInclinaison/8), 0);
    game->add_Segment(mapx - mapx*(0.35/8), mapy*(0.60/4), mapx, mapy*(cornerHoleInclinaison/4));
    game->add_Segment(mapx - mapx*(0.35/8), mapy*(0.60/4), mapx - mapx*(0.35/8), mapy - mapy*(0.60/4));
    game->add_Segment(mapx - mapx*(0.35/8), mapy - mapy*(0.60/4), mapx, mapy - mapy*(cornerHoleInclinaison/4));
    game->add_Segment(mapx - mapx*(0.68/8), mapy - mapy*(0.35/4), mapx - mapx*(cornerHoleInclinaison/8), mapy);

    game->add_Segment(mapx/2 + mapx*(0.23/8), mapy - mapy*(0.35/4), mapx - mapx*(0.68/8), mapy - mapy*(0.35/4));
    game->add_Segment(mapx/2 - mapx*(0.23/8), mapy - mapy*(0.35/4), mapx/2-mapx*(middleHoleInclinaison/8), mapy);
    game->add_Segment(mapx/2 + mapx*(0.23/8), mapy - mapy*(0.35/4), mapx/2+mapx*(middleHoleInclinaison/8), mapy);
    game->add_Segment(mapx*(0.68/8), mapy - mapy*(0.35/4), mapx/2 - mapx*(0.23/8), mapy - mapy*(0.35/4));

    game->add_Segment(mapx*(0.68/8), mapy - mapy*(0.35/4), mapx*(cornerHoleInclinaison/8), mapy);
    game->add_Segment(mapx*(0.35/8), mapy - mapy*(0.60/4), 0, mapy - mapy*(cornerHoleInclinaison/4));
    game->add_Segment(mapx*(0.35/8), mapy*(0.60/4), mapx*(0.35/8), mapy - mapy*(0.60/4));
    game->add_Segment(mapx*(0.35/8), mapy*(0.60/4), 0, mapy*(cornerHoleInclinaison/4));
    game->add_Segment(mapx*(0.68/8), mapy*(0.35/4), mapx*(cornerHoleInclinaison/8), 0);

    game->add_Zone(mapx*(0.1/8), 0, mapx*(0.55/8), entryHole);
    game->add_Zone(mapx/2, 0, mapx*(0.35/8), entryHole);
    game->add_Zone(mapx - mapx*(0.1/8), 0, mapx*(0.55/8), entryHole);
    game->add_Zone(mapx - mapx*(0.1/8), mapy, mapx*(0.55/8), entryHole);
    game->add_Zone(mapx/2, mapy, mapx*(0.35/8), entryHole);
    game->add_Zone(mapx*(0.1/8), mapy, mapx*(0.55/8), entryHole);

	/*game->add_Zone(mapx*(0.1/8), 0, mapx*(0.55/8), entryBlue);
    game->add_Zone(mapx/2, 0, mapx*(0.35/8), entryBlue);
    game->add_Zone(mapx - mapx*(0.1/8), 0, mapx*(0.55/8), entryBlue);
    game->add_Zone(mapx - mapx*(0.1/8), mapy, mapx*(0.55/8), entryBlue);
    game->add_Zone(mapx/2, mapy, mapx*(0.35/8), entryBlue);
    game->add_Zone(mapx*(0.1/8), mapy, mapx*(0.55/8), entryBlue);*/

    vec2f rackPlacement(mapx*(5.8/8), mapy/2);
    float ecart = sqrt(mapy*(0.18/4)*mapy*(0.18/4) - mapy*(0.09/4)*mapy*(0.09/4));


    Circle* tempc;
	Mat temp_texture;
    tempc = game->add_Circle(mapx*(2.18/8), mapy/2, 0, 0, mapy*(0.09/4));
	temp_texture = imread("../Assets/pool_ball_white.png", -1);
	resize(temp_texture, tempc->objTexture, Size(mapy*(0.09/4)*2, mapy*(0.09/4)*2), 0, 0, INTER_LINEAR);
	tempc->tags += "WHITE ";
	tempc->setColor(255,255,255);
    tempc = game->add_Circle(rackPlacement.x, rackPlacement.y, 0, 0, mapy*(0.09/4));
	temp_texture = imread("../Assets/pool_ball_1.png", -1);
	resize(temp_texture, tempc->objTexture, Size(mapy*(0.09/4)*2, mapy*(0.09/4)*2), 0, 0, INTER_LINEAR);
    tempc->tags += "RED ";
	tempc->setColor(255,0,0);

    tempc = game->add_Circle(rackPlacement.x + ecart, rackPlacement.y + mapy*(0.09/4), 0, 0, mapy*(0.09/4));
	temp_texture = imread("../Assets/pool_ball_10.png", -1);
	resize(temp_texture, tempc->objTexture, Size(mapy*(0.09/4)*2, mapy*(0.09/4)*2), 0, 0, INTER_LINEAR);
    tempc->tags += "RED ";
	tempc->setColor(255,0,0);
    tempc = game->add_Circle(rackPlacement.x + ecart, rackPlacement.y - mapy*(0.09/4), 0, 0, mapy*(0.09/4));
	temp_texture = imread("../Assets/pool_ball_11.png", -1);
	resize(temp_texture, tempc->objTexture, Size(mapy*(0.09/4)*2, mapy*(0.09/4)*2), 0, 0, INTER_LINEAR);
    tempc->tags += "YELLOW ";
	tempc->setColor(255,255,0);

    tempc = game->add_Circle(rackPlacement.x + 2*ecart, rackPlacement.y + mapy*(0.18/4), 0, 0, mapy*(0.09/4));
	temp_texture = imread("../Assets/pool_ball_6.png", -1);
	resize(temp_texture, tempc->objTexture, Size(mapy*(0.09/4)*2, mapy*(0.09/4)*2), 0, 0, INTER_LINEAR);
    tempc->tags += "YELLOW ";
	tempc->setColor(255,255,0);
    tempc = game->add_Circle(rackPlacement.x + 2*ecart, rackPlacement.y, 0, 0, mapy*(0.09/4));
	temp_texture = imread("../Assets/pool_ball_8.png", -1);
	resize(temp_texture, tempc->objTexture, Size(mapy*(0.09/4)*2, mapy*(0.09/4)*2), 0, 0, INTER_LINEAR);
    tempc->tags += "BLACK ";
	tempc->setColor(125,125,125);
    tempc = game->add_Circle(rackPlacement.x + 2*ecart, rackPlacement.y - mapy*(0.18/4), 0, 0, mapy*(0.09/4));
	temp_texture = imread("../Assets/pool_ball_9.png", -1);
	resize(temp_texture, tempc->objTexture, Size(mapy*(0.09/4)*2, mapy*(0.09/4)*2), 0, 0, INTER_LINEAR);
    tempc->tags += "RED ";
	tempc->setColor(255,0,0);

    tempc = game->add_Circle(rackPlacement.x + 3*ecart, rackPlacement.y + 3*mapy*(0.09/4), 0, 0, mapy*(0.09/4));
	temp_texture = imread("../Assets/pool_ball_13.png", -1);
	resize(temp_texture, tempc->objTexture, Size(mapy*(0.09/4)*2, mapy*(0.09/4)*2), 0, 0, INTER_LINEAR);
    tempc->tags += "RED ";
	tempc->setColor(255,0,0);
    tempc = game->add_Circle(rackPlacement.x + 3*ecart, rackPlacement.y + mapy*(0.09/4), 0, 0, mapy*(0.09/4));
	temp_texture = imread("../Assets/pool_ball_7.png", -1);
	resize(temp_texture, tempc->objTexture, Size(mapy*(0.09/4)*2, mapy*(0.09/4)*2), 0, 0, INTER_LINEAR);
    tempc->tags += "YELLOW ";
	tempc->setColor(255,255,0);
    tempc = game->add_Circle(rackPlacement.x + 3*ecart, rackPlacement.y - mapy*(0.09/4), 0, 0, mapy*(0.09/4));
	temp_texture = imread("../Assets/pool_ball_12.png", -1);
	resize(temp_texture, tempc->objTexture, Size(mapy*(0.09/4)*2, mapy*(0.09/4)*2), 0, 0, INTER_LINEAR);
    tempc->tags += "RED ";
	tempc->setColor(255,0,0);
    tempc = game->add_Circle(rackPlacement.x + 3*ecart, rackPlacement.y - 3*mapy*(0.09/4), 0, 0, mapy*(0.09/4));
	temp_texture = imread("../Assets/pool_ball_5.png", -1);
	resize(temp_texture, tempc->objTexture, Size(mapy*(0.09/4)*2, mapy*(0.09/4)*2), 0, 0, INTER_LINEAR);
    tempc->tags += "YELLOW ";
	tempc->setColor(255,255,0);

    tempc = game->add_Circle(rackPlacement.x + 4*ecart, rackPlacement.y + 2*mapy*(0.18/4), 0, 0, mapy*(0.09/4));
	temp_texture = imread("../Assets/pool_ball_4.png", -1);
	resize(temp_texture, tempc->objTexture, Size(mapy*(0.09/4)*2, mapy*(0.09/4)*2), 0, 0, INTER_LINEAR);
    tempc->tags += "YELLOW ";
	tempc->setColor(255,255,0);
    tempc = game->add_Circle(rackPlacement.x + 4*ecart, rackPlacement.y + mapy*(0.18/4), 0, 0, mapy*(0.09/4));
	temp_texture = imread("../Assets/pool_ball_3.png", -1);
	resize(temp_texture, tempc->objTexture, Size(mapy*(0.09/4)*2, mapy*(0.09/4)*2), 0, 0, INTER_LINEAR);
    tempc->tags += "YELLOW ";
	tempc->setColor(255,255,0);
    tempc = game->add_Circle(rackPlacement.x + 4*ecart, rackPlacement.y, 0, 0, mapy*(0.09/4));
	temp_texture = imread("../Assets/pool_ball_14.png", -1);
	resize(temp_texture, tempc->objTexture, Size(mapy*(0.09/4)*2, mapy*(0.09/4)*2), 0, 0, INTER_LINEAR);
    tempc->tags += "RED ";
	tempc->setColor(255,0,0);
    tempc = game->add_Circle(rackPlacement.x + 4*ecart, rackPlacement.y - mapy*(0.18/4), 0, 0, mapy*(0.09/4));
	temp_texture = imread("../Assets/pool_ball_2.png", -1);
	resize(temp_texture, tempc->objTexture, Size(mapy*(0.09/4)*2, mapy*(0.09/4)*2), 0, 0, INTER_LINEAR);
    tempc->tags += "YELLOW ";
	tempc->setColor(255,255,0);
    tempc = game->add_Circle(rackPlacement.x + 4*ecart, rackPlacement.y - 2*mapy*(0.18/4), 0, 0, mapy*(0.09/4));
	temp_texture = imread("../Assets/pool_ball_15.png", -1);
	resize(temp_texture, tempc->objTexture, Size(mapy*(0.09/4)*2, mapy*(0.09/4)*2), 0, 0, INTER_LINEAR);
    tempc->tags += "RED ";
	tempc->setColor(255,0,0);

}

void DEngine::run_until_nospeed(Mat* renderer) {
    bool end = false;
	float totalspeed = 1;
	while(!end && totalspeed > 0)
	{
		totalspeed = 0;
		step(renderer);
		Mat window_copy = renderer->clone();
		step(&window_copy);
		displayOpenCV(&window_copy);
		imshow( "lol", window_copy );
		for(int i=0 ; i<CircleList.size() ; i++)
		{
			CircleList[i]->vel *= 0.98;
			float currentSpeed = CircleList[i]->vel.length();
			if(currentSpeed < 0.1f)  CircleList[i]->vel.set(0,0);
			totalspeed += currentSpeed;
		}
		waitKey(30);
		//outputvid << window_copy;
	}
}

void CallBackMouse(int event, int x, int y, int flags, void* userdata) 
{
	if(event==EVENT_MOUSEMOVE) {
		mouse_x = x;
		mouse_y = y;
	} else if (event==EVENT_LBUTTONDOWN) {
		mouse_x = x;
		mouse_y = y;
		mouse_click = true;
	}
}

int main(int argc, char **argv)
{	
	//init
	DEngine game(0,WINDOW_WIDTH,0, WINDOW_HEIGHT);
	namedWindow("lol", 1);
	setMouseCallback("lol", CallBackMouse, NULL);
	game_ptr = &game;

	//Loading of assets and creation of the table
	Mat window(WINDOW_HEIGHT, WINDOW_WIDTH, CV_8UC3);
	Mat background = imread("../Assets/PoolTableReferenceTop.png");
	Mat resized_background;
	resize(background, resized_background, Size(800, 400), 0, 0, INTER_LINEAR);
	addWeighted(window, 0.0, resized_background, 1.0, 0.0, window);
	create_pool(&game);

	//Main loop
	int number_of_turns = 0;
	while(number_of_turns < 1) {
		//Waiting for user inputs
		mouse_click = false;
		float dx = 0;
		float dy = 0;
		while(mouse_click != true) {
			dx = (game.CircleList[0]->pos.x - mouse_x)/10.0;
			dy = (game.CircleList[0]->pos.y - mouse_y)/10.0;
			Mat window_copy = window.clone();
			game.displayOpenCV(&window_copy);
			line(window_copy, Point(game.CircleList[0]->pos.x, game.CircleList[0]->pos.y), Point(mouse_x, mouse_y), Scalar(255, 255, 255), 1, 8, 0);
			imshow( "lol", window_copy );
			Mat lolilol;
			window_copy.convertTo(lolilol, CV_8UC3);
			//For recording purposes
			//outputvid << lolilol;
			waitKey(30);
		}
		//Do the physics stuff
		game.CircleList[0]->vel.x += dx;
		game.CircleList[0]->vel.y += dy;
		game.run_until_nospeed(&window);
		number_of_turns++;
	}
	return 0;
}
