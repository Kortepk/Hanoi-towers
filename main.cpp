#include <iostream>
#include <SFML/Graphics.hpp> //Библиотека SFML;
#include <stack>
#include <cmath>
#define ANIM_TIME 1.5
#define RSVERTEX 40

using namespace std;
using namespace sf; // Пространство имён SFML;
unsigned short WIDTH = 1500, HEIGHT = 800, SCENE = 0, ELEM, ANIM_MODE = 0, CHOOSE=0, CH_DSK=0; // CHOOSE - выбор башни, CH_DSK - выбор диска
double SHIFT_X = 0, SPEED_X = 0, AX = 0, T1 = 0; // сдвиг для анимации
bool CLICKED_INPUT = -1; // -1 не кликнуто, 0 - инпут на сцене 0
string ELEM_TEXT;
bool CLICK = false;
short PREV_BUT = -1;

Text TEXT_WHITE;
Text TEXT_BLACK;
Text TEXT_CONG;
Font MAIN_FONT;
Clock ANIM_CLOCK, FRAME, GAMETIME; // Время для анимации

RenderWindow window(VideoMode(WIDTH, HEIGHT), "Algorithm", Style::Default, ContextSettings(0,0,8));
RectangleShape CLICK_CURSOR(Vector2f(1, 1)); 
RectangleShape MOVE_CURSOR(Vector2f(1, 1)); 

VertexArray ROUNSQ(TriangleFan, RSVERTEX + 2);

void Scene1(double shfx);


double Radiusxy(Vector2f v1, Vector2f v2);


//__________________________________________________________________________
//==========================================================================
//__________________________________________________________________________

class RoundShape : public sf::Shape
{
public :

    explicit RoundShape(const sf::Vector2f& sz = sf::Vector2f(0.f, 0.f)) :
    size(sz)
    {
        update();
    }
    
    void setPosition(const sf::Vector2f& xy)
    {
        pos = xy;
        update();
    }

    void setRadius(const float radius)
    {
        rad = radius;
        update();
    }
	
	/*void setOutlineThickness(float thickness)	
	{
		Shape::setOutlineThickness(thickness);
		update();
	}
	
	void setFillColor(Color clr)
	{
		Shape::setFillColor(clr);
		update();
	}
	
	void setOutlineColor(const Color &color)	
	{
		Shape::setOutlineColor(color);
		update();
	}
	*/
	void setSize(const sf::Vector2f& sz)
	{
		size = sz;
		update();
	}
	
    virtual std::size_t getPointCount() const
    {
        return RSVERTEX; // fixed, but could be an attribute of the class if needed
    }

    virtual sf::Vector2f getPoint(std::size_t index) const
    {    	
        static const float pi = 3.141592654f;

        float angle = index * 2 * pi / getPointCount() - pi / 2;
        float x = std::cos(angle) * rad;
        float y = std::sin(angle) * rad;
				
		if(index < getPointCount()/4)
		{
        	x += size.x - rad;
        	y += rad;
		}
        else
        {
	        if(index < getPointCount()/2)
			{
				x += size.x - rad;
	        	y += size.y - rad;
			}	
			else
			{
				if(index >= getPointCount()/2 and index < 0.75 * getPointCount())
				{
		        	x += rad;
        			y += size.y - rad;
				}	
				else
				{
					x += rad;
        			y += rad;
				}
			}
		}
		
        return sf::Vector2f(pos.x + x, pos.y + y);
    }

private :

    sf::Vector2f size;
    sf::Vector2f pos;
    float rad = 10;
};

class SdfDisk{
	public:
		Vector2f pos; // Координаты центра фигуры/диска
		Vector2f speed; // Скорости на оси
		double spd;
		double angle;
		Color col; // Цвет бублика
		Vector2f size; // Размер бублика
		Vector2f VectTo;
		short numb; // Какой номер у него
		Clock passt; // Записывает когда началась анимация
		bool started = false; // Стартанула анимация ?
		bool bond = false;
		bool flvt = false;
		
		SdfDisk(const sf::Vector2f& sz = sf::Vector2f(0.f, 0.f), int number = 0) :
	    size(sz), numb(number)
	    { }
	    
	    void SetPosition(const sf::Vector2f& xy)
	    {
	    	pos = xy - size * 0.5f;
	    	
		}
		
		void SetColor(Color& cl)
		{
			col = cl;
		}
		
		void SetSize(const sf::Vector2f& sz)
		{
			size = sz;
		}
		
		void SetNumber(short& nm)
		{
			numb = nm;
		}
		
		void SetSpeed(const sf::Vector2f& spd)
		{
			speed = spd; // + size * 0.5f
		}
		
		Vector2f GetPosition()
		{
			return pos;
		}
		
		void bondMode()
		{
			pos = MOVE_CURSOR.getPosition() - size*0.5f;
			//cout<<"bondage";
		}
		
		void CalcSpeed(Vector2f moveto)
		{
			double angto = atan2(moveto.x, moveto.y); // сосчитать угол
			spd = sqrt(speed.x*speed.x + speed.y*speed.y);
	
			speed.x = -spd * sin(angto);
			speed.y = -spd * cos(angto);
								
			if(Radiusxy(MOVE_CURSOR.getPosition() - size*0.5f, pos) < 1)
			{
				started = false;
				speed = Vector2f(0, 0);
				pos = MOVE_CURSOR.getPosition() - size*0.5f;
				bond = true;	
			}
		}
		
		void SetVectTo(Vector2f moveto)
		{
			flvt = true;
			VectTo = moveto;
		}
};
//__________________________________________________________________________
//==========================================================================
//__________________________________________________________________________


//__________________________________________________________________________
//============Честно спизжено у Вовы :-)=== (02.06.23)======================
//============Лять что за хуета, переделать всё=== (04.06.23)===============
//__________________________________________________________________________

// диск
class disk {   
public:
	int size;
    disk() {};
    disk(int size) { this->size = size; }

    bool operator<= (const disk& other) {
        return (this->size <= other.size);
    }
    bool operator< (const disk& other) {
        return (this->size < other.size);
    }
    
    friend ostream& operator<< (ostream& os, const disk& obj);
};
ostream& operator<< (ostream& os, const disk& obj) {
    os << obj.size;
    return os;
}


// башня, любая из трёх
class Tower : public disk {
public:
	stack <disk> disks;
    Tower() {};
    // добавить диск
    void add(disk disk) {
        if (!disks.empty() and disks.top() <= disk) {
            cout << "Error placing disk " << disk << endl;
        }
        else {
            disks.push(disk);
        }
    }

    // берётся первый диск
    bool remove() {
        if (disks.empty()) {
            cout << "Error removing disk" << endl;
            return false;
        }
        else {
            disks.pop();
			return true;
        }
    }

    // показать состояние башни (вывести диски)
    void show() {
        stack <disk> temp = disks;
        stack <disk> temp_flipped;

        while (!temp.empty()) {
            temp_flipped.push(temp.top());
            temp.pop();
        }

        while (!temp_flipped.empty()) {
            cout << temp_flipped.top() << " ";
            temp_flipped.pop();
        }
        cout << endl;
    }
	
    // проверка, является ли башня пустой
    bool is_empty() { return disks.empty(); }

    disk& top() {
        return disks.top();
    }
};

class Game {
private:
    int num_moves;
public:
	Tower towers[3];
    Game() {
        num_moves = 0;
    }
	
	void start(int num_disks){
        //cout << "Enter the number of disks: ";
        //cin >> num_disks;
        num_moves = 0;
        for (int i = num_disks; i >= 1; i--) {
            towers[0].add(i);
        }
	}
	
	void clear(){
		for (int i = 0; i < 3; i++) 
			while(towers[i].remove()); // Пока тру
	}
	
    bool move(int from, int to) {
        if (!towers[from].is_empty() and (towers[to].is_empty() || towers[from].top() < towers[to].top())) {
            disk disk = towers[from].top();
            towers[from].remove();
            towers[to].add(disk);
            num_moves++;
            cout << "Disk " << disk << " moved from " << from + 1 << " to " << to + 1 << endl;
            show();
            return true;
        }
        else {
            cout << "Invalid move" << endl;
            return false;
        }
    }

    void show() {
        cout << endl;
        for (int i = 0; i < 3; i++) {
            cout << "Tower_" << i + 1 << ":  ";
            towers[i].show();
        }
        cout << endl << endl;
    }


    bool is_finished() {
        return towers[0].is_empty() and towers[1].is_empty();
    }
    int GetNumMoves() {
        return num_moves;
    }
};

//__________________________________________________________________________
//==========================================================================
//__________________________________________________________________________

Game gameplay;


Color GetRainbow(int x){
	x %= 1530; //ограничиваем
	
	int layer = x/255; // узнаём какой сектор
	
	x %= 255; // делаем до цветового диапазона
	
	switch(layer){
		case 0:{
			return Color(255, x, 0);
		}
		case 1:{
			return Color(255 - x, 255, 0);
		}
		case 2:{
			return Color(0, 255, x);
		}
		case 3:{
			return Color(0, 255 - x, 255);
		}
		case 4:{
			return Color(x, 0, 255);
		}
		case 5:{
			return Color(255, 0, 255-x);
		}
	}
}

void Start(){
	MAIN_FONT.loadFromFile("arkhip.ttf");
	
	TEXT_WHITE.setFont(MAIN_FONT);
	TEXT_WHITE.setCharacterSize(20);
	
	TEXT_BLACK.setFont(MAIN_FONT);
	TEXT_BLACK.setCharacterSize(20);
	TEXT_BLACK.setFillColor(Color::Black); 
	
	TEXT_CONG.setFont(MAIN_FONT);
	TEXT_CONG.setCharacterSize(50);
	
	SPEED_X = 1.25 * WIDTH/ANIM_TIME; // 1.25 = 5/4
	T1 = WIDTH/(4 * SPEED_X);
	AX = SPEED_X / T1; // a = v/t1   t1 = s/(4v)   a = 
}

void Scene0(){	
	if(ANIM_MODE == 1) // LEN_ANIM = anim 0.5 + anim 1 + anim 1.5 т.е. по 1/3 времени, но расстояния не 1/3
	{
		float len_anim = WIDTH;
		if(ANIM_CLOCK.getElapsedTime().asSeconds() > ANIM_TIME){ // анимация вышла за время
			SCENE = 1;
			ANIM_MODE = 0;
			GAMETIME.restart();
			SHIFT_X = 0;
		}
		else{
			double dt = FRAME.getElapsedTime().asMicroseconds(), da = ANIM_CLOCK.getElapsedTime().asSeconds();		
			if(ANIM_CLOCK.getElapsedTime().asSeconds() < T1){ 
				SHIFT_X = AX * da*da/2; // at^2 /2
				//cout<<"(";
			}
			else
			{
				if(ANIM_CLOCK.getElapsedTime().asSeconds() > ANIM_TIME - T1){ 
					//cout<<"[";
					SHIFT_X = WIDTH - AX * (ANIM_TIME - da)*(ANIM_TIME - da)/2; 
				}
				else{
					SHIFT_X = AX * T1 * T1/2  + SPEED_X * (da - T1); 
					//cout<<5./8. * WIDTH<<"; ";
				}
			}
			//cout<<da<<" "<<SHIFT_X<<" "<<endl;
		}
	}
	
	
	int wd2 = WIDTH/2, hg2 = HEIGHT/2;
			
	TEXT_WHITE.setString(L"Введите число дисков:");
	TEXT_WHITE.setPosition(wd2 - 110 - SHIFT_X, hg2 - 35);
	window.draw(TEXT_WHITE);
	
	/*TEXT_WHITE.setString(L"Иди нахуй, пидр");
	TEXT_WHITE.setPosition(wd2 + WIDTH - 125 - SHIFT_X , hg2 - 35);
	window.draw(TEXT_WHITE);*/
			
	RectangleShape window_input_0(Vector2f(40, 20));  // Создаем прямоугольник размером 70х100		
	window_input_0.setPosition(wd2 - SHIFT_X , hg2);  
 			
 	window_input_0.setOutlineThickness(2);
 	window_input_0.setOutlineColor(Color::Black);
 	
 	
 	if(window_input_0.getGlobalBounds().intersects(CLICK_CURSOR.getGlobalBounds())){
	 	CLICKED_INPUT = true;
		window_input_0.setFillColor(Color(200, 200, 200));
	}
	else{
		CLICKED_INPUT = false;
		window_input_0.setFillColor(Color::White);
	}
 	
	window.draw(window_input_0);	 //Отрисовка прямоугольника
						
	TEXT_BLACK.setString(ELEM_TEXT);
	TEXT_BLACK.setPosition(wd2 - SHIFT_X , hg2 - 2);
	window.draw(TEXT_BLACK);		
	
	if(ANIM_MODE == 1){
		Scene1(WIDTH - SHIFT_X);
	}
}

/*
void Disk_draw(Vector2f xy, Vector2f size, float rad, Color clr){
	ROUNSQ[0].position = xy + size * 0.5f; // Аналогично было бы, если брал бы xy.x + size.x/2 и тд
	
	window.draw(ROUNSQ);
	//RSVERTEX 
}
*/

void RecalcDisks(SdfDisk* disks, int ind)
{
	int y = HEIGHT - 75 - 2, x = WIDTH/4 + WIDTH/100; // y -= sz + 1;
	float sz = 0;
												
	if(ELEM < 8)
		sz = HEIGHT * 0.1f;//y -= HEIGHT * 0.05f;
	else
	{
		sz = HEIGHT * 0.75f;
		sz /= (ELEM+1);
	}	
	y -= sz/2;
	        
	// Ахтунг! Часть Володиного код
								
	for(int i = 0; i < 3; i++)
	{
		y = HEIGHT - 75 - 2;
		y -= sz/2;
		
	    stack <disk> temp = gameplay.towers[i].disks;
	    stack <disk> temp_flipped;
	
	    while (!temp.empty()) {
	    	temp_flipped.push(temp.top());
	        temp.pop();
	    }
	
	    while (!temp_flipped.empty()) {
	    	disks[temp_flipped.top().size-1].SetSize(Vector2f((temp_flipped.top().size) * WIDTH/(4 * ELEM) + 20, sz));
	    	if(ind == -1 or ind == (temp_flipped.top().size-1))
	        	disks[temp_flipped.top().size-1].SetVectTo(Vector2f(x, y) - disks[temp_flipped.top().size-1].size * 0.5f);
	        y -= sz;
	    	//cout << disks[temp_flipped.top().size-1].VectTo.x<<"-"<< disks[temp_flipped.top().size-1].VectTo.y << " ";
	        temp_flipped.pop();
	    }
	    x += WIDTH/4;
	}
	//
}


double Radiusxy(Vector2f v1, Vector2f v2)
{
	return sqrt(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2));
}


void MoveDisks(SdfDisk* disks)
{
	double ang;
	Vector2f tm;
	for(int i = 0; i < ELEM; i++)
	{
		if(disks[i].flvt)
		{
			tm = disks[i].GetPosition() - disks[i].VectTo;
			ang = atan2(tm.x, tm.y);
			tm = Vector2f(1500 * sin(ang), 1500 * cos(ang));
			disks[i].pos -= tm * FRAME.getElapsedTime().asSeconds();
			if(Radiusxy(disks[i].GetPosition(), disks[i].VectTo) < 0.5)
			{
				disks[i].pos = disks[i].VectTo;
				disks[i].flvt = false;
			}
		}
	}
}

void CalcPhis(SdfDisk* disks)
{
	float sec = disks[CH_DSK].passt.getElapsedTime().asSeconds();
	if(disks[CH_DSK].started){
		Vector2f tm = disks[CH_DSK].speed;
		if(sec < 0.3){
			tm = -(disks[CH_DSK].GetPosition() - MOVE_CURSOR.getPosition() + disks[CH_DSK].size * 0.5f) * 20.f * (sec/0.3f); // [0;1]
		
			disks[CH_DSK].spd = Radiusxy(tm, Vector2f(0, 0));
			disks[CH_DSK].SetSpeed(tm);
		}
		else{
			tm = disks[CH_DSK].GetPosition() - MOVE_CURSOR.getPosition() + disks[CH_DSK].size * 0.5f;
			disks[CH_DSK].CalcSpeed(tm);
		}
		disks[CH_DSK].pos += disks[CH_DSK].speed * FRAME.getElapsedTime().asSeconds();
	}
	else
		disks[CH_DSK].bondMode();
}

void CheckArea(SdfDisk* disks)
{
	if(CLICK)
	{
		if(CLICK_CURSOR.getPosition().x < 3 * WIDTH/8 and !gameplay.towers[0].is_empty()){ // MOVE_CURSOR
		
			CHOOSE = 0;
			CH_DSK = gameplay.towers[0].top().size - 1;
		}
		else
		{
			if(CLICK_CURSOR.getPosition().x < 5 * WIDTH/8 and !gameplay.towers[1].is_empty()){
				CHOOSE = 1;
				CH_DSK = gameplay.towers[1].top().size - 1;
			}
			else
				if(!gameplay.towers[2].is_empty()){
					CHOOSE = 2;
					CH_DSK = gameplay.towers[2].top().size - 1;
				}
		}
		
		if(!disks[CH_DSK].started and !disks[CH_DSK].bond)
		{
			disks[CH_DSK].passt.restart();
			disks[CH_DSK].started = true;
				
			Vector2f tm = disks[CH_DSK].GetPosition() - MOVE_CURSOR.getPosition();
			disks[CH_DSK].SetSpeed(tm);
		}
		
		CalcPhis(disks);
	}
	else{
		if(disks[CH_DSK].started or disks[CH_DSK].bond)
		{
			disks[CH_DSK].started = false;
			disks[CH_DSK].bond = false;
			disks[CH_DSK].SetSpeed(Vector2f(0, 0));
			
			if(MOVE_CURSOR.getPosition().x < 3 * WIDTH/8)
		        gameplay.move(CHOOSE, 0);
			else
			{
				if(MOVE_CURSOR.getPosition().x < 5 * WIDTH/8)
					gameplay.move(CHOOSE, 1);
				else
					gameplay.move(CHOOSE, 2);
			}
			
			RecalcDisks(disks, CH_DSK); // CH_DSK = gameplay.towers[0].top().size - 1;
		}
	}
}

void Scene1(double shfx){
	RoundShape post_rect(Vector2f(WIDTH - 50, 50));  		
	
 	post_rect.setOutlineThickness(3);
 	post_rect.setOutlineColor(Color(130, 145, 229, 100));
 
	post_rect.setFillColor(Color(130, 145, 229));
 	
 	post_rect.setPosition(Vector2f(25 + shfx, HEIGHT - 75));  
	window.draw(post_rect);
	
	
	post_rect.setSize(Vector2f(WIDTH/50, 3*HEIGHT/4));		
	post_rect.setRadius(15);
 	post_rect.setPosition(Vector2f(WIDTH/4 + shfx, HEIGHT/4 - 60));   // HEIGHT - 75 - 3*HEIGHT/4 + 15
	window.draw(post_rect);	 //Отрисовка прямоугольника
	
	for(short i=0;i<2;i++){
		post_rect.move(WIDTH/4, 0);  
		window.draw(post_rect);	
	}
	
	if(ANIM_MODE == 1)
		return;
	
	TEXT_WHITE.setString(to_string(
								int(floor(GAMETIME.getElapsedTime().asSeconds()))
								));
	TEXT_WHITE.setPosition(WIDTH - TEXT_WHITE.getGlobalBounds().width - 10, 0);
		
	float fl = 1;	
		
	if(GAMETIME.getElapsedTime().asSeconds() < 2.55)
		fl = GAMETIME.getElapsedTime().asSeconds() / 2.55;
	
	int a = GAMETIME.getElapsedTime().asSeconds() * 1000;
	if(a%1000 < 700) // (0; 0.255)
	{
		TEXT_WHITE.setFillColor(Color(255, 255, 255, fl * 255));
		window.draw(TEXT_WHITE);	
	}
	else
	{
		if(a%1000 < 850)
		{
			TEXT_WHITE.setFillColor(Color(255, 255, 255, fl * (255 - (a%1000 - 700) * 1.7))); // a /150 * 255
			window.draw(TEXT_WHITE); // предыдущее время	
		}
		else
		{
			TEXT_WHITE.setString(to_string(
					int(floor(GAMETIME.getElapsedTime().asSeconds()) + 1) 
					));
			TEXT_WHITE.setPosition(WIDTH - TEXT_WHITE.getGlobalBounds().width - 10, 0);
			TEXT_WHITE.setFillColor(Color(255, 255, 255, fl * ((a%1000 - 850) * 1.7))); // a / 150 * 255
			window.draw(TEXT_WHITE);
		}
	}
		
	TEXT_WHITE.setFillColor(Color::White);
}


void SceneDsk(SdfDisk* disks)
{
	RoundShape dsk(Vector2f(WIDTH/4, 50));
	dsk.setOutlineThickness(2);
	
	dsk.setRadius(10);
	
	TEXT_WHITE.setString(to_string(disks[0].numb));
	Vector2f tm = Vector2f(TEXT_WHITE.getGlobalBounds().height, TEXT_WHITE.getGlobalBounds().width);
	tm *= 0.5f;
	TEXT_WHITE.setPosition(disks[0].pos - tm);  
 	// post_rect.setOutlineColor(Color(255, 0, 0, 100));
	// post_rect.setFillColor(Color::Red));
 	//	post_rect.setPosition(Vector2f(25 + shfx, HEIGHT - 75));  
	
	for(int i = ELEM-1; i >= 0; i--)
	{
		dsk.setFillColor(disks[i].col);
		dsk.setOutlineColor(Color(disks[i].col.r, disks[i].col.g, disks[i].col.b, 100));
	   	dsk.setSize(disks[i].size);
	    dsk.setPosition(disks[i].pos);
	    window.draw(dsk);
	    
	    TEXT_WHITE.setString(to_string(disks[i].numb));
	    tm = Vector2f(TEXT_WHITE.getGlobalBounds().height/2, TEXT_WHITE.getGlobalBounds().width/2);
		TEXT_WHITE.setPosition(disks[i].pos + disks[i].size * 0.5f - tm);  
		//tm = disks[i].pos - tm;
		//cout<<tm.x<<" "<<tm.y<<"\n";
		window.draw(TEXT_WHITE);
	}
	// cout<<disks[10].numb;
}


void SetStartDisks(SdfDisk* disks)
{
	int y = HEIGHT - 75 - 2;
	float sz = 0;
												
	if(ELEM < 8)
	sz = HEIGHT * 0.1f;//y -= HEIGHT * 0.05f;
	else
	{
	sz = HEIGHT * 0.75f;
	sz /= (ELEM+1);
	}	
	y -= sz/2;
	        				
	for(int i = ELEM-1; i >= 0; i--)
	{
	    if(ELEM < 8){
		    disks[i].col = GetRainbow(i * 765 / ELEM); 
		    disks[i].SetSize(Vector2f((i + 1.f) * WIDTH/(4 * ELEM) + 20, sz));
			disks[i].SetPosition(Vector2f(WIDTH/4 + WIDTH/100, y-HEIGHT-20*(ELEM - i)));
			y -= sz;
		}
		else{
			disks[i].col = GetRainbow(i * 1530 / ELEM);
			disks[i].SetSize(Vector2f((i + 1.f) * WIDTH/(4 * ELEM) + 20, sz));
			disks[i].SetPosition(Vector2f(WIDTH/4 + WIDTH/100, y-HEIGHT-20*(ELEM - i)));
			y -= sz + 1;
		}
			disks[i].numb = i+1;
	}
}


void Congration(SdfDisk* disks)
{
	for(int i = 0; i < ELEM; i++)
	{
		if(!disks[i].flvt)
		{
			disks[i].SetVectTo(Vector2f(rand()%(WIDTH + 150) - 150, rand()%(HEIGHT + 150) - 150));
		} // GAMETIME 1530
		if(ELEM < 8)
		    disks[i].col = GetRainbow(i * 765 / ELEM + GAMETIME.getElapsedTime().asSeconds() * 100); 
		else
			disks[i].col = GetRainbow(i * 1530 / ELEM + GAMETIME.getElapsedTime().asSeconds() * 100); 
	}
	
	TEXT_CONG.setString(L"Хорош, но давай теперь\nпопробуй решить с " + to_string(ELEM + 1) + L" дисками(ом)\nТвоё количество ходов: " + to_string(gameplay.GetNumMoves()) );

	TEXT_CONG.setPosition(Vector2f(WIDTH/2 - TEXT_CONG.getGlobalBounds().width/2, HEIGHT/2 - TEXT_CONG.getGlobalBounds().height/2));
	
	window.draw(TEXT_CONG);
}

int main()
{
	Start();
	
    //game.show();
	/*RoundShape rs(Vector2f(300, 100));
	
	rs.setFillColor(Color::Red);
	rs.setOutlineThickness(5);
	rs.setOutlineColor(Color::Blue);
	window.draw(rs);
	*/
	//rs.setPosition(Vector2f(100, 100));
	
	SdfDisk disks[100];
	
	CircleShape shape(10);
	shape.setFillColor(Color(230, 0, 230)); 
	

    while(window.isOpen()) // .isOpen() - Пока открыто окно;
    {
        Event event; // Event - Класс событий окна; event - Объект событий окна;
        while (window.pollEvent(event)) // .pollEvent(Событие) - Перебор событий; 
        {
            if (event.type == Event::Closed) window.close(); 
        	if (event.type == Event::Resized) 
			{
	            WIDTH = event.size.width;
	            HEIGHT = event.size.height;          
	            window.setView(
	                View(
	                   Vector2f(WIDTH / 2.0, HEIGHT / 2.0), 
	                   Vector2f(WIDTH, HEIGHT)
	                )
	            );
	            ANIM_MODE = 0;
	            
	            SPEED_X = 1.25 * WIDTH/ANIM_TIME; // 1.25 = 5/4
				T1 = WIDTH/(4 * SPEED_X);
				AX = SPEED_X / T1;
				
				RecalcDisks(disks, -1);
	        }
	        if (event.type == Event::MouseButtonPressed and event.mouseButton.button == Mouse::Left){
		        int x = event.mouseButton.x, y = event.mouseButton.y;
		        if(x <= WIDTH and x >= 0){
		        	CLICK_CURSOR.setPosition(x, y);
		        	CLICK = true;
		        }
			}
			if(event.type == sf::Event::MouseMoved and CLICK)
			{
				MOVE_CURSOR.setPosition(Vector2f(event.mouseMove.x, event.mouseMove.y));
				//rs.setPosition(Vector2f(event.mouseMove.x - 150, event.mouseMove.y - 50));
			}
			if (event.type == Event::MouseButtonReleased)
				CLICK = false;
			
			if (event.type == Event::KeyPressed and CLICKED_INPUT){ // ввод только цифрами!!!!!!!!!
	        	int sz = ELEM_TEXT.size();
	         	if(25 < event.key.code and event.key.code < 36){
	         		if(sz == 0){
	         			if(event.key.code != 26){
	         				ELEM_TEXT += char(48 + event.key.code - 26); 
	         				ELEM = event.key.code - 26;
						}
					}
	         		else
		         		if(sz < 2){  
		        			ELEM_TEXT += char(48 + event.key.code - 26); 	
		        			ELEM = ELEM * 10 + event.key.code - 26;
		        		}
				}
	        	
	        	if(event.key.code == 58)	// enter
	        		if(SCENE == 0 and ELEM_TEXT != ""){
	        			ANIM_MODE = 1;
	        			ANIM_CLOCK.restart();
	        			SHIFT_X = 0;
	        			gameplay.start(ELEM);
	        			gameplay.show();
	        			//gameplay.move(0, 1);
	        			
	        			SetStartDisks(disks);
	        			RecalcDisks(disks, -1);
	        			//rs.setRadius(ELEM);
					}
					
	        	if(event.key.code == 59 and ELEM_TEXT != ""){  // backspace
	        		ELEM_TEXT.erase(sz - 1, 1);
	        		ELEM /= 10;
	        	}
			}
			if (event.type == Event::KeyPressed) //Esc
			{
				if(event.key.code == 36)
				{
					if(SCENE == 1)
					{
						SCENE = 0;
						gameplay.clear();
					}
					else
						window.close(); 
				}
				if(SCENE == 1)
				{
					int a = -1;
					switch(event.key.code)
					{
						case 27:{
							a = 0;
							break;
						}
						case 28:{
							a = 1;
							break;
						}
						case 29:{
							a = 2;
							break;
						}
					}
					if(a != -1 and PREV_BUT == -1)
						PREV_BUT = a;
					else
					if(a != -1 and PREV_BUT != -1)
					{
						cout<<gameplay.towers[0].top().size - 1<<"\n";
					    if(!gameplay.towers[PREV_BUT].is_empty())
							RecalcDisks(disks, gameplay.towers[PREV_BUT].top().size - 1); // CH_DSK = gameplay.towers[0].top().size - 1;
					    gameplay.move(PREV_BUT, a);
					    PREV_BUT = -1;
					}
				}
			}
			if(event.type == Event::KeyPressed and event.key.code == 58 and SCENE == 1)
				RecalcDisks(disks, -1);
			
		}
        
		window.clear(Color(211,160, 185)); 
		
		//Disk_draw(Vector2f(100, 100), Vector2f(100, 50), 10, Color::Red);

        switch(SCENE)
        {
        	case 0:{
        		Scene0();
				break;
			}
        	case 1:{
        		CheckArea(disks);
        		MoveDisks(disks);
        		
        		Scene1(0);
        		SceneDsk(disks);
        		
        		if(gameplay.is_finished())
        			Congration(disks);
        			
				break;
			}	
		}
		//window.draw(shape);
		
        FRAME.restart();

        window.display(); 
    }
    return 0; 
}